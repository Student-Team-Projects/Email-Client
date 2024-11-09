#include <vmime/vmime.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <nlohmann/json.hpp>  // JSON library header

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

int main() {
    try {
        // Load configuration from config.json
        std::ifstream configFile("config.json");
        if (!configFile) throw std::runtime_error("Failed to open config.json");
        nlohmann::json config;
        configFile >> config;

        std::string senderEmail = config["sender_email"];
        std::string appPassword = config["app_password"];

        // Read recipient emails from recipients.txt
        std::ifstream recipientsFile("recipients.txt");
        if (!recipientsFile) throw std::runtime_error("Failed to open recipients.txt");

        vmime::addressList to;
        std::string recipientEmail;
        while (std::getline(recipientsFile, recipientEmail)) {
            to.appendAddress(vmime::make_shared<vmime::mailbox>(recipientEmail));
        }

        // Create a message
        vmime::messageBuilder mb;
        mb.setExpeditor(vmime::mailbox(senderEmail));
        mb.setRecipients(to);
        mb.setSubject(vmime::text("Test Email from vmime!"));
        mb.getTextPart()->setText(vmime::make_shared<vmime::stringContentHandler>(
            "This is a test email sent using vmime with Gmail's app password."));

        // Create the transport service to send the email (using Gmail's SMTP)
        vmime::utility::url url("smtps://smtp.gmail.com:465");
        vmime::shared_ptr<vmime::net::session> session = vmime::net::session::create();

        // Set up the certificate verifier
        vmime::shared_ptr<certificateVerifier> verifier = vmime::make_shared<certificateVerifier>();
        verifier->loadRootCertificates("/etc/ssl/cert.pem");

        // Create transport and set the certificate verifier
        vmime::shared_ptr<vmime::net::transport> transport = session->getTransport(url);
        transport->setCertificateVerifier(verifier);

        // Set authentication for the app password
        transport->setProperty("options.need-authentication", true);
        transport->setProperty("auth.username", senderEmail);
        transport->setProperty("auth.password", appPassword);

        // Connect to the SMTP server and send the message
        transport->connect();
        transport->send(mb.construct());
        transport->disconnect();

        std::cout << "Email sent successfully using Gmail app password!" << std::endl;
    } catch (vmime::exception& e) {
        std::cerr << "Error sending email: " << e.what() << std::endl;
        return 1;
    } catch (std::exception& e) {
        std::cerr << "General error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
