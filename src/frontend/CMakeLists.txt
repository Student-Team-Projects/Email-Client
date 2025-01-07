target_sources(email_client 
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app_frontend.cpp
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/log_in.cpp
)
add_compile_options(-Wall -Wextra -Wpedantic)