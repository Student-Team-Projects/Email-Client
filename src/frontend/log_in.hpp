#pragma once
#include "ftxui/component/component.hpp"

class Application;
namespace log_in{
    struct Log_in_data{
        int page = 0;
        constexpr static int page_size = 4;  

        std::string new_account_mail = "";
        std::string new_account_password = "";

        ftxui::Component signed_in_accounts;
        ftxui::Component accounts_buttons;
        ftxui::Component accounts;
        ftxui::Component new_account;

        ftxui::Component visuals;
        Application& app;

        Log_in_data(const Log_in_data&);
        Log_in_data(Application&);
        void Update_signed_in_accounts();
        void Log_in_as(const std::string);
    };

    void add_account(const std::pair<std::string, std::string>& new_account);
    std::vector<std::pair<std::string, std::string>> get_signed_in_accounts();
    Log_in_data get_log_in_data(Application& app);
}