#include "smtp_sender.h"
#include "backend_new/mailbox.h"
#include "certificate.h"
#include "vmime/addressList.hpp"
#include "vmime/messageBuilder.hpp"
#include "vmime/stringContentHandler.hpp"
#include <memory>

#include <vmime/net/smtp/SMTPTransport.hpp>

SmtpSender::SmtpSender(
  std::string address,
  unsigned short port,
  std::optional<CertificateVerifier> cert,
  std::string username,
  std::string password
) {
  auto url = vmime::utility::url(
    std::string("smtp") + (cert.has_value() ? "s" : "")
    + "://" + address + ":" + std::to_string(port)
  );
  auto session = vmime::net::session::create();
  transport = session->getTransport(url);

  if (cert.has_value()) {
    transport->setCertificateVerifier(cert.value());
  }
  transport->setProperty("options.need-authentication", true);
  transport->setProperty("auth.username", username);
  transport->setProperty("auth.password", password);

  mailbox = vmime::mailbox(username);
}
void SmtpSender::send_mail(const Message& msg) {
  const auto UTF8 = vmime::charset("utf-8");

  vmime::messageBuilder builder;
  builder.setExpeditor(mailbox);
  
  vmime::addressList recipient;
  recipient.appendAddress(
    vmime::make_shared<vmime::mailbox>(msg.meta.to)
  );
  builder.setRecipients(recipient);

  builder.setSubject(vmime::text(msg.meta.subject, UTF8));
  builder.getTextPart()->setText(
    vmime::make_shared<vmime::stringContentHandler>(
      msg.data
    )    
  );
  builder.getTextPart()->setCharset(UTF8);
  
  transport->connect();
  transport->send(builder.construct());
  transport->disconnect();
}
