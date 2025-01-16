#include "mail_storage.h"

#include "html_parser.h"
#include "certificate.h"

#include <iostream>
#include <filesystem>
#include <vector>
#include <cassert>

#include <vmime/vmime.hpp>

std::string get_db_path(const std::string &email) noexcept
{
  return "data/" + email + ".db";
}

void init_db(sqlite3* db) noexcept
{
  char* err_msg = nullptr;
  const char* sql = "CREATE TABLE Mails ("
                    "ID INTEGER PRIMARY KEY,"
                    "Sender TEXT NOT NULL,"
                    "Subject TEXT NOT NULL,"
                    "Body TEXT NOT NULL,"
                    "Recipient TEXT,"
                    "Folder TEXT NOT NULL);";

  int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
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
  const char* sql = "SELECT COUNT(*) FROM Mails;";
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

bool save_emails(std::vector<Folder>& emails, sqlite3* db) {
  for (Folder& folder : emails) {
    const char* mail_sql = "INSERT INTO Mails (Sender, Subject, Body, Folder, Recipient) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    for (Message& message : folder.messages) {
      if (sqlite3_prepare_v2(db, mail_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
      }

      sqlite3_bind_text(stmt, 1, message.sender.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 2, message.subject.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 3, message.body.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 4, folder.name.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 5, message.recipient.c_str(), -1, SQLITE_STATIC);

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

std::pair<std::vector<Folder>, bool> fetch_emails(const std::string &email, const std::string &password) noexcept
{
  std::vector<Folder> folders;

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

    // Loading messages is SO slow
    std::size_t count = 0;
    
    auto root_folder = store->getRootFolder();
    for(auto& folder : root_folder->getFolders(true)) {
      auto flag_no_open = folder->getAttributes().getFlags() & vmime::net::folderAttributes::Flags::FLAG_NO_OPEN;
      
      if (flag_no_open) {
        continue;
      }
      
      std::vector<Message> emails;

      // Open the folder
      folder->open(vmime::net::folder::MODE_READ_ONLY);

      // Limit the number of messages to 10, change to (1, -1) for all messages
      int messageCount = folder->getMessageCount();
      if (messageCount == 0) {
        folder->close(false);
        continue;
      }
      int start = std::max(1, messageCount - 30 + 1);
      auto messages = folder->getMessages(vmime::net::messageSet::byNumber(start, messageCount));

      folder->fetchMessages(messages, vmime::net::fetchAttributes::FLAGS | vmime::net::fetchAttributes::ENVELOPE);
      vmime::utility::outputStreamAdapter os(std::cout);

    int counter = 0;
    for (const auto& message : messages) {
      try{
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
              toText.getWholeBuffer(),
              senderText.getWholeBuffer(),
              subjectText.getWholeBuffer(),
              HtmlParser::extractText(contentString)
          });
        }
        catch (vmime::exception& e) {
          std::cerr << "Error processing email: " << e.what() << std::endl;
        }
        catch (std::exception& e) {
          std::cerr << "General error: " << e.what() << std::endl;
        }
      }

      folders.push_back(Folder{folder->getFullPath().toString("/", "utf-8"), emails});

      // Disconnect and close the folder and store
      folder->close(false);  // `false` means do not expunge deleted messages
    }
    store->disconnect();
  }
  catch (vmime::exception& e) {
    std::cerr << "Error retrieving emails: " << e.what() << std::endl;
    return {folders, false};
  }
  catch (std::exception& e) {
    std::cerr << "General error: " << e.what() << std::endl;
    return {folders, false};
  }

  return {folders, true};
}

bool MailStorage::synchronize(const std::string &email, const std::string &password) noexcept
{
  // Fetch emails before deleting (this takes time)
  auto [emails, status] = fetch_emails(email, password);

  sqlite3* db = open_db(email);

  const char* sql = "DELETE FROM Mails;";
  char* err_msg = nullptr;
  int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
  if (rc != SQLITE_OK) {
      std::cerr << "SQL error: " << err_msg << std::endl;
      sqlite3_free(err_msg);
      sqlite3_close(db);
      return false;
  }

  if(!status || !save_emails(emails, db)){
    sqlite3_close(db);
    return false;
  }
  
  sqlite3_close(db);
  return true;
}

std::vector<Folder> load_emails(const std::string &email) noexcept {
  std::vector<Folder> folders;

  sqlite3* db = open_db(email);

  // Query to get all unique folders
  sqlite3_stmt* folder_stmt;
  const char* folder_sql = "SELECT DISTINCT Folder FROM Mails;";
  if (sqlite3_prepare_v2(db, folder_sql, -1, &folder_stmt, nullptr) != SQLITE_OK) {
    std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
    return folders;
  }

  while (sqlite3_step(folder_stmt) == SQLITE_ROW) {
    const char* folder_name = reinterpret_cast<const char*>(sqlite3_column_text(folder_stmt, 0));
    std::vector<Message> emails;

    sqlite3_stmt* stmt;
    const char* sql = "SELECT ID, Sender, Subject, Body, Recipient FROM Mails WHERE Folder = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
      continue;
    }

    sqlite3_bind_text(stmt, 1, folder_name, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
      long long id = sqlite3_column_int64(stmt, 0);
      const char* sender = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
      const char* subject = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
      const char* body = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
      const char* recipient = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

      emails.push_back(Message{recipient, sender, subject, body});
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    folders.push_back(Folder{folder_name, emails});
  }

  sqlite3_finalize(folder_stmt);

  return folders;
}

std::vector<Folder> MailStorage::get_emails(const std::string &email) noexcept
{
  return load_emails(email);
}