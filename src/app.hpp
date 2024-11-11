#include <string>
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/component/component.hpp"

class application{
public:
    void run();
    application();
private:
    enum class state{
        EMAIL_DRAFT,
        INBOX,
        SENT_ITEMS
    };
    void change_state(state);

    state current_state;
    std::string current_draft;
    ftxui::Component email_draft;
    ftxui::Component inbox;
    ftxui::Component sent_items;
    ftxui::Component main_component;
    ftxui::Component control_panel;
    ftxui::Component layout;
    ftxui::ScreenInteractive screen;
};