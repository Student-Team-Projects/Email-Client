cmake_minimum_required(VERSION 3.10)
project(MyVMimeProject)

# Enable C++11 standard (or higher if needed)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find pkg-config package
find_package(PkgConfig REQUIRED)

# Use pkg-config to find VMime
pkg_check_modules(VMIME REQUIRED vmime)

# Add the directories for VMime headers
include_directories(${VMIME_INCLUDE_DIRS})

# Link VMime libraries
link_directories(${VMIME_LIBRARY_DIRS})

# Add the nlohmann-json library
find_package(nlohmann_json REQUIRED)

# Add the source files for your project (adjust to your source file names)
add_executable(my_executable RetrieveEmail.cpp)  # Replace 'main.cpp' with your actual source files

# Link your executable with the VMime library and nlohmann JSON
target_link_libraries(my_executable ${VMIME_LIBRARIES} nlohmann_json::nlohmann_json)
