#include "ftxui/component/component.hpp"
#include "backend/mail_types.h"
class Application;
/*
* Class containing information needed to show new email draft layout.
*/
class Email_draft_layout{
public:
    Email_draft_layout(Application& app);
    ftxui::Component get_email_draft_component();
private:
    Application& app;
    MessageToSend current_email_draft;
};
