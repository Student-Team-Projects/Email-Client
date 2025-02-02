#include "logging.hpp"

void logging::log(const std::string& message) {
#ifdef LOG
    std::cerr << message << std::endl;
#endif
}