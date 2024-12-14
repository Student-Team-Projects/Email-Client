#pragma once

#include <vector>
#include <string>

struct Message {
  std::vector<std::string> recipients;
  std::string sender;
  std::string subject;
  std::string body;
};