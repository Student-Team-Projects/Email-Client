#include "frontend/app_frontend.hpp"
#include "frontend/select_input.cpp"
#include "app.hpp"
#include "frontend/log_in.hpp"
#include "iostream"

constexpr std::size_t page_size = 10;

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

std::vector<ftxui::Component> show_folder(Application& app, std::vector<Message>& messages,
    Message& current_message, Application::State state, std::size_t start_index, std::size_t count) {
    std::vector<ftxui::Component> buttons;
    for (size_t i = start_index; i < std::min(start_index + page_size,messages.size()); ++i) {
        // Be carefull what you pass as a reference -- state would be a dangling reference

        //change in case of many recipients!!
        Message& message=messages[i];
        std::string label = message.subject;//+"\nTO:"+message.recipients[0]+"\nFrom:"+message.sender;
        buttons.push_back(ftxui::Button(label, [&messages, &app, &current_message, state, i] {
            current_message = messages[i];
            app.change_state(state);
    }));
    }
    return buttons;
}


std::vector<ftxui::Component> show_menu(Application& app, std::vector<Folder>& folders,
    Folder& current_folder, Application::State state,Message& current_message,std::vector<Message>& email_vector,ftxui::Component& inbox,int& page) {
    std::vector<ftxui::Component> buttons;
    for (size_t i = 0; i < folders.size(); ++i) {
        // Be carefull what you pass as a reference -- state would be a dangling reference
        buttons.push_back(ftxui::Button(folders[i].name, [&folders, &app, &current_folder, &current_message,&email_vector,&inbox,&page, state, i] {
            page=0;
            current_folder = folders[i];
            email_vector = current_folder.messages;
            inbox->DetachAllChildren();
            std::vector<ftxui::Component> email_buttons = show_folder(app,folders[i].messages,current_message,Application::State::EMAIL_VIEW,0,page_size);
            for(auto b:email_buttons){
                inbox->Add(b);
            }
            app.change_state(state);
    }));
    }
    return buttons;
}

