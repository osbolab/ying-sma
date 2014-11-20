if(test_cmake_included)
  return()
endif()
set(test_cmake_included ON)

option(BUILD_TESTS "Enables the output of test executables." ON)
set(GTEST_VERSION  "1.7.0" CACHE STRING "Google Test library version")

if(BUILD_TESTS)
  enable_testing()
  set(gtest_name "gtest-${GTEST_VERSION}")
  set(gtest_dir "${EXT_DIR}/${gtest_name}")
  if(NOT EXISTS "${gtest_dir}/" OR NOT EXISTS "${gtest_dir}/CMakeLists.txt")
    message(FATAL_ERROR "Expected to find gtest source in '${gtest_dir}', but it doesn't exist.")
  endif()
  # When we build gtest it'll go in our build path instead of whatever it
  # specifies.
  add_subdirectory("${gtest_dir}" "${EXT_OUTPUT_DIRECTORY}/gtest")
  include_directories("${gtest_dir}/include")

  # Automatically generate an executable and link it with gtest
  macro(add_test_exe name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} gtest)
    # Text executables get their own output directory
    set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_OUTPUT_DIRECTORY})
    add_test(NAME ${name} COMMAND ${name})
  endmacro()
else()
  macro(add_test_exe)
    message (STATUS "Tests are disabled. Use -DBUILD_TESTS=ON to build tests.")
  endmacro()
endif()
