#include <sma/messagedispatch.hpp>
#include <sma/message.hpp>
#include <sma/actor.hpp>
#include <sma/sink.hpp>
#include <sma/concurrent/rwsmutex.hpp>

#include <sma/io/log>

#include <cstdint>
#include <cassert>
#include <memory>


namespace sma
{

MessageDispatch::MessageDispatch() {}
MessageDispatch::MessageDispatch(Sink<Message const&>* outbox)
  : outbox_(outbox)
{
}
MessageDispatch::MessageDispatch(MessageDispatch&& rhs)
  : subs(std::move(rhs.subs))
  , outbox_(std::move(rhs.outbox_))
{
}
MessageDispatch& MessageDispatch::operator=(MessageDispatch&& rhs)
{
  std::swap(subs, rhs.subs);
  std::swap(outbox_, rhs.outbox_);
  return *this;
}

Messenger& MessageDispatch::forward(Message const& msg)
{
  assert(outbox_);
  outbox_->accept(msg);
  return *this;
}

void MessageDispatch::outbox(Sink<Message const&>* outbox) { outbox_ = outbox; }

Messenger& MessageDispatch::subscribe(Message::Type type, Actor* subscriber)
{
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

Messenger& MessageDispatch::unsubscribe(Message::Type type, Actor* subscriber)
{
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
  auto it = subs.begin();
  while (it != subs.end())
    if (it->second == nullptr || it->second == subscriber)
      it = subs.erase(it);
    else
      ++it;
  return *this;
}

void MessageDispatch::accept(const Message& msg)
{
  bool handled = false;
  if (!subs.empty())
    for (std::size_t i = 0; i < subs.size(); ++i) {
      if (subs[i].first == msg.type()) {
        while (subs[i].first == msg.type())
          subs[i++].second->receive(msg);
        handled = true;
      } else if (handled)
        return;
    }
  if (!handled)
    LOG(WARNING) << "Unhandled " << msg;
}

ConcurrentDispatch::ConcurrentDispatch() {}

ConcurrentDispatch::ConcurrentDispatch(Sink<Message const&>* outbox)
  : MessageDispatch(outbox)
{
}

ConcurrentDispatch::ConcurrentDispatch(ConcurrentDispatch&& r)
  : MessageDispatch(std::move(r))
{
}

ConcurrentDispatch& ConcurrentDispatch::operator=(ConcurrentDispatch&& r)
{
  MessageDispatch::operator=(std::move(r));
  return *this;
}

Messenger& ConcurrentDispatch::subscribe(Message::Type type, Actor* subscriber)
{
  WriterLock lock(mx);
  return MessageDispatch::subscribe(std::move(type), std::move(subscriber));
}

Messenger& ConcurrentDispatch::unsubscribe(Message::Type type,
                                           Actor* subscriber)
{
  WriterLock lock(mx);
  return MessageDispatch::unsubscribe(std::move(type), std::move(subscriber));
}

Messenger& ConcurrentDispatch::unsubscribe(Actor* subscriber)
{
  WriterLock lock(mx);
  return MessageDispatch::unsubscribe(std::move(subscriber));
}

void ConcurrentDispatch::accept(const Message& msg)
{
  ReaderLock lock(mx);
  MessageDispatch::accept(msg);
}
}
