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
        std::ifstream config_file;
        config_file.open ("config.json", std::ifstream::in);
        
        if(config_file.peek() == std::ifstream::traits_type::eof()){
            config_file.close();
            std::ofstream config_file_new("config.json");
            config_file_new << "[]" << std::endl; 
            config_file_new.close();
        }
    }
}

void Application::Run(std::unique_ptr<Application_frontend> front){
    frontend = std::move(front);
    frontend->Loop();
}

bool Application::Is_in_state(State state){
    return state == current_state;
}

void Application::Change_state(State new_state){
    if(new_state == State::INBOX){
        frontend->Synchronize();
    }
    current_state = new_state;
}

void Application::Send_email(const Email_draft& email){
    std::cerr << "sending email..." << std::endl;
    if(email.recipient.empty() || email.subject.empty() || email.message.empty()){
        return;
    }

    std::ifstream configFile("config.json");
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
    Message message;
    message.recipients = {email.recipient};
    message.subject = email.subject;
    message.body = email.message;
    mailbox.send(message);
    std::cerr << "email sent"<< std::endl;
}

std::vector<Message> Application::fetch_received_emails(){
    std::cerr << "fetching emails..." << std::endl;

    std::ifstream configFile("config.json");
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
        std::cerr << "No app password provided for the current email address!";
        return {};
    }
    appPassword = (*user)["app_password"];

    
    Mailbox mailbox(senderEmail, appPassword);
    std::vector<Message> emails = mailbox.get_emails();
    std::cerr << "emails retrieved"<< std::endl;
    return emails;
}

std::vector<Message> Application::fetch_sent_emails(){
    std::cerr << "fetching emails..." << std::endl;

    std::ifstream configFile("config.json");
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
        std::cerr << "No app password provided for the current email address!";
        return {};
    }
    appPassword = (*user)["app_password"];

    
    Mailbox mailbox(senderEmail, appPassword);
    std::vector<Message> emails = mailbox.get_sent_emails();
    std::cerr << "emails retrieved"<< std::endl;
    return emails;
}

void Application::Set_current_email_address(std::string new_address){
    current_email_address = new_address;
}

std::string Application::Get_current_email_address(){
    return current_email_address;
}

Application::Application() 
:   current_state(State::LOG_IN)
{
    setup_config_file();
}