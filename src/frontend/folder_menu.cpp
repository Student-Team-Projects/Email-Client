
#include "folder_menu.hpp"

#include <codecvt>
#include <locale>
#include <iostream>
#include <thread>
namespace{
    constexpr std::size_t page_size = 7;

    std::wstring add_elipsis(const std::wstring &str, size_t max_size)
    {
        if (str.size() <= max_size) {
            return str;
        }
        return str.substr(0, max_size - 3) + L"...";
    }
}

Folder_menu::Folder_menu(Application& app, DisplayMessage& current_email, 
        std::vector<Folder>& folder_vector, Folder& current_folder, 
        std::function<void(bool)> set_email_body_dim)
: app(app), current_email(current_email), folder_vector(folder_vector), 
    current_folder(current_folder), set_email_body_dim(set_email_body_dim){
        
    menu = ftxui::Container::Vertical({
        show_menu(folder_vector, current_folder, Application::State::MENU, current_email, email_vector, inbox, page)
    });

    inbox = ftxui::Container::Vertical({ftxui::Renderer([] {
            return ftxui::text("Loading...");
    })});

    auto next_prev_buttons = 
        ftxui::Container::Horizontal({
            ftxui::Button("←", [&]{
                page--;
                inbox->DetachAllChildren();
                std::vector<ftxui::Component> buttons
                    = show_folder(email_vector, current_email, Application::State::EMAIL_VIEW, page_size*page, page_size);
                for(auto b : buttons){
                    inbox->Add(b);
                }
            },ftxui::ButtonOption::Animated(ftxui::Color::DarkTurquoise)) | ftxui::Maybe([&]{return page>0 && app.is_in_state(Application::State::MENU);}),
            ftxui::Button("→", [&]{
                page++;
                inbox->DetachAllChildren();
                std::vector<ftxui::Component> buttons 
                    = show_folder(email_vector, current_email, Application::State::EMAIL_VIEW, page_size*page, page_size);
                for(auto b : buttons){
                    inbox->Add(b);
                }
                
            },ftxui::ButtonOption::Animated(ftxui::Color::Cyan3)) | ftxui::Maybe([&]{return page_size*(page+1)<email_vector.size() && app.is_in_state(Application::State::MENU);}),
            
        });

    inbox_layout = ftxui::Container::Vertical({
            next_prev_buttons,inbox
        }) | ftxui::vscroll_indicator | ftxui::frame |ftxui::xflex_grow; 
           //| ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, 20);
}

void Folder_menu::regenerate_menu(){
    menu->DetachAllChildren();
    std::vector<ftxui::Component> buttons
        = show_menu(folder_vector, current_folder, Application::State::MENU, current_email, email_vector, inbox, page);
    for(auto b : buttons){
        menu->Add(b);
    }
}

ftxui::Component Folder_menu::get_menu_layout(){
    return menu;
}
ftxui::Component Folder_menu::get_inbox_layout(){
    return inbox_layout;
}

std::vector<ftxui::Component> Folder_menu::show_menu(std::vector<Folder>& folders,
    Folder& current_folder, Application::State state,DisplayMessage& current_message,std::vector<MessageHeader>& email_vector,ftxui::Component& inbox,int& page) {
    std::vector<ftxui::Component> buttons;
    if(folders.size()==0){
        return {ftxui::Renderer([] {
            return ftxui::text("Loading...");
            })
        };
    }else if(current_folder.name==""){
        //code duplication, violates something, idc
        page=0;
        current_folder = folders[0];
        email_vector = current_folder.messages;
        inbox->DetachAllChildren();
        std::vector<ftxui::Component> email_buttons = show_folder(folders[0].messages,current_message,Application::State::EMAIL_VIEW,0,page_size);
        for(auto b:email_buttons){
            inbox->Add(b);
        }
        app.change_state(state);
    }
    for (size_t i = 0; i < folders.size(); ++i) {
        // Be carefull what you pass as a reference -- state would be a dangling reference
        buttons.push_back(ftxui::Button(folders[i].name, [&folders, &current_folder, &current_message,&email_vector,&inbox,&page, state, i, this] {
            page=0;
            current_folder = folders[i];
            email_vector = current_folder.messages;
            inbox->DetachAllChildren();
            std::vector<ftxui::Component> email_buttons = show_folder(folders[i].messages,current_message,Application::State::EMAIL_VIEW,0,page_size);
            for(auto b:email_buttons){
                inbox->Add(b);
            }
            app.change_state(state);
        },ftxui::ButtonOption::Animated(ftxui::Color::Cyan)));
    }
    return buttons;
}

std::vector<ftxui::Component> Folder_menu::show_folder(std::vector<MessageHeader>& messages,
    DisplayMessage& current_message, Application::State state, std::size_t start_index, std::size_t count) {
    std::vector<ftxui::Component> buttons;
    for (size_t i = start_index; i < std::min(start_index + page_size,messages.size()); ++i) {
        // Be carefull what you pass as a reference -- state would be a dangling reference

        //change in case of many recipients!!
        MessageHeader message=messages[i];

        // delete when supporting wstrings everywhere
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

        std::wstring subject = add_elipsis(L"Subject: " + converter.from_bytes(message.subject), 100);
        std::wstring sender = add_elipsis(converter.from_bytes(message.sender), 40);
        std::wstring recipient = add_elipsis(converter.from_bytes(message.recipient), 40);

        buttons.push_back(ftxui::Renderer([sender, recipient] { return ftxui::bgcolor(ftxui::Color::DarkSeaGreen4, ftxui::text(L"From: " + sender + L" To: " + recipient)); }));
        buttons.push_back(ftxui::Button(subject, [message, &current_message, state, this] {
            current_message.recipient = message.recipient;
            current_message.sender = message.sender;
            current_message.subject = message.subject;
            current_message.body = "Loading...";

            set_email_body_dim(true);

            std::thread([this, message, &current_message]{
                std::string body = app.get_email_body(message.uid, message.folder);
                if (current_message.subject == message.subject && 
                    current_message.sender == message.sender && 
                    current_message.recipient == message.recipient) {
                    current_message.body = body;
                    set_email_body_dim(false);
                }
            }).detach();

            app.change_state(state);
            },ftxui::ButtonOption::Animated(ftxui::Color::DarkSeaGreen3)));
        //buttons.push_back(ftxui::Renderer([] { return ftxui::separator(); }));
    }
    return buttons;
}