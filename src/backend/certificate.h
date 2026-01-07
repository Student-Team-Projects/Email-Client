#pragma once

#include <vmime/vmime.hpp>
#include <fstream>
#include "logging/logging.hpp"

class certificateVerifier : public vmime::security::cert::defaultCertificateVerifier {
public:
    void verify(const vmime::shared_ptr<vmime::security::cert::certificateChain>& chain,
                const vmime::string& hostname) override {
        logging::log("certficateVerifier_verify");
        try {
            setX509TrustedCerts(m_trustedCerts);
            defaultCertificateVerifier::verify(chain, hostname);
        } catch (vmime::security::cert::certificateException&) {
            vmime::shared_ptr<vmime::security::cert::certificate> cert = chain->getAt(0);
            if (cert->getType() == "X.509") {
                m_trustedCerts.push_back(vmime::dynamicCast<vmime::security::cert::X509Certificate>(cert));
                setX509TrustedCerts(m_trustedCerts);
                defaultCertificateVerifier::verify(chain, hostname);
            }
        }
    }

    void loadRootCertificates() {
        logging::log("load_root_certificates");
        std::ifstream certFile;
        for(auto& path:defaultPaths) {
            certFile = std::ifstream(path, std::ios::binary);
            if(certFile) break;
        }
        if (!certFile) throw std::runtime_error("Failed to open certificate file");
        vmime::utility::inputStreamAdapter is(certFile);
        vmime::security::cert::X509Certificate::import(is, m_trustedCerts);
    }

private:
    std::vector<vmime::shared_ptr<vmime::security::cert::X509Certificate>> m_trustedCerts;

    std::vector<std::string> defaultPaths = {
        "/etc/ssl/cert.pem", // alpine
        "/etc/ssl/certs/ca-certificates.crt", // ubuntu, debian
        "/etc/pki/tls/certs/ca-bundle.crt", // redhat, centos, fedora, opensuse
        "/etc/ssl/ca-bundle.pem" // opensuse
    };
};
