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
  auto sockaddr = SocketAddress(InetAddress::ANY, port);

  ASSERT_EQ(0, sock->bind(sockaddr));

  const int buf_len = 32;
  char buf[buf_len];
  memset(buf, 'A', buf_len);

  std::size_t sent = sock->send(buf, buf_len,
                                SocketAddress(InetAddress("192.168.0.1"), port));

  ASSERT_EQ(buf_len, sent);

  std::cout << "waiting..." << std::endl;
  std::size_t recv = sock->recv(buf, buf_len);
  std::cout << "received " << recv << " bytes: " << std::string(buf, buf+recv) << std::endl;
}

}