#pragma once
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"
#include "backend/mailbox.h"
#include "frontend/log_in.hpp"
#include "app.hpp"
#include "email_types.hpp"
#include "email_draft_layout.hpp"

class Application;
class Application_frontend{
public:
    Application_frontend(Application& app);
    void loop();
    void set_up_synchronization();
    void synchronize();
    void refresh_emails();
private:
    //void regenerate_folder(const std::string& folder_name);
    void regenerate_menu();
    std::wstring add_elipsis(const std::wstring& str, size_t max_size);
    std::vector<ftxui::Component> show_folder(std::vector<Message>& messages, Message& current_message, 
        Application::State state, std::size_t start_index, std::size_t count);
    std::vector<ftxui::Component> show_menu(std::vector<Folder>& folders, Folder& current_folder, 
        Application::State state, Message& current_message, std::vector<Message>& email_vector,
        ftxui::Component& inbox, int& page);


    Application& app;
    log_in::Log_in_data log_in;
    std::vector<Message> email_vector;
    //std::vector<Message> sent_email_vector;
    std::vector<Folder> folder_vector;
    int page;
    Message current_email;
    Folder current_folder;
    ftxui::Component next_prev_buttons;
    ftxui::Component back_button;
    ftxui::Component email_control;
    Email_draft_layout email_draft_layout;
    //ftxui::Component received_email_layout;
    ftxui::Component email_layout;
    ftxui::Component email_layout_wrapper;
    ftxui::Component inbox;
    ftxui::Component inbox_wrapper;
    ftxui::Component new_mail_button;
    ftxui::Component folder_menu;
    ftxui::Component menu_component;
    ftxui::Component sent_items;
    ftxui::Component main_component;
    ftxui::Component control_panel;
    ftxui::Component layout;
    ftxui::ScreenInteractive screen;

    std::condition_variable synch_cv;
    std::mutex synch_m;
    static inline constexpr std::size_t synch_time_in_seconds = 30;
};