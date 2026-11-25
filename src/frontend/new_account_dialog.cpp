#include "new_account_dialog.hpp"
#include "app_theme.hpp"
#include "account_helper.hpp"
#include <filesystem>

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
    if (event.what == evCommand && event.message.command == cmLogin){
        endModal(cmLogin);
        clearEvent(event);
        return;
    }

    TDialog::handleEvent(event);
}

// "Create" ends the dialog only after the form is valid and the account is saved
Boolean NewAccountDialog::valid(ushort command){
    if(!TDialog::valid(command)){
        return false;
    }
    if(command != cmOK){
        return true;
    }

    Account new_account = account();
    if(!validate(new_account) || !add_account(new_account)){
        return false;
    }

    try {
        app.set_current_email_address(new_account.username);
    } catch (const std::exception& e) {
        logging::log(std::string("Login failed: ") + e.what());
        messageBox(std::string("Could not log in:\n") + e.what(), mfError | mfOKButton);
        return false;
    }
    return true;
}

bool NewAccountDialog::validate(const Account& account){
    auto fail = [](const char* msg) {
        messageBox(msg, mfError | mfOKButton);
        return false;
    };

    if(account.username.empty() || account.password.empty()
       || account.smtpHost.empty() || account.imapHost.empty()){
        return fail("All fields are required.");
    }
    if(account.username.find('@') == std::string::npos){
        return fail("User must be an email address, e.g. alice@example.com.");
    }
    if(account.smtpHost.find("://") == std::string::npos){
        return fail("SMTP host must be a URL, e.g. smtps://smtp.gmail.com:465.");
    }
    if(account.imapHost.find("://") == std::string::npos){
        return fail("IMAP host must be a URL, e.g. imaps://imap.gmail.com:993.");
    }
    return true;
}

bool NewAccountDialog::add_account(const Account& new_account){
    const std::string path = Application::get_config_path();

    nlohmann::json config = nlohmann::json::array();
    try {
        std::ifstream configFile(path);
        if (configFile){
            configFile >> config;
        }
    } catch (const std::exception& e) {
        logging::log(std::string("Failed to read config.json: ") + e.what());
        messageBox("config.json is damaged, the account was not saved.", mfError | mfOKButton);
        return false;
    }

    for (const auto& existing : config) {
        if (existing.value("username", "") == new_account.username) {
            messageBox("This account already exists.", mfError | mfOKButton);
            return false;
        }
    }

    config.push_back(accountToJson(new_account));

    std::ofstream new_config_file(path, std::ios::trunc);
    new_config_file << config;
    new_config_file.close();
    if (!new_config_file) {
        logging::log("Failed to write config.json");
        messageBox("Could not save config.json.", mfError | mfOKButton);
        return false;
    }

    // The file holds passwords: readable only by the user (was 0664)
    std::error_code ec;
    std::filesystem::permissions(path,
        std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
        std::filesystem::perm_options::replace, ec);
    if (ec) {
        logging::log("Failed to set config.json permissions: " + ec.message());
    }
    return true;
}

TColorAttr NewAccountDialog::mapColor(uchar index) noexcept {
    TColorAttr defaultColor = TDialog::mapColor(index);
    return AppTheme::getColor(index, defaultColor);
}
