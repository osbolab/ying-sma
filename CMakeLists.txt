cmake_minimum_required(VERSION 2.8)

option(build_tests "Build all tests." OFF)

set(PROJ_NAME ying-sma)
set(SRC		    src/main)
set(INC		    include)
set(TEST	    src/test)
set(GTEST     lib/gtest-1.7.0)

project(${PROJ_NAME})

############################################
# COMPILE OPTIONS
############################################

add_definitions(-std=c++11)
# add_definitions(-Wall)

if (MSVC)
  include(cmake/msvc.cmake REQUIRED)
  configure_msvc_runtime()
endif()

############################################
# OBJECTS
############################################

include_directories(${INC})

############################################
# TESTS            (cmake -Dbuild_tests=ON)
############################################
if (build_tests)
  add_subdirectory(${GTEST})
  #enable_testing()
  include_directories(${gtest_SOURCE_DIR}/include)

  add_executable(
	  MessageTest 
	  ${SRC}/msg/Message.cc 
	  ${INC}/msg/Message.hh 
    ${TEST}/msg/MessageTest.cc 
  )
  target_link_libraries(MessageTest gtest_main)

  add_executable(
    MessageQueueTest
    ${SRC}/msg/Message.cc
    ${INC}/msg/Message.hh
    ${SRC}/msg/MessageQueue.cc
    ${INC}/msg/MessageQueue.hh
    ${TEST}/msg/MessageQueueTest.cc
  )
  target_link_libraries(MessageQueueTest gtest_main)

endif()