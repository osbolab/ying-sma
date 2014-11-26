#pragma once

#include <sma/message.hpp>

namespace sma
{
/* Given an untyped source Message and a strong message type,
 * read the message data and construct an instance of that type
 * by calling its deserializing constructor.
 * The new object and the original message header are passed to the object
 * reference given in the constructor argument.
 */
template <typename ReceiverT, template <typename> class Formatter>
struct MessageUnmarshaller {
  MessageUnmarshaller(ReceiverT* receiver)
    : receiver(receiver)
  {
  }

  template <typename MessageT>
  void apply(Message&& m)
  {
    // Do unformatting here!
    receiver->receive(MessageT(data));
  }

private:
  ReceiverT* receiver;
};
}
