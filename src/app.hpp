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
        INBOX,
        SENT_ITEMS,
        RECEIVED_EMAIL,
        SENT_EMAIL,
    };
    void Run(std::unique_ptr<Application_frontend> front);
    bool Is_in_state(State state);
    void Change_state(State new_state);
    void Send_email(const Email_draft& email);
    std::vector<Message> fetch_received_emails();
    std::vector<Message> fetch_sent_emails();
    Application();
private:
    State current_state;
    std::unique_ptr<Application_frontend> frontend;
};