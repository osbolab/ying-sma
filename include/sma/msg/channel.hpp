#pragma once

#include <mutex>
#include <condition_variable>


namespace sma
{

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
 *
 * An ns3 channel should just notify a condition variable when a packet arrives
 * via callback and hold onto it for readers.
 */
class channel
{
public:
  ~channel()
  {
  }

  virtual std::size_t write(const std::uint8_t* src, std::size_t len) = 0;
  // Read up to len bytes from the channel into dst and return the number of
  // bytes read, or block until the channel is readable.
  virtual std::size_t wait_for_read(std::uint8_t* dst, std::size_t len) = 0;

protected:
  // Guarantee that whichever waiting reader is woken gets to read
  std::mutex reader_mutex;
  // Readers are blocked until this is set
  std::condition_variable avail;
};
}
