#pragma once

#include "backend/account.h"
#include "nlohmann/json.hpp"

#define DEFAULT_CERT "/etc/ssl/cert.pem"

inline Account accountFromJson(const nlohmann::json& json) {
    Account account;
    account.username = json["username"];
    account.password = json["password"];
    account.smtpHost = json["smtpHost"];
    account.imapHost = json["imapHost"];
    account.certPath = json.value("certPath", DEFAULT_CERT);
    return account;
}
inline nlohmann::json accountToJson(const Account& account) {
    nlohmann::json json = {
        {"username", account.username},
        {"password", account.password},
        {"smtpHost", account.smtpHost},
        {"imapHost", account.imapHost},
    };

    if (account.certPath != DEFAULT_CERT) {
        json["certPath"] = account.certPath;
    }

    return json;
}
