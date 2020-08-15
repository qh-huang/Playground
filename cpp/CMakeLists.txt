project(cpp)


# Enabled C++11 support
if(CMAKE_VERSION VERSION_LESS "3.1")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
  endif()
else()
  set(CMAKE_CXX_STANDARD 11)
endif()

include_directories(include)

add_executable(app_main app_main.cc)
add_executable(utils_main utils_main.cc)

add_executable(actor_main actor_main.cc)
target_link_libraries(actor_main pthread)