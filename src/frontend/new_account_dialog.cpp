#include "new_account_dialog.hpp"
#include "app_theme.hpp"

NewAccountDialog::NewAccountDialog(TRect r, Application& app) :
    TDialog(r, "Create new account"),
    TWindowInit(&TDialog::initFrame),
    app(app) {
    options |= ofCentered;

    int labelW = 6;
    int inputW = 26;
    int gap = 1;
    int formTotalWidth = labelW + gap + inputW;
    int formStartX = (size.x - formTotalWidth) / 2;

    int wCreate = 12;
    int wLogin = 17;
    int btnGap = 2;
    int rowBtnWidth = wCreate + btnGap + wLogin;
    int rowBtnX = (size.x - rowBtnWidth) / 2;

    int groupHeight = 6;

    int availableHeight = size.y - 3; 
    int y = (availableHeight - groupHeight) / 2;
    
    if (y < 1) y = 1;

    insert(new TLabel(TRect(formStartX, y, formStartX + labelW, y + 1), "User:", this));
    user = new TInputLine(TRect(formStartX + labelW + gap, y, formStartX + labelW + gap + inputW, y + 1), 30);
    insert(user);

    y += 2;

    insert(new TLabel(TRect(formStartX, y, formStartX + labelW, y + 1), "Pass:", this));
    pass = new TPasswordInputLine(TRect(formStartX + labelW + gap, y, formStartX + labelW + gap + inputW, y + 1), 30);
    insert(pass);

    y += 2;

    insert(new TButton(TRect(rowBtnX, y, rowBtnX + wCreate, y + 2), 
                       "~C~reate", cmOK, bfDefault));
                       
    rowBtnX += wCreate + btnGap;
    
    insert(new TButton(TRect(rowBtnX, y, rowBtnX + wLogin, y + 2), 
                       "Cancel", cmLogin, bfNormal));

    int wExit = 10;
    int exitX = (size.x - wExit) / 2;
    int bottomY = size.y - 3; // 3 linie od dołu

    insert(new TButton(TRect(exitX, bottomY, exitX + wExit, bottomY + 2), 
                       "Exit", cmCancel, bfNormal));
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

TColorAttr NewAccountDialog::mapColor(uchar index) noexcept {
    TColorAttr defaultColor = TDialog::mapColor(index);
    return AppTheme::getColor(index, defaultColor);
}