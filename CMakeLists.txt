cmake_minimum_required(VERSION 3.22)

project(email_client
  LANGUAGES CXX
  VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


option(ENABLE_LOG "Enable logging" OFF)

if(ENABLE_LOG)
    add_definitions(-DLOG)
endif()

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

# nlohmann_json
find_package(nlohmann_json REQUIRED)

# libxml2
find_package(LibXml2 REQUIRED)

# Define the email_client executable
add_executable(email_client 
  src/main.cpp
  src/app.cpp
)

# Include headers and subdirectories
target_include_directories(email_client PUBLIC src)
add_subdirectory(src/backend)
add_subdirectory(src/frontend)
add_subdirectory(src/logging)

target_link_libraries(email_client 
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component 
  ${VMIME_LIBRARIES}
  nlohmann_json::nlohmann_json
  ${LIBXML2_LIBRARIES}
  PRIVATE sqlite3
)
        

# Include libxml2 headers
target_include_directories(email_client PRIVATE ${LIBXML2_INCLUDE_DIR})