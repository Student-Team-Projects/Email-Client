#pragma once

#include <string>
#include <cstddef>

#include <sqlite3.h>

class MailStorage {
public:
  static std::size_t get_mail_count(const std::string& email) noexcept;
};