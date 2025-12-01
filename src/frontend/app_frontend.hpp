#pragma once
#include "app.hpp"
#include <condition_variable>

#define Uses_TApplication
#define Uses_TStatusDef
#define Uses_TStatusItem
#define Uses_TKeys
#define Uses_TStatusLine
#define Uses_TDeskTop
#include <tvision/tv.h>

class Application;
/*
* Main frontend class contains layouts, and classes wrapping the layouts and synchronization logic.
*/
class Application_frontend : public TApplication {
public:
    Application_frontend(Application& app);
    void set_up_synchronization();
    void synchronize();
    void refresh_emails();
private:
    void set_email_body_dim(bool value);
    static TStatusLine* initStatusLine(TRect T);
    static TDeskTop* initDeskTop(TRect T);

    Application& app;

    std::condition_variable synch_cv;
    std::mutex synch_m;
    static inline constexpr std::size_t synch_time_in_seconds = 30;
};
