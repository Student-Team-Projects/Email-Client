# Add source files from backend to the email_client target
target_sources(email_client
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/mailbox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/html_parser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/mail_storage.cpp
)
