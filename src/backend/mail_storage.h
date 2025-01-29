#pragma once

#include <string>
#include <cstddef>

#include <sqlite3.h>

#include "mail_types.h"

class MailStorage {
public:
  static std::size_t get_mail_count(const std::string& email) noexcept;
  static bool synchronize(const std::string& email, const std::string& password) noexcept;
  static std::vector<Folder> get_email_headers(const std::string& email) noexcept;
  static std::string get_email_body(const std::string& uid) noexcept;
};