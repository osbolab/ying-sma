#pragma once

#include "node.hh"

#include <cstdint>
#include <vector>


namespace sma
{

class Messenger;

/**
 * A lightweight wrapper around serialized message content that attaches
 * low-level network routing information, namely the message's sending
 * node and optional recipient nodes. Also specifies parameters on which
 * the message can be sorted for delivery at its destination node.
 */
struct Message {
  using Type = std::uint8_t;
  using Id = std::uint64_t;

  class Builder
  {
  };

  Message(bytebuf::view src);
  Message(const std::uint8_t* src, std::size_t len);

  Message(const Message& rhs) = delete;
  Message& operator=(const Message& rhs) = delete;


  std::size_t total_len() const;
  std::size_t write_to(bytebuf dst) const;
  std::size_t write_to(std::uint8_t* dst, std::size_t len) const;

  const Node::Id& sender() const { return senderid; }
  const std::vector<Node::Id>& recipients() const { return recpids; }
  Type type() const { return t; }
  const std::uint8_t* cdata() const { return data; }
  std::size_t data_len() const { return len; }

private:
  // Populate a message object with a pointer to its content array.
  // Does *not* copy the array or transfer ownership of any memory.
  Message(Type type,
          Node::Id sender,
          std::vector<Node::Id> recpids,
          const std::uint8_t* data,
          std::size_t len);

  // In packet order
  Node::Id senderid;
  // The field type storing the number of dest IDs specified
  using field_nrecp_type = std::uint8_t;
  // Optional
  std::vector<Node::Id> recpids;
  // Used to sort messages for delivery
  Type t{0};
  // Used for guaranteed ordering and response delivery
  Id id{0};
  // The field type storing the length of the content data
  using field_len_type = std::uint16_t;
  const std::uint8_t* data;
  std::size_t len;
};
}
