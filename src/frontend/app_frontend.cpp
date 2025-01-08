#include "frontend/app_frontend.hpp"
#include "frontend/select_input.cpp"
#include "app.hpp"

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
    for (size_t i = start_index; i < std::min(start_index + 4,messages.size()); ++i) {
        // Be carefull what you pass as a reference -- state would be a dangling reference
        buttons.push_back(ftxui::Button(messages[i].subject, [&messages, &app, &current_message, state, i] {
            current_message = messages[i];
            app.Change_state(state);
    }));
    }
    return buttons;
}

Application_frontend::Application_frontend(Application& app) :
    app(app),
    
    inbox_page(0),
    sent_page(0),
    current_email_draft(),
    current_received_email(),
    current_sent_email(),
    received_email_vector(app.fetch_received_emails()),
    sent_email_vector(app.fetch_sent_emails()),
    
    email_draft_layout( ftxui::Container::Vertical({
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
    })),
    received_email_layout( ftxui::Container::Vertical({
        //ftxui::SelectableText(
            //&current_received_email.sender,
            //mail_input_style("From:")
        //),
        ftxui::SelectableText(
            &current_received_email.subject,
            mail_input_style("Subject:")
        ),
        ftxui::SelectableText(
            &current_received_email.body,
            mail_input_style("Email")
        )
    })),
    sent_email_layout( ftxui::Container::Vertical({
        //ftxui::SelectableText(
            //&current_send_email.sender,
            //mail_input_style("From:")
        //),
        ftxui::SelectableText(
            &current_sent_email.subject,
            mail_input_style("Subject:")
        ),
        ftxui::SelectableText(
            &current_sent_email.body,
            mail_input_style("Email")
        )
    })),
    
    inbox(
        ftxui::Container::Vertical({
            show_folder(app, received_email_vector, current_received_email, Application::State::RECEIVED_EMAIL, 0, 4)})
    ),
    
    sent_items( 
        ftxui::Container::Vertical({
            show_folder(app, sent_email_vector, current_sent_email, Application::State::SENT_EMAIL, 0, 4)
            })
    ),
    
    main_component(ftxui::CatchEvent(ftxui::Container::Vertical({
        email_draft_layout | ftxui::Maybe([&]{return app.Is_in_state(Application::State::EMAIL_DRAFT);}),
        sent_items  | ftxui::Maybe([&]{return app.Is_in_state(Application::State::SENT_ITEMS);}),
        inbox       | ftxui::Maybe([&]{return app.Is_in_state(Application::State::INBOX);}),
        received_email_layout | ftxui::Maybe([&]{ return app.Is_in_state(Application::State::RECEIVED_EMAIL);}),
        sent_email_layout | ftxui::Maybe([&]{return app.Is_in_state(Application::State::SENT_EMAIL);})
    }), [&](ftxui::Event event){return Copy_selected_text(event);})),
    
    control_panel(ftxui::Container::Vertical({
        ftxui::Container::Horizontal({
            ftxui::Button("Send Email", [&]{
                app.Send_email(current_email_draft);
                current_email_draft = Email_draft();
            }) 
            | ftxui::Maybe([&]{return app.Is_in_state(Application::State::EMAIL_DRAFT);}),
            ftxui::Button("Reset", [&]{
                current_email_draft = Email_draft();
            }) | ftxui::Maybe([&]{return app.Is_in_state(Application::State::EMAIL_DRAFT);})
        }),
        ftxui::Container::Horizontal({
            ftxui::Button("New mail", [&]{
                app.Change_state(Application::State::EMAIL_DRAFT);
            }),
            ftxui::Button("Inbox", [&]{
                app.Change_state(Application::State::INBOX);
            }),
            ftxui::Button("Sent items", [&]{
                app.Change_state(Application::State::SENT_ITEMS);
            })
        }),
        // All of this should be unified -- violates DRY
        ftxui::Container::Horizontal({
            ftxui::Button("Next", [&]{
                inbox_page++;
                inbox->DetachAllChildren();
                std::vector<ftxui::Component> buttons 
                    = show_folder(app, received_email_vector, current_received_email, Application::State::RECEIVED_EMAIL, 4*inbox_page, 4);
                for(auto b : buttons){
                    inbox->Add(b);
                }
                
            }) | ftxui::Maybe([&]{return app.Is_in_state(Application::State::INBOX);}),
            ftxui::Button("Previous", [&]{
                inbox_page--;
                inbox->DetachAllChildren();
                std::vector<ftxui::Component> buttons
                    = show_folder(app, received_email_vector, current_received_email, Application::State::RECEIVED_EMAIL, 4*inbox_page, 4);
                for(auto b : buttons){
                    inbox->Add(b);
                }
            }) | ftxui::Maybe([&]{return inbox_page>0 && app.Is_in_state(Application::State::INBOX);}),
        }),
        ftxui::Container::Horizontal({
            ftxui::Button("Next", [&]{
                sent_page++;
                sent_items->DetachAllChildren();
                std::vector<ftxui::Component> buttons 
                    = show_folder(app, sent_email_vector, current_sent_email, Application::State::SENT_EMAIL, 4*sent_page, 4);
                for(auto b : buttons){
                    sent_items->Add(b);
                }
                
            }) | ftxui::Maybe([&]{return app.Is_in_state(Application::State::SENT_ITEMS);}),
            ftxui::Button("Previous", [&]{
                sent_page--;
                sent_items->DetachAllChildren();
                std::vector<ftxui::Component> buttons 
                    = show_folder(app, sent_email_vector, current_sent_email, Application::State::SENT_EMAIL, 4*sent_page, 4);
                for(auto b : buttons){
                    sent_items->Add(b);
                }
            }) | ftxui::Maybe([&]{return sent_page>0 && app.Is_in_state(Application::State::SENT_ITEMS);}),
        })
    })),
    
    layout(ftxui::Container::Vertical({main_component | ftxui::flex_shrink, control_panel | ftxui::flex_grow})),

    screen(ftxui::ScreenInteractive::Fullscreen())
{}

void Application_frontend::Loop(){
    screen.Loop(layout);
}



std::vector<Message> Application_frontend::fetch_send_emails(){
    std::vector<Message> emails(5,{{},"send subject","send body"});
    return {emails};
}

bool Application_frontend::Copy_selected_text(ftxui::Event event) {
    // if (event == ftxui::Event::Special("\x19")) { //Ctrl+Y
    //     std::string command = "echo '" + current_email_draft.message + "' | xclip -selection clipboard";
    //     std::system(command.c_str());
    //     return true;
    // }
    return false;
}