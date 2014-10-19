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

if (MSVC)
  include(cmake/msvc.cmake REQUIRED)
  configure_msvc_runtime()
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 ")
endif()


############################################
# OBJECTS

object(Main ""
  ${SRC}/main.cc
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

object(Bits "memory"
  ${INC}/memory/bits.hh
)
test_object(Bits "memory"
  ${TEST}/memory/BitsTest.cc
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
  ${SRC}/memory/BufferPool.cc
  ${INC}/memory/pool_buf.hh
  ${SRC}/memory/pool_buf.cc
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
# channel

object(Channel "channel"
  ${INC}/channel/Channel.hh
  ${SRC}/channel/Channel.cc
)

#-------------------------------------------
# net

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

object(Packet "net"
  ${INC}/net/Packet.hh
  ${SRC}/net/Packet.cc
)

object(Socket "net"
  ${INC}/net/Socket.hh
  ${INC}/net/SocketFactory.hh
)

#-------------------------------------------
# net

object(Ns3Socket "net\\\\ns3"
  ${INC}/net/Ns3Socket.hh
  ${SRC}/net/Ns3Socket.cc
)

object(NativeSocket "net\\\\native"
  ${INC}/net/NativeSocket.hh
  ${SRC}/net/NativeSocket.cc
)
object(NativeSocketFactory "net\\\\native"
  ${INC}/net/NativeSocketFactory.hh
  ${SRC}/net/NativeSocketFactory.cc
)
object(NativeSocketChannel "net\\\\native"
  ${INC}/net/NativeSocketChannel.hh
  ${SRC}/net/NativeSocketChannel.cc
)
object(NativeSocketSelector "net\\\\native"
  ${INC}/net/NativeSocketSelector.hh
  ${SRC}/net/NativeSocketSelector.cc
)

object(Winsock "net\\\\native"
  ${INC}/net/Winsock.hh
  ${SRC}/net/Winsock.cc
)

object(SocketsTest ""
  ${SRC}/SocketsTest.cc
)


############################################
# TARGETS

include_directories(${INC})

add_test(BlockingQueue CyclicBarrier)

add_test(Message)

add_test(SocketAddress
  Address
  Ipv4Address
  Winsock
)

add_test(BufferPool)

add_test(Bits)

add_exe(Main
  # Utility
  CyclicBarrier
  BufferPool

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
  Packet
  Address
  InetAddress
  SocketAddress

  # Interfaces for abstracting platform-specific networking
  Channel

  # BSD-specific networking
  NativeSocket           # Berkely sockets implementation
  NativeSocketChannel    # Abstraction
  NativeSocketSelector
  NativeSocketFactory
  Winsock

  # NS3-specific networking
  # Ns3Socket
)

add_exe(SocketsTest
  NativeSocket
  Socket)