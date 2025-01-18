#include <vector>
#include "backend/mail_types.h"
#include "app.hpp"
#include "ftxui/component/component.hpp"

class Folder_menu{
public:
    Folder_menu(Application&, Message&, std::vector<Folder>&, Folder&);
    void regenerate_menu();
    ftxui::Component get_menu_layout();
    ftxui::Component get_inbox_layout();
private:
    Application& app;
    std::vector<Folder>& folder_vector;
    Folder& current_folder;
    Message& current_email;
    std::vector<Message> email_vector;
    
    int page{0};
    ftxui::Component menu;
    ftxui::Component inbox;
    ftxui::Component inbox_layout;

    std::vector<ftxui::Component> show_menu(std::vector<Folder>& folders, Folder& current_folder, Application::State state,Message& current_message,std::vector<Message>& email_vector,ftxui::Component& inbox,int& page);
    std::vector<ftxui::Component> show_folder(std::vector<Message>& messages, Message& current_message, Application::State state, std::size_t start_index, std::size_t count);
};