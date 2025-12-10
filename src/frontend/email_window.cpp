#include "email_window.hpp"
#include "app_frontend.hpp"
#include "app_theme.hpp"
#include "commands.hpp"

EmailWindow::EmailWindow(const TRect &bounds) :
    TWindow(bounds, "Compose Email", wnNoNumber),
    TWindowInit(&EmailWindow::initFrame) {
    
    int margin = 2;
    int rightEdge = size.x - 2;
    int labelW = 9;
    int inputX = margin + labelW + 1;
    
    int y = 1;

    insert(new TLabel(TRect(margin, y, margin + labelW, y + 1), "~T~o:", this));
    toField = new TInputLine(TRect(inputX, y, rightEdge, y + 1), 128);
    insert(toField);

    y += 2;

    insert(new TLabel(TRect(margin, y, margin + labelW, y + 1), "~F~rom:", this));
    fromField = new TInputLine(TRect(inputX, y, rightEdge, y + 1), 128);
    insert(fromField);

    y += 2;

    insert(new TLabel(TRect(margin, y, margin + labelW, y + 1), "~S~ubject:", this));
    subjectField = new TInputLine(TRect(inputX, y, rightEdge, y + 1), 256);
    insert(subjectField);

    y += 2;

    int bottomMargin = 3;
    TRect editorRect(margin, y, rightEdge - 1, size.y - bottomMargin);
    
    TRect vScrollRect = editorRect;
    vScrollRect.a.x = editorRect.b.x;
    vScrollRect.b.x = editorRect.b.x + 1;

    TRect hScrollRect = editorRect;
    hScrollRect.a.y = editorRect.b.y;
    hScrollRect.b.y = editorRect.b.y + 1;
    
    hScrollRect.b.x = vScrollRect.b.x; 

    vScroll = new TScrollBar(vScrollRect);
    hScroll = new TScrollBar(hScrollRect);

    insert(vScroll);
    insert(hScroll);

    bodyEditor = new TEditor(editorRect, hScroll, vScroll, nullptr, 0xFFFF);
    insert(bodyEditor);

    int btnY = size.y - 2;
    int btnW = 10;
    int gap = 2;
    
    int totalW = (btnW * 2) + gap;
    int startX = (size.x - totalW) / 2;

    insert(new TButton(TRect(startX, btnY, startX + btnW, btnY + 2), 
                       "~S~end", cmSend, bfDefault));

    startX += btnW + gap;

    insert(new TButton(TRect(startX, btnY, startX + btnW, btnY + 2), 
                       "~C~ancel", cmCancel, bfNormal));
}

void EmailWindow::handleEvent(TEvent& event){
    TWindow::handleEvent(event);
    if (event.what == evCommand) {
        if(event.message.command == cmSend){
            // tutaj musimy jeszcze dodać wpisywanie do listy maili (przy podpinaniu backendu)
            close(); 
            clearEvent(event);
            return;
        }
        else if(event.message.command == cmCancel){
            close(); 
            clearEvent(event);
            return;
        }
    }
}

TColorAttr EmailWindow::mapColor(uchar index) noexcept {
    TColorAttr defaultColor = TWindow::mapColor(index);
    return AppTheme::getColor(index, defaultColor);
}
