set(FRONTEND_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/app_frontend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/email_window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/login_dialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/new_account_dialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TPasswordInputLine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inbox_window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/app_theme.cpp
)
target_sources(email_client_core PRIVATE ${FRONTEND_SOURCES})

# Warnings only for the frontend sources, so the rest of the project is not
# flooded. (add_compile_options() here had no effect: the target already exists.)
set_source_files_properties(${FRONTEND_SOURCES} TARGET_DIRECTORY email_client_core
    PROPERTIES COMPILE_OPTIONS "-Wall;-Wextra;-Wpedantic"
)
