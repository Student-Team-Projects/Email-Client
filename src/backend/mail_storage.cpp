#include "mail_storage.h"

#include "html_parser.h"
#include "certificate.h"
#include "app.hpp"

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <cassert>

#include <vmime/vmime.hpp>
#include "logging/logging.hpp"
#include "mail_types.h"

std::filesystem::path get_db_path(const std::string &email) noexcept
{
  logging::log("mail_storage_get_db_path");
  return Application::get_data_home_path() / (email + ".db");
}

void init_db(sqlite3* db) noexcept
{
  logging::log("mail_storage_init_db");
  char* err_msg = nullptr;
  const char* sql = "CREATE TABLE MailHeaders ("
                    "ID INTEGER PRIMARY KEY,"
                    "Sender TEXT NOT NULL,"
                    "Subject TEXT NOT NULL,"
                    "Recipient TEXT,"
                    "UID TEXT NOT NULL,"
                    "Folder TEXT NOT NULL);"
                    ""
                    "CREATE TABLE SentEmails ("
                    "ID INTEGER PRIMARY KEY,"
                    "Sender TEXT NOT NULL,"
                    "Subject TEXT NOT NULL,"
                    "Recipient TEXT,"
                    "Content TEXT NOT NULL"
                    ");";

  int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
  if (rc != SQLITE_OK) {
      logging::log("SQL error: " + (std::string)(err_msg));
      sqlite3_free(err_msg);
  }
}

sqlite3 *open_db(const std::string &email) noexcept
{
  logging::log("mail_storage_open_db");
  sqlite3* db;

  try{
    std::filesystem::create_directories(get_db_path(email).parent_path());
  }
  catch(const std::exception& e){
    logging::log("Failed to create directory: " + (std::string)(e.what()));
    return nullptr;
  }

  std::string db_path = get_db_path(email);

  bool needs_init = !std::filesystem::exists(db_path);

  int rc = sqlite3_open(db_path.c_str(), &db);
  if (rc) {
      logging::log("Can't open database: " + (std::string)sqlite3_errmsg(db));
      return nullptr;
  }

  if (needs_init) {
      init_db(db);
  }

  return db;
}

std::size_t MailStorage::get_mail_count(const Account& account) noexcept
{
  logging::log("mail_storage_get_mail_count");
  sqlite3* db = open_db(account.username);

  sqlite3_stmt* stmt;
  const char* sql = "SELECT COUNT(*) FROM MailHeaders;";
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
      logging::log("Failed to prepare statement: " + (std::string)sqlite3_errmsg(db));
      return 0;
  }

  rc = sqlite3_step(stmt);
  if (rc != SQLITE_ROW) {
      logging::log("Failed to step: " + (std::string)sqlite3_errmsg(db));
      return 0;
  }

  std::size_t count = sqlite3_column_int(stmt, 0);

  sqlite3_finalize(stmt);
  sqlite3_close(db);

  return count;
}

