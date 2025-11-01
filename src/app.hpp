#pragma once
#include <memory>
#include <vector>
#include <filesystem>
#include "backend/mailbox.h"
#include <functional>

class Application_frontend;
/*
* Main application class, contains state information and allows for communication between backend and frontend.
*/
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
    void send_email(const MessageToSend& email);
    void synchronize();
    std::vector<Folder> fetch_email_headers();
    std::string get_email_body(const std::string& uid, const std::string& folder_path);

    static std::filesystem::path get_config_home_path() noexcept;
    static std::filesystem::path get_data_home_path() noexcept;
    static std::string get_config_path() noexcept;

    Application();
private:
    Mailbox get_current_mailbox();
    State current_state;
    std::unique_ptr<Application_frontend> frontend;
    /*Email account the user is currently logged in and using.*/
    std::string current_email_address = "";
    std::vector<std::function<void(State, State)>> on_state_change_events{};
};
