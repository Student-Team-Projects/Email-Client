#pragma once

#include <string>

#include "mail_types.h"

class Mailbox {
public:
  Mailbox(const std::string& email, const std::string& password);
  void send(const Message& message) noexcept;

private:
  std::string email;
  std::string password;
};