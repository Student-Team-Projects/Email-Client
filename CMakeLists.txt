cmake_minimum_required(VERSION 3.10)
project(EmailClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)

pkg_check_modules(VMIME REQUIRED vmime)

include_directories(${VMIME_INCLUDE_DIRS})

link_directories(${VMIME_LIBRARY_DIRS})

find_package(nlohmann_json REQUIRED)

add_executable(email_client src/main.cpp)

target_link_libraries(email_client ${VMIME_LIBRARIES} nlohmann_json::nlohmann_json)
