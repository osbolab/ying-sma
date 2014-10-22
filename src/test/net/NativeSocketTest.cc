#include <iostream>
#include <cassert>
#include <memory>

#include "gtest/gtest.h"

#include "net/NativeSocket.hh"
#include "net/InetAddress.hh"
#include "net/SocketAddress.hh"


namespace sma
{

TEST(Create_Socket, AssertionTrue)
{
  auto sock_factory = NativeSocket::Factory();

  std::unique_ptr<Socket> sock;
  int result = sock_factory.create(Address::Family::IPv4, Socket::Type::Datagram,
                                   Socket::Protocol::Udp, sock);

  ASSERT_EQ(0, result);


  int port = 9997;
  auto sockaddr = SocketAddress(InetAddress("127.0.0.1"), port);

  ASSERT_EQ(0, sock->bind(sockaddr));
}

}