#include "net/NativeSocket.hh"
#include "net/InetAddress.hh"
#include "net/SocketAddress.hh"

#include "gtest/gtest.h"

#include <iostream>
#include <cassert>
#include <memory>


namespace sma
{

TEST(NativeSocket, bind_send_recv)
{
  int port = 9997;

  const int buf_len = 32;
  char buf[buf_len];
  memset(buf, 'A', buf_len);
  buf[buf_len - 1] = 0;

  auto sock_factory = NativeSocket::Factory();
  std::unique_ptr<Socket> listener;
  std::unique_ptr<Socket> sender;


  ASSERT_EQ(0,
            sock_factory.create(Address::Family::IPv4,
                                Socket::Type::Datagram,
                                Socket::Protocol::Udp,
                                listener));

  ASSERT_EQ(0,
            sock_factory.create(Address::Family::IPv4,
                                Socket::Type::Datagram,
                                Socket::Protocol::Udp,
                                sender));


  ASSERT_EQ(0, listener->bind(SocketAddress(InetAddress::ANY, port)));
  ASSERT_EQ(
      buf_len,
      sender->send(buf, buf_len, SocketAddress(InetAddress::LOOPBACK, port)));

  char recvbuf[buf_len];
  std::size_t recv = listener->recv(recvbuf, buf_len);
  recvbuf[buf_len - 1] = 0;

  ASSERT_STREQ(buf, recvbuf);
}
}
