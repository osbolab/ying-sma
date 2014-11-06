#include "nativesocket.hh"
#include "inetaddress.hh"
#include "socketaddress.hh"

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
  uint8_t buf[buf_len];
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

  uint8_t recvbuf[buf_len];
  size_t recv = listener->recv(recvbuf, buf_len);
  recvbuf[buf_len - 1] = 0;

  ASSERT_STREQ(reinterpret_cast<const char*>(buf),
               reinterpret_cast<const char*>(recvbuf));
}
}
