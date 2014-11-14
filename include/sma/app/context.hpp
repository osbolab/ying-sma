#pragma once

#include <cstdint>

namespace sma
{
class messenger;
class scheduler;

struct context {
  std::size_t node_id;
  messenger* msgr;
  scheduler* sched;
};
}
