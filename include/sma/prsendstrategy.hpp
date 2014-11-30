#pragma once

#include <sma/sendstrategy.hpp>

#include <mutex>
#include <chrono>
#include <random>

namespace sma
{
class LinkLayer;
struct Context;

class PrSendStrategy : public SendStrategy
{
public:
  PrSendStrategy(Context& context);

  virtual ~PrSendStrategy() {}

  virtual void notify() override;

private:
  using Lock = std::lock_guard<std::recursive_mutex>;

  void schedule_timeslot();
  void do_timeslot();

  std::chrono::nanoseconds timestep = std::chrono::nanoseconds(100000);
  double threshold_min = 0.0;
  double threshold_max = 0.8;

  std::recursive_mutex mx;
  bool is_scheduled = false;

  std::uniform_real_distribution<> distribute;

  Context* context;
};
}
