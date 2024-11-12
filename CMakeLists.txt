cmake_minimum_required(VERSION 3.22)

project(email_client
  LANGUAGES CXX
  VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Fetch FTXUI ----------------------------------------------------
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/arthursonzogni/ftxui.git
  GIT_TAG        v5.0.0
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ftxui)
# ------------------------------------------------------------------------------

find_package(PkgConfig REQUIRED)
pkg_check_modules(VMIME REQUIRED vmime)
include_directories(${VMIME_INCLUDE_DIRS})
link_directories(${VMIME_LIBRARY_DIRS})

find_package(nlohmann_json REQUIRED)

add_executable(email_client 
  src/main.cpp
  src/app.cpp
  src/frontend/app_frontend.cpp
)

target_include_directories(email_client PUBLIC src)

add_subdirectory(src/backend)
add_subdirectory(src/frontend)

target_link_libraries(email_client 
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component 
  ${VMIME_LIBRARIES}
  nlohmann_json::nlohmann_json
)
