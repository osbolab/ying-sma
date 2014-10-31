cmake_minimum_required(VERSION 2.8)

###############################################################################
# SOURCES

set(INC   "include")
set(SRC   "src")
set(TEST  "test")


set(LIB_SOURCES
  "${SRC}/concurrent/CyclicBarrier.cc"
  "${SRC}/msg/Message.cc"
  "${SRC}/net/Winsock.cc"
  "${SRC}/net/Address.cc"
  "${SRC}/net/InetAddress.cc"
  "${SRC}/net/SocketAddress.cc"
  "${SRC}/net/NativeSocket.cc"
  "${SRC}/schedule/ThreadScheduler.cc"
)

set(TEST_SOURCES
  "${TEST}/test_main.cc"
)


###############################################################################
# CONFIGURATION

option(build_tests "Build all tests." ON)
option(use_folders "Group projects in folders." ON)

SET(gtest_version "1.7.0" CACHE STRING "Google Test library version.")

set_property(GLOBAL PROPERTY USE_FOLDERS ${use_folders})
if (${use_folders})
  Message(STATUS "Using project folders")
endif()

set(PROJ              "ying-sma")
set(LIB_GTEST         "lib/gtest-${gtest_version}")
set(TEST_PROJ_FOLDER  "Test")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

project("${PROJ}")


###############################################################################
# COMPILE OPTIONS

if (MSVC)
  include(../cmake/msvc.cmake REQUIRED)
  configure_msvc_runtime()
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")
endif()


###############################################################################
# TESTING            (cmake -Dbuild_tests=ON)

if (build_tests)
  message               (STATUS "Will build test projects")
  add_subdirectory      ("${LIB_GTEST}")
  include_directories   ("${gtest_SOURCE_DIR}/include")
  set_target_properties (gtest       PROPERTIES FOLDER "${TEST_PROJ_FOLDER}")
  set_target_properties (gtest_main  PROPERTIES FOLDER "${TEST_PROJ_FOLDER}")

  macro(add_test OBJ)
      add_executable(${OBJ} ${ARGN})
      target_link_libraries (${OBJ} gtest_main)
  endmacro(add_test)
else()
  macro(add_test)
  endmacro(add_test)
endif()


###############################################################################
# TARGETS

include_directories(${INC})

add_library(smacore STATIC ${LIB_SOURCES})

add_test(Tests ${TEST_SOURCES})
target_link_libraries(Tests 
  smacore
  gtest_main
)
