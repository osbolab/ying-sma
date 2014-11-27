#pragma once

#include <sma/nodeid.hpp>

#include <vector>

namespace sma
{
struct MessageHeader {
  NodeId sender;
  std::vector<NodeId> recipients;

  template <typename Reader>
  MessageHeader(Reader&& r) {
    r >> sender;
    r >> recipients;
  }

  template <typename Writer>
  void write_fields(Writer&& w) const {
    w << sender;
    w << recipients;
  }
};
}
