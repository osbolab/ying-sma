#pragma once

#include "rws_mutex.hh"

#include <chrono>
#include <vector>
#include <thread>
#include <atomic>
#include <iostream>

#include "gtest/gtest.h"


namespace sma
{

TEST(rws_mutex, lock_unlock)
{
  using Clock = std::chrono::high_resolution_clock;

  rws_mutex mx;

  std::vector<std::thread> threads;

  const int nreaders = 4;
  const int nwriters = 1;

  std::atomic_int scratch{0};

  std::atomic_int to_write{1024 * 10};
  std::atomic_int to_read{1024 * 1024};

#if 0
  to_read=0;
#endif
#if 0
  to_write=0;
#endif

  auto start = Clock::now();

  volatile unsigned char src[1024];
  for (int i = 0; i < 1024; ++i)
    src[i] = 0;

  volatile int rol;

  for (int i = 0; i < nreaders; ++i)
    threads.emplace_back([&]() {
      while (to_read > 0) {
        reader_lock lock(mx);
        for (int i = 0; i < 32; ++i)
          if (src[i] == 0xFF)
            std::cout << "hi";
        --to_read;
      }
    });

  volatile int vol;
  for (int i = 0; i < nwriters; ++i)
    threads.emplace_back([&]() {
      while (to_write > 0) {
        writer_lock lock(mx);
        for (int i = 0; i < 1024; ++i)
          src[i] = 1 + (i % 10);
        --to_write;
      }
    });

  for (auto& th : threads)
    if (th.joinable())
      th.join();

  auto time = std::chrono::duration_cast<std::chrono::milliseconds>(
                  Clock::now() - start).count();
  double written = 1024 * 1024 * 10;
  double read = 32 * 1024 * 1024;

  double wps = (written / 1024 / 1024 / time) * 1000.0;
  double rps = (read / 1024 / 1024 / time) * 1000.0;
  std::cout << "wrote " << int(wps) << " Mbps; read " << int(rps) << " Mbps"
            << std::endl;

  EXPECT_GE(0, to_write);
  EXPECT_GE(0, to_read);
}
}
