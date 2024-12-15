#include <vmime/vmime.hpp>
#include <fstream>

class certificateVerifier : public vmime::security::cert::defaultCertificateVerifier {
public:
    void verify(const vmime::shared_ptr<vmime::security::cert::certificateChain>& chain,
                const vmime::string& hostname) override {
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

    void loadRootCertificates(const std::string& path) {
        std::ifstream certFile(path, std::ios::binary);
        if (!certFile) throw std::runtime_error("Failed to open certificate file");
        vmime::utility::inputStreamAdapter is(certFile);
        vmime::security::cert::X509Certificate::import(is, m_trustedCerts);
    }

private:
    std::vector<vmime::shared_ptr<vmime::security::cert::X509Certificate>> m_trustedCerts;
};