#include "email_draft_layout.hpp"
#include "app.hpp"
#include "select_input.hpp"

namespace{
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
}

Email_draft_layout::Email_draft_layout(Application& _app)
: app(_app)
{}

ftxui::Component Email_draft_layout::get_email_draft_component(){
    auto back_button = ftxui::Button("Back", [&]{
        app.change_state(Application::State::MENU);
    },ftxui::ButtonOption::Animated(ftxui::Color::DarkOliveGreen2));
    
    auto email_draft_layout = ftxui::Container::Vertical({
        ftxui::SelectableInput(
            &current_email_draft.recipient,
            mail_input_style("To:")
        ) | ftxui::flex_shrink,
        ftxui::SelectableInput(
            &current_email_draft.subject,
            mail_input_style("Subject:")
        ) | ftxui::flex_shrink,
        SelectableInput(
            &current_email_draft.body,
            mail_input_style("Email")
        ) | ftxui::flex_shrink,
        SelectableText(
            "\nGet Email client for Arch!",
            mail_input_style("")
        )
    });

    auto email_control = ftxui::Container::Horizontal({
        ftxui::Button("Send Email", [&]{
            app.send_email(current_email_draft);
            current_email_draft = MessageToSend();
        },ftxui::ButtonOption::Animated(ftxui::Color::DarkTurquoise)),
        ftxui::Button("Reset", [&]{
            current_email_draft = MessageToSend();
        },ftxui::ButtonOption::Animated(ftxui::Color::Cyan3) ) 
    });

    auto email_draft_wrapper = ftxui::Container::Vertical({
        back_button,
        email_draft_layout,
        email_control
    });

    return email_draft_wrapper;
}