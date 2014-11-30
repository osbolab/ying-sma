#pragma once

namespace sma
{
struct BroadcastRejectPolicy
{
  bool loopback = true;
  bool not_addressed_to_me = true;
};
}
