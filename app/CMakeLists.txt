cmake_minimum_required(VERSION 2.8)

# Set this first
set(PROJECT_NAME "smaapp")

# Include config options, gtest setup, and output paths
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/project.cmake" REQUIRED)


###############################################################################
# SOURCES

set(LIB_SOURCES
  "${SRC}/md5.cc"
  "${SRC}/json.cc"
  "${SRC}/split_regroup.cc"

  "${SRC}/echoclient.cc"
  "${SRC}/echoserver.cc"

  "${SRC}/emulator.cc"
  "${SRC}/batchscriptgenerator.cc"

  "${SRC}/devicewithgps.cc"
  "${SRC}/devicelogger.cc"

  "${SRC}/datablock.cc"
  "${SRC}/adhocnetworkemulator.cc"

  "${SRC}/plainchunkstore.cc"
  "${SRC}/pendingchunkmanager.cc"
  "${SRC}/flowtable.cc"
  "${SRC}/datalayer.cc"

  "${SRC}/signalhandler.cc"
  "${SRC}/segmenter.cc"
  "${SRC}/pendingfilemanager.cc"
  "${SRC}/neighborrecords.cc"
  "${SRC}/neighborrecordentry.cc"
  "${SRC}/gpsdriver.cc"
  "${SRC}/controllayer.cc"
  "${SRC}/contentdiscription.cc"
  "${SRC}/contentdirectory.cc"
)

set(TEST_SOURCES
  "${TEST}/test_main.cc"
)


###############################################################################
# TARGETS

add_library("${PROJECT_NAME}" STATIC ${LIB_SOURCES})

add_test_exe("${PROJECT_NAME}" ${TEST_SOURCES})

