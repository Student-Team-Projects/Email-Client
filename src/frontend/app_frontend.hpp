#pragma once
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"
#include "backend/mailbox.h"
#include "frontend/log_in.hpp"
#include "app.hpp"
#include "email_draft_layout.hpp"
#include "folder_menu.hpp"

class Application;
class Application_frontend{
public:
    Application_frontend(Application& app);
    void loop();
    void set_up_synchronization();
    void synchronize();
    void refresh_emails();
private:
    Application& app;
    log_in::Log_in_data log_in;
    std::vector<Folder> folder_vector;
    DisplayMessage current_email;
    Folder current_folder;
    Email_draft_layout email_draft_layout;
    ftxui::Component email_layout;
    ftxui::Component layout;
    ftxui::ScreenInteractive screen;
    Folder_menu folder_menu;

    std::condition_variable synch_cv;
    std::mutex synch_m;
    static inline constexpr std::size_t synch_time_in_seconds = 30;
};