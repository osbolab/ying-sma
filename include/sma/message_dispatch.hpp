#pragma once

#include <sma/messenger.hpp>
#include <sma/message.hpp>
#include <sma/sink.hpp>
#include <sma/node.hpp>
#include <sma/rws_mutex.hpp>

#include <cstdint>
#include <memory>
#include <atomic>
#include <functional>


namespace sma
{
class message_dispatch : public messenger, public csink<message>
{
public:
  // Create a new instance of a message_dispatch with no synchronization between
  // subscription and dispatch. The result of performing any operations on this
  // instance from concurrent threads is undefined.
  static std::unique_ptr<message_dispatch>
  new_single_threaded(node::id this_sender);
  // Create a new instance of a message_dispatch that synchronizes subscription
  // and
  // dispatch so that both can safely occur with any degree of concurrency.
  // Generally this would be implemented with a Readers/Writer lock that allows
  // unlimited concurrent reading, but blocks all readers and writers for each
  // write access.
  static std::unique_ptr<message_dispatch> new_concurrent(node::id this_sender);

  message_dispatch(message_dispatch&& rhs);
  message_dispatch& operator=(message_dispatch&& rhs);

  // Set the outgoing message channel.
  message_dispatch& post_via(csink<message>* outbound);
  // Distribute the given message to all handlers subscribed to its domain.
  // The concurrency factor of distribution is implementation-defined.
  // Handlers are not allowed to modify the message and may not expect it to
  // be valid beyond the call stack in which they are called; that is no
  // handler may store the message and expect it to contain valid data
  // after the handler has returned.
  virtual void accept(const message& msg) override;

  virtual messenger& subscribe(message::domain_type domain,
                               messenger::handler h) override;
  virtual messenger& post(const message::stub& stub) override;

protected:
  message_dispatch(node::id this_sender);
  message_dispatch(node::id this_sender, csink<message>* outbound);

  using mapping = std::pair<message::domain_type, messenger::handler>;
  std::vector<mapping> handlers;

  std::atomic<message::id_type> next_id{0};
  node::id this_sender;
  csink<message>* outbound{nullptr};
};

namespace detail
{
  class concurrent_dispatch final : public message_dispatch
  {
  public:
    concurrent_dispatch(node::id this_sender);
    concurrent_dispatch(node::id this_sender, csink<message>* outbound);
    concurrent_dispatch(concurrent_dispatch&& rhs);
    concurrent_dispatch& operator=(concurrent_dispatch&& rhs);

    virtual messenger& subscribe(message::domain_type domain,
                           messenger::handler handler) override;
    virtual void accept(const message& msg) override;

  private:
    // Readers/writer mutex synchronizing the handlers table
    rws_mutex mx;
  };
}
}
