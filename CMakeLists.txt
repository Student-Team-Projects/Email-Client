cmake_minimum_required(VERSION 3.10)
project(EmailClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)

# VMIME
pkg_check_modules(VMIME REQUIRED vmime)
include_directories(${VMIME_INCLUDE_DIRS})
link_directories(${VMIME_LIBRARY_DIRS})

# nlohmann_json
find_package(nlohmann_json REQUIRED)

# libxml2
find_package(LibXml2 REQUIRED)

# Define the email_client executable
add_executable(email_client src/main.cpp)

# Include headers and subdirectories
target_include_directories(email_client PUBLIC src)
add_subdirectory(src/backend)
add_subdirectory(src/frontend)

# Link libraries to email_client
target_link_libraries(email_client
    PRIVATE
        ${VMIME_LIBRARIES}
        ${LIBXML2_LIBRARIES}
        nlohmann_json::nlohmann_json
)

# Include libxml2 headers
target_include_directories(email_client PRIVATE ${LIBXML2_INCLUDE_DIR})
