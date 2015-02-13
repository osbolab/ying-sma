cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE "Debug")

# Allow C++11/14
# For Mac platform
if(APPLE)
  set(LIBSTD "c++1y" CACHE STRING "C++ Standard Library")
# For Ubuntu Platform
else(APPLE)
  set(LIBSTD "c++14" CACHE STRING "C++ Standard Library [nothing/c++11/c++14]")
endif()

if(LIBSTD)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=${LIBSTD}")
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

set(${PROJECT_NAME}_sources
  "src/stats.cpp"

  "src/util/hash.cpp"
  "src/util/detail/sha256.cpp"

  "src/link.cpp"
  "src/linklayerimpl.cpp"
  "src/forwardstrategy.cpp"
  "src/prforwardstrategy.cpp"

  "src/ccn/ccnnode.cpp"
  "src/helper.cpp"

  "src/neighborhelper.cpp"
  "src/neighborhelperimpl.cpp"

  "src/ccn/interesthelper.cpp"
  "src/ccn/interesthelperimpl.cpp"
  "src/ccn/remoteinterest.cpp"

  "src/ccn/contenthelper.cpp"
  "src/ccn/contenthelperimpl.cpp"
  "src/ccn/contentcache.cpp"
  "src/ccn/blockdata.cpp"

  "src/chrono.cpp"
  "src/context.cpp"
  "src/gps.cpp"

  "src/util/binaryinput.cpp"
  "src/util/binaryoutput.cpp"
  
  "src/schedule/forwardschedulerimpl.cpp"
  "src/schedule/forwardscheduler.cpp"

  "src/ccn/behaviorhelper.cpp"
  "src/ccn/behaviorhelperimpl.cpp"
)

include_directories("include")

add_library("${PROJECT_NAME}" STATIC ${${PROJECT_NAME}_sources})
target_link_libraries("${PROJECT_NAME}" "glpk")
#add_test_exe("${PROJECT_NAME}_test" "test/test_main.cpp")
#target_link_libraries("${PROJECT_NAME}_test" "${PROJECT_NAME}")

###############################################################################
# Subprojects

include_directories("ns3/include")
add_subdirectory("ns3")
