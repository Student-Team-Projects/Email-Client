#pragma once
#include "mailbox.h"
#include "certificate.h"
#include "vmime/net/transport.hpp"
#include "vmime/security/cert/certificateVerifier.hpp"
#include "vmime/types.hpp"
#include <optional>

struct SmtpSender : virtual MailSender {
  SmtpSender(
    std::string address,
    unsigned short port,
    std::optional<CertificateVerifier> cert,
    std::string username,
    std::string password
  );

  virtual void send_mail(const Message &msg) override;
private:
  std::shared_ptr<vmime::net::transport> transport;
  vmime::mailbox mailbox;
};
