cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE "Debug")

# Allow C++11/14
set(LIBSTD "c++14" CACHE STRING "C++ Standard Library [nothing/c++11/c++14]")
if(LIBSTD)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=${LIBSTD} ")
endif()

# Locate external libaries
set(EXT_DIR "${CMAKE_SOURCE_DIR}/ext" CACHE PATH "External library sources")
if(NOT EXISTS "${EXT_DIR}/")
  message(FATAL_ERROR "External library directory doesn't exist: ${EXT_DIR}")
endif()

#------------------------------------------------------------------------------
# Set up output directory by binary type
macro(set_build_dir path)
  # Public headers
  set(PUBLIC_HEADER_DIRECTORY  "${path}/include")
  # Executables
  set(RUNTIME_OUTPUT_DIRECTORY "${path}")
  # Test executables
  set(TEST_OUTPUT_DIRECTORY    "${path}/test")
  # Shared and static libraries
  set(LIBRARY_OUTPUT_DIRECTORY "${path}")
  set(ARCHIVE_OUTPUT_DIRECTORY "${path}")
  # Third-party libraries
  set(EXT_OUTPUT_DIRECTORY     "${path}/ext")
endmacro(set_build_dir)
# Everything outputs in the root build dir by defailt
set_build_dir("${CMAKE_BINARY_DIR}")

#------------------------------------------------------------------------------
# Pull in config scripts
set(CMAKE_SCRIPTS_DIR "${CMAKE_SOURCE_DIR}/cmake")
if(NOT EXISTS "${CMAKE_SCRIPTS_DIR}")
  message(FATAL_ERROR "Can't find configuration scripts in ${CMAKE_SCRIPTS_DIR}")
endif()

file(GLOB cmake_scripts "${CMAKE_SCRIPTS_DIR}/*.cmake")
foreach(cmake_script ${cmake_scripts})
  include("${cmake_script}")
endforeach()

###############################################################################
###############################################################################

set(SMA_LIB     "sma")
set(SMA_NS3_LIB "smans3")

###############################################################################
# Main SMA library project

depend(SMA_LIB)
project(${SMA_LIB})

include_directories("include")

set(${PROJECT_NAME}_sources
  "src/util/hash.cpp"
  "src/util/detail/sha256.cpp"

  "src/link.cpp"
  "src/linklayer.cpp"
  "src/forwardstrategy.cpp"
  "src/prforwardstrategy.cpp"

  "src/ccn/ccnnode.cpp"

  "src/ccn/interesthelper.cpp"
  "src/ccn/remoteinterest.cpp"

  "src/ccn/contenthelper.cpp"

  "src/neighborhelper.cpp"
  "src/neighbortable.cpp"

  "src/chrono.cpp"
  "src/context.cpp"
  "src/gps.cpp"

  "src/util/binaryinput.cpp"
  "src/util/binaryoutput.cpp"
  "src/util/bufferdest.cpp"
  "src/util/buffersource.cpp"
)

add_library("${PROJECT_NAME}" STATIC ${${PROJECT_NAME}_sources})
#add_test_exe("${PROJECT_NAME}_test" "test/test_main.cpp")
#target_link_libraries("${PROJECT_NAME}_test" "${PROJECT_NAME}")

###############################################################################
# Subprojects

include_directories("ns3/include")
add_subdirectory("ns3")
