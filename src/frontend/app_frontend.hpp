#pragma once
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"
#include "backend/mailbox.h"

class Application;

struct Email_draft{
    std::string recipient = "";
    std::string subject = ""; 
    std::string message = "";
};


class Application_frontend{
public:
    Application_frontend(Application& app);
    void Loop();
private:
    bool Copy_selected_text(ftxui::Event event);
    std::vector<Message> fetch_received_emails();
    std::vector<Message> fetch_send_emails();

    Application& app;
    std::vector<Message> received_email_vector;
    std::vector<Message> send_email_vector;
    int inbox_page;
    int send_page;
    Email_draft current_email_draft;
    Message current_received_email;
    Message current_send_email;
    ftxui::Component email_draft_layout;
    ftxui::Component received_email_layout;
    ftxui::Component send_email_layout;
    ftxui::Component inbox;
    ftxui::Component sent_items;
    ftxui::Component main_component;
    ftxui::Component control_panel;
    ftxui::Component layout;
    ftxui::ScreenInteractive screen;
};