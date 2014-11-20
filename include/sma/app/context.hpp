#pragma once

#include <cstdint>

namespace sma
{
class messenger;

struct context {
  std::size_t node_id;
  messenger* msgr;
};
}
