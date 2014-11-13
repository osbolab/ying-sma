#pragma once

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

class messenger : public csink<message>
{
public:
  // Create a new instance of a messenger with no synchronization between
  // subscription and dispatch. The result of performing any operations on this
  // instance from concurrent threads is undefined.
  static std::unique_ptr<messenger>
  new_single_threaded(node::id this_sender, csink<message>* outbound);
  // Create a new instance of a messenger that synchronizes subscription and
  // dispatch so that both can safely occur with any degree of concurrency.
  // Generally this would be implemented with a Readers/Writer lock that allows
  // unlimited concurrent reading, but blocks all readers and writers for each
  // write access.
  static std::unique_ptr<messenger> new_concurrent(node::id this_sender,
                                                   csink<message>* outbound);

  // The callback type for receiving incoming messages.
  using msg_handler = std::function<void(const message&) >;

  messenger(messenger&& rhs);
  messenger& operator=(messenger&& rhs);

  void deliver_to(csink<message>* outbound);

  // Attach the given message handler callback to the message domain such that
  // all incoming messages with that domain are delivered to the handler.
  // Multiple handlers for a domain and duplicate handlers are acceptable.
  virtual void subscribe(message::domain_type domain, msg_handler handler);

  // Distribute the given message to all handlers subscribed to its domain.
  // The concurrency factor of distribution is implementation-defined.
  // Handlers are not allowed to modify the message and may not expect it to
  // be valid beyond the call stack in which they are called; that is no
  // handler may store the message and expect it to contain valid data
  // after the handler has returned.
  virtual void accept(const message& msg);

  // Construct a message from the given incomplete builder and send it to the
  // messenger's sink.
  // Whether this is blocking depends on the implementation of the sink provided
  // to this messenger.
  void send(const message::builder& builder);

protected:
  messenger(node::id this_sender);
  messenger(node::id this_sender, csink<message>* outbound);

  using mapping = std::pair<message::domain_type, msg_handler>;
  std::vector<mapping> handlers;

  std::atomic<message::id_type> next_id{0};
  node::id this_sender;
  csink<message>* outbound{nullptr};
};

namespace detail
{
  class concurrent_messenger final : public messenger
  {
  public:
    concurrent_messenger(node::id this_sender);
    concurrent_messenger(node::id this_sender, csink<message>* outbound);
    concurrent_messenger(concurrent_messenger&& rhs);
    concurrent_messenger& operator=(concurrent_messenger&& rhs);

    virtual void subscribe(message::domain_type domain,
                           msg_handler handler) override;
    virtual void accept(const message& msg) override;

  private:
    // Readers/writer mutex synchronizing the handlers table
    rws_mutex mx;
  };
}
}
