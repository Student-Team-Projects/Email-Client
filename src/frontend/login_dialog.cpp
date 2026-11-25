#include "login_dialog.hpp"
#include "app_theme.hpp"
#include "account_helper.hpp"

LoginDialog::LoginDialog(TRect r, Application& app) :
    TWindowInit(&TDialog::initFrame),
    TDialog(r, "Login"),
    app(app) {
    options |= ofCentered;

    int marginX = 2;
    int topY = 2;
    int bottomY = size.y - 4;

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

    // Start with the account list focused. Otherwise the last inserted view
    // ("Exit") has the focus and pressing Enter closes the application.
    list->select();
}

std::string LoginDialog::username() const {
    if(list == nullptr || list->range == 0 || list->focused < 0 || list->focused >= list->range){
        return "";
    }
    // TListBox::getText() writes the terminating '\0' at dest[maxLen],
    // so pass one less than the buffer size
    char user[256];
    list->getText(user, list->focused, sizeof(user) - 1);
    return std::string(user);
}

void LoginDialog::handleEvent(TEvent& event){
    if (event.what == evCommand){
        if(event.message.command == cmNewAccount) {
            endModal(cmNewAccount);
        }
    }

    TDialog::handleEvent(event);

    // Double click or Space on an account works like "Choose"
    // (Enter already does, through the default button).
    if (event.what == evBroadcast && event.message.command == cmListItemSelected
        && event.message.infoPtr == list) {
        clearEvent(event);
        endModal(cmOK);
    }
}

Boolean LoginDialog::valid(ushort command){
    if(!TDialog::valid(command)){
        return false;
    }

    if(command == cmOK){
        std::string selected_user = username();

        if(selected_user.empty()){
            messageBox("No account selected. Create one with \"New account\".",
                       mfInformation | mfOKButton);
            return false;
        }

        try {
            app.set_current_email_address(selected_user);
        } catch (const std::exception& e) {
            logging::log(std::string("Login failed: ") + e.what());
            messageBox(std::string("Could not log in:\n") + e.what(), mfError | mfOKButton);
            return false;
        }
    }

    return true;
}

std::vector<Account> LoginDialog::get_accounts(){
    std::vector<Account> accounts;

    std::ifstream configFile(Application::get_config_path());
    if (!configFile){
        logging::log("Failed to open config.json");
        return accounts;
    }

    // A broken config.json used to crash the whole application here
    try {
        nlohmann::json config;
        configFile >> config;
        for(auto& account : config) {
            accounts.push_back(accountFromJson(account));
        }
    } catch (const std::exception& e) {
        logging::log(std::string("Failed to read config.json: ") + e.what());
        accounts.clear();
    }

    return accounts;
}

TColorAttr LoginDialog::mapColor(uchar index) noexcept {
    TColorAttr defaultColor = TDialog::mapColor(index);
    return AppTheme::getColor(index, defaultColor);
}
