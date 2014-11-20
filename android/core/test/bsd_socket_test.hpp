#include <sma/linux/bsd_socket.hpp>
#include <sma/core/inet_address.hpp>
#include <sma/core/bytes.hpp>

#include "gtest/gtest.h"

#include <iostream>
#include <cassert>
#include <memory>


namespace sma
{

TEST(bsd_socket, bind_send_recv)
{
  int port = 9997;

  const int buf_len = 32;
  std::uint8_t buf[buf_len];
  memset(buf, 'A', buf_len);
  buf[buf_len - 1] = 0;

  auto sock_factory = bsd_socket::Factory();
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


  ASSERT_EQ(0, listener->bind(socket_addr(inet_addr::ANY, port)));
  ASSERT_EQ(
      buf_len,
      sender->send(buf, buf_len, socket_addr(inet_addr::LOOPBACK, port)));

  std::uint8_t recvbuf[buf_len];
  std::size_t recv = listener->recv(recvbuf, buf_len);
  recvbuf[buf_len - 1] = 0;

  ASSERT_STREQ(char_cp(buf), char_cp(recvbuf));
}
}
