#include "mail_storage.h"
#include <iostream>

std::string get_db_path(const std::string &email) noexcept
{
  return "data/" + email + ".db";
}

sqlite3 *open_db(const std::string &email) noexcept
{
  sqlite3* db;

  std::string db_path = get_db_path(email);

  bool needs_init = !std::filesystem::exists(db_path);

  int rc = sqlite3_open(db_path, &db);
  if (rc) {
      std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
      return rc;
  }

  if (needs_init) {
      init_db(db);
  }

  return db;
}

void init_db(sqlite* db) noexcept
{
  char* err_msg = nullptr;
  const char* sql = "CREATE TABLE MailsReceived("
                    "ID INT PRIMARY KEY NOT NULL,"
                    "Sender TEXT NOT NULL,"
                    "Recipient TEXT NOT NULL,"
                    "Subject TEXT NOT NULL,"
                    "Body TEXT NOT NULL);";

  int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
  if (rc != SQLITE_OK) {
      std::cerr << "SQL error: " << err_msg << std::endl;
      sqlite3_free(err_msg);
  }
}

std::size_t MailStorage::get_mail_count(const std::string &email) noexcept
{
  sqlite3* db = open_db(email);

  sqlite3_stmt* stmt;
  const char* sql = "SELECT COUNT(*) FROM MailsReceived;";
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
      std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
      return 0;
  }

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_ROW) {
      std::cerr << "Failed to step: " << sqlite3_errmsg(db) << std::endl;
      return 0;
  }

  std::size_t count = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  return count;
}

