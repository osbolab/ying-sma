cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME "smacore")

# Include config options, gtest setup, and output paths
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/project.cmake" REQUIRED)


###############################################################################
# SOURCES

set(LIB_SOURCES
  "${SRC}/cyclicbarrier.cc"

  "${SRC}/message.cc"

  "${SRC}/address.cc"
  "${SRC}/inetaddress.cc"
  "${SRC}/socketaddress.cc"
  "${SRC}/nativesocket.cc"
  "${SRC}/winsock.cc"
)

set(TEST_SOURCES
  "${TEST}/test_main.cc"
)


###############################################################################
# TARGETS

add_library("${PROJECT_NAME}" STATIC ${LIB_SOURCES})
profile("${PROJECT_NAME}")

# add_test_exe automatically adds profile()
add_test_exe("${PROJECT_NAME}_test" ${TEST_SOURCES})
target_link_libraries("${PROJECT_NAME}_test" "${PROJECT_NAME}")
