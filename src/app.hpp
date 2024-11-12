#pragma once
#include <memory>

class Application_frontend;

class Application {
public:
    enum class State {
        EMAIL_DRAFT,
        INBOX,
        SENT_ITEMS
    };
    void Run(std::unique_ptr<Application_frontend> front);
    bool Is_in_state(State state);
    void Change_state(State new_state);
    Application();
private:
    State current_state;
    std::unique_ptr<Application_frontend> frontend;
};