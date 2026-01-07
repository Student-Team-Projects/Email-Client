#include "app.hpp"
#include "backend/mail_types.h"
#define Uses_TWindow
#define Uses_TInputLine
#define Uses_TEditor
#define Uses_TLabel
#define Uses_TScrollBar
#define Uses_TListBox
#define Uses_TEvent
#define Uses_TStringCollection
#define Uses_TButton
#define Uses_TProgram
#define Uses_TDeskTop
#include <tvision/tv.h>
#include <vector>

class InboxWindow : public TWindow {
public:
    InboxWindow(const TRect& bounds, Application& app);
    virtual ~InboxWindow();

    virtual void handleEvent(TEvent& event) override;

private:
    Application& app;
    TListBox* list;
    TView* currentContentView;
    TRect contentRect;

    std::vector<Folder> mailData;
    const Folder* currentFolder;

    void fetchMailData();
    void updateContent(int index);
    void setFolder(int index);
    void openComposeWindow();

    virtual TColorAttr mapColor(uchar) noexcept override;
};
