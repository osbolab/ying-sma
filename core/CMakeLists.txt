cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME "smacore")

# Include config options, gtest setup, and output paths
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/project.cmake" REQUIRED)


###############################################################################
# SOURCES

set(LIB_SOURCES
  "${SRC}/bytebuffer.cc"

  "${SRC}/cyclicbarrier.cc"

  "${SRC}/message.cc"
  "${SRC}/messenger.cc"

  "${SRC}/inetaddress.cc"
  "${SRC}/nativesocket.cc"
  "${SRC}/winsock.cc"

  "${SRC}/nativechannel.cc"
)

set(TEST_SOURCES
  "${TEST}/test_main.cc"
)


###############################################################################
# TARGETS

add_library("${PROJECT_NAME}" STATIC ${LIB_SOURCES})
add_test_exe("${PROJECT_NAME}_test" ${TEST_SOURCES})
target_link_libraries("${PROJECT_NAME}_test" "${PROJECT_NAME}")
