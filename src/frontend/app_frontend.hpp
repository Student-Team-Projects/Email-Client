#pragma once
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"
#include "backend/mailbox.h"
#include "frontend/log_in.hpp"

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
    void Synchronize();
    void refresh_emails();
private:
    bool Copy_selected_text(ftxui::Event event);
    void regenerate_folder(const std::string& folder_name);

    Application& app;
    log_in::Log_in_data log_in;
    std::vector<Message> received_email_vector;
    std::vector<Message> sent_email_vector;
    int inbox_page;
    int sent_page;
    Email_draft current_email_draft;
    Message current_received_email;
    Message current_sent_email;
    ftxui::Component email_draft_layout;
    ftxui::Component received_email_layout;
    ftxui::Component sent_email_layout;
    ftxui::Component inbox;
    ftxui::Component sent_items;
    ftxui::Component main_component;
    ftxui::Component control_panel;
    ftxui::Component layout;
    ftxui::ScreenInteractive screen;
};