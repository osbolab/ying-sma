#pragma once

#include <sma/linux/select_channel.hpp>
#include <sma/linux/bsd_socket.hpp>
#include <sma/core/inet_address.hpp>
#include <sma/core/bytes.hpp>

#include "gtest/gtest.h"

#include <thread>
#include <string>
#include <chrono>


namespace sma
{

TEST(select_channel, select_one_sockets)
{
  int port = 9997;

  const int buf_len = 32;
  std::uint8_t buf[buf_len];
  memset(buf, 'A', buf_len);
  buf[buf_len - 1] = 0;

  auto sock_factory = bsd_socket::Factory();
  std::unique_ptr<Socket> listener;
  std::unique_ptr<Socket> listener2;
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
                                listener2));

  ASSERT_EQ(0,
            sock_factory.create(Address::Family::IPv4,
                                Socket::Type::Datagram,
                                Socket::Protocol::Udp,
                                sender));

  ASSERT_EQ(0, listener->bind(socket_addr(inet_addr::ANY, port)));
  ASSERT_EQ(0, listener2->bind(socket_addr(inet_addr::ANY, port + 1)));

  std::vector<bsd_socket*> sockets;
  sockets.push_back(dynamic_cast<bsd_socket*>(listener.get()));
  sockets.push_back(dynamic_cast<bsd_socket*>(listener2.get()));
  select_channel channel(sockets);

  volatile int to_finish = 10;
  volatile int waiting = 0;


  std::vector<std::thread> threads;
  for (int i = 0; i < 2; ++i) {
    threads.emplace_back([&, i]() {
      while (--to_finish > 0) {
        std::uint8_t buf[48];
        ++waiting;
        std::size_t read = channel.wait_for_read(buf, 48);
        --waiting;
        buf[read] = 0;
        LOG(DEBUG) << "thread " << i << " read " << read
                   << " bytes from channel! "
                   << copy_string(buf));
        LOG(DEBUG) << to_finish << " to go";
      }
    });
  }


  std::thread thsend([&]() {
    for (int i = 0; i < 11; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      LOG(DEBUG) << "sending " << buf_len << " bytes from background thread";
      sender->send(
          buf, buf_len, socket_addr(inet_addr::LOOPBACK, port + (i % 2)));
    }
  });

  while (to_finish > 0 || waiting > 0) {
    LOG(DEBUG) << "blocking main thread on select";
    channel.select();
    LOG(DEBUG) << "selected! joining read thread";
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  thsend.join();
  for (auto& th : threads)
    th.join();
}
}
