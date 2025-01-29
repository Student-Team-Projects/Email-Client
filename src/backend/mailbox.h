#pragma once

#include <string>
#include <vector>

#include "mail_types.h"

class Mailbox {
public:
  Mailbox(const std::string& email, const std::string& password);
  void send(const MessageToSend& message) noexcept;
  void synchronize() noexcept;
  std::vector<Folder> get_email_headers() noexcept;
  std::string get_email_body(const std::string& uid) noexcept;
private:
  std::string email;
  std::string password;
};