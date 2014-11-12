CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(sma)

OPTION(build_tests "Enables the output of test executables." ON)
SET(gtest_version "1.7.0" CACHE STRING "Google Test library version")

SET(ext            "${CMAKE_SOURCE_DIR}/ext")

MACRO(set_build_dir path)
  SET(RUNTIME_OUTPUT_DIRECTORY    ${path})
  SET(LIBRARY_OUTPUT_DIRECTORY    ${path})
  SET(ARCHIVE_OUTPUT_DIRECTORY    ${path})
  SET(THIRDPARTY_OUTPUT_DIRECTORY "${path}/ext")
  SET(TEST_OUTPUT_DIRECTORY       "${path}/test")
ENDMACRO(set_build_dir)

# Everything outputs in the root build dir by defailt
set_build_dir(${CMAKE_BINARY_DIR})


IF (NOT DEFINED tests_defined)
  IF (build_tests)
    SET(tests_defined ON)
    ENABLE_TESTING()
    SET(gtest_dir "${ext}/gtest-${gtest_version}")
    ADD_SUBDIRECTORY("${gtest_dir}" "${THIRDPARTY_OUTPUT_DIRECTORY}/gtest")
    INCLUDE_DIRECTORIES("${gtest_dir}/include")
  
    MACRO(add_test_exe name)
      ADD_EXECUTABLE(${name} ${ARGN})
      TARGET_LINK_LIBRARIES(${name} gtest)
      SET_TARGET_PROPERTIES(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
      ADD_TEST(NAME ${name} COMMAND ${name})
    ENDMACRO()
  ELSE()
    MACRO(add_test_exe)
      MESSAGE (STATUS "TESTS ARE DISABLED. Use -Dbuild_tests=ON to build tests.")
    ENDMACRO()
  ENDIF()
ENDIF()

MACRO(ADD_PROJECT PROJECT)
  IF(${PROJECT} OR ALL)
    add_subdirectory(${PROJECT})
  ENDIF()
ENDMACRO()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 ")

SET(user_headers   "${CMAKE_SOURCE_DIR}/include")
SET(ext_headers    "${ext}/easyloggingpp/include")
INCLUDE_DIRECTORIES(BEFORE SYSTEM ${user_headers} ${ext_headers})

ADD_PROJECT("core")
#ADD_SUBDIRECTORY("app")
#ADD_SUBDIRECTORY("platform")
