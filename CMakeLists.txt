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

#add_definitions(-std=c++11)
# add_definitions(-Wall)

if (MSVC)
  include(cmake/msvc.cmake REQUIRED)
  configure_msvc_runtime()
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

object(BlockingQueue "" ${INC}/BlockingQueue.hh)
test_object(BlockingQueue "" ${TEST}/BlockingQueueTest.cc)

object(Sump "" ${INC}/Sump.hh)
object(Sink "" ${INC}/Sink.hh)
object(BlockingSource "" ${INC}/BlockingSource.hh)

#-------------------------------------------
# msg

object(Message "msg"
  ${INC}/msg/Message.hh 
  ${SRC}/msg/Message.cc
)
test_object(Message "msg" ${TEST}/msg/MessageTest.cc)

#-------------------------------------------
# channel

object(Channel "channel"
  ${INC}/channel/Channel.hh
)
object(SelectableChannel "channel"
  ${INC}/channel/SelectableChannel.hh
)
object(Selector "channel"
  ${INC}/channel/Selector.hh
)

#-------------------------------------------
# net

object(Packet "net"
  ${INC}/net/Packet.hh
  ${SRC}/net/Packet.cc
)
object(Socket "net"
  ${INC}/net/Socket.hh
  ${SRC}/net/Socket.cc
)
object(UdpSocket "net"
  ${INC}/net/UdpSocket.hh
  ${SRC}/net/UdpSocket.cc
)

#-------------------------------------------
# net/ns3

object(Ns3UdpSocketImpl "net"
  ${INC}/net/Ns3UdpSocketImpl.hh
  ${SRC}/net/Ns3UdpSocketImpl.cc
)

#-------------------------------------------
# net/bsd

object(BsdSocket "net"
  ${INC}/net/BsdSocket.hh
)
object(BsdUdpSocketImpl "net"
  ${INC}/net/BsdUdpSocketImpl.hh
  ${SRC}/net/BsdUdpSocketImpl.cc
)
object(BsdSocketChannel "net"
  ${INC}/net/BsdSocketChannel.hh
  ${SRC}/net/BsdSocketChannel.cc
)


############################################
# TARGETS

include_directories(${INC})

add_test(BlockingQueue CyclicBarrier)
add_test(Message)

add_exe(Main
  Message
  CyclicBarrier
  Sink
  BlockingSource
  BlockingQueue
  Sump
  Message
  Packet
  Socket
  UdpSocket
  BsdSocket
  BsdUdpSocketImpl
  Ns3UdpSocketImpl
  Channel
  SelectableChannel
  Selector
  BsdSocketChannel
)