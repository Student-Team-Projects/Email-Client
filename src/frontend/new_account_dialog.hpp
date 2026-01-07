#define Uses_TDialog
#define Uses_TInputLine
#define Uses_TLabel
#define Uses_TButton
#define Uses_TEvent
#include <tvision/tv.h>
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include "TPasswordInputLine.hpp"
#include "commands.hpp"
#include "app.hpp"
#include "logging/logging.hpp"

class NewAccountDialog : public TDialog {
    Application& app;

    TInputLine *user;
    TPasswordInputLine *pass;
    TInputLine *smtpHost;
    TInputLine *imapHost;

    void add_account(const Account& new_account);

    virtual TColorAttr mapColor(uchar) noexcept override;

    virtual void handleEvent(TEvent& event) override;

public:
    NewAccountDialog(TRect r, Application& app);
    Account account() const;
};
