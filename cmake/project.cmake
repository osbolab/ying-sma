###############################################################################
# PROJECT SETUP

option(build_tests "Build all tests." ON)
option(use_folders "Group projects in folders." ON)

SET(gtest_version "1.7.0" CACHE STRING "Google Test library version.")

set_property(GLOBAL PROPERTY USE_FOLDERS "${use_folders}")

set(LIB_GTEST         "${CMAKE_CURRENT_SOURCE_DIR}/../lib/gtest-${gtest_version}")
set(TEST_PROJ_FOLDER  "Test")

set(OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/../build")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT}/bin")

set(INC    "include")
set(SRC    "src")
set(TEST   "test")


project("${PROJECT_NAME}")
include_directories("${INC}")


###############################################################################
# COMPILE OPTIONS

if (MSVC)
  include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/msvc.cmake" REQUIRED)
  configure_msvc_runtime()
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")
endif()


###############################################################################
# TESTING            (cmake -Dbuild_tests=ON)

if (build_tests)
  message               (STATUS "Will build test projects")
  enable_testing()
  add_subdirectory      ("${LIB_GTEST}" "${CMAKE_CURRENT_BINARY_DIR}/lib/gtest")
  include_directories   ("${gtest_SOURCE_DIR}/include")
  set_target_properties (gtest PROPERTIES
    FOLDER "${TEST_PROJ_FOLDER}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib/gtest"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib/gtest"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib/gtest"
  )
  set_target_properties (gtest_main PROPERTIES
    FOLDER "${TEST_PROJ_FOLDER}"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib/gtest"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib/gtest"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib/gtest"
  )

  macro(add_test_exe name)
    set                   (test_obj    "${name}_test")
    add_executable        ("${test_obj}" ${ARGN})
    target_link_libraries ("${test_obj}" "${PROJECT_NAME}" gtest_main)
    set_target_properties ("${test_obj}" PROPERTIES
                            RUNTIME_OUTPUT_DIRECTORY "${OUTPUT}/test/")
    add_test(
      NAME "${name}"
      COMMAND "${test_obj}"
    )
  endmacro(add_test_exe)
else()
  macro(add_test_exe)
  endmacro(add_test_exe)
endif()

