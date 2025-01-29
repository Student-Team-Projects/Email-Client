#include "mailbox.h"

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

#include <vmime/vmime.hpp>

#include "html_parser.h"
#include "certificate.h"
#include "mail_storage.h"

Mailbox::Mailbox(const std::string &email, const std::string &password)
  : email(email), password(password)
{}

void Mailbox::send(const MessageToSend &message) noexcept
{
  try {
    vmime::addressList to;
    to.appendAddress(vmime::make_shared<vmime::mailbox>(message.recipient));

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

void Mailbox::synchronize() noexcept
{
  MailStorage::synchronize(email, password);
}

std::vector<Folder> Mailbox::get_email_headers() noexcept
{
  return MailStorage::get_email_headers(email);
}

std::string Mailbox::get_email_body(const std::string &uid, const std::string& folder_path) noexcept
{
  return MailStorage::get_email_body(uid, folder_path, email, password);
}
