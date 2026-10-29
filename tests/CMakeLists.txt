set(PKGVER_GOOGLETEST v1.17.0)

# GoogleTest is only linked into the test binary, so build it statically
# (the top-level CMakeLists.txt turns BUILD_SHARED_LIBS on)
set(BUILD_SHARED_LIBS OFF)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/tags/${PKGVER_GOOGLETEST}.tar.gz
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# One test binary for the whole project. Test files mirror src/,
# e.g. src/backend/html_parser.cpp is tested in tests/backend/html_parser_test.cpp
add_executable(email_client_tests
  backend/html_parser_test.cpp
)
target_link_libraries(email_client_tests PRIVATE
  email_client_core
  GTest::gmock_main
)
target_compile_options(email_client_tests PRIVATE -Wall -Wextra -Wpedantic)

gtest_discover_tests(email_client_tests)
