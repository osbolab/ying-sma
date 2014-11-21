#pragma once

#include <sma/async.hpp>

#include <chrono>


namespace sma
{
class Ns3Async final : public Async
{
  template<typename F, typename... A>
  friend class Async::Task;

public:
  virtual ~Ns3Async() {}

protected:
  virtual void schedule(std::function<void()> f,
                        std::chrono::nanoseconds delay) override;
};
}
