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
    const int N = 1;  // Set the number of recent emails to retrieve

    try {
        // Load configuration from config.json
        std::ifstream configFile("config.json");
        if (!configFile) throw std::runtime_error("Failed to open config.json");
        nlohmann::json config;
        configFile >> config;

        std::string senderEmail = config["sender_email"];
        std::string appPassword = config["app_password"];

        // Create the IMAP session and service to retrieve emails
        vmime::utility::url url("imaps://imap.gmail.com:993");
        vmime::shared_ptr<vmime::net::session> session = vmime::net::session::create();

        // Set up the certificate verifier
        vmime::shared_ptr<certificateVerifier> verifier = vmime::make_shared<certificateVerifier>();
        verifier->loadRootCertificates("/etc/ssl/cert.pem");

        // Create the IMAP store and set the certificate verifier
        vmime::shared_ptr<vmime::net::store> store = session->getStore(url);
        store->setCertificateVerifier(verifier);

        // Set authentication for the app password
        store->setProperty("options.need-authentication", true);
        store->setProperty("auth.username", senderEmail);
        store->setProperty("auth.password", appPassword);

        // Connect to the IMAP server
        store->connect();

        // Open the INBOX folder
        vmime::shared_ptr<vmime::net::folder> inbox = store->getFolder(vmime::net::folder::path("INBOX"));
        inbox->open(vmime::net::folder::MODE_READ_ONLY);

        // Get the total message count
        int messageCount = inbox->getMessageCount();
        int start = std::max(1, messageCount - N + 1);  // Calculate the starting point for fetching emails
        std::cout << "GOT " << messageCount << " EMAILS" << std::endl;

        // Fetch the recent emails
        auto messages = inbox->getMessages(vmime::net::messageSet::byNumber(start, messageCount));
        inbox->fetchMessages(messages, vmime::net::fetchAttributes::FLAGS | vmime::net::fetchAttributes::ENVELOPE);
        vmime::utility::outputStreamAdapter os(std::cout);

        // Print the recent emails
        for (const auto& message : messages) {
            message->extract(os);
        }
        
        // Disconnect and close the folder and store
        inbox->close(false);  // `false` means do not expunge deleted messages
        store->disconnect();

    } catch (vmime::exception& e) {
        std::cerr << "Error retrieving emails: " << e.what() << std::endl;
        return 1;
    } catch (std::exception& e) {
        std::cerr << "General error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
