#pragma once

#include "actor.hh"
#include "message.hh"
#include "channel.hh"
#include "rws_mutex.hh"

#include <cstdint>
#include <functional>


namespace sma
{

class Messenger final
{
public:
  using msg_handler = std::function<void(const Message&) >;


  Messenger(Node::Id this_sender, Channel* outbound)
    : outbound(outbound)
    , this_sender(this_sender)
  {
  }

  Messenger(Messenger&& rhs) = default;
  Messenger& operator=(Messenger&& rhs) = default;

  // Do not block.
  // handler may be called in parallel from any number of threads.
  void subscribe(Message::Type type, msg_handler handler);

  void dispatch(const Message& msg);

  // Block the caller.
  // return 0 on success
  int send(Message builder);

private:
  Messenger(const Messenger& o) = delete;
  Messenger& operator=(const Messenger& o) = delete;

  using mapping = std::pair<Message::Type, msg_handler>;
  std::vector<mapping> vec;
  using vec_type = decltype(handlers);
  using vec_size = vec_type::size_type;
  // Readers/writer mutex for vec
  rws_mutex mx;

  Node::Id this_sender;
  Channel* outbound;

  static const std::size_t SEND_BUFFER_SIZE = 1024;
};
}
