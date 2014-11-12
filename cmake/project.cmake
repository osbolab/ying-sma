###############################################################################
# PROJECT SETUP

option(build_tests "Build all tests." ON)
option(group_output "Group output into bin/, lib/, test/ directories under each project's build directory." OFF)
option(global_output_path "Group all projects' output into common bin/, lib/, and test/ directories in the root build directory." OFF)

SET(gtest_version "1.7.0" CACHE STRING "Google Test library version.")

SET(ROOT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../")

########################################
# OUTPUT PATHS

SET(THIRDPARTY_LIBS_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/lib")

IF (group_output_globally)
  SET(output_dir "${ROOT_DIRECTORY}/build")
ELSE()
  SET(output_dir "${CMAKE_CURRENT_BINARY_DIR}")
ENDIF()
# Uncomment to put all output in a common dir
IF (group_output)
  SET(TEST_OUTPUT_DIRECTORY "${output_dir}/test")
  SET(ARCHIVE_OUTPUT_DIRECTORY "${output_dir}/lib")
  SET(LIBRARY_OUTPUT_DIRECTORY "${output_dir}/lib")
  SET(RUNTIME_OUTPUT_DIRECTORY "${output_dir}/bin")
ELSE()
  SET(TEST_OUTPUT_DIRECTORY "${output_dir}")
  SET(ARCHIVE_OUTPUT_DIRECTORY "${output_dir}")
  SET(LIBRARY_OUTPUT_DIRECTORY "${output_dir}")
  SET(RUNTIME_OUTPUT_DIRECTORY "${output_dir}")
ENDIF()

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${ARCHIVE_OUTPUT_DIRECTORY}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_DIRECTORY}")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${RUNTIME_OUTPUT_DIRECTORY}")


########################################
# INPUT PATHS

SET(thirdparty_libs "${ROOT_DIRECTORY}/lib")

SET(inc    "INCLUDE")
SET(SRC    "src")
SET(TEST   "test")

SET(lib_easylogging "${thirdparty_libs}/easyloggingpp")
SET(LIB_GTEST       "${thirdparty_libs}/gtest-${gtest_version}")


PROJECT("${project_name}")
# INCLUDE the default project headers (every project should use ./INCLUDE/)
INCLUDE_DIRECTORIES("${inc}")
# INCLUDE any library headers
INCLUDE_DIRECTORIES("${lib_easylogging}/INCLUDE")


###############################################################################
# COMPILE OPTIONS

IF (MSVC)
  INCLUDE("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/msvc.cmake" REQUIRED)
  configure_msvc_runtime()
ELSE()
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 ")
ENDIF()


###############################################################################
# TESTING            (cmake -Dbuild_tests=ON)

IF (NOT DEFINED gtest_added)
  IF (build_tests)
    SET(gtest_added ON)
    MESSAGE               (STATUS "Will build test projects")
    ENABLE_TESTING()
    ADD_SUBDIRECTORY      ("${LIB_GTEST}" "${THIRDPARTY_LIBS_OUTPUT_DIR}/gtest")
    INCLUDE_DIRECTORIES   ("${gtest_SOURCE_DIR}/INCLUDE")
    SET_TARGET_PROPERTIES (gtest PROPERTIES
      FOLDER "${TEST_PROJ_FOLDER}"
      ARCHIVE_OUTPUT_DIRECTORY "${THIRDPARTY_LIBS_OUTPUT_DIR}/gtest"
      LIBRARY_OUTPUT_DIRECTORY "${THIRDPARTY_LIBS_OUTPUT_DIR}/gtest"
      RUNTIME_OUTPUT_DIRECTORY "${THIRDPARTY_LIBS_OUTPUT_DIR}/gtest"
    )
    SET_TARGET_PROPERTIES (gtest_main PROPERTIES
      FOLDER "${TEST_PROJ_FOLDER}"
      ARCHIVE_OUTPUT_DIRECTORY "${THIRDPARTY_LIBS_OUTPUT_DIR}/gtest"
      LIBRARY_OUTPUT_DIRECTORY "${THIRDPARTY_LIBS_OUTPUT_DIR}/gtest"
      RUNTIME_OUTPUT_DIRECTORY "${THIRDPARTY_LIBS_OUTPUT_DIR}/gtest"
    )
  
    MACRO(add_test_exe name)
      ADD_EXECUTABLE        ("${name}" ${ARGN})
      TARGET_LINK_LIBRARIES ("${name}" gtest_main)
      SET_TARGET_PROPERTIES ("${name}" PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${TEST_OUTPUT_DIRECTORY}")
      add_test(
        NAME "${name}"
        COMMAND "${name}"
      )
    ENDMACRO(add_test_exe)
  ELSE()
    MACRO(add_test_exe)
      MESSAGE (STATUS "Use -Dbuild_tests=ON to enable test compilation.")
    ENDMACRO(add_test_exe)
  ENDIF()
ENDIF()