bool save_emails(std::vector<Folder>& emails, sqlite3* db) {
  logging::log("mail_storage_save_emails");
  for (Folder& folder : emails) {
    const char* mail_sql = "INSERT INTO MailHeaders (Sender, Subject, Folder, UID, Recipient) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    for (MessageHeader& message : folder.messages) {
      if (sqlite3_prepare_v2(db, mail_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        logging::log("Failed to prepare statement: " + (std::string)sqlite3_errmsg(db));
        return false;
      }

      sqlite3_bind_text(stmt, 1, message.sender.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 2, message.subject.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 3, folder.name.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 4, message.uid.c_str(), -1, SQLITE_STATIC);
      sqlite3_bind_text(stmt, 5, message.recipient.c_str(), -1, SQLITE_STATIC);

      if (sqlite3_step(stmt) != SQLITE_DONE) {
        logging::log("Failed to step: " + (std::string)sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
      }
      sqlite3_finalize(stmt);
    }
  }
  return true;
}

std::shared_ptr<vmime::net::store> get_store(const Account& account)
{
  logging::log("mail_storage_get_store");
  vmime::utility::url url(account.imapHost);
  vmime::shared_ptr<vmime::net::session> session = vmime::net::session::create();

  vmime::shared_ptr<certificateVerifier> verifier = vmime::make_shared<certificateVerifier>();
  verifier->loadRootCertificates();

  vmime::shared_ptr<vmime::net::store> store = session->getStore(url);
  store->setCertificateVerifier(verifier);

  store->setProperty("options.need-authentication", true);
  store->setProperty("auth.username", account.username);
  store->setProperty("auth.password", account.password);

  return store;
}

std::pair<std::vector<Folder>, bool> fetch_emails(const Account& account) noexcept
{
  logging::log("mail_storage_fetch_emails");
  std::vector<Folder> folders;

  try {
    vmime::shared_ptr<vmime::net::store> store = get_store(account);

    // Connect to the IMAP server
    store->connect();

    std::size_t count = 0;

    auto root_folder = store->getRootFolder();
    for(auto& folder : root_folder->getFolders(true)) {
      auto flag_no_open = folder->getAttributes().getFlags() & vmime::net::folderAttributes::Flags::FLAG_NO_OPEN;

      if (flag_no_open) {
        continue;
      }

      std::vector<MessageHeader> emails;

      // Open the folder
      folder->open(vmime::net::folder::MODE_READ_ONLY);

      int messageCount = folder->getMessageCount();
      if (messageCount == 0) {
        folder->close(false);
        continue;
      }
      auto messages = folder->getMessages(vmime::net::messageSet::byNumber(1, -1));

      folder->fetchMessages(messages, vmime::net::fetchAttributes::ENVELOPE | vmime::net::fetchAttributes::UID);
      vmime::utility::outputStreamAdapter os(std::cout);

      int counter = 0;
      for (const auto& message : messages) {
        try{
          // Extract header and body content
          auto header = message->getHeader();
          // auto content = message->getParsedMessage()->getBody()->getContents();

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

          emails.push_back(MessageHeader {
              toText.getWholeBuffer(),
              senderText.getWholeBuffer(),
              subjectText.getWholeBuffer(),
              message->getUID()
          });
          HtmlParser::decode_quoted_printable(emails.back());
        }
        catch (vmime::exception& e) {
          logging::log("Error processing email: " +(std::string)e.what());
        }
        catch (std::exception& e) {
          logging::log("General error: " + (std::string)e.what());
        }
      }

      std::reverse(emails.begin(), emails.end());

      folders.push_back(Folder{folder->getFullPath().toString("/", "utf-8"), emails});

      // Disconnect and close the folder and store
      folder->close(false);  // `false` means do not expunge deleted messages
    }
    store->disconnect();
  }
  catch (vmime::exception& e) {
    logging::log("Error retrieving emails: " + (std::string)e.what());
    return {folders, false};
  }
  catch (std::exception& e) {
    logging::log("General error: " +(std::string)e.what());
    return {folders, false};
  }

  return {folders, true};
}

bool MailStorage::synchronize(const Account& account) noexcept
{
  logging::log("mail_storage_synchronize");
  // Fetch emails before deleting
  auto [emails, status] = fetch_emails(account);

  sqlite3* db = open_db(account.username);

  const char* sql = "DELETE FROM MailHeaders;";
  char* err_msg = nullptr;
  int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
  if (rc != SQLITE_OK) {
      logging::log("SQL error: " + (std::string)err_msg);
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
  logging::log("mail_storage_load_emails");
  std::vector<Folder> folders;

  sqlite3* db = open_db(email);

  // Query to get all unique folders
  sqlite3_stmt* folder_stmt;
  const char* folder_sql = "SELECT DISTINCT Folder FROM MailHeaders;";
  if (sqlite3_prepare_v2(db, folder_sql, -1, &folder_stmt, nullptr) != SQLITE_OK) {
    logging::log("Failed to prepare statement: " + (std::string)sqlite3_errmsg(db));
    return folders;
  }

  while (sqlite3_step(folder_stmt) == SQLITE_ROW) {
    const char* folder_name = reinterpret_cast<const char*>(sqlite3_column_text(folder_stmt, 0));
    std::vector<MessageHeader> emails;

    sqlite3_stmt* stmt;
    const char* sql = "SELECT Sender, Subject, UID, Recipient, Folder FROM MailHeaders WHERE Folder = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      logging::log("Failed to prepare statement: " +(std::string)sqlite3_errmsg(db));
      continue;
    }

    sqlite3_bind_text(stmt, 1, folder_name, -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
      const char* sender = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
      const char* subject = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
      const char* uid = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
      const char* recipient = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
      const char* folder = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

      emails.push_back(MessageHeader{recipient, sender, subject, uid, folder});
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    folders.push_back(Folder{folder_name, emails});
  }

  sqlite3_finalize(folder_stmt);

  return folders;
}

Folder MailStorage::get_sent_emails(const std::string &email) noexcept {
    logging::log("mail_storage_get_sent_emails");

    sqlite3* db = open_db(email);
    sqlite3_stmt* stmt;
    const char* sql = "SELECT Sender, Subject, ID, Recipient FROM SentEmails;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      logging::log("Failed to prepare statement: " +(std::string)sqlite3_errmsg(db));
      return Folder{"Sent", {}};
    }

    std::vector<MessageHeader> res;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      const char* sender = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
      const char* subject = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
      const char* uid = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
      const char* recipient = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

      res.push_back(MessageHeader{recipient, sender, subject, uid, "Sent"});
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return Folder{"Sent", res};
}

std::vector<Folder> MailStorage::get_email_headers(const Account& account) noexcept
{
  logging::log("mail_storage_get_email_headers");
  auto res = load_emails(account.username);
  res.push_back(get_sent_emails(account.username));
  return res;
}

std::string MailStorage::get_sent_email_body(const std::string &email, const std::string &id) noexcept {
    logging::log("mail_storage_get_sent_email_body");

    sqlite3* db = open_db(email);
    sqlite3_stmt* stmt;
    const char* sql = "SELECT Content FROM SentEmails WHERE ID = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      logging::log("Failed to prepare statement: " +(std::string)sqlite3_errmsg(db));
      return "";
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);

    std::string res = "";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      const char* content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
      res = content;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return res;
}

