#pragma once

#include <sma/messagetype.hpp>
#include <sma/forwardpolicy.hpp>
#include <sma/nodeid.hpp>

#include <sma/serial/vector.hpp>
#include <sma/serial/binaryformatter.hpp>

#include <sma/util/buffer.hpp>
#include <sma/util/buffersource.hpp>
#include <sma/util/bufferdest.hpp>

#include <cstdint>
#include <vector>
#include <memory>
#include <iosfwd>
#include <type_traits>

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

  Message(MessageType type, NodeId sender, body_type body);
  Message(Message&& r) = default;
  Message(Message const& r) = default;
  Message& operator=(Message&& r) = default;
  Message& operator=(Message const& r) = default;

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

template <typename Writer>
void Message::write_fields(Writer* out) const
{
  *out << sender;
  *out << vec_writer(&recipients);
  *out << type;
  *out << body;
}


template <typename Buf,
          typename M,
          template <typename> class Formatter = BinaryFormatter>
Buf to_buffer(M&& m)
{
  BufferDest buf;
  buf.format<Formatter>() << std::forward<M>(m);
  return Buf(buf);
}

template <typename M>
Message to_message(NodeId sender, M&& m)
{
  return Message(
      M::TYPE, sender, to_buffer<Message::body_type>(std::forward<M>(m)));
}

template <typename M, typename... Args>
Message make_message(NodeId sender, Args&&... args)
{
  return make_message<M>(std::move(sender), M(std::forward<Args>(args)...));
}


template <typename M>
M read_message(std::uint8_t const* src, std::size_t size)
{
  auto reader = BufferSource(src, size).format<BinaryFormatter>();
  return M(&reader);
}


// Autobots.... ROLL OUT
template <typename M, typename Into>
void receive_into(Message msg, Into& into)
{
  auto body = std::move(msg.body);
  auto m = read_message<M>(body.cdata(), body.size());
  into.receive(std::move(msg), std::move(m));
}



std::ostream& operator<<(std::ostream& os, Message const& m);
}
