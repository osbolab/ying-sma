#pragma once

#include <sma/messenger.hpp>
#include <sma/message.hpp>
#include <sma/sink.hpp>
#include <sma/rws_mutex.hpp>

#include <cstdint>
#include <memory>
#include <atomic>


namespace sma
{
class message_dispatch : public messenger, public sink<message const&>
{
public:
  // Create a new instance of a message_dispatch with no synchronization between
  // subscription and dispatch. The result of performing any operations on this
  // instance from concurrent threads is undefined.
  static std::unique_ptr<message_dispatch> new_single_threaded();
  // Create a new instance of a message_dispatch that synchronizes subscription
  // and
  // dispatch so that both can safely occur with any degree of concurrency.
  // Generally this would be implemented with a Readers/Writer lock that allows
  // unlimited concurrent reading, but blocks all readers and writers for each
  // write access.
  static std::unique_ptr<message_dispatch> new_concurrent();

  message_dispatch(message_dispatch&& rhs);
  message_dispatch& operator=(message_dispatch&& rhs);

  // Set the outgoing message channel.
  message_dispatch& post_via(sink<message const&>* outbound);
  // Distribute the given message to all handlers subscribed to its type.
  // The concurrency factor of distribution is implementation-defined.
  // Handlers are not allowed to modify the message and may not expect it to
  // be valid beyond the call stack in which they are called; that is no
  // handler may store the message and expect it to contain valid data
  // after the handler has returned.
  virtual void accept(message const& msg) override;

  virtual messenger& subscribe(message_type type, actor* subscriber) override;

  virtual messenger& post(message const& msg) override;

protected:
  message_dispatch(sink<message const&>* outbound);
  message_dispatch();

  using mapping = std::pair<message_type, actor*>;
  std::vector<mapping> subs;

  sink<message const&>* outbound{nullptr};
};

namespace detail
{
  class concurrent_dispatch final : public message_dispatch
  {
  public:
    concurrent_dispatch();
    concurrent_dispatch(sink<message const&>* outbound);
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