std::string MailStorage::get_email_body(const Account& account, const std::string& uid, const std::string& folder_path) noexcept
{
  logging::log("mail_storage_get_email_body");

  if (folder_path == "Sent") return get_sent_email_body(account.username, uid);

  try{
    std::shared_ptr<vmime::net::store> store = get_store(account);

    // Connect to the IMAP server
    store->connect();

    auto path = vmime::utility::path::fromString(folder_path, "/", "utf-8");
    auto folder = store->getFolder(path);

    folder->open(vmime::net::folder::MODE_READ_ONLY);

    auto messages = folder->getMessages(vmime::net::messageSet::byUID(uid, uid));

    folder->fetchMessages(messages, vmime::net::fetchAttributes::UID);

    if (messages.empty()) {
      folder->close(false);
      store->disconnect();
      return "";
    }

    auto message = messages.front();

    std::string body = "";

    vmime::messageParser mp(message->getParsedMessage());

		// Enumerate text parts
		for (size_t i = 0 ; i < mp.getTextPartCount() ; ++i) {

			const vmime::textPart& part = *mp.getTextPartAt(i);

      if (part.getType().getSubType() == vmime::mediaTypes::TEXT_PLAIN) {
        const vmime::textPart& tp = dynamic_cast<const vmime::textPart&>(part);
        vmime::utility::outputStreamStringAdapter textStream(body);
				tp.getText()->extract(textStream);

        break;
      }
    }

    folder->close(false);
    store->disconnect();

    return body;
  }
  catch (vmime::exception& e) {
    logging::log("Error retrieving email body: " +(std::string)e.what());
    return "";
  }
  catch (std::exception& e) {
    logging::log("General error: " + (std::string)e.what());
    return "";
  }
}

bool MailStorage::save_sent_email(const MessageToSend &message, const std::string &email) noexcept {
    logging::log("mail_storage_save_sent_email");
    sqlite3* db = open_db(email);

    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO SentEmails (Sender, Subject, Recipient, Content) VALUES (?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
      logging::log("Failed to prepare statement: " +(std::string)sqlite3_errmsg(db));
      return false;
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, message.subject.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, message.recipient.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, message.body.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        logging::log("Failed to step: " + (std::string)sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}
