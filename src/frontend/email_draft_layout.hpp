#include "ftxui/component/component.hpp"
#include "email_types.hpp"
class Application;

class Email_draft_layout{
public:
    Email_draft_layout(Application& app);
    ftxui::Component get_email_draft_component();
private:
    Application& app;
    Email_draft current_email_draft;
};
