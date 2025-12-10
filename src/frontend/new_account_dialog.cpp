#include "new_account_dialog.hpp"

NewAccountDialog::NewAccountDialog(TRect r, Application& app) :
    TDialog(r, "Create new account"),
    TWindowInit(&TDialog::initFrame),
    app(app) {
    options |= ofCentered;

    insert(new TLabel(TRect(3, 3, 10, 4), "User:", this));
    user = new TInputLine(TRect(12, 3, 32, 4), 30);
    insert(user);

    insert(new TLabel(TRect(3, 5, 10, 6), "Pass:", this));
    pass = new TPasswordInputLine(TRect(12, 5, 32, 6), 30);
    insert(pass);

    insert(new TButton(TRect(8, 8, 18, 10), "Create", cmOK, bfDefault));
    insert(new TButton(TRect(22, 8, 32, 10), "Cancel", cmCancel, 0));
    insert(new TButton(TRect(10, 12, 30, 14), "Login instead", cmLogin, 0));
}

std::string NewAccountDialog::username() const { return user->data; }
std::string NewAccountDialog::password() const { return pass->data; }

void NewAccountDialog::handleEvent(TEvent& event){
    if (event.what == evCommand){
        switch (event.message.command) {
        case cmOK:
            add_account({username(),password()});
            app.set_current_email_address(username());
            break;
        case cmLogin:
            endModal(cmLogin);
            break;
        }
    }

    TDialog::handleEvent(event);
}

TColorAttr NewAccountDialog::mapColor(uchar index) noexcept {
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

void NewAccountDialog::add_account(const std::pair<std::string, std::string>& new_account){
    std::ifstream configFile(Application::get_config_path());
    if (!configFile){
        logging::log("Failed to open config.json");
        return;
    }

    nlohmann::json config;
    configFile >> config;

    nlohmann::json new_account_json;
    new_account_json["sender_email"] = new_account.first;
    new_account_json["app_password"] = new_account.second;
    config.push_back(new_account_json);

    configFile.close();
    std::ofstream new_config_file(Application::get_config_path(), std::ios::trunc);
    new_config_file << config;
}
