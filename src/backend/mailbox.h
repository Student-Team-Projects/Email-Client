#pragma once

#include <string>
#include <vector>

#include "mail_types.h"

class Mailbox {
public:
  Mailbox(const std::string& email, const std::string& password);
  void send(const Message& message) noexcept;
  void synchronize() noexcept;
  std::vector<Message> get_emails() noexcept;
private:
  std::string email;
  std::string password;
};