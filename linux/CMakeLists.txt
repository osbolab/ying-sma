cmake_minimum_required(VERSION 2.8)

# Include config options, gtest setup, and output paths
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/project.cmake" REQUIRED)

set(LIB_LINUX "smalinux" CACHE STRING "Linux platform services library")


###############################################################################
# SOURCES

set(LIB_LINUX_SOURCES
  "${SRC}/bsd_socket.cpp"
  "${SRC}/select_channel.cpp"
)


###############################################################################
# TARGETS

add_library("${LIB_LINUX}" STATIC ${LIB_LINUX_SOURCES})
add_test_exe("${LIB_LINUX}_test" "${TEST}/test_main.cc")
target_link_libraries("${LIB_LINUX}_test" "${LIB_LINUX}")
