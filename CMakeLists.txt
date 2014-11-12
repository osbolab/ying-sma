CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

OPTION(build_tests "Enables the output of test executables." ON)
SET(gtest_version "1.7.0" CACHE STRING "Google Test library version")


PROJECT("sma-libs")

SET(ext         "${CMAKE_SOURCE_DIR}/lib")

SET(user_headers    "${CMAKE_SOURCE_DIR}/include")
SET(ext_headers     "${ext}/easyloggingpp/include")

SET(RUNTIME_OUTPUT_DIRECTORY    "${CMAKE_SOURCE_DIR}/build")
SET(TEST_OUTPUT_DIRECTORY       "${RUNTIME_OUTPUT_DIRECTORY}/test")
SET(LIBRARY_OUTPUT_DIRECTORY    "${CMAKE_SOURCE_DIR}/build")
SET(ARCHIVE_OUTPUT_DIRECTORY    ${LIBRARY_OUTPUT_DIRECTORY})
SET(THIRDPARTY_OUTPUT_DIRECTORY "${LIBRARY_OUTPUT_DIRECTORY}/ext")


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 ")

INCLUDE_DIRECTORIES(BEFORE SYSTEM ${user_headers} ${ext_headers})


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
    ENDMACRO(add_test_exe)
  ELSE()
    MACRO(add_test_exe)
      MESSAGE (STATUS "TESTS ARE DISABLED. Use -Dbuild_tests=ON to build tests.")
    ENDMACRO(add_test_exe)
  ENDIF()
ENDIF()


ADD_SUBDIRECTORY("core")
#ADD_SUBDIRECTORY("app")
#ADD_SUBDIRECTORY("platform")
