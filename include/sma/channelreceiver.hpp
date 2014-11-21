#pragma once

#include <sma/sink.hpp>

#include <vector>


namespace sma
{
struct Message;

class ChannelReceiver
{
public:
  virtual ~ChannelReceiver() noexcept = 0;

  /*! \brief  Get the sinks that receive messages from this channel.
   */
  SinkSet<Message const&>& inboxes() noexcept { return inboxes_; }

protected:
  SinkSet<Message const&> inboxes_;
};

inline ChannelReceiver::~ChannelReceiver() {}


}
