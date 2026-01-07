#include "mailbox.h"

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

#include <vmime/vmime.hpp>

#include "account.h"
#include "html_parser.h"
#include "certificate.h"
#include "mail_storage.h"
#include "logging/logging.hpp"

Mailbox::Mailbox(Account account)
  : account(account) {}

void Mailbox::send(const MessageToSend &message) noexcept
{
  logging::log("mailbox_send");
  try {
    vmime::addressList to;
    to.appendAddress(vmime::make_shared<vmime::mailbox>(message.recipient));

    vmime::messageBuilder mb;
    mb.setExpeditor(vmime::mailbox(account.username));
    mb.setRecipients(to);
    mb.setSubject(vmime::text(message.subject, "utf-8"));
    mb.getTextPart()->setText(vmime::make_shared<vmime::stringContentHandler>(message.body));
    mb.getTextPart()->setCharset(vmime::charset("utf-8"));

    vmime::utility::url url(account.smtpHost);
    vmime::shared_ptr<vmime::net::session> session = vmime::net::session::create();

    vmime::shared_ptr<certificateVerifier> verifier = vmime::make_shared<certificateVerifier>();
    verifier->loadRootCertificates();

    vmime::shared_ptr<vmime::net::transport> transport = session->getTransport(url);
    transport->setCertificateVerifier(verifier);

    transport->setProperty("options.need-authentication", true);
    transport->setProperty("auth.username", account.username);
    transport->setProperty("auth.password", account.password);

    transport->connect();
    transport->send(mb.construct());
    transport->disconnect();
  }
  catch (vmime::exception& e) {
    logging::log("Error sending email: " + (std::string)e.what());
  }
  catch (std::exception& e) {
    logging::log("General error: " + (std::string)e.what());
  }
}

void Mailbox::synchronize() noexcept
{
  logging::log("mailbox_synchronize");
  MailStorage::synchronize(account);
}

std::vector<Folder> Mailbox::get_email_headers() noexcept
{
  logging::log("mailbbox_get_email_headers");
  return MailStorage::get_email_headers(account);
}

std::string Mailbox::get_email_body(const std::string &uid, const std::string& folder_path) noexcept
{
  logging::log("mailbox_get_email_body: uid "+uid+", path: "+folder_path);
  return MailStorage::get_email_body(account, uid, folder_path);
}
