#include "app_frontend.hpp"
#include "email_window.hpp"
#include "login_dialog.hpp"

Application_frontend::Application_frontend(Application& app) :
    TProgInit(&Application_frontend::initStatusLine, &Application_frontend::initMenuBar, &Application_frontend::initDeskTop),
    app(app),
    loginSucceeded(false) {
    TRect r = deskTop->getExtent();
    r = r.grow(0,1);
    LoginDialog *dlg = new LoginDialog(r);
    ushort res = execView(dlg);
    destroy(dlg);

    if (res == cmOK) {
        loginSucceeded = true;
        deskTop->insert(new EmailWindow(deskTop->getExtent()));
    } else loginSucceeded = false;
}


TStatusLine* Application_frontend::initStatusLine( TRect r )
{
    r.a.y = r.b.y-1;
    return new TStatusLine( r,
        *new TStatusDef( 0, 0xFFFF ) +
            *new TStatusItem( "~Alt-X~ Exit", kbAltX, cmQuit ) +
            *new TStatusItem( 0, kbF10, cmMenu )
            );
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
