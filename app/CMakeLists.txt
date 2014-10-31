cmake_minimum_required(VERSION 2.8)

###############################################################################
# SOURCES

set(INC    "include")
set(SRC    "src")
set(TEST   "test")


set(LIB_SOURCES
  "${SRC}/md5.cc"
  "${SRC}/json/json.cc"
  "${SRC}/split_regroup.cc"

  "${SRC}/EchoClient.cc"
  "${SRC}/EchoServer.cc"

  "${SRC}/emulator/Emulator.cc"
  "${SRC}/emulator/BatchScriptGenerator.cc"

  "${SRC}/node/DeviceWithGPS.cc"
  "${SRC}/node/DeviceLogger.cc"

  "${SRC}/network/DataBlock.cc"
  "${SRC}/network/AdHocNetworkEmulator.cc"

  "${SRC}/data/PlainChunkStore.cc"
  "${SRC}/data/PendingChunkManager.cc"
  "${SRC}/data/FlowTable.cc"
  "${SRC}/data/DataLayer.cc"

  "${SRC}/control/SignalHandler.cc"
  "${SRC}/control/Segmenter.cc"
  "${SRC}/control/PendingFileManager.cc"
  "${SRC}/control/NeighborRecords.cc"
  "${SRC}/control/NeighborRecordEntry.cc"
  "${SRC}/control/GPSDriver.cc"
  "${SRC}/control/ControlLayer.cc"
  "${SRC}/control/ContentDiscription.cc"
  "${SRC}/control/ContentDirectory.cc"
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

set(PROJ              "smapp")
set(LIB_GTEST         "${CMAKE_CURRENT_SOURCE_DIR}/../lib/gtest-${gtest_version}")
set(TEST_PROJ_FOLDER  "Test")

set(OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/../build")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OUTPUT}/bin")

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
    set                   (test_obj    "test_${name}")
    add_executable        (${test_obj} ${ARGN})
    target_link_libraries (${test_obj} ${PROJ} gtest_main)
    set_target_properties (${test_obj} PROPERTIES 
              RUNTIME_OUTPUT_DIRECTORY "${OUTPUT}/test/")
    add_test(
      NAME ${name}
      COMMAND ${test_obj}
    )
  endmacro(add_test_exe)
else()
  macro(add_test_exe)
  endmacro(add_test_exe)
endif()


###############################################################################
# TARGETS

include_directories("${INC}")

add_library("${PROJ}" STATIC ${LIB_SOURCES})

add_test_exe("${PROJ}" ${TEST_SOURCES})

