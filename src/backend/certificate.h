#pragma once

#include <vmime/vmime.hpp>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>
#include "logging/logging.hpp"

class certificateVerifier : public vmime::security::cert::defaultCertificateVerifier {
public:
    void loadRootCertificates() {
        logging::log("load_root_certificates");
        std::ifstream certFile;
        // Same override as OpenSSL/curl, used by dev/mail-server.sh to trust the dev CA
        if (const char* envPath = std::getenv("SSL_CERT_FILE")) {
            certFile = std::ifstream(envPath, std::ios::binary);
            if (!certFile) throw std::runtime_error(std::string("Failed to open SSL_CERT_FILE: ") + envPath);
        }
        else for(auto& path:defaultPaths) {
            certFile = std::ifstream(path, std::ios::binary);
            if(certFile) break;
        }
        if (!certFile) throw std::runtime_error("Failed to open certificate file");

        std::vector<vmime::shared_ptr<vmime::security::cert::X509Certificate>> rootCAs;
        vmime::utility::inputStreamAdapter is(certFile);
        vmime::security::cert::X509Certificate::import(is, rootCAs);
        // Root CAs are trusted to issue server certificates. (setX509TrustedCerts
        // would only accept a server whose own certificate is on the list.)
        setX509RootCAs(rootCAs);
    }

private:
    std::vector<std::string> defaultPaths = {
        "/etc/ssl/cert.pem", // alpine
        "/etc/ssl/certs/ca-certificates.crt", // ubuntu, debian
        "/etc/pki/tls/certs/ca-bundle.crt", // redhat, centos, fedora, opensuse
        "/etc/ssl/ca-bundle.pem" // opensuse
    };
};

// Makes the service verify server certificates and never send credentials in plain text:
// imaps:// and smtps:// use TLS from the start, imap:// and smtp:// must upgrade with STARTTLS.
inline void setupTls(const vmime::shared_ptr<vmime::net::service>& service) {
    logging::log("setup_tls");
    vmime::shared_ptr<certificateVerifier> verifier = vmime::make_shared<certificateVerifier>();
    verifier->loadRootCertificates();
    service->setCertificateVerifier(verifier);

    service->setProperty("connection.tls", true);
    service->setProperty("connection.tls.required", true);
}
