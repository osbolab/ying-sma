#pragma once

#include <sma/messagetype.hpp>
#include <sma/forwardpolicy.hpp>
#include <sma/nodeid.hpp>

#include <sma/util/buffer.hpp>
#include <sma/serial/vector.hpp>

#include <cstdint>
#include <vector>
#include <memory>
#include <iosfwd>

namespace sma
{
struct Message final {
  using recp_count = std::uint8_t;
  using size_type = std::uint16_t;
  using body_type = Buffer<size_type>;

  /****************************************************************************
   * Serialized fields - order matters!
   * (12.6.2 says we can do this, leave me alone)
   */
  NodeId sender;
  std::vector<NodeId> recipients;
  MessageType type;
  body_type body;
  /***************************************************************************/

  /****************************************************************************
   * Transient fields
   */
  ForwardPolicy policy;
  /***************************************************************************/

  Message(NodeId sender,
          MessageType type,
          body_type body,
          ForwardPolicy policy);

  Message(Message const& r);
  Message& operator=(Message const& r);
  Message(Message&& r);
  Message& operator=(Message&& r);

  // Serialization

  template <typename Reader>
  Message(Reader* in);

  template <typename Writer>
  void write_fields(Writer* out) const;

private:
  using vec_writer = VectorWriter<NodeId, recp_count>;
  using vec_reader = VectorReader<NodeId, recp_count>;
};

template <typename Reader>
Message::Message(Reader* in)
  : sender(in->template get<decltype(sender)>())
  , recipients(vec_reader::read(in))
  , type(in->template get<decltype(type)>())
  , body(in->template get<decltype(body)>())
{
}

Message::Message(NodeId sender,
                 MessageType type,
                 body_type body,
                 ForwardPolicy policy)
  : sender(std::move(sender))
  , type(type)
  , body(std::move(body))
  , policy(std::move(policy))
{
}

template <typename Writer>
void Message::write_fields(Writer* out) const
{
  *out << sender;
  *out << vec_writer(&recipients);
  *out << type;
  *out << body;
}

std::ostream& operator<<(std::ostream& os, Message const& m);
}
