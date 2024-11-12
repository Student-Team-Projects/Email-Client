#pragma once
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"

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
    Application& app;
    Email_draft current_email_draft;
    ftxui::Component email_draft_layout;
    ftxui::Component inbox;
    ftxui::Component sent_items;
    ftxui::Component main_component;
    ftxui::Component control_panel;
    ftxui::Component layout;
    ftxui::ScreenInteractive screen;
};