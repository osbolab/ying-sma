###############################################################################
# PROJECT SETUP

option(build_tests "Build all tests." ON)
option(profiling "Enable profiling by linking gperftools." OFF)
option(use_folders "Group projects in folders." ON)
option(group_output "Group output into bin/, lib/, test/ directories under each project's build directory." OFF)
option(global_output_path "Group all projects' output into common bin/, lib/, and test/ directories in the root build directory." OFF)

SET(gtest_version "1.7.0" CACHE STRING "Google Test library version.")
SET(LIB_PROFILER "profiler" CACHE STRING "Profiler library name.")
SET(LIB_PROFILER_DIR "/usr/lib" CACHE STRING "Profiler library directory.")

set_property(GLOBAL PROPERTY USE_FOLDERS "${use_folders}")

set(ROOT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../")

# This is just a visual thing for output that supports virtual folders
set(TEST_PROJ_FOLDER  "Test")

########################################
# OUTPUT PATHS

set(THIRDPARTY_LIBS_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib")

if (group_output_globally)
  set(OUTPUT_DIRECTORY "${ROOT_DIRECTORY}/build")
else()
  set(OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endif()
# Uncomment to put all output in a common dir
if (group_output)
  set(TEST_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/test")
  set(ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/lib")
  set(LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/lib")
  set(RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}/bin")
else()
  set(TEST_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}")
  set(ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}")
  set(LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}")
  set(RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${ARCHIVE_OUTPUT_DIRECTORY}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_DIRECTORY}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${RUNTIME_OUTPUT_DIRECTORY}")


########################################
# INPUT PATHS

set(THIRDPARTY_LIBS_DIR "${ROOT_DIRECTORY}/lib")

set(INC    "include")
set(SRC    "src")
set(TEST   "test")

set(LIB_EASYLOGGING "${THIRDPARTY_LIBS_DIR}/easyloggingpp")
set(LIB_GTEST       "${THIRDPARTY_LIBS_DIR}/gtest-${gtest_version}")


project("${PROJECT_NAME}")
# Include the default project headers (every project should use ./include/)
include_directories("${INC}")
# Include any library headers
include_directories("${LIB_EASYLOGGING}/include")


###############################################################################
# COMPILE OPTIONS

if (MSVC)
  include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/msvc.cmake" REQUIRED)
  configure_msvc_runtime()
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 ")
endif()


###############################################################################
# PROFILING (cmake -Dprofiling=ON)

if (profiling)
  message (STATUS "Using profiler library '${LIB_PROFILER_DIR}/${LIB_PROFILER}'")
  link_directories("${LIB_PROFILER_DIR}")
  macro(profile name)
    target_link_libraries("${name}" "${LIB_PROFILER}")
    message (STATUS "Profiling target '${name}' with '${LIB_PROFILER_DIR}/${LIB_PROFILER}'")
  endmacro(profile)
else()
  macro(profile)
    message (STATUS "Use -Dprofiling=ON to enable profiling.")
  endmacro(profile)
endif()

###############################################################################
# TESTING            (cmake -Dbuild_tests=ON)

if (build_tests)
  message               (STATUS "Will build test projects")
  enable_testing()
  add_subdirectory      ("${LIB_GTEST}" "${THIRDPARTY_LIBS_OUTPUT_DIR}/gtest")
  include_directories   ("${gtest_SOURCE_DIR}/include")
  set_target_properties (gtest PROPERTIES
    FOLDER "${TEST_PROJ_FOLDER}"
    ARCHIVE_OUTPUT_DIRECTORY "${THIRDPARTY_LIBS_OUTPUT_DIR}/gtest"
    LIBRARY_OUTPUT_DIRECTORY "${THIRDPARTY_LIBS_OUTPUT_DIR}/gtest"
    RUNTIME_OUTPUT_DIRECTORY "${THIRDPARTY_LIBS_OUTPUT_DIR}/gtest"
  )
  set_target_properties (gtest_main PROPERTIES
    FOLDER "${TEST_PROJ_FOLDER}"
    ARCHIVE_OUTPUT_DIRECTORY "${THIRDPARTY_LIBS_OUTPUT_DIR}/gtest"
    LIBRARY_OUTPUT_DIRECTORY "${THIRDPARTY_LIBS_OUTPUT_DIR}/gtest"
    RUNTIME_OUTPUT_DIRECTORY "${THIRDPARTY_LIBS_OUTPUT_DIR}/gtest"
  )

  macro(add_test_exe name)
    add_executable        ("${name}" ${ARGN})
    target_link_libraries ("${name}" gtest_main)
    profile               ("${name}")
    set_target_properties ("${name}" PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${TEST_OUTPUT_DIRECTORY}")
    add_test(
      NAME "${name}"
      COMMAND "${name}"
    )
  endmacro(add_test_exe)
else()
  macro(add_test_exe)
    message (STATUS "Use -Dbuild_tests=ON to enable test compilation.")
  endmacro(add_test_exe)
endif()

