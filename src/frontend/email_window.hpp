#define Uses_TWindow
#define Uses_TInputLine
#define Uses_TEditor
#define Uses_TLabel
#define Uses_TScrollBar
#include <tvision/tv.h>


class EmailWindow : public TWindow {
    TInputLine *toField;
    TInputLine *fromField;
    TInputLine *subjectField;
    TEditor *bodyEditor;
    TScrollBar *vScroll, *hScroll;

    virtual TColorAttr mapColor(uchar index) noexcept override;

public:
    EmailWindow(const TRect &bounds);
};
