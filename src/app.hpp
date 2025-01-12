#pragma once
#include <memory>
#include <vector>
#include "backend/mailbox.h"


class Application_frontend;
class Email_draft;


class Application {
public:
    enum class State {
        EMAIL_DRAFT,
        MENU,
        EMAIL_VIEW,
        LOG_IN
    };
    void Run(std::unique_ptr<Application_frontend> front);
    bool Is_in_state(State state);
    void Change_state(State new_state);
    void Set_current_email_address(std::string new_address);
    std::string Get_current_email_address();
    void Send_email(const Email_draft& email);
    void synchronize();
    std::vector<Folder> fetch_emails();
    Application();
private:
    Mailbox get_current_mailbox();
    State current_state;
    std::unique_ptr<Application_frontend> frontend;
    std::string current_email_address = "";
};