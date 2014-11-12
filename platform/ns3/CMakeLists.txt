cmake_minimum_required(VERSION 2.8)

# Include config options, gtest setup, and output paths
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/project.cmake" REQUIRED)

set(LIB_NS "smans3" CACHE STRING "NS3 platform services library.")
set(LIB_NS_INC "${CMAKE_CURRENT_SOURCE_DIR}/${INC}" 
  CACHE STRING "NS3 platform services include directory.")


###############################################################################
# SOURCES

set(LIB_NS_SOURCES
  "${SRC}/ns3socket.cc"
  "${INC}/ns3socket.hh"

)


###############################################################################
# TARGETS

add_library("${LIB_NS}" STATIC ${LIB_NS_SOURCES})
add_test_exe("${LIB_NS}_test" "${TEST}/test_main.cc")
target_link_libraries("${LIB_NS}_test" "${LIB_NS}")
