#include "login_dialog.hpp"

LoginDialog::LoginDialog(TRect r) :
    TDialog(r, "Login"),
    TWindowInit(&TDialog::initFrame) {
    options |= ofCentered;

    TScrollBar* listScroll = new TScrollBar(TRect(20, 8, 21, 15));
    list = new TListBox(TRect(3, 8, 20, 15),1, listScroll);
    TStringCollection *accountStrings = new TStringCollection(50, 50); 
    for (const auto& account : get_accounts()) {
        logging::log(account.first);
        char* copy = new char[account.first.size() + 1];
        std::strcpy(copy, account.first.c_str());
        accountStrings->insert(copy);
    }
    list->newList(accountStrings);


    insert(new TLabel(TRect(3, 5, 20, 6), "Choose account:", this));
    insert(list);
    insert(listScroll);

    insert(new TLabel(TRect(3, 17, 10, 18), "Pass:", this));
    pass = new TPasswordInputLine(TRect(12, 17, 32, 18), 30);
    insert(pass);

    insert(new TButton(TRect(8, 20, 18, 22), "Login", cmOK, bfDefault));
    insert(new TButton(TRect(22, 20, 32, 22), "Cancel", cmCancel, 0));
    insert(new TButton(TRect(10, 24, 30, 26), "New account", cmNewAccount, 0));
}

std::string LoginDialog::user() const {
    if(list->focused == -1) return "";

    int idx=list->focused;
    char* user = new char[50];
    list->getText(user, idx, 50);
    return std::string(user);
}

std::string LoginDialog::password() const { return pass->data; }

void LoginDialog::handleEvent(TEvent& event){
    TDialog::handleEvent(event);

    if (event.what == evCommand && event.message.command == cmNewAccount){
        endModal(cmNewAccount);
    }
}

TColorAttr LoginDialog::mapColor(uchar index) noexcept {
    using RGB = TColorRGB;

    TColorAttr c = TDialog::mapColor(index);
    switch (index) {
    case 1:  // Frame passive
        return { RGB(180, 200, 255), RGB(30, 50, 100) };
    case 2:  // Frame active
        return { RGB(255, 255, 255), RGB(40, 70, 130) };
    case 6:  // StaticText
        return { RGB(240, 240, 255), RGB(20, 40, 80) };
    case 7:  // Label normal
        return { RGB(230,230,255), RGB(20,40,80) };
    case 8:  // Label selected
        return { RGB(255,255,255), RGB(60,90,150) };
    case 9:  // Label shortcut
        return { RGB(255,220,140), RGB(20,40,80) };
    case 10: // Button normal
        return { RGB(250,250,255), RGB(50,70,120) };
    case 11: // Button default
        return { RGB(255,255,255), RGB(70,100,160) };
    case 12: // Button selected
        return { RGB(255,255,255), RGB(90,120,180) };
    case 13: // Button disabled
        return { RGB(150,150,150), RGB(50,50,50) };
    case 14: // Button shortcut
        return { RGB(255,220,120), RGB(50,70,120) };
    case 15: // Button shadow
        return { RGB(0,0,0), RGB(0,0,0) };
    case 16: // Cluster normal (checkbox/radio)
        return { RGB(230,230,255), RGB(20,40,80) };
    case 17: // Cluster selected
        return { RGB(255,255,255), RGB(60,90,150) };
    case 18: // Cluster shortcut
        return { RGB(255,220,140), RGB(20,40,80) };
    case 19: // InputLine normal text
        return { RGB(255,255,255), RGB(30,50,110) };
    case 20: // InputLine selected text
        return { RGB(0,0,0), RGB(200,220,255) };
    case 21: // InputLine arrows
        return { RGB(255,255,255), RGB(40,70,130) };
    case 22: // History arrow
        return { RGB(255,255,255), RGB(40,70,130) };
    case 23: // History sides
        return { RGB(255,255,255), RGB(30,50,100) };
    case 26: // ListViewer normal
        return { RGB(240,240,255), RGB(20,40,80) };
    case 27: // ListViewer focused
        return { RGB(255,255,255), RGB(60,90,150) };
    case 28: // ListViewer selected
        return { RGB(255,255,255), RGB(90,120,180) };
    case 30: // InfoPanel
        return { RGB(200,220,255), RGB(30,50,90) };
    default:
        return c;
    }
}

std::vector<std::pair<std::string, std::string>> LoginDialog::get_accounts(){
    std::ifstream configFile(Application::get_config_path());
    if (!configFile){ 
        logging::log("Failed to open config.json");
    }

    nlohmann::json config;
    configFile >> config;

    std::vector<std::pair<std::string, std::string>> emails_passwords;
    for(auto& account : config){
        emails_passwords.push_back({std::string(account["sender_email"]), std::string(account["app_password"])});
    }

    return emails_passwords;
}
