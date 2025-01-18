#include "frontend/app_frontend.hpp"
#include "frontend/select_input.cpp"
#include "frontend/log_in.hpp"
#include "iostream"

#include <codecvt>
#include <locale>
#include <string>

ftxui::InputOption mail_input_style(const std::string& placeholder) {
    ftxui::InputOption option;
    option.content = "Type your email here";
    option.placeholder = placeholder;
    option.transform = [](ftxui::InputState state) {
        state.element |= ftxui::borderEmpty;
        state.element |= ftxui::color(ftxui::Color::White);

        if (state.is_placeholder) {
            state.element |= ftxui::dim;
        }

        if (state.focused) {
            state.element |= ftxui::bgcolor(ftxui::Color::Grey35);
        }
        state.element |= ftxui::bgcolor(ftxui::Color::Black);
        return state.element;
    };

    return option;
}

<<<<<<< HEAD
std::vector<ftxui::Component> Application_frontend::show_folder(std::vector<Message>& messages,
    Message& current_message, Application::State state, std::size_t start_index, std::size_t count) {
    std::vector<ftxui::Component> buttons;
    for (size_t i = start_index; i < std::min(start_index + page_size,messages.size()); ++i) {
        // Be carefull what you pass as a reference -- state would be a dangling reference

        //change in case of many recipients!!
        Message& message=messages[i];

        // delete when supporting wstrings everywhere
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

        std::wstring subject = add_elipsis(L"Subject: " + converter.from_bytes(message.subject), 50);
        std::wstring sender = add_elipsis(converter.from_bytes(message.sender), 20);
        std::wstring recipient = add_elipsis(converter.from_bytes(message.recipient), 20);

        buttons.push_back(ftxui::Renderer([sender, recipient] { return ftxui::text(L"From: " + sender + L" To: " + recipient); }));
        buttons.push_back(ftxui::Button(subject, [&messages, &current_message, state, i, this] {
            current_message = messages[i];
            app.change_state(state);
            }));
        buttons.push_back(ftxui::Renderer([] { return ftxui::separator(); }));
    }
    return buttons;
}


std::vector<ftxui::Component> Application_frontend::show_menu(std::vector<Folder>& folders,
    Folder& current_folder, Application::State state,Message& current_message,std::vector<Message>& email_vector,ftxui::Component& inbox,int& page) {
    std::vector<ftxui::Component> buttons;
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
    }));
    }
    return buttons;
}

=======
>>>>>>> refactoring phase 2 - inbox & folders
Application_frontend::Application_frontend(Application& app) :
    app(app),
    folder_vector(),
    log_in(log_in::get_log_in_data(app)),
    current_email(),
    current_folder(),
    screen(ftxui::ScreenInteractive::Fullscreen()),
    email_draft_layout(app),
    folder_menu(app, current_email, folder_vector, current_folder)
{   

    //once
    email_layout = ftxui::Container::Vertical({
        ftxui::SelectableText(
            &current_email.sender,
            mail_input_style("From:")
        ),
        ftxui::SelectableText(
            &current_email.recipient,
            mail_input_style("To:")
        ),
        ftxui::SelectableText(
            &current_email.subject,
            mail_input_style("Subject:")
        ),
        ftxui::SelectableText(
            &current_email.body,
            mail_input_style("Email")
        )
    });

    auto new_mail_button = 
        ftxui::Button("New mail", [&]{
            app.change_state(Application::State::EMAIL_DRAFT);
        });

    auto menu_component =
        ftxui::Container::Vertical({
            new_mail_button,
            folder_menu.get_menu_layout()
        });

    auto back_button = 
        ftxui::Button("Back", [&]{
            app.change_state(Application::State::MENU);
        });

    auto email_layout_wrapper = 
        ftxui::Container::Vertical({
            back_button,
            email_layout,
            //respond??
        });

    auto main_component = ftxui::CatchEvent(ftxui::Container::Horizontal({
        email_draft_layout.get_email_draft_component() | ftxui::Maybe([&]{return app.is_in_state(Application::State::EMAIL_DRAFT);}),
        folder_menu.get_inbox_layout() | ftxui::Maybe([&]{return app.is_in_state(Application::State::MENU);}),
        email_layout_wrapper | ftxui::Maybe([&]{return app.is_in_state(Application::State::EMAIL_VIEW);}),
        log_in.visuals | ftxui::Maybe([&]{return app.is_in_state(Application::State::LOG_IN);}),
    }), [&](ftxui::Event event){
        // If downloading emails finished, update the view
        if (event.input() == "refresh_emails") {
            refresh_emails();
        }

        return false;
    });
    
    layout = ftxui::Container::Horizontal({
        menu_component | ftxui::Maybe([&]{return !app.is_in_state(Application::State::LOG_IN);}) | ftxui::flex_shrink,
        main_component | ftxui::flex_shrink
    });
}

void Application_frontend::loop(){
    screen.Loop(layout);
}

/** Start synchronizing the mailbox periodically.
 * Call this method once when user logs in to a mailbox, and then
 * the synchronization will be done with breaks of synch_time_in_seconds.
 */
void Application_frontend::set_up_synchronization(){
    std::thread synchronize_mailbox([this]{
        while(true) {
            app.synchronize();
            screen.PostEvent(ftxui::Event::Special("refresh_emails"));

            std::unique_lock<std::mutex> lock(synch_m);
            synch_cv.wait_for(lock, std::chrono::seconds(synch_time_in_seconds));
        }
    });

    synchronize_mailbox.detach();
}

/** Synchronize the mailbox once.
 * Call this method when user requests to refresh the mailbox.
 */
void Application_frontend::synchronize()
{
    synch_cv.notify_one();
}

<<<<<<< HEAD
void Application_frontend::regenerate_menu()
{
    folder_menu->DetachAllChildren();
    std::vector<ftxui::Component> buttons
        = show_menu(folder_vector, current_folder, Application::State::MENU,current_email,email_vector,inbox,page);
    for(auto b : buttons){
        folder_menu->Add(b);
    }
}

std::wstring Application_frontend::add_elipsis(const std::wstring &str, size_t max_size)
{
    if (str.size() <= max_size) {
        return str;
    }
    return str.substr(0, max_size - 3) + L"...";
}

void Application_frontend::refresh_emails()
{
    folder_vector = app.fetch_emails();
    regenerate_menu();
=======
void Application_frontend::refresh_emails()
{
    folder_vector = app.fetch_emails();
    folder_menu.regenerate_menu();
>>>>>>> refactoring phase 2 - inbox & folders
}
