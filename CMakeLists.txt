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

object(Sump "" 
  ${INC}/Sump.hh
)
object(Sink "" 
  ${INC}/Sink.hh
)
object(BlockingSource "" 
  ${INC}/BlockingSource.hh
)

object(ObjectPool "memory"
  ${INC}/memory/ObjectPool.hh
  ${INC}/memory/pooled_ptr.hh
)
test_object(ObjectPool "memory" 
  ${TEST}/memory/ObjectPoolTest.cc
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
object(SelectableChannel "channel"
  ${INC}/channel/SelectableChannel.hh
)
object(Selector "channel"
  ${INC}/channel/Selector.hh
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
# net/ns3

object(Ns3Socket "net"
  ${INC}/net/Ns3Socket.hh
  ${SRC}/net/Ns3Socket.cc
)

#-------------------------------------------
# net/bsd

object(BsdSocket "net"
  ${INC}/net/BsdSocket.hh
  ${SRC}/net/BsdSocket.cc
)
object(BsdSocketFactory "net"
  ${INC}/net/BsdSocketFactory.hh
  ${SRC}/net/BsdSocketFactory.cc
)
object(BsdSocketChannel "net"
  ${INC}/net/BsdSocketChannel.hh
  ${SRC}/net/BsdSocketChannel.cc
)

object(Winsock "net"
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

add_test(ObjectPool)

add_exe(Main
  # Utility
  CyclicBarrier
  ObjectPool

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
  SelectableChannel
  Selector

  # BSD-specific networking
  BsdSocket           # Berkely sockets implementation
  BsdSocketChannel    # Abstraction
  Winsock

  # NS3-specific networking
  # Ns3Socket
)

add_exe(SocketsTest
  BsdSocket
  Socket)