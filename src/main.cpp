#include "app.hpp"
#include "frontend/app_frontend.hpp"
#include <memory>

int main(){
    Application app;
    auto frontend = std::make_unique<Application_frontend>(app);
    app.Run(std::move(frontend));
}