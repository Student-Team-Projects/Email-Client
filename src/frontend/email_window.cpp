#include "email_window.hpp"
#include "app.hpp"
#include "app_frontend.hpp"
#include "app_theme.hpp"
#include "backend/mail_types.h"
#include "commands.hpp"

EmailWindow::EmailWindow(const TRect &bounds, Application& app) :
    TWindowInit(&EmailWindow::initFrame),
    TWindow(bounds, "Compose Email", wnNoNumber),
    app(app) {

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
    strnzcpy(fromField->data, app.get_current_email_address(), 128);
    // Mail is always sent from the logged in account, so the field is read-only
    fromField->options &= ~ofSelectable;
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

    // Start in the "To" field, not on the last inserted button
    toField->select();
}

// TEditor keeps the text in a gap buffer, so `buffer` is not a plain string:
// after editing in the middle it holds the gap and may not end with '\0'.
std::string EmailWindow::bodyText() const {
    std::string text;
    text.reserve(bodyEditor->bufLen);
    for (uint i = 0; i < bodyEditor->bufLen; ++i) {
        text += bodyEditor->bufChar(i);
    }
    return text;
}

void EmailWindow::send() {
    MessageToSend outgoing;
    outgoing.recipient = toField->data;
    outgoing.subject = subjectField->data;
    outgoing.body = bodyText();

    if (outgoing.recipient.empty()) {
        messageBox("Enter the recipient (To).", mfError | mfOKButton);
        toField->select();
        return;
    }
    if (outgoing.subject.empty()) {
        messageBox("Enter the subject.", mfError | mfOKButton);
        subjectField->select();
        return;
    }
    if (outgoing.body.empty()) {
        messageBox("The message is empty.", mfError | mfOKButton);
        bodyEditor->select();
        return;
    }

    if (!app.send_email(outgoing)) {
        // Keep the window open so the user does not lose the text
        messageBox("Could not send the email. Check the recipient address and your connection.",
                   mfError | mfOKButton);
        return;
    }
    // TODO: also copy_to Sent mails (need backend fix)

    messageBox("Email sent.", mfInformation | mfOKButton);
    close();
}

void EmailWindow::handleEvent(TEvent& event){
    TWindow::handleEvent(event);
    if (event.what == evCommand) {
        if(event.message.command == cmSend){
            clearEvent(event);
            send();
            return;
        }
        else if(event.message.command == cmCancel){
            // close() deletes this window, so nothing may touch it afterwards
            clearEvent(event);
            close();
            return;
        }
    }
}

TColorAttr EmailWindow::mapColor(uchar index) noexcept {
    TColorAttr defaultColor = TWindow::mapColor(index);
    return AppTheme::getColor(index, defaultColor);
}
