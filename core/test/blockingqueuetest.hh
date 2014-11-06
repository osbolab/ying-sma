#include "blockingqueue.hh"
#include "cyclicbarrier.hh"

#include "gtest/gtest.h"

#include <string>
#include <cstdlib>
#include <utility>
#include <thread>
#include <vector>


namespace sma
{

TEST(BlockingQueue, offer_poll)
{
  auto mq = BlockingQueue<std::string>(1);
  std::string msg("Hello, world!");

  ASSERT_TRUE(mq.offer(msg));
  std::string msg2;
  ASSERT_TRUE(mq.poll(msg2));
  ASSERT_EQ(msg, msg2);
}

TEST(BlockingQueue, offer_exceeds_bounds)
{
  auto mq = BlockingQueue<std::string>(1);
  std::string msg("Hello, world!");

  ASSERT_TRUE(mq.offer(msg));
  ASSERT_FALSE(mq.offer(msg));
}

TEST(BlockingQueue, take_uncontended)
{
  auto mq = BlockingQueue<std::string>(1);
  std::string msg("Hello, world!");

  ASSERT_TRUE(mq.offer(msg));
  std::string msg2 = std::move(mq.take());
  ASSERT_EQ(msg, msg2);
}

TEST(BlockingQueue, concurrent_producer_consumer)
{
  const std::size_t nr_threads = 3;

  auto mq = BlockingQueue<std::string>(nr_threads);
  std::string msg("Hello, world!");

  CyclicBarrier production(nr_threads);
  CyclicBarrier consumption(nr_threads, [&] {
    for (std::size_t i = 0; i < nr_threads; ++i) {
      std::thread th([&] {
        production.wait();
        ASSERT_TRUE(mq.offer(msg));
      });
      th.detach();
    }
  });

  int successes = 0;

  std::vector<std::thread> consumers;
  for (std::size_t i = 0; i < nr_threads; ++i) {
    std::thread th([&] {
      consumption.wait();
      if (mq.take() == msg)
        ++successes;
    });
    consumers.push_back(std::move(th));
  }

  for (std::thread& th : consumers) {
    th.join();
  }

  ASSERT_EQ(successes, nr_threads);
  ASSERT_FALSE(mq.poll(msg));
}
}
