#pragma once

#include <sma/core/message.hpp>
#include <sma/core/node.hpp>
#include <sma/core/channel.hpp>
#include <sma/core/rws_mutex.hpp>

#include <cstdint>
#include <functional>


namespace sma
{

class messenger final
{
public:
  using msg_handler = std::function<void(const message&) >;


  messenger(node::id this_sender, Channel* outbound)
    : outbound(outbound)
    , this_sender(this_sender)
  {
  }

  messenger(messenger&& rhs) = default;
  messenger& operator=(messenger&& rhs) = default;

  // Do not block.
  // handler may be called in parallel from any number of threads.
  void subscribe(message::domain_type domain, msg_handler handler);

  void dispatch(const message& msg);

  // Block the caller.
  // return 0 on success
  int send(message builder);

private:
  messenger(const messenger& o) = delete;
  messenger& operator=(const messenger& o) = delete;

  using mapping = std::pair<message::domain_type, msg_handler>;
  std::vector<mapping> handlers;
  // Readers/writer mutex for handlers
  rws_mutex mx;

  node::id this_sender;
  Channel* outbound;

  static const std::size_t SEND_BUFFER_SIZE = 1024;
};
}
