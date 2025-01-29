#pragma once

#include <vector>
#include <string>

#include "html_parser.h"

struct MessageHeader {
  std::string recipient;
  std::string sender;
  std::string subject;
  std::string uid;
  std::string folder;
};

struct DisplayMessage {
  std::string recipient;
  std::string sender;
  std::string subject;
  std::string body;
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