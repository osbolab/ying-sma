#pragma once

#include <sma/message.hpp>
#include <sma/messenger.hpp>
#include <sma/detail/message_type.hpp>
#include <sma/sink.hpp>
#include <sma/concurrent/rws_mutex.hpp>

#include <cstdint>
#include <memory>
#include <atomic>


namespace sma
{
class actor;

class message_dispatch : public messenger, public sink<message const&>
{
public:
  /*! \brief  Create a new message dispatch that is not thread-safe with regards
   *          to subscribing and dispatching messages.
   */
  static std::unique_ptr<message_dispatch>
  new_single_threaded(sink<message const&>* outbox);
  /*! \brief  Create a new message dispatch that is thread-safe with regard to
   *          susbcribing and unsubscribing handlers and dispatching messages.
   */
  static std::unique_ptr<message_dispatch>
  new_concurrent(sink<message const&>* outbox);

  message_dispatch(message_dispatch&& rhs);
  message_dispatch& operator=(message_dispatch&& rhs);

  virtual ~message_dispatch() {}

  /*! \brief  Distribute the given message to all handlers subscribed to its
   *          type.
   *
   *  The concurrency factor of distribution, that is how many handlers might be
   *  active at once, is implementation-defined.
   *  Handlers are not allowed to modify the message and may not expect it to
   *  be valid beyond the call stack in which they are called; that is no
   *  handler may store the message and expect it to contain valid data
   *  after the handler has returned.
   */
  virtual void accept(message const& msg) override;

  virtual messenger& subscribe(message_type type, actor* subscriber) override;
  virtual messenger& unsubscribe(message_type type, actor* subscriber) override;
  virtual messenger& unsubscribe(actor* subscriber) override;

  virtual messenger& post(message const& msg) override;

protected:
  message_dispatch(sink<message const&>* outbox);

  using mapping = std::pair<message_type, actor*>;
  std::vector<mapping> subs;

  sink<message const&>* outbox{nullptr};
};

namespace detail
{
  class concurrent_dispatch final : public message_dispatch
  {
  public:
    concurrent_dispatch();
    concurrent_dispatch(sink<message const&>* outbox);
    concurrent_dispatch(concurrent_dispatch&& rhs);
    concurrent_dispatch& operator=(concurrent_dispatch&& rhs);

    virtual messenger& subscribe(message_type type, actor* subscriber) override;

    virtual void accept(message const& msg) override;

  private:
    // Readers/writer mutex synchronizing the handlers table
    rws_mutex mx;
  };
}
}
