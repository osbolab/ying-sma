#pragma once

#include <sma/message.hpp>
#include <sma/messenger.hpp>
#include <sma/sink.hpp>
#include <sma/concurrent/rwsmutex.hpp>

#include <cstdint>
#include <memory>
#include <atomic>


namespace sma
{
class actor;

class MessageDispatch : public Messenger, public Sink<Message const&>
{
public:
  MessageDispatch();
  MessageDispatch(Sink<Message const&>* outbox);
  MessageDispatch(MessageDispatch&& rhs);
  MessageDispatch& operator=(MessageDispatch&& rhs);

  virtual ~MessageDispatch() {}

  void outbox(Sink<Message const&>* outbox);

  // Sink

  /*! \brief  Distribute the given Message to all handlers subscribed to its
   *          type.
   *
   * The concurrency factor of distribution, that is how many handlers might be
   * active at once, is implementation-defined.
   * Handlers are not allowed to modify the Message and may not expect it to
   * be valid beyond the call stack in which they are called; that is no
   * handler may store the Message and expect it to contain valid data
   * after the handler has returned.
   */
  virtual void accept(Message const& msg) override;

  // Messenger

  virtual Messenger& subscribe(Message::Type type, actor* subscriber) override;
  virtual Messenger& unsubscribe(actor* subscriber) override;
  virtual Messenger& unsubscribe(Message::Type type,
                                 actor* subscriber) override;

  virtual Messenger& forward(Message const& msg) override;

protected:
  using mapping = std::pair<Message::Type, actor*>;
  std::vector<mapping> subs;
  Sink<Message const&>* outbox_;
};


class ConcurrentDispatch final : public MessageDispatch
{
public:
  ConcurrentDispatch();
  ConcurrentDispatch(Sink<Message const&>* outbox);
  ConcurrentDispatch(ConcurrentDispatch&& rhs);
  ConcurrentDispatch& operator=(ConcurrentDispatch&& rhs);

  // Sink

  void accept(Message const& msg) override;

  // Messenger

  Messenger& subscribe(Message::Type type, actor* subscriber) override;
  Messenger& unsubscribe(actor* subscriber) override;
  Messenger& unsubscribe(Message::Type type, actor* subscriber) override;

private:
  // Readers/writer mutex synchronizing the handlers table
  RwsMutex mx;
};
}
