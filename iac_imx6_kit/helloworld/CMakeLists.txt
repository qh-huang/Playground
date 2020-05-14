project(hello_iac_imx6)

# Enabled C++11 support
#if(CMAKE_VERSION VERSION_LESS "3.1")
#  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
#  endif()
#else()
#  set(CMAKE_CXX_STANDARD 11)
#endif()

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_SYSROOT /home/qiao/Tools/fsl-linaro-toolchain/arm-fsl-linux-gnueabi/multi-libs)
#set(CMAKE_SYSROOT /home/qiao/Tools/sysroot-glibc-linaro-2.25-2019.12-arm-linux-gnueabihf)
#set(CMAKE_STAGING_PREFIX /home/devel/stage)

set(tools /home/qiao/Tools/fsl-linaro-toolchain)
#set(tools /home/qiao/Tools/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf)
set(CMAKE_C_COMPILER ${tools}/bin/arm-fsl-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER ${tools}/bin/arm-fsl-linux-gnueabi-g++)

set(CMAKE_FIND_ROOT_PATH /home/qiao/Tools/fsl-linaro-toolchain/arm-fsl-linux-gnueabi/multi-libs/armv7-a/default)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

add_executable(hello hello_main.cc)


