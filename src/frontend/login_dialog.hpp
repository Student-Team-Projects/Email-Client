#define Uses_TDialog
#define Uses_TInputLine
#define Uses_TLabel
#define Uses_TButton
#define Uses_TEvent
#define Uses_TListBox
#define Uses_TScrollBar
#define Uses_TStringCollection
#define Uses_TProgram
#define Uses_TDeskTop
#include <tvision/tv.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <fstream>
#include "TPasswordInputLine.hpp"
#include "commands.hpp"
#include "logging/logging.hpp"
#include "app.hpp"

class LoginDialog : public TDialog {
    Application& app;

    TListBox *list;

    std::vector<std::pair<std::string, std::string>> get_accounts();

    virtual TColorAttr mapColor(uchar) noexcept override;

    virtual void handleEvent(TEvent& event) override;

public:
    LoginDialog(TRect r, Application& app);;
    std::string username() const;
};
