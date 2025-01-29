#pragma once

#include <vector>
#include <string>

#include "html_parser.h"

struct MessageHeader {
  std::string recipient;
  std::string sender;
  std::string subject;
  std::string body;
  std::string uid;

  void decode_quoted_printable() {
    recipient = HtmlParser::decode_quoted_printable(recipient);
    sender = HtmlParser::decode_quoted_printable(sender);
    subject = HtmlParser::decode_quoted_printable(subject);
    // body = HtmlParser::decode_quoted_printable(body);
  }
};

struct MessageToSend {
  std::string recipient;
  std::string subject;
  std::string body;
};

struct Folder {
  std::string name;
  std::vector<MessageHeader> messages;
};