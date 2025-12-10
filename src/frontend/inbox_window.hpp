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

struct MockEmail {
    std::string sender;
    std::string subject;
    std::string body;
};

class InboxWindow : public TWindow {
public:
    InboxWindow(const TRect& bounds);
    virtual ~InboxWindow();

    virtual void handleEvent(TEvent& event) override;

private:
    TListBox* list;
    TView* currentContentView;
    TRect contentRect; 
    
    std::vector<MockEmail> emails; // testowe dane 

    void initMockData();
    void updateContent(int index);
    void openComposeWindow();
};