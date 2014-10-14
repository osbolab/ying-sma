cmake_minimum_required(VERSION 2.8)

option(build_tests "Build all tests." ON)
option(use_folders "Group projects in folders." ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ${use_folders})
if (${use_folders})
  message(STATUS "Using project folders")
endif()

set(PROJ          ying-sma)
set(INC           include)
set(SRC	          src/main)
set(TEST          src/test)
set(LIB_GTEST     lib/gtest-1.7.0)
set(FOLDER_TEST   Test)

set(PLATFORM  platform)

project(${PROJ})

# Must come here, depends on above settings
include(cmake/macros.cmake REQUIRED)

############################################
# COMPILE OPTIONS

add_definitions(-std=c++11)
# add_definitions(-Wall)

if (MSVC)
  include(cmake/msvc.cmake REQUIRED)
  configure_msvc_runtime()
endif()


############################################
# OBJECTS

object(Main "" ${SRC}/main.cc)

object(CyclicBarrier ""
  ${INC}/CyclicBarrier.hh
  ${SRC}/CyclicBarrier.cc
)

object(BlockingQueue "" ${INC}/BlockingQueue.hh)
test_object(BlockingQueue "" ${TEST}/BlockingQueueTest.cc)

object(Message "msg"
  ${INC}/msg/Message.hh 
  ${SRC}/msg/Message.cc 
)
test_object(Message "msg" ${TEST}/msg/MessageTest.cc)

############################################
# TARGETS

include_directories(${INC})

add_test(BlockingQueue
  CyclicBarrier
)

add_test(Message)

add_exe(Main
  CyclicBarrier
  BlockingQueue
  Message
)