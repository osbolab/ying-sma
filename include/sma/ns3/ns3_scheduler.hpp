#pragma once

#include <sma/scheduler.hpp>

#include <cstdint>
#include <vector>
#include <memory>
#include <functional>


namespace sma
{
class ns3_scheduler : public scheduler
{
public:
  virtual ~ns3_scheduler();

protected:
  virtual void schedule(std::function<void()> f, millis delay) override;

private:
  struct wrapper
  {
    void operator()();
    std::function<void()> proxied;
  };
};
}
