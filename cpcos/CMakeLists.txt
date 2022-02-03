cmake_minimum_required(VERSION 3.14)
project(emdev
    VERSION 0.0.1
    LANGUAGES C CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_CXX_STANDARD 14)

enable_testing()

add_executable(
    hello_test
    tests/hello_test.cpp)
target_link_libraries(
    hello_test
    ${CONAN_LIBS}
    #PRIVATE GTest::gmock GTest::gtest GTest::gmock_main GTest::gtest_main
)

add_executable(
    boost_uuid_test
    tests/boost_uuid_test.cpp)
target_link_libraries(
    boost_uuid_test
    ${CONAN_LIBS}
)

add_executable(
    boost_log_test
    tests/boost_log_test.cpp)
target_link_libraries(
    boost_log_test
    ${CONAN_LIBS}
)

include(GoogleTest)
gtest_discover_tests(hello_test)
