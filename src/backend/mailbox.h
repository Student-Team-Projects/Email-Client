#pragma once

#include <string>
#include <vector>

#include "account.h"
#include "mail_types.h"

class Mailbox {
public:
  Mailbox(Account account);
  void send(const MessageToSend& message) noexcept;
  void synchronize() noexcept;
  std::vector<Folder> get_email_headers() noexcept;
  std::string get_email_body(const std::string& uid, const std::string& folder_path) noexcept;
private:
  Account account;
};
