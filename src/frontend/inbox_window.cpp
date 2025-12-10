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

InboxWindow::InboxWindow(const TRect& bounds) :
    TWindow(bounds, "Inbox", wnNoNumber),
    TWindowInit(&InboxWindow::initFrame),
    currentContentView(nullptr)
{
    initMockData();

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
    
    UnsortedStringCollection* items = new UnsortedStringCollection(emails.size(), 10);
    for (const auto& mail : emails) {
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

    if (!emails.empty()) {
        list->focusItem(0);
        list->selectItem(0);
        updateContent(0);
    }
}

InboxWindow::~InboxWindow() {
}

void InboxWindow::initMockData() {
    emails = {
        {"GitHub", "[GitHub] Security alert", "We noticed a new sign-in to your account from a new device.\nLocation: Warsaw, Poland.\nIf this was you, you can ignore this email."},        
        {"AWS Billing", "Invoice available", "Your invoice for the billing period November 1-30 is now available.\nTotal amount: $24.50 USD.\nPayment will be processed automatically."},        
        {"Jira Cloud", "[JIRA] Assigned: (DEV-402)", "Ticket DEV-402 'Memory leak in TListBox destructor' has been assigned to you.\nPriority: High\nStatus: In Progress."},        
        {"Michal (Team Lead)", "Code Review - Login Module", "Hej,\nrzuciłem okiem na Twój PR z logowaniem.\nWygląda dobrze, ale dodałem kilka komentarzy odnośnie obsługi błędów sieciowych.\nZerknij proszę przed mergem."},        
        {"Stack Overflow", "New answer to your question", "Someone posted a new answer to: 'How to handle TEvent::cmCommand in Turbo Vision?'.\nClick here to view the solution."},        
        {"HR Department", "Holiday Schedule Update", "Dear Team,\nPlease note that the office will be closed on December 24th and 31st.\nEnsure all critical services are monitored during the break."},        
        {"LinkedIn", "New connection request", "Anna K. (Tech Recruiter) wants to connect with you.\n'I was impressed by your C++ portfolio...'"},        
        {"Server Monitor", "ALERT: Disk Space Low", "Warning: Server 'production-db-01' is running low on disk space.\nVolume /var is at 92% capacity.\nImmediate action required."},        
        {"Katarzyna W.", "Spotkanie projektowe", "Cześć, czy możemy przesunąć dzisiejsze daily na 10:30?\nMam konflikt w kalendarzu.\nDajcie znać czy pasuje."},        
        {"Spotify", "Your subscription receipt", "Thanks for sticking with Premium.\nWe've charged 19.99 PLN to your card ending in 4242.\nEnjoy the music!"},        
        {"Marcin (Dev)", "Re: Dokumentacja API", "Dzięki za update dokumentacji.\nJeden szczegół - endpoint /login zwraca teraz JSON zamiast XML, prawda?\nTrzeba to poprawić w sekcji 3.2."},
        {"Coursera", "Course Completion", "Congratulations! You have successfully completed 'Advanced Data Structures in C++'.\nYour certificate is ready to download."}
    };
}

void InboxWindow::updateContent(int index) {

    if (index < 0 || index >= emails.size()) return;

    if (currentContentView != nullptr) {
        remove(currentContentView);
        destroy(currentContentView); // !!! zwalniam pamięć
        currentContentView = nullptr;
    }

    const auto& mail = emails[index];

    std::stringstream ss;
    ss << "From: " << mail.sender << "\n"
       << "Subject: " << mail.subject << "\n"
       << "----------------------\n\n"
       << mail.body;


    currentContentView = new TStaticText(contentRect, ss.str().c_str());
    insert(currentContentView);
    
    // wymuszam, żeby od nowa załadował mi widok
    currentContentView->drawView();
}

void InboxWindow::openComposeWindow() {
    // wymiary pulpitu
    TRect r = TProgram::deskTop->getExtent();
    EmailWindow* composeWin = new EmailWindow(r);
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