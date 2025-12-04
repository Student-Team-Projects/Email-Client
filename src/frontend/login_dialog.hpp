#define Uses_TDialog
#define Uses_TInputLine
#define Uses_TLabel
#define Uses_TButton
#define Uses_TString
#include <tvision/tv.h>
#include <string>
#include "TPasswordInputLine.hpp"

class LoginDialog : public TDialog {
    TInputLine *user;
    TPasswordInputLine *pass;

    virtual TColorAttr mapColor(uchar) noexcept override;

public:
    LoginDialog(TRect r);
    std::string username() const;
    std::string password() const;
};
