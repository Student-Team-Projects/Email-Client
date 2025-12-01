#include "app_frontend.hpp"
#include "email_window.hpp"

Application_frontend::Application_frontend(Application& app) :
    TProgInit(&Application_frontend::initStatusLine, &Application_frontend::initMenuBar, &Application_frontend::initDeskTop),
    app(app)
{}


TStatusLine* Application_frontend::initStatusLine( TRect r )
{
    r.a.y = r.b.y-1;
    return new TStatusLine( r,
        *new TStatusDef( 0, 0xFFFF ) +
            *new TStatusItem( "~Alt-X~ Exit", kbAltX, cmQuit ) +
            *new TStatusItem( 0, kbF10, cmMenu )
            );
} 

class MyDeskTop : public TDeskTop {
public:
    MyDeskTop(TRect r) : TDeskTop(r), TDeskInit(&MyDeskTop::initBackground) {}

    virtual TColorAttr mapColor(uchar index) noexcept override {
        // Get original attributes first:
        TColorAttr color = TDeskTop::mapColor(index);

        if (index == 1) { 
            color = TColorAttr(0xFF7700, '\x3'); 
        }

        return color;
    }
};

TDeskTop* Application_frontend::initDeskTop(TRect r) {
    auto* win = new EmailWindow(r);
    auto* desktop = new MyDeskTop(r);
    desktop->insert(win);
    return desktop;
}

/** Start synchronizing the mailbox periodically.
 * Call this method once when user logs in to a mailbox, and then
 * the synchronization will be done with breaks of synch_time_in_seconds.
 */
void Application_frontend::set_up_synchronization(){
    std::thread synchronize_mailbox([this]{
        while(true) {
            app.synchronize();

            std::unique_lock<std::mutex> lock(synch_m);
            synch_cv.wait_for(lock, std::chrono::seconds(synch_time_in_seconds));
        }
    });

    synchronize_mailbox.detach();
}

/** Synchronize the mailbox once.
 * Call this method when user requests to refresh the mailbox.
 */
void Application_frontend::synchronize()
{
    synch_cv.notify_one();
}

void Application_frontend::refresh_emails()
{
}
