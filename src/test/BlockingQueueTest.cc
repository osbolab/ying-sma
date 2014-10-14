#include <string>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

#include "BlockingQueue.hh"
#include "CyclicBarrier.hh"
#include "msg/Message.hh"


namespace sma
{

using std::string;
using std::uint8_t;

Message makeMessage()
{
  const string smsg = "Hello, world!";
  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(std::move(smsg.c_str()));
  return Message(bytes, smsg.length() + 1);
}

TEST(Offer_Poll, AssertionTrue)
{
  auto mq = BlockingQueue<Message>(1);
  Message msg = makeMessage();

  ASSERT_TRUE(mq.offer(msg));
  Message msg3;
  ASSERT_TRUE(mq.poll(msg3));
  ASSERT_EQ(msg, msg3);
}

TEST(Offer_Bound_Exceeded, AssertionTrue)
{
  auto mq = BlockingQueue<Message>(1);
  Message msg = makeMessage();

  ASSERT_TRUE(mq.offer(msg));
  ASSERT_FALSE(mq.offer(msg));
}

TEST(Pop_Uncontended, AssertionTrue)
{
  auto mq = BlockingQueue<Message>(1);
  Message msg = makeMessage();

  ASSERT_TRUE(mq.offer(msg));
  Message msg2 = std::move(mq.take());
  ASSERT_EQ(msg, msg2);
}

TEST(Concurrent_Producer_Consumer, AssertionTrue)
{
  const std::size_t numThreads = 3;

  BlockingQueue<Message> mq(numThreads);
  Message msg = makeMessage();

  CyclicBarrier consumption(numThreads, [&] {
    CyclicBarrier production(numThreads);
    for (std::size_t i = 0; i < numThreads; ++i) {
      std::thread th([&] {
        production.wait();
        ASSERT_TRUE(mq.offer(msg));
      });
      th.detach();
    }
  });

  int successes = 0;

  std::vector<std::thread> consumers;
  for (std::size_t i = 0; i < numThreads; ++i) {
    std::thread th([&] { 
      consumption.wait();
      if (msg == mq.take()) ++successes;
    });
    consumers.push_back(std::move(th));
  }

  for (std::thread &th : consumers) {
    th.join();
  }

  ASSERT_EQ(successes, numThreads);
  ASSERT_FALSE(mq.poll(msg));
}

}