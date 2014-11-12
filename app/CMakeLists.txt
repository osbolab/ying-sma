cmake_minimum_required(VERSION 2.8)

# Include config options, gtest setup, and output paths
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/project.cmake" REQUIRED)

set(LIB_APP "smaapp" CACHE STRING "Application library")
set(LIB_APP_INC "${CMAKE_CURRENT_SOURCE_DIR}/${INC}"
    CACHE STRING "Application library include directory")


###############################################################################
# SOURCES

set(LIB_APP_SOURCES
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


###############################################################################
# TARGETS

# Depend on the core library project
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../core/" "core")
include_directories("${LIB_CORE_INC}")

add_library("${LIB_APP}" STATIC ${LIB_APP_SOURCES})
target_link_libraries("${LIB_APP}" "${LIB_CORE}")

add_test_exe("${LIB_APP}_test" "${TEST}/test_main.cc")
  