Application_frontend::Application_frontend(Application& app) :
    app(app),
    
    page(0),
    log_in(log_in::get_log_in_data(app)),
    current_email_draft(),
    current_email(),
    current_folder(),
    screen(ftxui::ScreenInteractive::Fullscreen())
{
    email_draft_layout = ftxui::Container::Vertical({
        ftxui::SelectableInput(
            &current_email_draft.recipient,
            mail_input_style("To:")
        ) | ftxui::flex_shrink,
        ftxui::SelectableInput(
            &current_email_draft.subject,
            mail_input_style("Subject:")
        ) | ftxui::flex_shrink,
        SelectableInput(
            &current_email_draft.message,
            mail_input_style("Email")
        ) | ftxui::flex_shrink,
        SelectableText(
            "\nGet Email client for Arch!",
            mail_input_style("")
        )
    });
    email_layout = ftxui::Container::Vertical({
        ftxui::SelectableText(
            &current_email.sender,
            mail_input_style("From:")
        ),
        ftxui::SelectableText(
            //change in case of many recipients!!!
            //Do recipients even work??
            //&current_email.recipients[0],
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

    inbox = 
        ftxui::Container::Vertical({
            show_folder(app, email_vector, current_email, Application::State::EMAIL_VIEW, 0, page_size)});
    
    next_prev_buttons = 
        ftxui::Container::Horizontal({
            ftxui::Button("←", [&]{
                page--;
                inbox->DetachAllChildren();
                std::vector<ftxui::Component> buttons
                    = show_folder(app, email_vector, current_email, Application::State::EMAIL_VIEW, page_size*page, page_size);
                for(auto b : buttons){
                    inbox->Add(b);
                }
            }) | ftxui::Maybe([&]{return page>0 && app.is_in_state(Application::State::MENU);}),
            ftxui::Button("→", [&]{
                page++;
                inbox->DetachAllChildren();
                std::vector<ftxui::Component> buttons 
                    = show_folder(app, email_vector, current_email, Application::State::EMAIL_VIEW, page_size*page, page_size);
                for(auto b : buttons){
                    inbox->Add(b);
                }
                
            }) | ftxui::Maybe([&]{return page_size*(page+1)<email_vector.size() && app.is_in_state(Application::State::MENU);}),
            
        });

    
    auto inner_inbox =
        ftxui::Container::Vertical({
            inbox, next_prev_buttons
        }) | ftxui::vscroll_indicator | ftxui::frame |
                   ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, 20);

    inbox_wrapper = ftxui::Container::Vertical({
        inner_inbox
    }) | ftxui::border
       | ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, 21);

    folder_menu = 
        ftxui::Container::Vertical({
            show_menu(app, folder_vector, current_folder, Application::State::MENU,current_email,email_vector,inbox,page)});

    new_mail_button = 
        ftxui::Button("New mail", [&]{
            app.change_state(Application::State::EMAIL_DRAFT);
        });

    menu_component =
        ftxui::Container::Vertical({
            new_mail_button,
            folder_menu
        });
    
    back_button = 
        ftxui::Button("Back", [&]{
            app.change_state(Application::State::MENU);
        });

    email_control = 
        ftxui::Container::Horizontal({
            ftxui::Button("Send Email", [&]{
                app.send_email(current_email_draft);
                current_email_draft = Email_draft();
            }),
            ftxui::Button("Reset", [&]{
                current_email_draft = Email_draft();
            }) 
        });

    email_draft_wrapper = 
        ftxui::Container::Vertical({
            back_button,
            email_draft_layout,
            email_control
        });

    email_layout_wrapper = 
        ftxui::Container::Vertical({
            back_button,
            email_layout,
            //respond??
        });

    main_component = ftxui::CatchEvent(ftxui::Container::Horizontal({
        email_draft_wrapper | ftxui::Maybe([&]{return app.is_in_state(Application::State::EMAIL_DRAFT);}),
        inbox_wrapper       | ftxui::Maybe([&]{return app.is_in_state(Application::State::MENU);}),
        email_layout_wrapper | ftxui::Maybe([&]{return app.is_in_state(Application::State::EMAIL_VIEW);}),
        log_in.visuals | ftxui::Maybe([&]{return app.is_in_state(Application::State::LOG_IN);}),
    }), [&](ftxui::Event event){
        // If downloading emails finished, update the view
        if (event.input() == "refresh_emails") {
            refresh_emails();
        }

        return false;
    });
    
    control_panel = ftxui::Container::Vertical({
        ftxui::Container::Horizontal({
            ftxui::Button("Send Email", [&]{
                app.send_email(current_email_draft);
                current_email_draft = Email_draft();
            }) 
            | ftxui::Maybe([&]{return app.is_in_state(Application::State::EMAIL_DRAFT);}),
            ftxui::Button("Reset", [&]{
                current_email_draft = Email_draft();
            }) | ftxui::Maybe([&]{return app.is_in_state(Application::State::EMAIL_DRAFT);})
        }),
        ftxui::Container::Horizontal({
            ftxui::Button("New mail", [&]{
                app.change_state(Application::State::EMAIL_DRAFT);
            }),
            ftxui::Button("Inbox", [&]{
                app.change_state(Application::State::MENU);
            }),
            ftxui::Button("Sent items", [&]{
                app.change_state(Application::State::MENU);
            })
        }),
        // All of this should be unified -- violates DRY
        // draj sraj
        
        
    })| ftxui::Maybe([&]{return !app.is_in_state(Application::State::LOG_IN);});
    
    layout = ftxui::Container::Horizontal({menu_component| ftxui::Maybe([&]{return !app.is_in_state(Application::State::LOG_IN);}) | ftxui::flex_shrink,
         main_component | ftxui::flex_shrink});
}

void Application_frontend::loop(){
    screen.Loop(layout);
}

void Application_frontend::synchronize(){
    std::thread synchronize_mailbox([this]{
        app.synchronize();
        screen.PostEvent(ftxui::Event::Special("refresh_emails"));
    });

    synchronize_mailbox.detach();
}

/*void Application_frontend::regenerate_folder(const std::string &folder_name)
{
    // All of these cases should be generalized; WIP
    ftxui::Component& items = folder_name == "inbox" ? inbox : sent_items;
    std::vector<Message>& messages = email_vector;
    Message& current_message = current_email;
    int& page = inbox_page;
    Application::State state = Application::State::MENU;
    
    items->DetachAllChildren();
    std::vector<ftxui::Component> buttons
        = show_folder(app, messages, current_message, state, 4*inbox_page, 4);
    for(auto b : buttons){
        items->Add(b);
    }
}*/

void Application_frontend::regenerate_menu()
{
    folder_menu->DetachAllChildren();
    std::vector<ftxui::Component> buttons
        = show_menu(app, folder_vector, current_folder, Application::State::MENU,current_email,email_vector,inbox,page);
    for(auto b : buttons){
        folder_menu->Add(b);
    }
}

void Application_frontend::refresh_emails()
{
    folder_vector = app.fetch_emails();
    /*for (const auto& folder : folder_vector) {
        email_vector = folder.messages;

    }*/
    //regenerate_folder("inbox");
    regenerate_menu();
    //regenerate_folder("sent_items");
}
