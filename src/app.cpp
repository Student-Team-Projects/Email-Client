#include "app.hpp"
#include "frontend/app_frontend.hpp"
#include <memory>

void Application::Run(std::unique_ptr<Application_frontend> front){
    frontend = std::move(front);
    frontend->Loop();
}

bool Application::Is_in_state(State state){
    return state == current_state;
}

void Application::Change_state(State new_state){
    current_state = new_state;
}

Application::Application() :
    current_state(State::INBOX)
<<<<<<< HEAD
{}
=======
{}
>>>>>>> fc83017f94d724256951f3d02c3404b252eff387
