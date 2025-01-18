#pragma once
#include <memory>
#include <vector>
#include "backend/mailbox.h"
#include <functional>

class Application_frontend;
class Email_draft;


class Application {
public:
    enum class State {
        EMAIL_DRAFT,
        MENU,
        EMAIL_VIEW,
        LOG_IN,
        INVALID
    };
    void run(std::unique_ptr<Application_frontend> front);
    bool is_in_state(State state);
    void change_state(State new_state);
    void add_on_state_change_event(std::function<void(State, State)>);
    void set_current_email_address(std::string new_address);
    std::string get_current_email_address();
    void send_email(const Email_draft& email);
    void synchronize();
    std::vector<Folder> fetch_emails();
    Application();
private:
    Mailbox get_current_mailbox();
    State current_state;
    std::unique_ptr<Application_frontend> frontend;
    std::string current_email_address = "";
    std::vector<std::function<void(State, State)>> on_state_change_events{};
};