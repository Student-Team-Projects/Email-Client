#define Uses_TInputLine
#define Uses_TRect
#include <tvision/tv.h>

class TPasswordInputLine : public TInputLine {
public:
    TPasswordInputLine(TRect bounds, int aMaxLen) 
        : TInputLine(bounds, aMaxLen) {}

    virtual void draw();
};