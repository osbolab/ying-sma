cmake_minimum_required(VERSION 2.8)

###############################################################################
# Root project

project(sma)

set(CMAKE_BUILD_TYPE "Debug")

option(ALL "Configure all projects" OFF)

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

# Set up output directory by binary type
macro(set_build_dir path)
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

###############################################################################
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
# Configure subprojects 
set(user_headers   "${CMAKE_SOURCE_DIR}/include")
set(ext_headers    "${EXT_DIR}/easyloggingpp/include")
include_directories(BEFORE SYSTEM ${user_headers} ${ext_headers})

while(subproject_count EQUAL 0)
  subproject("core")
  subproject("app")
  subproject("ns3")
  if (subproject_count EQUAL 0)
    message(STATUS "No projects were specified; configuring everything")
    set(ALL ON)
  endif()
endwhile()
message(STATUS "")
