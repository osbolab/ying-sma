cmake_minimum_required(VERSION 2.8)

option(build_tests "Build all tests." ON)
option(use_folders "Group projects in folders." ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ${use_folders})
if (${use_folders})
  Message(STATUS "Using project folders")
endif()

set(PROJ          ying-sma)
set(INC           include)
set(SRC	          src/Main)
set(TEST          src/test)
set(LIB_GTEST     lib/gtest-1.7.0)
set(FOLDER_TEST   Test)

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
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 ")
endif()


############################################
# OBJECTS

object(Main ""
  ${SRC}/Main.cc
)

object(Log ""
	${INC}/Log.hh
)

object(Bits ""
  ${INC}/Bits.hh
)
test_object(Bits ""
  ${TEST}/BitsTest.cc
)

object(CyclicBarrier ""
  ${INC}/CyclicBarrier.hh
  ${SRC}/CyclicBarrier.cc
)

object(BlockingQueue "" 
  ${INC}/BlockingQueue.hh
)
test_object(BlockingQueue "" 
  ${TEST}/BlockingQueueTest.cc
)

object(Sump "" 
  ${INC}/Sump.hh
)
object(Sink "" 
  ${INC}/Sink.hh
)
object(BlockingSource "" 
  ${INC}/BlockingSource.hh
)

object(BufferPool "memory"
  ${INC}/memory/BufferPool.hh
  ${INC}/memory/PooledBuffer.hh
)
test_object(BufferPool "memory"
  ${TEST}/memory/BufferPoolTest.cc
)

#-------------------------------------------
# msg

object(Message "msg"
  ${INC}/msg/Message.hh 
  ${SRC}/msg/Message.cc
)
test_object(Message "msg" 
  ${TEST}/msg/MessageTest.cc
)

object(Messenger "msg"
  ${INC}/msg/Messenger.hh 
)

#-------------------------------------------
# Channel

object(Channel "Channel"
  ${INC}/Channel/Channel.hh
  ${SRC}/Channel/Channel.cc
)

#-------------------------------------------
# net

object(Socket "net"
  ${INC}/net/Socket.hh
)

object(Address "net"
  ${INC}/net/Address.hh
  ${SRC}/net/Address.cc
)
object(InetAddress "net"
  ${INC}/net/InetAddress.hh
  ${SRC}/net/InetAddress.cc
)
object(SocketAddress "net"
  ${INC}/net/SocketAddress.hh
  ${SRC}/net/SocketAddress.cc
)
test_object(SocketAddress "net"
  ${TEST}/net/SocketAddressTest.cc
)

object(NativeSocket "net"
  ${INC}/net/NativeSocket.hh
  ${SRC}/net/NativeSocket.cc
)
test_object(NativeSocket ""
  ${TEST}/net/NativeSocketTest.cc
)

object(Winsock "net"
  ${INC}/net/Winsock.hh
  ${SRC}/net/Winsock.cc
)


############################################
# TARGETS

include_directories(${INC})

add_test(Bits)
add_test(Message)
add_test(BufferPool    Log)
add_test(NativeSocket	Socket)
add_test(BlockingQueue	CyclicBarrier)
add_test(SocketAddress	Address InetAddress Winsock)

add_exe(Main
  # Utility
  Bits
  BufferPool
  CyclicBarrier

  # Message queueing
  Message
  Messenger
  BlockingQueue

  # Interfaces for connecting messaging to network  
  Sink
  Sump
  BlockingSource
  
  # Cross-platform abstractions
  Socket              # Interface
  Address
  InetAddress
  SocketAddress

  # Interfaces for abstracting platform-specific networking
  Channel

  # BSD/Win32-specific networking
  NativeSocket				  # Berkely sockets/Winsock implementation
  Winsock               # Winsock includes and defines
)