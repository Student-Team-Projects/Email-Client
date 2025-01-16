#pragma once

#include <vector>
#include <string>

struct Message {
  std::string recipient;
  std::string sender;
  std::string subject;
  std::string body;
};

struct Folder {
  std::string name;
  std::vector<Message> messages;
};