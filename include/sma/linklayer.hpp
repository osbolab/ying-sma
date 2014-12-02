#pragma once

#include <sma/messagetypes.hpp>
#include <sma/messageheader.hpp>

#include <sma/util/binaryformat.hpp>
#include <sma/util/bufferdest.hpp>

namespace sma
{
class CcnNode;

class LinkLayer
{
  void receive_to(CcnNode& node);

  template <typename M>
  void enqueue(MessageHeader const& header, M const& msg);

protected:
  CcnNode* node{nullptr};

private:
  virtual BufferDest& message_buffer() = 0;
};

template <typename M>
void LinkLayer::enqueue(MessageHeader const& header, M const& msg)
{
  auto& buf = message_buffer();
  // clang-format off
  buf.format<BinaryOutput>()
    << header
    << MessageTypes::typecode<M>()
    << msg;
  // clang-format on
}
}
