cmake_minimum_required(VERSION 3.22)

project(Email-Client
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

add_executable(Email-Client 
  src/main.cpp
  src/app.cpp
)
target_compile_definitions(Email-Client PRIVATE src)
target_include_directories(Email-Client PRIVATE src)

target_link_libraries(Email-Client
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component

  PRIVATE vmime
  
  PRIVATE nlohmann_json::nlohmann_json
)