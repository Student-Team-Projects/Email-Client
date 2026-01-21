cmake_minimum_required(VERSION 3.22)

project(email_client
  LANGUAGES CXX
  VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BUILD_SHARED_LIBS ON)

# == PACKAGE VERSIONS ==
set(PKGVER_JSON v3.12.0)
set(PKGVER_LIBXML2 v2.15.0)
set(PKGVER_SQLITE3 v3.50.2)
set(PKGVER_VMIME 7046a4360bbeea21d1d8b5cfa4589bb4df7f980d)
set(PKGVER_TVISION 85aaeca0999c8b53006c556ccda063f143d259ca)

option(ENABLE_LOG "Enable logging" OFF)

option(EXTERNAL_VMIME "If OFF, then build VMime and link to it for development" OFF)
option(EXTERNAL_SQLITE "If OFF, then build SQLite3 and link to it for development" OFF)
option(EXTERNAL_LIBXML2 "If OFF, then build LibXml2 and link to it for development" OFF)

if(ENABLE_LOG)
  add_definitions(-DLOG)
endif()

set(_deferred_link_libraries ";")
function(deferred_link_libraries)
  set(_deferred_link_libraries "${_deferred_link_libraries};${ARGN}" PARENT_SCOPE)
endfunction(deferred_link_libraries)

set(_deferred_link_directories ";")
function(deferred_link_directories)
  set(_deferred_link_directories "${_deferred_link_directories};${ARGN}" PARENT_SCOPE)
endfunction(deferred_link_directories)

set(_deferred_include_directories ";")
function(deferred_include_directories)
  set(_deferred_include_directories "${_deferred_include_directories};${ARGN}" PARENT_SCOPE)
endfunction(deferred_include_directories)

# == FETCH LIBRARIES =================================================
include(FetchContent)
include(ExternalProject)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)

# TVISION
FetchContent_Declare(tvision
  GIT_REPOSITORY https://github.com/magiblot/tvision.git
  GIT_TAG        ${PKGVER_TVISION}
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(tvision)
deferred_link_libraries(
  PRIVATE tvision
)

# JSON
FetchContent_Declare(nlohmann_json
  URL https://github.com/nlohmann/json/releases/download/${PKGVER_JSON}/json.tar.xz
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(nlohmann_json)
deferred_link_libraries(
  nlohmann_json::nlohmann_json
)

# libxml2
FetchContent_Declare(libxml2
  URL https://github.com/GNOME/libxml2/archive/refs/tags/${PKGVER_LIBXML2}.tar.gz
  EXCLUDE_FROM_ALL
)
if (NOT EXTERNAL_LIBXML2)
  FetchContent_MakeAvailable(libxml2)
else()
  find_package(libxml2 REQUIRED)
endif()
deferred_link_libraries(
  LibXml2::LibXml2
)

# sqlite3
FetchContent_Declare(sqlite3
  GIT_REPOSITORY https://github.com/sjinks/sqlite3-cmake.git
  GIT_TAG ${PKGVER_SQLITE3}
  GIT_PROGRESS TRUE
  GIT_SHALLOW TRUE
  EXCLUDE_FROM_ALL
)
if (NOT EXTERNAL_SQLITE)
  FetchContent_MakeAvailable(sqlite3)
else()
  find_package(SQLite3 REQUIRED)
endif()
deferred_link_libraries(
  PRIVATE SQLite::SQLite3
)

# NOTE: VMime is a library long-overdue for an update to it's build infrastructure
# FetchContent misbehaved, littered our source directory and failed to build.
# TLDR: It really wants to be installed globally, but that's annoying for us so
#   we don't let it. Instead we use the ExternalProject cannon to make it work.

set(PKGVER_VMIME master)
if(NOT EXTERNAL_VMIME)
  set(VMIME_INSTALL_DIR ${CMAKE_BINARY_DIR}/_deps/external-vmime)
  ExternalProject_Add(vmime
  GIT_REPOSITORY https://github.com/kisli/vmime.git
  GIT_TAG ${PKGVER_VMIME}
  GIT_PROGRESS TRUE
  GIT_SHALLOW TRUE
  PREFIX ${VMIME_INSTALL_DIR}
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${VMIME_INSTALL_DIR}
    -DCMAKE_BUILD_TYPE=Release
    -DVMIME_HAVE_MESSAGING_PROTO_SENDMAIL=OFF
# ^ above requires the sendmail util. we don't need it for now, so let's minimize deps while we can.
    -DVMIME_HAVE_SASL_SUPPORT=OFF
# ^ above requires the gsasl library; same as above, disabled until required.
    -DVMIME_BUILD_SAMPLES=OFF
# ^ above requires gtk, this is giant bloat we don't need.
    -DVMIME_BUILD_DOCUMENTATION=OFF
# ^ this also fails due to some doxygen issues, but it's not a big deal.
    -DVMIME_BUILD_STATIC_LIBRARY=OFF
    -DVMIME_BUILD_SHARED_LIBRARY=ON
# ^ NOTE: As much as I'd like to link statically, I gave up after fighting w/ the archaic VMime CMake for ~6 hours, 
#       we just live with the fact that vmime must be linked dynamically
  )
  deferred_include_directories(${VMIME_INSTALL_DIR}/include)
  deferred_link_directories(${VMIME_INSTALL_DIR}/lib)
  deferred_link_libraries(
    -lvmime
  )
else()
  find_package(vmime REQUIRED)
  deferred_include_directories(${vmime_INCLUDE_DIRS})
  deferred_link_libraries(
    -lvmime ${VMIME_LIBRARIES}
  )
endif()


# == END FETCH LIBRARIES ===============================================

# Define the email_client executable
add_executable(email_client
  src/main.cpp
  src/app.cpp
)
message("libs ${_deferred_link_libraries}")
message("libdirs ${_deferred_link_directories}")
message("incdirs ${_deferred_include_directories}")

# Include headers and subdirectories
target_include_directories(email_client PUBLIC src)
target_include_directories(email_client PRIVATE ${_deferred_include_directories})

add_subdirectory(src/backend)
add_subdirectory(src/frontend)
add_subdirectory(src/logging)

target_link_libraries(email_client PRIVATE ${_deferred_link_libraries})
target_link_directories(email_client PRIVATE ${_deferred_link_directories})
