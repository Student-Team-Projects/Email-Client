#include "app_frontend.hpp"
#include "email_window.hpp"
#include "login_dialog.hpp"
#include "new_account_dialog.hpp"

Application_frontend::Application_frontend(Application& app) :
    TProgInit(&Application_frontend::initStatusLine, &Application_frontend::initMenuBar, &Application_frontend::initDeskTop),
    app(app), loginSucceeded(false){
    deskTop->insert(new EmailWindow(deskTop->getExtent()));
}


TStatusLine* Application_frontend::initStatusLine(TRect r){
    r.a.y = r.b.y-1;
    return new TStatusLine(r,
        *new TStatusDef(0, 0xFFFF) +
            *new TStatusItem("~Alt-X~ Exit", kbAltX, cmQuit) +
            *new TStatusItem(0, kbF10, cmMenu)
            );
}

void Application_frontend::run(){
    TRect r = deskTop->getExtent();
    r = r.grow(0,1);

    ushort status=cmLogin;
    do{
        TDialog *dlg;
        if(status == cmLogin){
            dlg = new LoginDialog(r);
        } else if(status == cmNewAccount){
            dlg = new NewAccountDialog(r);
        }
        status = execView(dlg);
        destroy(dlg);
    } while(status!=cmOK && status!=cmCancel);

    if (status == cmOK) {
        loginSucceeded = true;
        TApplication::run();
    } else loginSucceeded = false;
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
void Application_frontend::synchronize(){
    synch_cv.notify_one();
}

void Application_frontend::refresh_emails(){}
