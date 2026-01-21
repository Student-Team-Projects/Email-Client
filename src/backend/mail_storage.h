#pragma once

#include <string>
#include <cstddef>

#include <sqlite3.h>

#include "backend/account.h"
#include "mail_types.h"

class MailStorage {
public:
  static std::size_t get_mail_count(const Account& account) noexcept;
  static bool synchronize(const Account& account) noexcept;
  static std::vector<Folder> get_email_headers(const Account& account) noexcept;
  static std::string get_email_body(const Account& account, const std::string& uid, const std::string& folder_path) noexcept;
  static bool save_sent_email(const MessageToSend& message, const std::string& email) noexcept;
  static Folder get_sent_emails(const std::string& email) noexcept;
  static std::string get_sent_email_body(const std::string& email, const std::string& id) noexcept;
};
