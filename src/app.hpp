#pragma once
#include <memory>
#include <vector>
#include <filesystem>
#include "backend/mailbox.h"


class Application_frontend;

class Application {
public:
    enum class State {
        EMAIL_DRAFT,
        MENU,
        EMAIL_VIEW,
        LOG_IN
    };
    void run(std::unique_ptr<Application_frontend> front);
    bool is_in_state(State state);
    void change_state(State new_state);
    void set_current_email_address(std::string new_address);
    std::string get_current_email_address();
    void send_email(const MessageToSend& email);
    void synchronize();
    std::vector<Folder> fetch_email_headers();
    std::string get_email_body(const std::string& uid, const std::string& folder_path);

    static std::filesystem::path get_home_path() noexcept;
    static std::string get_config_path();

    Application();
private:
    Mailbox get_current_mailbox();
    State current_state;
    std::unique_ptr<Application_frontend> frontend;
    std::string current_email_address = "";
};