#pragma once

#include <string>
#include <vector>

#include "mail_types.h"

class Mailbox {
public:
  Mailbox(const std::string& email, const std::string& password);
  void send(const Message& message) noexcept;
  std::vector<Message> retrieve_emails(int count) noexcept; // retrieve most recent emails

private:
  std::string email;
  std::string password;
};