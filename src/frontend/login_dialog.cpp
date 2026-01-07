#include "login_dialog.hpp"
#include "app_theme.hpp"
#include "account_helper.hpp"

LoginDialog::LoginDialog(TRect r, Application& app) :
    TDialog(r, "Login"),
    TWindowInit(&TDialog::initFrame),
    app(app) {
    options |= ofCentered;

    int marginX = 2;
    int topY = 2;
    int bottomY = size.y - 4;
    int listWidth = size.x - (2 * marginX);

    insert(new TLabel(TRect(marginX, 1, size.x, 2), "Choose account:", this));

    TRect scrollRect(size.x - marginX - 1, topY, size.x - marginX, bottomY);
    TScrollBar* listScroll = new TScrollBar(scrollRect);
    insert(listScroll);

    TRect listRect(marginX, topY, size.x - marginX - 1, bottomY);
    list = new TListBox(listRect, 1, listScroll);

    TStringCollection *accountStrings = new TStringCollection(10, 10);
    for (const auto& account : get_accounts()) {
        char* copy = newStr(account.username.c_str()); // newStr to bezpieczna funkcja TVision
        accountStrings->insert(copy);
    }
    list->newList(accountStrings);
    insert(list);

    int btnY = size.y - 3;
    int btnGap = 2;

    int wChoose = 12;
    int wCancel = 12;
    int wNew    = 16;

    int totalBtnWidth = wChoose + wCancel + wNew + (2 * btnGap);

    int startX = (size.x - totalBtnWidth) / 2;

    insert(new TButton(TRect(startX, btnY, startX + wChoose, btnY + 2),
                       "~C~hoose", cmOK, bfDefault));
    startX += wChoose + btnGap;

    insert(new TButton(TRect(startX, btnY, startX + wNew, btnY + 2),
                       "~N~ew account", cmNewAccount, bfNormal));
    startX += wNew + btnGap;

    insert(new TButton(TRect(startX, btnY, startX + wCancel, btnY + 2),
                       "Exit", cmCancel, bfNormal));
}

std::string LoginDialog::username() const {
    if(list->focused == -1) return "";

    int idx=list->focused;
    char* user = new char[50];
    list->getText(user, idx, 50);
    return std::string(user);
}

void LoginDialog::handleEvent(TEvent& event){
    if (event.what == evCommand){
        switch (event.message.command) {
        case cmNewAccount:
            endModal(cmNewAccount);
            break;
        case cmOK:
            app.set_current_email_address(username());
            break;
        }
    }

    TDialog::handleEvent(event);
}

std::vector<Account> LoginDialog::get_accounts(){
    std::ifstream configFile(Application::get_config_path());
    if (!configFile){
        logging::log("Failed to open config.json");
    }

    nlohmann::json config;
    configFile >> config;

    std::vector<Account> emails_passwords;
    for(auto& account : config) {
        emails_passwords.push_back(accountFromJson(account));
    }

    return emails_passwords;
}

TColorAttr LoginDialog::mapColor(uchar index) noexcept {
    TColorAttr defaultColor = TDialog::mapColor(index);
    return AppTheme::getColor(index, defaultColor);
}
