#include "mailbox.h"

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

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

std::vector<Message> Mailbox::retrieve_emails(int count) noexcept
{
  std::vector<Message> emails;

  try {
    vmime::utility::url url("imaps://imap.gmail.com:993");
    vmime::shared_ptr<vmime::net::session> session = vmime::net::session::create();

    vmime::shared_ptr<certificateVerifier> verifier = vmime::make_shared<certificateVerifier>();
    verifier->loadRootCertificates("/etc/ssl/cert.pem");

    vmime::shared_ptr<vmime::net::store> store = session->getStore(url);
    store->setCertificateVerifier(verifier);

    store->setProperty("options.need-authentication", true);
    store->setProperty("auth.username", email);
    store->setProperty("auth.password", password);

    // Connect to the IMAP server
    store->connect();

    // Open the INBOX folder
    vmime::shared_ptr<vmime::net::folder> inbox = store->getFolder(vmime::net::folder::path("INBOX"));
    inbox->open(vmime::net::folder::MODE_READ_ONLY);

    // Get the total message count
    int messageCount = inbox->getMessageCount();
    int start = std::max(1, messageCount - count + 1);  // Calculate the starting point for fetching emails
    std::cout << "GOT " << messageCount << " EMAILS" << std::endl;

    // Fetch the recent emails
    auto messages = inbox->getMessages(vmime::net::messageSet::byNumber(start, messageCount));
    inbox->fetchMessages(messages, vmime::net::fetchAttributes::FLAGS | vmime::net::fetchAttributes::ENVELOPE);
    vmime::utility::outputStreamAdapter os(std::cout);

    // Print the recent emails
    for (const auto& message : messages) {
        std::cout << "Message: " << message->getNumber() << std::endl;

        // Extract header and body content
        auto header = message->getHeader();
        auto content = message->getParsedMessage()->getBody()->getContents();

        // Extract Subject
        auto subject = header->Subject()->getValue()->generate();
        vmime::text subjectText;
        vmime::text::decodeAndUnfold(subject, &subjectText);

        // Extract Sender
        auto sender = header->From()->getValue()->generate();
        vmime::text senderText;
        vmime::text::decodeAndUnfold(sender, &senderText);

        // Extract Recipients
        auto to = header->To()->getValue()->generate();
        vmime::text toText;
        vmime::text::decodeAndUnfold(to, &toText);

        // Extract Content
        vmime::string contentString;
        vmime::utility::outputStreamStringAdapter contentStream(contentString);
        content->extract(contentStream);

        vmime::utility::inputStreamStringAdapter inStr(contentString);
        auto decoder = vmime::utility::encoder::encoderFactory::getInstance()->create("base64");
        vmime::string outString;
        vmime::utility::outputStreamStringAdapter outStr(outString);
        decoder->decode(inStr, outStr);
        //std::cout << "Raw content: " << contentString << std::endl;
        //std::cout << "Decoded content: " << outString << std::endl;

        vmime::text contentText;
        vmime::text::decodeAndUnfold(outString, &contentText);

        // Print the extracted information
        /*std::cout << "Subject: " << subjectText.getWholeBuffer() << std::endl;
        std::cout << "Sender: " << senderText.getWholeBuffer() << std::endl;
        std::cout << "Recipients: " << toText.getWholeBuffer() << std::endl;
        std::cout << "Content: " << contentText.getWholeBuffer() << std::endl;
        std::cout << std::endl;*/

        emails.push_back(Message {
            {toText.getWholeBuffer()},
            senderText.getWholeBuffer(),
            subjectText.getWholeBuffer(),
            contentString
        });
    }

    // Disconnect and close the folder and store
    inbox->close(false);  // `false` means do not expunge deleted messages
    store->disconnect();
  }
  catch (vmime::exception& e) {
    std::cerr << "Error retrieving emails: " << e.what() << std::endl;
  }
  catch (std::exception& e) {
    std::cerr << "General error: " << e.what() << std::endl;
  }

  return emails;
}