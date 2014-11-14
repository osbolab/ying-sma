#pragma once

#include <sma/node.hpp>
#include <sma/buffer.hpp>
#include <sma/bytes.hpp>

#include <cstdint>
#include <vector>
#include <memory>
#include <iostream>


namespace sma
{

class messenger;


/**
 * A lightweight wrapper around serialized message data that attaches
 * low-level network routing information, namely the message's sending
 * node and optional recipient nodes. Also specifies parameters on which
 * the message can be sorted for delivery at its destination node.
 */
class message final
{
public:
  using domain_type = std::uint8_t;
  // The type of the sender-unique ID embedded in every message.
  // The ID is only unique for a given sender; there are no synchronization
  // requirements or uniqueness/sequentiality guarantees between senders.
  using id_type = std::uint32_t;

  // Since the sender of a message shouldn't be concerned with the details of
  // her node configuration, she's only responsible for building up a stub of
  // her message containing what's relevant to her. The rest, like the unique
  // ID, is completed further down the line.
  class stub final
  {
    friend class message;

  public:
    stub(domain_type domain);
    stub& add_recipient(node::id recipient);
    stub& wrap_contents(const std::uint8_t* src, std::size_t len);
    message build(node::id sender, id_type id) const;

    friend std::ostream& operator<<(std::ostream& os, const stub& stb);

  private:
    domain_type domain;
    std::vector<node::id> recipients;
    const std::uint8_t* content;
    std::size_t len;
  };

  // Let the stub construct instances of us from itself
  friend class stub;

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

  // Deserialize a message from the contents of the given byte buffer.
  // If the buffer does not contain a full message, or contains an invalid
  // message, then behavior is undefined (very likely an access violation
  // will occur).
  message(buffer::view src);
  // Deserialize a message from the datas of the given byte array.
  // The array is wrapped by an immutable byte buffer and the message
  // constructed as if by call to message(buffer::view).
  message(const std::uint8_t* src, std::size_t len);
  // Serialize this message into the given buffer. The number of bytes
  // that will be written can be obtained via total_len().
  friend buffer& operator<<(buffer& dst, const message& rhs);
  // Serialize this message into the given byte array.
  // Partial writes are illegal; the array's length must be
  // at least equal to that given by total_size().
  std::size_t serialize_to(buffer dst) const;
  std::size_t serialize_to(std::uint8_t* dst, std::size_t len) const;
  // Gets the total size of this message when serialized including any padding
  // or termination.
  std::size_t serial_size() const;

  // The base message domain as used for dispatching arriving messages.
  domain_type domain() const { return domain_; }
  id_type id() const { return id_; }
  // The id_ of the node that sent this message.
  node::id sender() const { return sender_; }
  // The optional IDs of nodes for whom this message is intended.
  const std::vector<node::id>& recipients() const { return recipients_; }

  // Get a pointer to this message's immutable content.
  // The lifetime of this array is undefined and it should be pinned,
  // copied, or read *immediately* on first observation.
  // The pointed to memory is very likely to be valid only within the call
  // stack that delivered this message.
  //
  // If the message has its lifetime extended via pin() then the above is
  // no longer true and this getter returns the pinned array.
  // The contents can also be made mutable by using pin() to claim a copy.
  const std::uint8_t* content() const { return content_; }
  // Get the length of the content data.
  std::size_t content_len() const { return content_len_; }

  // Extend the message's lifetime by copying its backing buffer into
  // locally-owned memory. The message must be pinned to allow mutating
  // its data or copying it as to do otherwise illegally expands the
  // visibility of the message's original backing buffer.
  // Calling this function multiple times has no effect and simply returns
  // the already pinned array.
  std::uint8_t* pin();

  // Pretty-pring the message to a stream (log helper)
  friend std::ostream& operator<<(std::ostream& os, const message& msg);

private:
  // Complete the given stub by designating the message's origin and unique
  // origin ID, producing a message.
  message(node::id sender, id_type id, const stub& stb);

  // Copy the message contents so the message can be copied or held.
  std::uint8_t* pin_copy(const void* src, std::size_t len);
  // Throw an exception if a copy operation is performed while the message is
  // not pinned. This is a pessimistic failure mode to avoid accidental copying.
  void throw_not_pinned();

  // ----------------- SERIALIZED FIELDS (in serial order) --------------------
  // The order reflects the order of the field's relevance:
  // The sender is serialized first so that broadcast devices can cull loopback
  // broadcast within a few bytes of deserialization.
  node::id sender_;
  // The type of the serialized field storing the number of dest IDs
  using field_nrecp_type = std::uint8_t;
  // 0 <= n < 2**bits(field_nrecp_type)
  // Being the second field allows nodes to disregard not-for-me messages within
  // a few bytes.
  std::vector<node::id> recipients_;
  // Messages are sorted for distribution in the node by the domain
  domain_type domain_{0};
  // Recipients, or a layer between distribution and the recipient, can consider
  // the ID unique among all messages they'll receive from the same sender.
  // Use this for acknowledgment/response.
  id_type id_{0};
  // The type of the serialized field storing the length of the content
  using field_len_type = std::uint16_t;
  // To avoid allocations and copying we just shuttle around a pointer to our
  // content. This means we don't own that memory, but the message contract
  // specifies that the underlying buffer is reliable within the call stack
  // that produced the message.
  // That means that if you're given a message as a parameter you can trust
  // this pointer until you return, but no longer.
  const std::uint8_t* content_{nullptr};
  std::size_t content_len_{0};
  // The aforementioned guarantee can be extended indefinitely by pinning the
  // contents in the heap. This just means we allocate and take a copy and you
  // gain ownership of its lifetime.
  std::unique_ptr<std::uint8_t[]> pinned{nullptr};
};

// Serialize the given message to the byte buffer, assuming it's of sufficient
// capacity.
buffer& operator<<(buffer& dst, const message& rhs);
// Pretty-printing (for logging)
std::ostream& operator<<(std::ostream& os, const message::stub& stb);
std::ostream& operator<<(std::ostream& os, const message& msg);
}
