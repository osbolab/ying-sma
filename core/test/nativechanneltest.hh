#pragma once

#include "nativechannel.hh"
#include "nativesocket.hh"
#include "inetaddress.hh"
#include "socketaddress.hh"

#include "gtest/gtest.h"

#include <thread>
#include <string>
#include <chrono>


namespace sma
{

TEST(NativeChannel, select_one_sockets)
{
  int port = 9997;

  const int buf_len = 32;
  std::uint8_t buf[buf_len];
  memset(buf, 'A', buf_len);
  buf[buf_len - 1] = 0;

  auto sock_factory = NativeSocket::Factory();
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

  ASSERT_EQ(0, listener->bind(SocketAddress(InetAddress::ANY, port)));
  ASSERT_EQ(0, listener2->bind(SocketAddress(InetAddress::ANY, port + 1)));

  std::vector<NativeSocket*> sockets;
  sockets.push_back(dynamic_cast<NativeSocket*>(listener.get()));
  sockets.push_back(dynamic_cast<NativeSocket*>(listener2.get()));
  NativeChannel channel(sockets);

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
                   << std::string(reinterpret_cast<const char*>(buf));
        LOG(DEBUG) << to_finish << " to go";
      }
    });
  }


  std::thread thsend([&]() {
    for (int i = 0; i < 11; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      LOG(DEBUG) << "sending " << buf_len << " bytes from background thread";
      sender->send(
          buf, buf_len, SocketAddress(InetAddress::LOOPBACK, port + (i%2)));
    }
  });

  while (to_finish>0 || waiting>0) {
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
