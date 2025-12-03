#include "app.hpp"
#include "frontend/app_frontend.hpp"

int main(){
    Application app;
    auto frontend = std::make_unique<Application_frontend>(app);
    app.run(std::move(frontend));
    return 0;
}
