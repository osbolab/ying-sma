#pragma once

#include "node.hh"
#include "bytebuffer.hh"

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <type_traits>


namespace sma
{

class Messenger;

struct Message {
  friend class Messenger;

  using Type = std::uint8_t;

  Message(const ByteView& csrc);

  Message(const Message& rhs) = delete;
  Message& operator=(const Message& rhs) = delete;


  std::size_t write_to(ByteBuffer& dst) const;
  std::size_t size() const
  {
    return Node::Id::size * (recipients_.size() + 1) + sizeof(Type)
           + body.limit();
  }

  Node::Id sender() const { return sender_; }
  const std::vector<Node::Id> recipients() const { return recipients_; }
  Type type() const { return type_; }

  bool operator==(const Message& other) const;
  bool operator!=(const Message& other) const { return !(*this == other); }

protected:
  virtual void get_fields(ByteView& src);

private:
  Message(Type type,
          Node::Id sender,
          std::vector<Node::Id> recipients,
          ByteView body);

  // In packet order
  Node::Id sender_;

  using field_nrecp = std::uint8_t;
  std::vector<Node::Id> recipients_;

  Type type_{0};

  using field_body_len = std::uint16_t;
  ByteView body;
};
}
