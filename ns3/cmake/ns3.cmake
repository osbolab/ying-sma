# Require 2.6 for empty endxxx()
cmake_minimum_required(VERSION 2.6)

# I'm not sure an include guard makes sense when we're using ${PROJECT_XXX}
# if(ns3_cmake_included)
#   return()
# endif()
# set(ns3_cmake_included true)

###############################################################################
# NS3 config
#  We need a pre-built NS3 tree that put all of the module shared libraries and
#  their headers in its output folder. This should be symlinked as our lib/
#  directory and as include/ns3/ so we can pull them in as if their part of
#  this project.
#  The alternative is to put all of our source in the NS3 tree and use it to
#  build our project; the benefit there is that we can pick which modules we
#  use and it will selectively build those.
#  That's enormously stupid, though, because the modules are shared libs anyway
#  so there's zero reason we should be rebuilding them *ever* instead of just
#  installing them to usr/local/lib/. 
#  Regardless, we may want to do a static link at some point and so it makes
#  some sense to keep the tree in-source like this. Deployment to other dev
#  environments should also be installation-free this way, which is nice.

option(LINK_VERBOSE "Show extra details about NS3 module linkage." OFF)
set(NS3_LIB_DIR     "${PROJECT_SOURCE_DIR}/lib" CACHE PATH "Path to directory containing NS3 shared libraries")
set(NS3_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include" CACHE PATH "Path to directory containing NS3 headers; must contain the 'ns3/' directory as a subdirectory.")
set(NS3_VERSION     "3.21" CACHE VERSION "The NS3 version string prefixing the module libraries (e.g. libns3.21-wifi-debug.so).")
set(NS3_BUILD_TYPE  "debug" CACHE STRING "The build type of the NS3 module libraries [debug/release]")
string(TOLOWER ${NS3_BUILD_TYPE} NS3_BUILD_TYPE)

set(NS3_LIB_PREFIX "ns${NS3_VERSION}-" CACHE STRING "NS3 module library file prefix")
set(NS3_LIB_SUFFIX "-${NS3_BUILD_TYPE}" CACHE STRING "NS3 module library file suffix")

# Check if we can find the libraries
if(NOT EXISTS "${NS3_LIB_DIR}/")
  message(FATAL_ERROR "Looking for NS3 module libraries in '${NS3_LIB_DIR}', but it doesn't exist.")
endif()
message(STATUS "Found NS3 module libraries in ${NS3_LIB_DIR}")
# Check if we can find the headers and that ns3/ is a subdirectory
if(EXISTS "${NS3_INCLUDE_DIR}/")
  if(NOT EXISTS "${NS3_INCLUDE_DIR}/ns3")
    message(FATAL_ERROR "Expecting to find the NS3 module headers in '${NS3_INCLUDE_DIR}', but it doesn't contain the ns3/ directory.")
  endif()
else()
  message(FATAL_ERROR "Expecting to find the NS3 module headers in '${NS3_INCLUDE_DIR}', but it doesn't exist.")
endif()
message(STATUS "Found NS3 headers in ${NS3_INCLUDE_DIR}")

# NS3 puts the build type in the ilbrary name instead of separating output
# directories by build type. Really clever.
if(NOT NS3_BUILD_TYPE STREQUAL "debug" AND NOT NS3_BUILD_TYPE STREQUAL "release")
  message(FATAL_ERROR "Unknown NS3 build type: ${NS3_BUILD_TYPE}")
endif()
if(NS3_BUILD_TYPE STREQUAL "debug" AND CMAKE_BUILD_TYPE STREQUAL "Release")
  message(WARNING "You're compiling in release mode, but using debug NS3 binaries.")
  message(WARNING "Did you mean to -DNS3_BUILD_TYPE=release ?")
endif()

# Help the linker find the NS3 module libs
# I'm not sure how this changes for static linkage.
link_directories("${NS3_LIB_DIR}")
# And the headers
include_directories("${NS3_INCLUDE_DIR}")

# This is part of what NS3 is doing when you specify the module names in your
# wscript and compile (link) with waf:
macro(use_ns3_modules target)
  # Get the library name for each module
  set(module_libs "")
  foreach(module ${ARGN})
    set(module_lib "${NS3_LIB_PREFIX}${module}${NS3_LIB_SUFFIX}")
    set(module_lib_path "${NS3_LIB_DIR}/lib${module_lib}.so")
    if(NOT EXISTS "${module_lib_path}")
      message(FATAL_ERROR "NS3 module library not found: ${module_lib_path}")
    endif()
    list(APPEND module_libs ${module_lib})
  endforeach()
  string(REPLACE ";" ", " modules "${ARGN}")
  message(STATUS "${target} using modules: ${modules}")
  if(LINK_VERBOSE)
    foreach(module_lib ${module_libs})
      message(STATUS " - ${module_lib}")
    endforeach()
  endif()
  TARGET_LINK_LIBRARIES(${target} ${module_libs})
endmacro()

