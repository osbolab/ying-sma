cmake_minimum_required(VERSION 2.8)

# Include config options, gtest setup, and output paths
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/project.cmake" REQUIRED)

set(LIB_CORE "smacore" CACHE STRING "Platform services core library")
set(LIB_CORE_INC "${CMAKE_CURRENT_SOURCE_DIR}/${INC}" 
    CACHE STRING "Core library include directory")


###############################################################################
# SOURCES

set(LIB_CORE_SOURCES
  "${SRC}/bytebuffer.cc"

  "${SRC}/cyclicbarrier.cc"
  "${SRC}/threadpool.cc"

  "${SRC}/message.cc"
  "${SRC}/messenger.cc"

  "${SRC}/inetaddress.cc"
  "${SRC}/nativesocket.cc"
  "${SRC}/winsock.cc"

  "${SRC}/nativechannel.cc"
)


###############################################################################
# TARGETS

add_library("${LIB_CORE}" STATIC ${LIB_CORE_SOURCES})
add_test_exe("${LIB_CORE}_test" "${TEST}/test_main.cc")
target_link_libraries("${LIB_CORE}_test" "${LIB_CORE}")
