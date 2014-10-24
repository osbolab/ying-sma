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

# util

object(Log  "util"  
  "${INC}/util/Log.hh"
)
object(Bits "util"  
  "${INC}/util/Bits.hh"
)
object(Pow2Math "util"
  "${INC}/util/Pow2Math.hh"
)
object(Sink "util"  
  "${INC}/util/Sink.hh"
)
object(Sump "util" 
  "${INC}/util/Sump.hh"
  Sink
)


# concurrent

object(CyclicBarrier "concurrent"
  "${INC}/concurrent/CyclicBarrier.hh"
  "${SRC}/concurrent/CyclicBarrier.cc"
)
object(BlockingQueue "concurrent"
  "${INC}/concurrent/BlockingQueue.hh"
)
object(BlockingSource "concurrent"
  "${INC}/concurrent/BlockingSource.hh"
)
object(DelayQueue "concurrent"
  "${INC}/concurrent/DelayQueue.hh"
)
object(BufferPool "memory"
  "${INC}/memory/BufferPool.hh"
  "${INC}/memory/PooledBuffer.hh"
  Pow2Math
  Bits
  Log
)


# msg

object(Message    "msg"
  "${INC}/msg/Message.hh" 
  "${SRC}/msg/Message.cc"
)
object(Messenger  "msg"
  "${INC}/msg/Messenger.hh" 
  Message
)


# net

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


# schedule

object(Scheduler        "schedule"
  "${INC}/schedule/Scheduler.hh"
)
object(ThreadScheduler  "schedule"
  "${INC}/schedule/ThreadScheduler.hh"
  "${SRC}/schedule/ThreadScheduler.cc"
  Scheduler
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
#
# __EVERYTHING__ includes all sources referenced above its use
#

include_directories(${INC})


# util
add_test(Bits             "${TEST}/util/BitsTest.cc")

# concurrent
add_test(DelayQueue       "${TEST}/concurrent/DelayQueueTest.cc")
add_test(BlockingQueue  
  "${TEST}/concurrent/BlockingQueueTest.cc"
  CyclicBarrier
)

# memory
add_test(BufferPool       "${TEST}/memory/BufferPoolTest.cc")

# msg
add_test(Message          "${TEST}/msg/MessageTest.cc")

# net
add_test(NativeSocket     "${TEST}/net/NativeSocketTest.cc")
add_test(Address          "${TEST}/net/SocketAddressTest.cc")

# schedule
add_test(ThreadScheduler  "${TEST}/schedule/ThreadSchedulerTest.cc")



add_exe(Main
  "${SRC}/main.cc"
  __EVERYTHING__
)