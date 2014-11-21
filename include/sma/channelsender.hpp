#pragma once

#include <sma/sink.hpp>
#include <sma/message.hpp>


namespace sma
{
class ChannelSender : public Sink<Message const&>
{
public:
  virtual ~ChannelSender() = 0;
};

inline ChannelSender::~ChannelSender() {}
}
