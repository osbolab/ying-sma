#pragma once

#include "messenger.hh"
#include "message.hh"
#include "channel.hh"
#include "bytes.hh"

#include <iostream>
#include <cstring>
#include <cstdint>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>

namespace sma
{

class DummyChannel : public Channel
{
  using Lock = std::unique_lock<std::mutex>;

public:
  std::size_t write(const std::uint8_t* src, std::size_t len) override
  {
    if (len > sizeof buf)
      len = sizeof buf;
    {
      Lock lock(mutex);
      std::memcpy(buf, src, len);
      buflen = len;
    }
    avail.notify_one();
    return buflen;
  }

  std::size_t wait_for_read(std::uint8_t* dst, std::size_t len) override
  {
    Lock lock(mutex);
    if (buflen == 0)
      avail.wait(lock, [this]() { return buflen > 0; });
    if (len > buflen)
      len = buflen;

    std::memcpy(dst, buf, len);

    buflen = 0;
    return len;
  }

private:
  std::uint8_t buf[32];
  std::size_t buflen{0};
  std::mutex mutex;
  std::condition_variable avail;
};

TEST(Messenger, movable_callback)
{
  DummyChannel chan;
  Messenger mgr(Message::Address{12792}, &chan);
  volatile int to_send{2};

  std::thread pump([&]() {
    std::uint8_t buf[32];
    std::cout << "  | Dispatch thread started" << std::endl;
    while (to_send) {
      std::cout << "  | Blocking on read" << std::endl;
      std::size_t read = chan.wait_for_read(buf, sizeof buf);
      std::cout << "  | Read " << read << " bytes" << std::endl;
      std::cout << "  |";
      if (read)
        mgr.dispatch(Message(buf, read));
    }
    std::cout << "  | Dispatch thread finished" << std::endl;
  });

  auto on_message = [&](const Message& msg) {
    auto s = copy_string(msg.body());
    std::cout << "> Message type " << std::size_t{msg.type()} << " from "
              << msg.sender() << " to " << *(msg.recipients().begin()) << ": "
              << s << std::endl;
    --to_send;
  };
  mgr.subscribe(Message::Type{182}, on_message);
  mgr.subscribe(Message::Type{179}, on_message);

  using namespace std::literals::chrono_literals;

  std::string sbody("Hello, world!");
  auto body = uint8_cp(sbody);
  auto msg = Message::build(Message::Type{182}, body, sbody.size());
  msg.add(Message::Address{13391});
  std::this_thread::sleep_for(10ms);
  std::cout << "[M] Sending message" << std::endl;
  mgr.send(std::move(msg));

  sbody = "Goodbye, cruel world!";
  body = uint8_cp(sbody);
  auto msg2 = Message::build(Message::Type{179}, body, sbody.size());
  msg2.add(Message::Address{17823});
  std::this_thread::sleep_for(10ms);
  std::cout << "[M] Sending message" << std::endl;
  mgr.send(std::move(msg2));

  std::cout << "[M] Joining" << std::endl;
  pump.join();
}
}
