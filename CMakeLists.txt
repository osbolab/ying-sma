cmake_minimum_required(VERSION 2.8)

option(build_tests "Build all tests." ON)
option(use_folders "Group projects in folders." ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ${use_folders})
if (${use_folders})
  Message(STATUS "Using project folders")
endif()

set(PROJ              ying-sma)
set(INC               include)
set(SRC	              src/main)
set(TEST              src/test)
set(LIB_GTEST         lib/gtest-1.7.0)
set(TEST_PROJ_FOLDER  Test)

set(PLATFORM  platform)

project(${PROJ})

# Must come here, depends on above settings
include(cmake/macros.cmake REQUIRED)

############################################
# COMPILE OPTIONS

if (MSVC)
  include(cmake/msvc.cmake REQUIRED)
  configure_msvc_runtime()
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=C++11 ")
endif()


############################################
# OBJECTS

# syntax:
#   object(Name "Project Folder (or empty)"
#     source_dependency.hh
#     source_dependency.cc
#     object_dependency
#     source_dependency2.hh
#     "source dependency with/spaces.hh"
#   )

# example:
#   object(Log ""
#     ${SRC}/log.hh
#   )
#   object(Main "app"
#     "${SRC}/app.cc"
#     Log
#   )

object(Log  ""  
  "${INC}/Log.hh"
)

object(Bits ""  
  "${INC}/Bits.hh"
)

object(Pow2Math ""
  "${INC}/Pow2Math.hh"
)

object(CyclicBarrier ""
  "${INC}/CyclicBarrier.hh"
  "${SRC}/CyclicBarrier.cc"
)

object(BlockingQueue "" 
  "${INC}/BlockingQueue.hh"
)

object(BlockingSource "" 
  "${INC}/BlockingSource.hh"
)

object(DelayQueue ""
  "${INC}/DelayQueue.hh"
)

object(Sink ""  
  "${INC}/Sink.hh"
)

object(Sump "" 
  "${INC}/Sump.hh"
  Sink
)

object(BufferPool "memory"
  "${INC}/memory/BufferPool.hh"
  "${INC}/memory/PooledBuffer.hh"
  Pow2Math
  Bits
  Log
)

object(Scheduler        "schedule"
  "${INC}/schedule/Scheduler.hh"
)

object(ThreadScheduler  "schedule"
  "${INC}/schedule/ThreadScheduler.hh"
  "${SRC}/schedule/ThreadScheduler.cc"
  Scheduler
)

object(Message    "msg"
  "${INC}/msg/Message.hh" 
  "${SRC}/msg/Message.cc"
)

object(Messenger  "msg"
  "${INC}/msg/Messenger.hh" 
  Message
)

object(Channel "channel"  
  "${INC}/Channel/Channel.hh"
)


object(Socket "net"
  "${INC}/net/Socket.hh"
)

object(Winsock "net"
  "${INC}/net/Winsock.hh"
  "${SRC}/net/Winsock.cc"
)

object(Address "net"
  "${INC}/net/Address.hh"
  "${SRC}/net/Address.cc"
  "${INC}/net/InetAddress.hh"
  "${SRC}/net/InetAddress.cc"
  "${INC}/net/SocketAddress.hh"
  "${SRC}/net/SocketAddress.cc"
  Winsock
)

object(NativeSocket "net"
  "${INC}/net/NativeSocket.hh"
  "${SRC}/net/NativeSocket.cc"
  Socket
  Address
  Log
)


############################################
# TARGETS

# syntax:
# - Automatically includes any object defined with object(Name ...)
# - Creates a separate make target Name_test
# - Create minimal targets for fast compilation/unit testing
#
#   add_test(Name
#     "test source.cc"
#     object_dependency
#     "test dependency.hh"
#     "test dependency.cc"
#   )
#
#   add_exe(Name
#     "source.cc"
#     "source.hh"
#     object
#     object
#   )

include_directories(${INC})

add_test(Bits         "${TEST}/BitsTest.cc")
add_test(BufferPool   "${TEST}/memory/BufferPoolTest.cc")

add_test(BlockingQueue  
  "${TEST}/BlockingQueueTest.cc"
  CyclicBarrier
)

add_test(DelayQueue   "${TEST}/DelayQueueTest.cc")

add_test(Message      "${TEST}/msg/MessageTest.cc")

add_test(NativeSocket "${TEST}/net/NativeSocketTest.cc")
add_test(Address      "${TEST}/net/SocketAddressTest.cc")

add_test(ThreadScheduler  "${TEST}/schedule/ThreadSchedulerTest.cc")

add_exe(Main
  "${SRC}/main.cc"
  __EVERYTHING__
)