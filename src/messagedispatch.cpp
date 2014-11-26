#include <sma/messagedispatch.hpp>
#include <sma/message.hpp>
#include <sma/actor.hpp>

#include <sma/util/sink.hpp>
#include <sma/concurrent/rwsmutex.hpp>

#include <sma/io/log>

#include <cstdint>
#include <cassert>
#include <memory>


namespace sma
{

MessageDispatch::MessageDispatch() {}
MessageDispatch::MessageDispatch(Sink<Message&&>* outbox)
  : outbox_(outbox)
{
}

void MessageDispatch::outbox(Sink<Message&&>* outbox) { outbox_ = outbox; }

void MessageDispatch::stop()
{
  LOG(TRACE);
  stopped = true;
}

Messenger& MessageDispatch::forward(Message&& msg)
{
  assert(outbox_);
  outbox_->accept(std::move(msg));
  return *this;
}

Messenger& MessageDispatch::subscribe(MessageType type, Actor* subscriber)
{
  if (stopped)
    return *this;
  // We could use a binary search to find the insertion point, but
  // it still then requires an O(n) step to create the gap, so
  // we'd have an O(log) search and an O(n) insert.
  // Instead just do them at the same time by sliding in the back (heh)
  assert(subscriber);

  subs.emplace_back(std::move(type), subscriber);
  auto i = subs.size();
  while (--i > 0 && subs[i].first < subs[i - 1].first)
    std::swap(subs[i - 1], subs[i]);
  return *this;
}

Messenger& MessageDispatch::unsubscribe(MessageType type, Actor* subscriber)
{
  if (stopped)
    return *this;

  bool type_checked = false;
  auto it = subs.begin();
  while (it != subs.end()) {
    if (it->first == type) {
      type_checked = true;
      if (it->second == subscriber)
        it = subs.erase(it);
    } else if (type_checked)
      break;
    ++it;
  }

  return *this;
}

Messenger& MessageDispatch::unsubscribe(Actor* subscriber)
{
  if (stopped)
    return *this;

  auto it = subs.begin();
  while (it != subs.end())
    if (it->second == nullptr || it->second == subscriber)
      it = subs.erase(it);
    else
      ++it;
  return *this;
}

void MessageDispatch::accept(Message&& msg)
{
  if (stopped)
    return;

  bool handled = false;
  if (!subs.empty())
    for (std::size_t i = 0; i < subs.size(); ++i) {
      if (subs[i].first == msg.type) {
        while (!stopped && i < subs.size() && subs[i].first == msg.type) {
          // Copy the message into the first N-1 and move it into the Nth
          if (i + 1 == subs.size() || subs[i + 1].first != msg.type)
            subs[i++].second->receive(std::move(msg));
          else
            subs[i++].second->receive(Message(msg));
        }
        handled = true;
      } else if (handled)
        return;
    }
  if (!handled)
    LOG(WARNING) << "Unhandled " << msg;
}

ConcurrentDispatch::ConcurrentDispatch() {}

ConcurrentDispatch::ConcurrentDispatch(Sink<Message&&>* outbox)
  : MessageDispatch(outbox)
{
}

Messenger& ConcurrentDispatch::subscribe(MessageType type, Actor* subscriber)
{
  WriterLock lock(mx);
  return MessageDispatch::subscribe(std::move(type), std::move(subscriber));
}

Messenger& ConcurrentDispatch::unsubscribe(MessageType type, Actor* subscriber)
{
  WriterLock lock(mx);
  return MessageDispatch::unsubscribe(std::move(type), std::move(subscriber));
}

Messenger& ConcurrentDispatch::unsubscribe(Actor* subscriber)
{
  WriterLock lock(mx);
  return MessageDispatch::unsubscribe(std::move(subscriber));
}

void ConcurrentDispatch::accept(Message&& msg)
{
  ReaderLock lock(mx);
  MessageDispatch::accept(std::move(msg));
}
}
