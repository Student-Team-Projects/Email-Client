#pragma once
#include <iostream>

namespace logging{
    /*
    * Use it for logging/debugging purposes. When CMake flag is set, it will use std::cerr to print the message.
    */
    void log(const std::string& message);
}
