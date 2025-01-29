#include "app.hpp"
#include "frontend/app_frontend.hpp"
#include "backend/mail_types.h"
#include "backend/mailbox.h"
#include <memory>
#include <algorithm> 

#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>

namespace{

    void setup_config_file(){
        std::filesystem::create_directories(Application::get_home_path() / ".email_client");

        bool needs_init = !std::filesystem::exists(Application::get_config_path());

        if(needs_init){
            std::ofstream config_file_new(Application::get_config_path());
            config_file_new << "[]" << std::endl; 
            config_file_new.close();
        }
    }
}

void Application::run(std::unique_ptr<Application_frontend> front){
    frontend = std::move(front);
    frontend->loop();
}

bool Application::is_in_state(State state){
    return state == current_state;
}

void Application::change_state(State new_state){
    current_state = new_state;
}

void Application::send_email(const MessageToSend& email){
    std::cerr << "sending email..." << std::endl;
    if(email.recipient.empty() || email.subject.empty() || email.body.empty()){
        return;
    }

    std::ifstream configFile(get_config_path());
    if (!configFile){
        std::cerr << "Failed to open config.json"<< std::endl;
    }

    std::cerr << "managed to open config.json"<< std::endl;

    nlohmann::json config;
    configFile >> config;

    std::string senderEmail = current_email_address;
    std::string appPassword = "";
    
    auto user = std::find_if(config.begin(), config.end(), [&](const auto& v){
        return v["sender_email"] == current_email_address;
    });
    if(user == config.end()){
        std::cerr << "No app password provided for the current email address!";
        return;
    }
    appPassword = (std::string)(*user)["app_password"];

    Mailbox mailbox(senderEmail, appPassword);
    mailbox.send(email);
    std::cerr << "email sent"<< std::endl;
}

void Application::synchronize()
{
    Mailbox mailbox = get_current_mailbox();
    mailbox.synchronize();
}

std::vector<Folder> Application::fetch_emails(){
    Mailbox mailbox = get_current_mailbox();
    std::vector<Folder> emails = mailbox.get_email_headers();
    std::cerr << "emails retrieved"<< std::endl;
    return emails;
}

std::filesystem::path Application::get_home_path() noexcept
{
    const char* home = std::getenv("HOME");
    if (home == nullptr) {
        std::cerr << "Failed to get home directory" << std::endl;
        return std::filesystem::current_path();
    }

    return std::filesystem::path(home);
}

std::string Application::get_config_path()
{
    return Application::get_home_path() / ".email_client/config.json";
}

void Application::set_current_email_address(std::string new_address){
    current_email_address = new_address;
    std::cerr<<"set_current"<<std::endl;
    // Refresh emails immidiately if we have some loaded
    frontend->refresh_emails();
    std::cerr<<"after_refresh"<<std::endl;
    // And download them
    frontend->set_up_synchronization();
}

std::string Application::get_current_email_address(){
    return current_email_address;
}

Application::Application() 
:   current_state(State::LOG_IN)
{
    setup_config_file();
}

Mailbox Application::get_current_mailbox()
{
    std::ifstream configFile(get_config_path());
    if (!configFile){
        std::cerr << "Failed to open config.json"<< std::endl;
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
