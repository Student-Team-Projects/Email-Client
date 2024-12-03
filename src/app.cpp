#include "app.hpp"
#include "frontend/app_frontend.hpp"
#include "backend/mail_types.h"
#include "backend/mailbox.h"
#include <memory>

#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>

void Application::Run(std::unique_ptr<Application_frontend> front){
    frontend = std::move(front);
    frontend->Loop();
}

bool Application::Is_in_state(State state){
    return state == current_state;
}

void Application::Change_state(State new_state){
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

    std::string senderEmail = config["sender_email"];
    std::string appPassword = config["app_password"];

    Mailbox mailbox(senderEmail, appPassword);
    Message message({{email.recipient}, email.subject, email.message});
    mailbox.send(message);
    std::cerr << "email sent"<< std::endl;
}

Application::Application() :
    current_state(State::INBOX)
{}