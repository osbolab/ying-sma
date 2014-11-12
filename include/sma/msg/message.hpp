#pragma once

#include <sma/node.hpp>

#include <cstdint>
#include <vector>
#include <memory>


namespace sma
{

class messenger;


/**
 * A lightweight wrapper around serialized message data that attaches
 * low-level network routing information, namely the message's sending
 * node and optional recipient nodes. Also specifies parameters on which
 * the message can be sorted for delivery at its destination node.
 */
struct message {
  using domain_type = std::uint8_t;
  using id_type = std::uint64_t;

  class Builder
  {
  };

  // Deserialize a message from the datas of the given byte buffer.
  // If the buffer does not contain a full message, or contains an invalid
  // message, then behavior is undefined (very likely an access violation
  // will occur).
  message(byte_buf::view src);
  // Deserialize a message from the datas of the given byte array.
  // The array is wrapped in an immutable byte buffer view and the message
  // constructed as if by call to message(byte_buf::view).
  message(const std::uint8_t* src, std::size_t len);

  message(message&& rhs);
  message& operator=(message&& rhs);

  // Copying a message is an error unless the message has been pinned by
  // a call to pin().
  // Pinning the message causes the backing buffer to be copied and stored
  // local to the message so that it can leave the scope of its
  // creator's call stack.
  //
  // The default error behavior is intended to draw attention to
  // erroneous move semantics that might otherwise lead to frequent
  // unexpected memory allocations.
  message(const message& rhs);
  message& operator=(const message& rhs);

  // Get the length in octets of this message including its header and
  // all of its data.
  std::size_t total_len() const;
  // Serialize this message into the given buffer. The number of bytes
  // that will be written can be obtained via total_len().
  std::size_t copy_to(byte_buf dst) const;
  // Serialize this message into the given byte array.
  // Partial writes are illegal; the array's length must be
  // at least equal to that given by total_len().
  std::size_t copy_to(std::uint8_t* dst, std::size_t len) const;

  // The ID of the node that sent this message.
  const node::id& sender() const { return senderid; }
  // The optional IDs of nodes for whom this message is intended.
  const std::vector<node::id>& recipients() const { return recpids; }
  // The base message domain as used for dispatching arriving messages.
  domain_type domain() const { return dom; }

  // Get a pointer to this message's immutable content.
  // The lifetime of this array is undefined and it should be pinned,
  // copied, or read *immediately* on first observation.
  // The pointed to memory is very likely to be valid only within the call
  // stack that delivered this message.
  //
  // If the message has its lifetime extended via pin() then the above is
  // no longer true and this getter returns the pinned array.
  // The contents can also be made mutable by using pin() to claim a copy.
  const std::uint8_t* content() const { return data; }
  // Get the length of the content data.
  std::size_t content_len() const { return len; }

  // Extend the message's lifetime by copying its backing buffer into
  // locally-owned memory. The message must be pinned to allow mutating
  // its data or copying it as to do otherwise illegally expands the
  // visibility of the message's original backing buffer.
  // Calling this function multiple times has no effect and simply returns
  // the already pinned array.
  std::uint8_t* pin();

private:
  // Populate a message object with a pointer to its data array.
  // Does *not* copy the array or transfer ownership of any memory.
  message(domain_type domain,
          node::id sender,
          std::vector<node::id> recpids,
          const std::uint8_t* data,
          std::size_t len);

  std::uint8_t* pin_copy_(const void* src, std::size_t len);

  void throw_not_pinned();

  // In packet order
  node::id senderid;
  // The field domain storing the number of dest IDs specified
  using field_nrecp_type = std::uint8_t;
  // Optional
  std::vector<node::id> recpids;
  // Used to sort messages for delivery
  domain_type dom{0};
  // Used for guaranteed ordering and response delivery
  id_type id{0};
  // The field domain storing the length of the data data
  using field_len_type = std::uint16_t;
  // This is owned by our lifetime manager; we never control its lifetime.
  const std::uint8_t* data{nullptr};
  std::size_t len{0};

  std::unique_ptr<std::uint8_t[]> pinned{nullptr};
};
}
