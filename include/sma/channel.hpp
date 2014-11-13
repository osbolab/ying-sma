#pragma once

#include <sma/sink.hpp>


namespace sma
{

  class message;
  class messenger;
/**
 * The channel is the interface between messaging and the underlying transport.
 * Messaging doesn't need to know from where it's reading a message or how it's
 * being blocked. Native sockets block by select() while ns3 signals with
 * callbacks; either way the message thread can block on the channel until one
 * of those happens.
 *
 * A select() channel can group any kind of sockets with other endpoints like
 * bluetooth if those endpoints implement some selectable descriptor like a
 * memory pipe. A bluetooth "socket" could write a single byte to the pipe when
 * data are available, for example.
 */
class channel : public csink<message>
{
public:
  channel();
  channel(csink<message>* inbound);

  channel(channel&& rhs);
  channel& operator=(channel&& rhs);
  channel(const channel& rhs);
  channel& operator=(const channel& rhs);

  virtual ~channel();

  virtual void deliver_to(csink<message>* inbound) = 0;

  // Send the given message to this channel.
  virtual void accept(const message& m) override = 0;
  // Close this channel and all underlying channels.
  virtual void close() = 0;

protected:
  csink<message>* inbound{nullptr};
};
}
