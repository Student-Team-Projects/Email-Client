#pragma once

#include <string>

struct Account {
    std::string username;
    std::string password;
    std::string smtpHost;
    std::string imapHost;
    std::string certPath;
};
