add_executable(sso_tests
    tests.cpp
)

target_link_libraries(sso_tests
    PRIVATE
    string-sso
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(sso_tests)
