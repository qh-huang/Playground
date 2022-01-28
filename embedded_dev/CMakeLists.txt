cmake_minimum_required(VERSION 3.14)
project(embedded_dev
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
)
add_executable(
    boost_test
    tests/boost_test.cpp)
target_link_libraries(
    boost_test
    ${CONAN_LIBS}
)

include(GoogleTest)
gtest_discover_tests(hello_test)
