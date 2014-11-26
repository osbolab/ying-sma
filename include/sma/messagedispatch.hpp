#pragma once

#include <sma/io/log>
#include <sma/message.hpp>
#include <sma/messenger.hpp>
#include <sma/util/sink.hpp>
#include <sma/concurrent/rwsmutex.hpp>

#include <cstdint>
#include <memory>
#include <atomic>


namespace sma
{
class Actor;

class MessageDispatch : public Messenger, public Sink<Message&&>
{
public:
  MessageDispatch();
  MessageDispatch(Sink<Message&&>* outbox);
  MessageDispatch(MessageDispatch const& rhs) = delete;
  MessageDispatch& operator=(MessageDispatch const& rhs) = delete;

  virtual ~MessageDispatch() { LOG(TRACE); }

  void outbox(Sink<Message&&>* outbox);

  void stop();

  // Sink

  /*! \brief  Distribute the given Message to all handlers subscribed to its
   *          type.
   *
   * The concurrency fActor of distribution, that is how many handlers might be
   * active at once, is implementation-defined.
   * Handlers are not allowed to modify the Message and may not expect it to
   * be valid beyond the call stack in which they are called; that is no
   * handler may store the Message and expect it to contain valid data
   * after the handler has returned.
   */
  virtual void accept(Message&& msg) override;

  // Messenger

  virtual Messenger& subscribe(MessageType type, Actor* subscriber) override;
  virtual Messenger& unsubscribe(Actor* subscriber) override;
  virtual Messenger& unsubscribe(MessageType type, Actor* subscriber) override;

  virtual Messenger& forward(Message&& msg) override;

protected:
  bool stopped{false};
  using mapping = std::pair<MessageType, Actor*>;
  std::vector<mapping> subs;
  Sink<Message&&>* outbox_;
};


class ConcurrentDispatch final : public MessageDispatch
{
public:
  ConcurrentDispatch();
  ConcurrentDispatch(Sink<Message&&>* outbox);
  ConcurrentDispatch(ConcurrentDispatch const& rhs) = delete;
  ConcurrentDispatch& operator=(ConcurrentDispatch const& rhs) = delete;

  // Sink

  void accept(Message&& msg) override;

  // Messenger

  Messenger& subscribe(MessageType type, Actor* subscriber) override;
  Messenger& unsubscribe(Actor* subscriber) override;
  Messenger& unsubscribe(MessageType type, Actor* subscriber) override;

private:
  // Readers/writer mutex synchronizing the handlers table
  RwsMutex mx;
};
}
