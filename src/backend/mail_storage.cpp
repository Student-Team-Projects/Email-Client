#include "mail_storage.h"
#include <iostream>
#include <filesystem>
#include <vector>

#include <vmime/vmime.hpp>

#include "html_parser.h"
#include "certificate.h"

std::string get_db_path(const std::string &email) noexcept
{
  return "data/" + email + ".db";
}

void init_db(sqlite3* db) noexcept
{
  char* err_msg = nullptr;
  const char* sql = "CREATE TABLE MailsReceived("
                    "ID INT PRIMARY KEY,"
                    "Sender TEXT NOT NULL,"
                    "Subject TEXT NOT NULL,"
                    "Body TEXT NOT NULL);";

  int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
  if (rc != SQLITE_OK) {
      std::cerr << "SQL error: " << err_msg << std::endl;
      sqlite3_free(err_msg);
  }

  sql = "CREATE TABLE Recipients("
        "MailID INT NOT NULL,"
        "Recipient TEXT NOT NULL,"
        "FOREIGN KEY(MailID) REFERENCES MailsReceived(ID));";
  rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
  if (rc != SQLITE_OK) {
      std::cerr << "SQL error: " << err_msg << std::endl;
      sqlite3_free(err_msg);
  }
}

sqlite3 *open_db(const std::string &email) noexcept
{
  sqlite3* db;

  std::string db_path = get_db_path(email);

  bool needs_init = !std::filesystem::exists(db_path);

  int rc = sqlite3_open(db_path.c_str(), &db);
  if (rc) {
      std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
      return nullptr;
  }

  if (needs_init) {
      init_db(db);
  }

  return db;
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

bool save_emails(std::vector<Message>& emails, sqlite3* db) {
  const char* mail_sql = "INSERT INTO MailsReceived (Sender, Subject, Body) VALUES (?, ?, ?);";
  const char* recipient_sql = "INSERT INTO Recipients (MailID, Recipient) VALUES (?, ?);";
  sqlite3_stmt* stmt;
  for (Message& message : emails) {
    if (sqlite3_prepare_v2(db, mail_sql, -1, &stmt, nullptr) != SQLITE_OK) {
      std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
      return false;
    }

    sqlite3_bind_text(stmt, 1, message.sender.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, message.subject.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, message.body.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
      std::cerr << "Failed to step: " << sqlite3_errmsg(db) << std::endl;
      sqlite3_finalize(stmt);
      return false;
    }
    sqlite3_finalize(stmt);

    sqlite3_int64 id = sqlite3_last_insert_rowid(db);

    for(const auto& recipient : message.recipients) {
      if (sqlite3_prepare_v2(db, recipient_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
      }

      sqlite3_bind_int64(stmt, 1, id);
      sqlite3_bind_text(stmt, 2, recipient.c_str(), -1, SQLITE_STATIC);

      if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to step: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
      }
      sqlite3_finalize(stmt);
    }
  }
  return true;
}

std::pair<std::vector<Message>, bool> fetch_emails(const std::string &email, const std::string &password) noexcept
{
  std::vector<Message> emails;

  try {
    vmime::utility::url url("imaps://imap.gmail.com:993");
    vmime::shared_ptr<vmime::net::session> session = vmime::net::session::create();

    vmime::shared_ptr<certificateVerifier> verifier = vmime::make_shared<certificateVerifier>();
    verifier->loadRootCertificates("/etc/ssl/cert.pem");

    vmime::shared_ptr<vmime::net::store> store = session->getStore(url);
    store->setCertificateVerifier(verifier);

    store->setProperty("options.need-authentication", true);
    store->setProperty("auth.username", email);
    store->setProperty("auth.password", password);

    // Connect to the IMAP server
    store->connect();

    // Open the INBOX folder
    vmime::shared_ptr<vmime::net::folder> inbox = store->getFolder(vmime::net::folder::path("INBOX"));
    inbox->open(vmime::net::folder::MODE_READ_ONLY);

    // Fetch the recent emails (-1 means the last)
    auto messages = inbox->getMessages(vmime::net::messageSet::byNumber(1, -1));
    inbox->fetchMessages(messages, vmime::net::fetchAttributes::FLAGS | vmime::net::fetchAttributes::ENVELOPE);
    vmime::utility::outputStreamAdapter os(std::cout);

    for (const auto& message : messages) {
        // Extract header and body content
        auto header = message->getHeader();
        auto content = message->getParsedMessage()->getBody()->getContents();

        // Extract Subject
        auto subject = header->Subject()->getValue()->generate();
        vmime::text subjectText;
        vmime::text::decodeAndUnfold(subject, &subjectText);

        // Extract Sender
        auto sender = header->From()->getValue()->generate();
        vmime::text senderText;
        vmime::text::decodeAndUnfold(sender, &senderText);

        // Extract Recipients
        auto to = header->To()->getValue()->generate();
        vmime::text toText;
        vmime::text::decodeAndUnfold(to, &toText);

        // Extract Content
        vmime::string contentString;
        vmime::utility::outputStreamStringAdapter contentStream(contentString);
        content->extract(contentStream);

        vmime::utility::inputStreamStringAdapter inStr(contentString);
        auto decoder = vmime::utility::encoder::encoderFactory::getInstance()->create("base64");
        vmime::string outString;
        vmime::utility::outputStreamStringAdapter outStr(outString);
        decoder->decode(inStr, outStr);

        vmime::text contentText;
        vmime::text::decodeAndUnfold(outString, &contentText);

        emails.push_back(Message {
            {toText.getWholeBuffer()},
            senderText.getWholeBuffer(),
            subjectText.getWholeBuffer(),
            HtmlParser::extractText(contentString)
        });
    }

    // Disconnect and close the folder and store
    inbox->close(false);  // `false` means do not expunge deleted messages
    store->disconnect();
  }
  catch (vmime::exception& e) {
    std::cerr << "Error retrieving emails: " << e.what() << std::endl;
    return {emails, false};
  }
  catch (std::exception& e) {
    std::cerr << "General error: " << e.what() << std::endl;
    return {emails, false};
  }

  return {emails, true};
}

bool MailStorage::synchronize(const std::string &email, const std::string &password) noexcept
{
  sqlite3* db = open_db(email);

  const char* sql = "DELETE FROM MailsReceived;";
  char* err_msg = nullptr;
  int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
  if (rc != SQLITE_OK) {
      std::cerr << "SQL error: " << err_msg << std::endl;
      sqlite3_free(err_msg);
      return false;
  }

  sql = "DELETE FROM Recipients;";
  rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
  if (rc != SQLITE_OK) {
      std::cerr << "SQL error: " << err_msg << std::endl;
      sqlite3_free(err_msg);
      return false;
  }

  auto [emails, status] = fetch_emails(email, password);
  if(!status || !save_emails(emails, db))
    return false;

  sqlite3_close(db);
  return true;
}

std::vector<Message> MailStorage::get_emails(const std::string &email) noexcept
{
  std::vector<Message> emails;

  sqlite3* db = open_db(email);

  sqlite3_stmt* stmt;
  const char* sql = "SELECT ID, Sender, Subject, Body FROM MailsReceived;";
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
    return emails;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    long long id = sqlite3_column_int64(stmt, 0);
    const char* sender = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    const char* subject = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    const char* body = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

    std::vector<std::string> recipients;
    const char* recipient_sql = "SELECT Recipient FROM Recipients WHERE MailID = ?;";
    sqlite3_stmt* recipient_stmt;
    if (sqlite3_prepare_v2(db, recipient_sql, -1, &recipient_stmt, nullptr) != SQLITE_OK) {
      std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
      return emails;
    }

    sqlite3_bind_int64(recipient_stmt, 1, id);
    while (sqlite3_step(recipient_stmt) == SQLITE_ROW) {
      const char* recipient = reinterpret_cast<const char*>(sqlite3_column_text(recipient_stmt, 0));
      recipients.push_back(recipient);
    }
    sqlite3_finalize(recipient_stmt);

    emails.push_back(Message{recipients, sender, subject, body});
  }

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  return emails;
}
