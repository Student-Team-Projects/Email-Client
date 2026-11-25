#include "new_account_dialog.hpp"
#include "app_theme.hpp"
#include "account_helper.hpp"

namespace {
    // One length limit for every field; hosts and app passwords can be long
    constexpr int fieldMaxLen = 256;
}

NewAccountDialog::NewAccountDialog(TRect r, Application& app) :
    TWindowInit(&TDialog::initFrame),
    TDialog(r, "Create new account"),
    app(app) {
    options |= ofCentered;

    int labelW = 11;   // fits the longest label, "SMTP Host:"
    int inputW = 40;
    int gap = 1;
    int formTotalWidth = labelW + gap + inputW;
    int formStartX = (size.x - formTotalWidth) / 2;
    if (formStartX < 1) formStartX = 1;
    int inputX = formStartX + labelW + gap;

    int wCreate = 12;
    int wLogin = 17;
    int btnGap = 2;
    int rowBtnWidth = wCreate + btnGap + wLogin;
    int rowBtnX = (size.x - rowBtnWidth) / 2;

    int groupHeight = 10;  // 4 fields every 2 rows + buttons

    int availableHeight = size.y - 3;
    int y = (availableHeight - groupHeight) / 2;

    if (y < 1) y = 1;

    auto addField = [&](const char* label, TInputLine* input) {
        insert(new TLabel(TRect(formStartX, y, formStartX + labelW, y + 1), label, input));
        insert(input);
        y += 2;
    };

    user = new TInputLine(TRect(inputX, y, inputX + inputW, y + 1), fieldMaxLen);
    addField("~U~ser:", user);

    pass = new TPasswordInputLine(TRect(inputX, y, inputX + inputW, y + 1), fieldMaxLen);
    addField("~P~ass:", pass);

    smtpHost = new TInputLine(TRect(inputX, y, inputX + inputW, y + 1), fieldMaxLen);
    strnzcpy(smtpHost->data, "smtps://smtp.gmail.com:465", fieldMaxLen);
    addField("~S~MTP Host:", smtpHost);

    imapHost = new TInputLine(TRect(inputX, y, inputX + inputW, y + 1), fieldMaxLen);
    strnzcpy(imapHost->data, "imaps://imap.gmail.com:993", fieldMaxLen);
    addField("~I~MAP Host:", imapHost);

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

    // Start typing in the first field instead of having "Exit" focused
    user->select();
}


Account NewAccountDialog::account() const {
    Account account;
    account.username = user->data;
    account.password = pass->data;
    account.smtpHost = smtpHost->data;
    account.imapHost = imapHost->data;
    return account;
}

void NewAccountDialog::handleEvent(TEvent& event){
    if (event.what == evCommand){
        switch (event.message.command) {
        case cmOK:
            add_account(account());
            app.set_current_email_address(account().username);
            break;
        case cmLogin:
            endModal(cmLogin);
            break;
        }
    }

    TDialog::handleEvent(event);
}

void NewAccountDialog::add_account(const Account& new_account){
    std::ifstream configFile(Application::get_config_path());
    if (!configFile){
        logging::log("Failed to open config.json");
        return;
    }

    nlohmann::json config;
    configFile >> config;

    nlohmann::json new_account_json = accountToJson(new_account);
    config.push_back(new_account_json);

    configFile.close();
    std::ofstream new_config_file(Application::get_config_path(), std::ios::trunc);
    new_config_file << config;
}

TColorAttr NewAccountDialog::mapColor(uchar index) noexcept {
    TColorAttr defaultColor = TDialog::mapColor(index);
    return AppTheme::getColor(index, defaultColor);
}
