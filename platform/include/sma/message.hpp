#pragma once

#include <sma/detail/message_type.hpp>

#include <sma/buffer.hpp>
#include <sma/bytes.hpp>

#include <cstdint>
#include <vector>
#include <memory>
#include <iostream>


namespace sma
{

class messenger;

struct message
{
public:
  message(message_type type, std::uint8_t const* data, std::size_t size);
  // Deserialize a message from the contents of the given byte buffer.
  // If the buffer does not contain a full message, or contains an invalid
  // message, then behavior is undefined (very likely an access violation
  // will occur).
  message(buffer::view src);
  // Deserialize a message from the datas of the given byte array.
  // The array is wrapped by an immutable byte buffer and the message
  // constructed as if by call to message(buffer::view).
  message(std::uint8_t const* src, std::size_t size);

  message(message&& rhs);
  message& operator=(message&& rhs);

  // Serialize this message into the given byte array.
  // Partial writes are illegal; the array's sizegth must be
  // at least equal to that given by total_size().
  std::size_t serialize_to(buffer& dst) const;
  // Serialize this message into the given buffer. The number of bytes
  // that will be written can be obtained via total_size().
  friend buffer& operator<<(buffer& dst, message const& rhs);
  // Wrap the given byte array in a buffer and serialize it as if by call to
  // serialize_to(buffer) const.
  std::size_t serialize_to(std::uint8_t* dst, std::size_t size) const;
  // Gets the total size of this message when serialized including any padding
  // or termination.
  std::size_t serialized_size() const;

  // The base message type as used for dispatching arriving messages.
  message_type type() const { return t; }

  // Get a pointer to this message's immutable data.
  // The lifetime of this array is undefined and it should be pinned,
  // copied, or read *immediately* on first observation.
  // The pointed to memory is very likely to be valid only within the call
  // stack that delivered this message.
  std::uint8_t const* cdata() const { return data; }
  std::size_t size() const { return szdata; }
  buffer::view view() { return buffer::view::of(data, szdata); }

  // Pretty-pring the message to a stream (log helper)
  friend std::ostream& operator<<(std::ostream& os, message const& msg);

protected:
  message(message const& rhs);
  message& operator=(message const& rhs);

  // ----------------- SERIALIZED FIELDS (in serial order) --------------------
  // Messages are sorted for distribution in the node by the type
  message_type t{0};

  using field_size_type = std::uint16_t;
  // To avoid allocations and copying we just shuttle around a pointer to our
  // data. This means we don't own that memory, but the message contract
  // specifies that the underlying buffer is reliable within the call stack
  // that produced the message.
  // That means that if you're given a message as a parameter you can trust
  // this pointer until you return, but no longer.
  std::uint8_t const* data{nullptr};
  std::size_t szdata{0};
};

// Serialize the given message to the byte buffer, assuming it's of sufficient
// capacity.
buffer& operator<<(buffer& dst, message const& rhs);
// Pretty-printing (for logging)
std::ostream& operator<<(std::ostream& os, message const& msg);
}
