#pragma once

#include <vector>
#include <string>

#include "html_parser.h"

struct Message {
  std::string recipient;
  std::string sender;
  std::string subject;
  std::string body;

  void decodeQuotedPrintable() {
    recipient = HtmlParser::decodeQuotedPrintable(recipient);
    sender = HtmlParser::decodeQuotedPrintable(sender);
    subject = HtmlParser::decodeQuotedPrintable(subject);
    body = HtmlParser::decodeQuotedPrintable(body);
  }
};

struct Folder {
  std::string name;
  std::vector<Message> messages;
};