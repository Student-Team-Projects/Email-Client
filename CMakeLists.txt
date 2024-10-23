cmake_minimum_required(VERSION 3.10)  # Minimum version of CMake
project(MyVMimeProject)               # Project name

# Enable C++11 standard (or higher if needed)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find pkg-config package
find_package(PkgConfig REQUIRED)

# Use pkg-config to find VMime
pkg_check_modules(VMIME REQUIRED vmime)

# Add the directories for VMime headers
include_directories(${VMIME_INCLUDE_DIRS})

# Link VMime libraries
link_directories(${VMIME_LIBRARY_DIRS})

# Add the source files for your project (adjust to your source file names)
add_executable(my_executable EmailClient.cpp)  # Replace 'main.cpp' with your actual source files

# Link your executable with the VMime library (and any other required libraries)
target_link_libraries(my_executable ${VMIME_LIBRARIES})
