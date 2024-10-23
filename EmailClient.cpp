#include <vmime/vmime.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>

class certificateVerifier : public vmime::security::cert::defaultCertificateVerifier {
public:
    void verify(const vmime::shared_ptr<vmime::security::cert::certificateChain>& chain,
                const vmime::string& hostname) override {
        try {
            // Set trusted root certificates (loaded externally)
            setX509TrustedCerts(m_trustedCerts);

            // Verify the certificate chain against trusted certificates
            defaultCertificateVerifier::verify(chain, hostname);
        } catch (vmime::security::cert::certificateException&) {
            // Obtain the server's certificate and ask the user to accept it
            vmime::shared_ptr<vmime::security::cert::certificate> cert = chain->getAt(0);

            if (cert->getType() == "X.509") {
                m_trustedCerts.push_back(vmime::dynamicCast<vmime::security::cert::X509Certificate>(cert));
                setX509TrustedCerts(m_trustedCerts);

                // Verify again with the new certificate
                defaultCertificateVerifier::verify(chain, hostname);
            }
            return;
        }
    }

    // Load X.509 root certificates from a file using a stream
    void loadRootCertificates(const std::string& path) {
        std::ifstream certFile(path, std::ios::binary);
        if (!certFile) {
            throw std::runtime_error("Failed to open certificate file");
        }

        vmime::utility::inputStreamAdapter is(certFile);
        vmime::security::cert::X509Certificate::import(is, m_trustedCerts);
    }

private:
    std::vector<vmime::shared_ptr<vmime::security::cert::X509Certificate>> m_trustedCerts;
};

int main() {
    try {
        // Gmail account credentials (app password)
        std::string recipientEmail = "emailclientt@gmail.com"; // Replace with your Gmail address
        std::string appPassword = "";      // Replace with your generated app password
        std::string senderEmail = "grzegorz.ryn@gmail.com"; // Replace with recipient's email

        // Create a message
        vmime::messageBuilder mb;
        mb.setExpeditor(vmime::mailbox(senderEmail));

        vmime::addressList to;
        to.appendAddress(vmime::make_shared<vmime::mailbox>(recipientEmail));
        to.appendAddress(vmime::make_shared<vmime::mailbox>("jakub.gonera@student.uj.edu.pl")); // Add more recipients if needed
        to.appendAddress(vmime::make_shared<vmime::mailbox>("m.brzozowski@student.uj.edu.pl"));
        to.appendAddress(vmime::make_shared<vmime::mailbox>("grzegorz.ryn@student.uj.edu.pl"));
        mb.setRecipients(to);

        mb.setSubject(vmime::text("Test Email from vmime!"));
        mb.getTextPart()->setText(vmime::make_shared<vmime::stringContentHandler>(
            "This is a test email sent using vmime with Gmail's app password."));

        // Create the transport service to send the email (using Gmail's SMTP)
        vmime::utility::url url("smtps://smtp.gmail.com:465");
        vmime::shared_ptr<vmime::net::session> session = vmime::net::session::create();

        // Set up the certificate verifier
        vmime::shared_ptr<certificateVerifier> verifier = vmime::make_shared<certificateVerifier>();
        verifier->loadRootCertificates("/etc/ssl/cert.pem");  // Load root certificates

        // Create transport and set the certificate verifier
        vmime::shared_ptr<vmime::net::transport> transport = session->getTransport(url);
        transport->setCertificateVerifier(verifier);

        // Set authentication for the app password
        transport->setProperty("options.need-authentication", true);
        transport->setProperty("auth.username", senderEmail);
        transport->setProperty("auth.password", appPassword);

        // Connect to the SMTP server
        transport->connect();

        // Send the message
        transport->send(mb.construct());

        // Disconnect from the server
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
