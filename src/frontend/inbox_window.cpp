#include "inbox_window.hpp"
#include "email_window.hpp"
#include "app_theme.hpp"
#include "commands.hpp"
#include <sstream>

class UnsortedStringCollection : public TStringCollection {
public:
    UnsortedStringCollection(short aLimit, short aDelta) :
        TStringCollection(aLimit, aDelta) {}

    virtual ccIndex insert(void *item) override {
        atInsert(count, item);
        return count - 1;
    }
};

InboxWindow::InboxWindow(const TRect& bounds, Application& app) :
    TWindow(bounds, "Inbox", wnNoNumber),
    TWindowInit(&InboxWindow::initFrame),
    currentContentView(nullptr),
    app(app)
{
    fetchMailData();

    int listW = size.x * 0.35;
    int bottomMargin = 3;
    int margin = 1;

    TRect listArea(margin, margin, listW, size.y - bottomMargin);

    TRect scrollRect = listArea;
    scrollRect.a.x = listArea.b.x - 1;

    TRect listRect = listArea;
    listRect.b.x -= 1;

    contentRect = TRect(listW + 1, margin, size.x - margin, size.y - bottomMargin);

    TScrollBar* scrollBar = new TScrollBar(scrollRect);
    insert(scrollBar);

    list = new TListBox(listRect, 1, scrollBar);

    UnsortedStringCollection* items = new UnsortedStringCollection(currentFolder->messages.size(), 10);
    for (const auto& mail : currentFolder->messages) {
        std::string label = "[ " + mail.sender + " ] " + mail.subject;
        items->insert(newStr(label.c_str()));
    }
    list->newList(items);
    insert(list);

    int btnX = margin + 1;
    int btnY = size.y - 2;
    int btnW = 15;

    insert(new TButton(TRect(btnX, btnY, btnX + btnW, btnY + 2),
                       "New Email", cmNewMail, bfNormal));

    if (!currentFolder->messages.empty()) {
        list->focusItem(0);
        list->selectItem(0);
        updateContent(0);
    }
}

InboxWindow::~InboxWindow() {
}

void InboxWindow::fetchMailData() {
    static Folder DUMMY = {"Dummy", {}};
    mailData = app.fetch_email_headers();
    if (mailData.size() > 0) {
        currentFolder = &mailData[0];
    } else {
        currentFolder = &DUMMY;
    }
}

void InboxWindow::updateContent(int index) {

    if (index < 0 || index >= currentFolder->messages.size()) return;

    if (currentContentView != nullptr) {
        remove(currentContentView);
        destroy(currentContentView); // !!! zwalniam pamięć
        currentContentView = nullptr;
    }

    const auto& mail = currentFolder->messages[index];
    const auto& body = app.get_email_body(mail.uid, mail.folder);

    std::stringstream ss;
    ss << "From: " << mail.sender << "\n"
       << "Subject: " << mail.subject << "\n"
       << "----------------------\n\n"
       << body;


    currentContentView = new TStaticText(contentRect, ss.str().c_str());
    insert(currentContentView);

    // wymuszam, żeby od nowa załadował mi widok
    currentContentView->drawView();
}

void InboxWindow::openComposeWindow() {
    // wymiary pulpitu
    TRect r = TProgram::deskTop->getExtent();
    EmailWindow* composeWin = new EmailWindow(r, app);
    if (owner) {
        owner->insert(composeWin);
    }
}

void InboxWindow::handleEvent(TEvent& event) {
    int oldFocus = list->focused;

    if (event.what == evCommand && event.message.command == cmNewMail) {
        openComposeWindow();
        clearEvent(event);
        return;
    }

    TWindow::handleEvent(event);

    if (list->focused != oldFocus) {
        updateContent(list->focused);
    }

    if (event.what == evBroadcast) {
        if (event.message.command == cmCommandSetChanged) {
            if(list->focused != oldFocus){
                updateContent(list->focused);
            }
        }
    }

    TWindow::handleEvent(event);
}

TColorAttr InboxWindow::mapColor(uchar index) noexcept {
    TColorAttr defaultColor = TWindow::mapColor(index);
    return AppTheme::getColor(index, defaultColor);
}
