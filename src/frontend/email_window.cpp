#include "email_window.hpp"
#include "app_frontend.hpp"

EmailWindow::EmailWindow(const TRect &bounds) :
    TWindow(bounds, "Compose Email", wnNoNumber),
    TWindowInit(&EmailWindow::initFrame) {
    int left = 2;
    int right = size.x - 2;
    int y = 1;

    insert(new TLabel(TRect(left, y, left+4, y+1), "~T~o:", 0));
    toField = new TInputLine(TRect(left+5, y, right, y+1), 128);
    insert(toField);

    y += 2;

    insert(new TLabel(TRect(left, y, left+7, y+1), "~F~rom:", 0));
    fromField = new TInputLine(TRect(left+8, y, right, y+1), 128);
    insert(fromField);

    y += 2;

    insert(new TLabel(TRect(left, y, left+9, y+1), "~S~ubject:", 0));
    subjectField = new TInputLine(TRect(left+10, y, right, y+1), 256);
    insert(subjectField);

    y += 2;

    TRect bodyRect(left, y, right-1, size.y-3);

    vScroll = new TScrollBar(TRect(right-1, y, right, size.y-3));
    hScroll = new TScrollBar(TRect(left, size.y-3, right-1, size.y-2));

    insert(vScroll);
    insert(hScroll);

    bodyEditor = new TEditor(bodyRect, hScroll, vScroll, nullptr, 0xFFFF);
    insert(bodyEditor);
}

TColorAttr EmailWindow::mapColor(uchar index) noexcept {
    TColorAttr color = TWindow::mapColor(index);

    if (index == 2) { 
        color = TColorAttr(0x00AAAA, 0x00AAAA); 
    }

    return color;
}
