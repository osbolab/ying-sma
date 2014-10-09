#include "msg/MessageQueue.hh"

using std::unique_lock;
using std::unique_ptr;

namespace sma
{

MessageQueue::MessageQueue(const MessageQueue& copy) {
}

MessageQueue::MessageQueue(MessageQueue&& move) {
}

MessageQueue& MessageQueue::operator =(MessageQueue copy) {
  return *this;
}

MessageQueue& MessageQueue::operator =(MessageQueue&& move) {
  return *this;
}

bool MessageQueue::offer(Message& message) {
  { // Release the lock before notifying so take() is not blocked
    unique_lock<std::mutex> lock(mutex);
    messages.push_front(message);
  }
  messageAvailable.notify_one();
}

unique_ptr<Message> MessageQueue::poll() {
  unique_lock<std::mutex> lock(mutex);
  if (messages.empty()) return unique_ptr<Message>(nullptr);
  Message msg(std::move(messages.back()));
  messages.pop_back();
  return unique_ptr<Message>(&msg);
}

unique_ptr<Message> MessageQueue::take() {
  unique_lock<std::mutex> lock(mutex);
  messageAvailable.wait(lock, [=] { return !messages.empty(); });
  Message msg(std::move(messages.back()));
  messages.pop_back();
  return unique_ptr<Message>(&msg);
}

}