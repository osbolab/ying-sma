#include <string>
#include <cstdlib>
#include <utility>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

#include "BlockingQueue.hh"
#include "CyclicBarrier.hh"


namespace sma
{

using std::string;

TEST(Offer_Poll, AssertionTrue)
{
  auto mq = BlockingQueue<string>(1);
  string msg("Hello, world!");

  ASSERT_TRUE(mq.offer(msg));
  string msg2;
  ASSERT_TRUE(mq.poll(msg2));
  ASSERT_EQ(msg, msg2);
}

TEST(Offer_Bound_Exceeded, AssertionTrue)
{
  auto mq = BlockingQueue<string>(1);
  string msg("Hello, world!");

  ASSERT_TRUE(mq.offer(msg));
  ASSERT_FALSE(mq.offer(msg));
}

TEST(Pop_Uncontended, AssertionTrue)
{
  auto mq = BlockingQueue<string>(1);
  string msg("Hello, world!");

  ASSERT_TRUE(mq.offer(msg));
  string msg2 = std::move(mq.take());
  ASSERT_EQ(msg, msg2);
}

TEST(Concurrent_Producer_Consumer, AssertionTrue)
{
  const std::size_t numThreads = 3;

  auto mq = BlockingQueue<string>(numThreads);
  string msg("Hello, world!");

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
      if (mq.take() == msg) ++successes;
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