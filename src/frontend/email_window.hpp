#pragma once
#include "app.hpp"
#define Uses_TWindow
#define Uses_TInputLine
#define Uses_TEditor
#define Uses_TLabel
#define Uses_TScrollBar
#define Uses_TButton
#define Uses_MsgBox
#include <tvision/tv.h>


class EmailWindow : public TWindow {
    Application& app;

    TInputLine *toField;
    TInputLine *fromField;
    TInputLine *subjectField;
    TEditor *bodyEditor;
    TScrollBar *vScroll, *hScroll;

    virtual TColorAttr mapColor(uchar index) noexcept override;
    std::string bodyText() const;
    void send();

public:
    EmailWindow(const TRect &bounds, Application& app);
    virtual void handleEvent(TEvent& event) override;
};
