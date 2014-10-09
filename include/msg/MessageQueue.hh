#ifndef MESSAGE_QUEUE_H_
#define MESSAGE_QUEUE_H_

#include <mutex>
#include <condition_variable>
#include <deque>
#include <memory>

#include "msg/Message.hh"

namespace sma
{

class MessageQueue {
public:
  MessageQueue(const MessageQueue& copy);
  MessageQueue(MessageQueue&& move);

  MessageQueue& operator =(MessageQueue copy);
  MessageQueue& operator =(MessageQueue&& move);

  bool offer(Message& message);
  std::unique_ptr<Message> poll();
  std::unique_ptr<Message> take();

private:
  std::mutex                mutex;
  std::condition_variable   messageAvailable;
  std::deque<Message>       messages;
};

}

#endif
