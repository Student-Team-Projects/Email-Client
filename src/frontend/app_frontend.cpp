#include "frontend/app_frontend.hpp"
#include "frontend/select_input.cpp"
#include "app.hpp"
#include "frontend/log_in.hpp"
#include "iostream"

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

Application_frontend::Application_frontend(Application& app) :
    app(app),
    
    inbox_page(0),
    send_page(0),
    log_in(log_in::get_log_in_data(app)),
    current_email_draft(),
    current_received_email(),
    current_send_email(),
    received_email_vector(app.fetch_received_emails()),
    send_email_vector(app.fetch_sent_emails()),
    
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
    send_email_layout( ftxui::Container::Vertical({
        //ftxui::SelectableText(
            //&current_send_email.sender,
            //mail_input_style("From:")
        //),
        ftxui::SelectableText(
            &current_send_email.subject,
            mail_input_style("Subject:")
        ),
        ftxui::SelectableText(
            &current_send_email.body,
            mail_input_style("Email")
        )
    })),
    
    inbox(
        ftxui::Container::Vertical({
            [&] {
                std::vector<ftxui::Component> buttons;
                for (size_t i = 0; i < std::min(4,(int)received_email_vector.size()); ++i) {
                    buttons.push_back(ftxui::Button(received_email_vector[i].subject, [&, i] {
                    current_received_email = received_email_vector[i];
                    app.Change_state(Application::State::RECEIVED_EMAIL);
                }));
            }
            return buttons;
            }()
            
        })
    ),
    
    sent_items( 
        ftxui::Container::Vertical({
            [&] {
                std::vector<ftxui::Component> buttons;
                for (size_t i = 0; i < std::min(4,(int)send_email_vector.size()); ++i) {
                    buttons.push_back(ftxui::Button(send_email_vector[i].subject, [&, i] {
                    current_send_email = send_email_vector[i];
                    app.Change_state(Application::State::SEND_EMAIL);
                }));
            }
            return buttons;
            }()
        })
    ),
    
    main_component(ftxui::CatchEvent(ftxui::Container::Vertical({
        email_draft_layout | ftxui::Maybe([&]{return app.Is_in_state(Application::State::EMAIL_DRAFT);}),
        sent_items  | ftxui::Maybe([&]{return app.Is_in_state(Application::State::SENT_ITEMS);}),
        inbox       | ftxui::Maybe([&]{return app.Is_in_state(Application::State::INBOX);}),
        received_email_layout | ftxui::Maybe([&]{return app.Is_in_state(Application::State::RECEIVED_EMAIL);}),
        send_email_layout | ftxui::Maybe([&]{return app.Is_in_state(Application::State::SEND_EMAIL);}),
        log_in.visuals | ftxui::Maybe([&]{return app.Is_in_state(Application::State::LOG_IN);}),
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
                send_email_vector = fetch_send_emails();
                app.Change_state(Application::State::SENT_ITEMS);
            })
        }),
        ftxui::Container::Horizontal({
            ftxui::Button("Next", [&]{
                inbox_page++;
                inbox->DetachAllChildren();
                std::vector<ftxui::Component> buttons;
                for (size_t i = 4*inbox_page; i < std::min(4*(inbox_page+1),(int)received_email_vector.size()); ++i) {
                    buttons.push_back(ftxui::Button(received_email_vector[i].subject, [&, i] {
                    current_received_email = received_email_vector[i];
                    app.Change_state(Application::State::RECEIVED_EMAIL);
                    }));
                }
                for(auto b : buttons){
                    inbox->Add(b);
                }
                
            }) | ftxui::Maybe([&]{return app.Is_in_state(Application::State::INBOX);}),
            ftxui::Button("Previous", [&]{
                inbox_page--;
                inbox->DetachAllChildren();
                std::vector<ftxui::Component> buttons;
                for (size_t i = 4*inbox_page; i < std::min(4*(inbox_page+1),(int)received_email_vector.size()); ++i) {
                    buttons.push_back(ftxui::Button(received_email_vector[i].subject, [&, i] {
                    current_received_email = received_email_vector[i];
                    app.Change_state(Application::State::RECEIVED_EMAIL);
                    }));
                }
                for(auto b : buttons){
                    inbox->Add(b);
                }
            }) | ftxui::Maybe([&]{return inbox_page>0 && app.Is_in_state(Application::State::INBOX);}),
        }),
    })| ftxui::Maybe([&]{return !app.Is_in_state(Application::State::LOG_IN);})),
    
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