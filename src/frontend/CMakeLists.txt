target_sources(email_client 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app_frontend.cpp
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/email_window.cpp
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/login_dialog.cpp
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/new_account_dialog.cpp
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/TPasswordInputLine.cpp
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inbox_window.cpp
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app_theme.cpp
)
add_compile_options(-Wall -Wextra -Wpedantic)
