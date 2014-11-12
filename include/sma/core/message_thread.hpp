#ifndef MESSAGE_THREAD_H_
#define MESSAGE_THREAD_H_


#include <sma/core/message.hpp>
#include <sma/core/messenger.hpp>
#include <sma/core/channel.hpp>

#include <cstdlib>
#include <cstdint>
#include <mutex>
#include <thread>

namespace sma
{

/**
 * The message thread constantly tries to get a message from the assigned
 * channel and dispatches it via the messenger's handler table.
 * The thread loop therefore is:
 *  1. Lock channel
 *  2. Wait until channel is not blocked
 *  3. Check if any socket is ready for reading
 *    no:   A. set blocked flag and enter select() wait
 *          B. go to 3
 *    yes:  A. remove one socket and unlock channel
 *          ---> (another thread can now enter 1)
 *          B. read from socket into thread-local buffer
 *          C. parse header and go to messenger with dispatch info
 *          D. pick a handler or, if none, go to 1
 *          E. execute handler and block
 *          F. return to 1 for a new message
 * If handlers are reasonably short and message wait times are reasonably
 * long then we could expect two threads to maximize usage as one is always
 * blocked on reading while one is always executing a handler. If too many
 * threads are in this loop then they'll contend over selecting a socket and
 * locking the channel and it'll be worse than a smaller number of threads.
 * Two is probably reasonable to maximize throughput.
 *
 * The only thread-local state necessary is:
 *  - the thread's read channel
 *  - the messenger with the handler dispatch table
 *  - the current message buffer, which lives for the duration of the handler.
 * Letting the thread do the socket read into a field, and only guaranteeing
 * its lifetime during the handler execution, means we can process messages
 * concurrently with zero allocations.
 */
class MessageThread
{
public:
  MessageThread();

  const Message& cmsg() const { return msg; }

private:
  // UDP packets should be pretty small to avoid fragmentation.
  // Random stackoverflow sources suggest 512-540 bytes.
  static const std::size_t MESSAGE_BUFFER_SIZE = 1024;

  std::thread th;
  std::uint8_t buf[MESSAGE_BUFFER_SIZE];
  std::size_t len{0};
  // Constructed from the contents of buf after reading
  Message msg;

  // Get our messages from here
  Channel* channel;
  // and send them here
  Messenger* messenger;
};
}

#endif
