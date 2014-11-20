#pragma once

#include <sma/sink.hpp>


namespace sma
{

  struct message;
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
class channel : public sink<message const&>
{
public:
  channel();
  channel(sink<message const&>* inbox);

  channel(channel&& rhs);
  channel& operator=(channel&& rhs);
  channel(channel const& rhs);
  channel& operator=(channel const& rhs);

  virtual ~channel();

  void inbox(sink<message const&>* inbox) { inbox_ = inbox; }

  // Send the given message to this channel.
  virtual void accept(message const& m) override = 0;
  // Close this channel and all underlying channels.
  virtual void close() = 0;

protected:
  sink<message const&>* inbox_{nullptr};
};
}
