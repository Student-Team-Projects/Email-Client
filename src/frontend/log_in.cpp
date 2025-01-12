#include "log_in.hpp"
#include "select_input.hpp"
#include "algorithm"
#include "vector"
#include "ftxui/dom/elements.hpp"
#include "app.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

namespace log_in{
    
    Log_in_data::Log_in_data(const Log_in_data& other)
    : 
        page(other.page),
        new_account_mail(other.new_account_mail),
        new_account_password(other.new_account_mail),
        signed_in_accounts(other.signed_in_accounts),
        accounts_buttons(other.accounts_buttons),
        accounts(other.accounts),
        new_account(other.new_account),
        visuals(other.visuals),
        app(other.app)
    {}

    Log_in_data::Log_in_data(Application& app) : app(app){
        accounts_buttons =ftxui::Container::Horizontal({
            ftxui::Button("Next", [&]{
                page++;
                Update_signed_in_accounts();
            }) | ftxui::Maybe([&]{return page_size*(page + 1) <= get_signed_in_accounts().size();}),
            ftxui::Button("Previous", [&]{
                page--;
                Update_signed_in_accounts();
            }) | ftxui::Maybe([&]{return page > 0;}),
        });
        
        signed_in_accounts = ftxui::Container::Vertical({});
        Update_signed_in_accounts();

        accounts = ftxui::Container::Vertical({
            signed_in_accounts,
            accounts_buttons
        });

        ftxui::InputOption password_option;
        password_option.password = true;
        password_option.content = &new_account_password;
        password_option.placeholder = "App password: ";

        new_account = ftxui::Container::Vertical({
            ftxui::Input(&new_account_mail, "Email address: ", {}),
            ftxui::Input(password_option),
            ftxui::Button("Add account", [&]{
                std::cerr << "ADDED NEW ACCOUNT " << new_account_mail << " " << "with some password" << std::endl;
                add_account({new_account_mail, new_account_password});
                Update_signed_in_accounts();
            })
        });

        visuals = ftxui::Container::Vertical({
            accounts,
            new_account
        });
    }

    void Log_in_data::Log_in_as(const std::string account_email){
        // std::cerr << std::endl << "Ten NIEFAJNY email POD" << std::endl;
        // std::cerr << std::endl << account_email << std::endl;
        // std::cerr << std::endl << "Ten NIEFAJNY email NAD" << std::endl;
        app.Set_current_email_address(account_email);
        std::cerr<<"set"<<std::endl;
        app.Change_state(Application::State::MENU);
        std::cerr<<"loged"<<std::endl;
    }

    std::vector<std::pair<std::string, std::string>> get_signed_in_accounts(){
        std::ifstream configFile("config.json");
        if (!configFile){ 
            std::cerr << "Failed to open config.json"<< std::endl;
        }

        std::cerr << "managed to open config.json"<< std::endl;

        nlohmann::json config;
        configFile >> config;

        // std::cerr << config << std::endl;

        std::vector<std::pair<std::string, std::string>> emails_passwords;
        for(auto& account : config){
            // std::cerr << account << std::endl;
            emails_passwords.push_back({std::string(account["sender_email"]), std::string(account["app_password"])});
        }

        return emails_passwords;
    }

    void Log_in_data::Update_signed_in_accounts(){
        signed_in_accounts->DetachAllChildren();
        const auto& signed_in_accounts_data = get_signed_in_accounts();

        for(int i = page * page_size; i >= 0 && i < (page + 1) * page_size && i < (int) signed_in_accounts_data.size(); ++i){
            // std::cerr<< signed_in_accounts_data[i].first << " " << signed_in_accounts_data[i].second << std::endl;
            const auto account = signed_in_accounts_data[i];
            std::string label = account.first;
            signed_in_accounts->Add(
                ftxui::Button(label, [&, account](){
                    std::cerr<<"clicked"<<std::endl;
                    Log_in_as(account.first);
                })
            );
        }
    }

    void add_account(const std::pair<std::string, std::string>& new_account){
        std::ifstream configFile("config.json");
        if (!configFile){ 
            std::cerr << "Failed to open config.json"<< std::endl;
        }

        std::cerr << "managed to open config.json"<< std::endl;

        nlohmann::json config;
        configFile >> config;

        nlohmann::json new_account_json;
        new_account_json["sender_email"] = new_account.first;
        new_account_json["app_password"] = new_account.second;
        config.push_back(new_account_json);

        configFile.close();
        std::ofstream new_config_file("config.json", std::ios::trunc);
        new_config_file << config;
    }

    Log_in_data get_log_in_data(Application& app){
        return Log_in_data(app);
    }
}