project(gtest_samples)

enable_testing()
find_package(GTest REQUIRED)

# demo library
include_directories(include)
add_library(gtest_samples src/gtest_samples_demo.cc)

# gtest
add_executable(test test/test.cc)
target_link_libraries(test gtest_samples GTest::GTest GTest::Main)

add_test(AllTests test)


