project(opencv_samples)

find_package(OpenCV 4 REQUIRED)

# Fix error "OpenCV 4.x+ requires enabled C++11 support"
if(CMAKE_VERSION VERSION_LESS "3.1")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
  endif()
else()
  set(CMAKE_CXX_STANDARD 11)
endif()


add_executable(app_zoom src/app_zoom_main.cc)
target_link_libraries(app_zoom ${OpenCV_LIBS})