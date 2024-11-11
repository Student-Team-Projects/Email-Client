#include "app.hpp"

ftxui::InputOption mailStyle() {
    ftxui::InputOption option;
    option.content = "Type your email here";
    option.placeholder = "Type your email here";
    option.transform = [](ftxui::InputState state) {
        state.element |= ftxui::borderEmpty;
        state.element |= ftxui::bgcolor(ftxui::Color::Black);
        state.element |= ftxui::color(ftxui::Color::White);

        if (state.is_placeholder) {
            state.element |= ftxui::dim;
        }

        if (state.focused) {
            state.element |= ftxui::bgcolor(ftxui::Color::BlueViolet);
        }

        return state.element;
    };
    return option;
}

void application::run(){
    screen.Loop(layout);
}

void application::change_state(state new_state){
    current_state = new_state;
}

bool application::copy_selected_text(ftxui::Event event) {
    if (event == ftxui::Event::Special("\x19")) { //Ctrl+Y
        std::string command = "echo '" + current_draft + "' | xclip -selection clipboard";
        std::system(command.c_str());
        return true;
    }
    return false;
}

application::application() :
    current_state(state::INBOX),
    current_draft(""),
    email_draft(ftxui::Input(&current_draft, mailStyle())),
    inbox(
        ftxui::Container::Vertical({
            ftxui::Button("mail 1 from Kuba", []{}),
            ftxui::Button("mail 2 from Grzegorz", []{}),
            ftxui::Button("mail 3 from Maciej", []{}),
            ftxui::Button("mail 4 from Hubert", []{})
        })
    ),
    sent_items( 
        ftxui::Container::Vertical({
            ftxui::Button("mail 1 sent to Kuba", []{}),
            ftxui::Button("mail 2 sent to Grzegorz", []{}),
            ftxui::Button("mail 3 sent to Maciej", []{}),
            ftxui::Button("mail 4 sent to Hubert", []{})
        })
    ),

    main_component(ftxui::CatchEvent(ftxui::Container::Vertical({
        email_draft | ftxui::Maybe([&]{return current_state == state::EMAIL_DRAFT;}),
        sent_items  | ftxui::Maybe([&]{return current_state == state::SENT_ITEMS;}),
        inbox       | ftxui::Maybe([&]{return current_state == state::INBOX;})
    }),[&](ftxui::Event event){return copy_selected_text(event);})),
    
    control_panel(ftxui::Container::Vertical({
        ftxui::Container::Horizontal({
            ftxui::Button("Send Email", []{}) 
            | ftxui::Maybe([&]{return current_state == state::EMAIL_DRAFT;}),
            ftxui::Button("Reset", [&]{
                current_draft = "";
            }) | ftxui::Maybe([&]{return current_state == state::EMAIL_DRAFT;})
        }),
        ftxui::Container::Horizontal({
            ftxui::Button("New mail", [&]{
                change_state(state::EMAIL_DRAFT);
            }),
            ftxui::Button("Inbox", [&]{
                change_state(state::INBOX);
            }),
            ftxui::Button("Sent items", [&]{
                change_state(state::SENT_ITEMS);
            })
        })
    })),
    layout(ftxui::Container::Vertical({main_component, control_panel})),
    screen(ftxui::ScreenInteractive::Fullscreen())
{
}