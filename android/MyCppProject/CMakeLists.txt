cmake_minimum_required(VERSION 3.10)
project(mycppmodule)

# Enabled C++11 support
if(CMAKE_VERSION VERSION_LESS "3.1")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
  endif()
else()
  set(CMAKE_CXX_STANDARD 11)
endif()

include_directories(include)

add_library(mycppmodule
  src/barrier.cpp
  src/combine.cpp
  src/geometry.cpp
  src/line.cpp
  src/polygon.cpp
  src/route_plan_check.cpp
  src/segment.cpp
  src/traverse_order.cpp
  src/tsp.cpp
)

