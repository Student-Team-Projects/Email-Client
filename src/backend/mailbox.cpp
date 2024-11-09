#include "mailbox.h"

#include <iostream>
#include <string>
#include <fstream>

#include <vmime/vmime.hpp>

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

Mailbox::Mailbox(const std::string &email, const std::string &password)
  : email(email), password(password)
{}

void Mailbox::send(const Message &message) noexcept
{
  try {
    vmime::addressList to;
    for (const auto &recipient : message.recipients) {
      to.appendAddress(vmime::make_shared<vmime::mailbox>(recipient));
    }

    vmime::messageBuilder mb;
    mb.setExpeditor(vmime::mailbox(email));
    mb.setRecipients(to);
    mb.setSubject(vmime::text(message.subject));
    mb.getTextPart()->setText(vmime::make_shared<vmime::stringContentHandler>(message.body));

    vmime::utility::url url("smtps://smtp.gmail.com:465");
    vmime::shared_ptr<vmime::net::session> session = vmime::net::session::create();

    vmime::shared_ptr<certificateVerifier> verifier = vmime::make_shared<certificateVerifier>();
    verifier->loadRootCertificates("/etc/ssl/cert.pem");

    vmime::shared_ptr<vmime::net::transport> transport = session->getTransport(url);
    transport->setCertificateVerifier(verifier);

    transport->setProperty("options.need-authentication", true);
    transport->setProperty("auth.username", email);
    transport->setProperty("auth.password", password);

    transport->connect();
    transport->send(mb.construct());
    transport->disconnect();
  }
  catch (vmime::exception& e) {
    std::cerr << "Error sending email: " << e.what() << std::endl;
  }
  catch (std::exception& e) {
    std::cerr << "General error: " << e.what() << std::endl;
  }
}
