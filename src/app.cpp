#include "app.hpp"
#include "frontend/app_frontend.hpp"
#include "backend/mail_types.h"
#include "backend/mailbox.h"
#include <memory>
#include <algorithm> 

#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>
#include "logging/logging.hpp"

namespace{

    void setup_config_file(){
        logging::log("setup_config_file");
        std::filesystem::create_directories(Application::get_config_home_path());
        std::filesystem::create_directories(Application::get_data_home_path());

        bool needs_init = !std::filesystem::exists(Application::get_config_path());

        if(needs_init){
            std::ofstream config_file_new(Application::get_config_path());
            config_file_new << "[]" << std::endl; 
            config_file_new.close();
        }
    }
}

void Application::run(std::unique_ptr<Application_frontend> front){
    logging::log("app_run");
    frontend = std::move(front);
    frontend->run();
    if(frontend->loginSucceeded == false) return;
}

bool Application::is_in_state(State state){
    logging::log("app_is_in_state");
    return state == current_state;
}

void Application::change_state(State new_state){
    logging::log("app_change_state");
    auto old_state = current_state;
    current_state = new_state;

    for(auto on_event : on_state_change_events){
        on_event(old_state, new_state);
    }
}

void Application::add_on_state_change_event(std::function<void(State, State)> on_event){
    logging::log("app_on_state_change_event");
    on_state_change_events.push_back(on_event);
}

/*
* Use this method if you want to send an email from currently selected email address.
*/
void Application::send_email(const MessageToSend& email){
    logging::log("app_send_email");
    if(email.recipient.empty() || email.subject.empty() || email.body.empty()){
        return;
    }

    std::ifstream configFile(get_config_path());
    if (!configFile){
        logging::log("Failed to open config.json");
        return;
    }

    logging::log("managed to open config.json");

    nlohmann::json config;
    configFile >> config;

    std::string senderEmail = current_email_address;
    std::string appPassword = "";
    
    auto user = std::find_if(config.begin(), config.end(), [&](const auto& v){
        return v["sender_email"] == current_email_address;
    });
    if(user == config.end()){
        logging::log("No app password provided for the current email address!");
        return;
    }
    appPassword = (std::string)(*user)["app_password"];

    Mailbox mailbox(senderEmail, appPassword);
    mailbox.send(email);
    logging::log("email sent");
}

void Application::synchronize()
{
    logging::log("app_synchronize");
    Mailbox mailbox = get_current_mailbox();
    mailbox.synchronize();
}

std::vector<Folder> Application::fetch_email_headers(){
    logging::log("app_fetch_email_headers");
    Mailbox mailbox = get_current_mailbox();
    std::vector<Folder> emails = mailbox.get_email_headers();
    logging::log("emails retrieved");
    return emails;
}

std::string Application::get_email_body(const std::string &uid, const std::string &folder_path)
{
    logging::log("app_get_email_body");
    Mailbox mailbox = get_current_mailbox();
    return mailbox.get_email_body(uid, folder_path);
}

std::filesystem::path Application::get_config_home_path() noexcept
{
    logging::log("app_get_config_home_path");
    const char* config_home = std::getenv("XDG_CONFIG_HOME");
    if (config_home != nullptr) return std::filesystem::path(config_home) / "email_client";
    const char* home = std::getenv("HOME");
    if (home != nullptr) return std::filesystem::path(home) / ".config" / "email_client";
    return std::filesystem::current_path() / ".email_client";
}

std::filesystem::path Application::get_data_home_path() noexcept
{
    logging::log("app_get_data_home_path");
    const char* data_home = std::getenv("XDG_DATA_HOME");
    if (data_home != nullptr) return std::filesystem::path(data_home) / "email_client";
    const char* home = std::getenv("HOME");
    if (home != nullptr) return std::filesystem::path(home) / ".local" / "share" / "email_client";
    return std::filesystem::current_path() / ".email_client";
}

std::string Application::get_config_path() noexcept
{
    logging::log("app_get_config_path");
    return Application::get_config_home_path() / "config.json";
}

void Application::set_current_email_address(std::string new_address){
    logging::log("app_set_current_email_address: "+new_address);
    current_email_address = new_address;
    logging::log("set_current");
    // Refresh emails immidiately if we have some loaded
    frontend->refresh_emails();
    logging::log("after_refresh");
    // And download them
    synchronize();
}

std::string Application::get_current_email_address(){
    logging::log("app_get_current_email_address");
    return current_email_address;
}

Application::Application() 
:   current_state(State::INVALID)
{
    setup_config_file();
    change_state(State::LOG_IN);
}

Mailbox Application::get_current_mailbox()
{
    logging::log("app_get_current_mailbox");
    std::ifstream configFile(get_config_path());
    if (!configFile){
        logging::log("Failed to open config.json");
    }

    nlohmann::json config;
    configFile >> config;

    std::string senderEmail = current_email_address;
    std::string appPassword = "";
    
    auto user = std::find_if(config.begin(), config.end(), [&](const auto& v){
        return v["sender_email"] == current_email_address;
    });
    if(user == config.end()){
        // We have to handle this more gracefully
        throw std::runtime_error("No app password provided for the current email address!");
    }
    appPassword = (*user)["app_password"];

    return Mailbox(senderEmail, appPassword);
}
