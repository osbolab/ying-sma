#pragma once

#include <sma/buffer.hpp>

#include <cstdint>
#include <vector>
#include <memory>
#include <iostream>


namespace sma
{
struct Message final {
  enum Weight { LIGHT_MESSAGE, HEAVY_MESSAGE };

  using Type = std::uint8_t;
  using data_size_type = std::uint16_t;

private:
  struct Header {
    Message::Type type;
    data_size_type data_size;
  };

public:
  /*! \brief  Create a message pointing to, but not copying, the given byte
   *          array as its data.
   */
  static Message wrap(Type type,
                      Weight weight,
                      std::uint8_t const* data,
                      data_size_type size) noexcept;
  /*! \brief  Create a message copying the given byte array as its data.
   */
  static Message copy(Type type,
                      Weight weight,
                      std::uint8_t const* data,
                      data_size_type size) noexcept;

  /*! \brief  Allocate a new message and copy this message's content into it.
   *
   * This is distinct from the message's copy constructor to avoid unintentional
   * allocations when the message is wrapping an existing data array.
   */
  Message duplicate() const;

  Message(Message&& rhs);
  Message& operator=(Message&& rhs);

  /*! \brief  Get the message type used for forwarding and dispatching this
   *          message.
   */
  Type type() const { return header.type; }

  /*! \brief  Get the immutable message contents. */
  std::uint8_t const* cdata() const noexcept { return data; }
  /*! \brief  Get the size in octets of the message contents. */
  std::size_t data_size() const noexcept
  {
    return std::size_t{header.data_size};
  }
  /*! \brief  Get an immutable view of the message's contents. */
  Buffer::View view() { return Buffer::View(data, data_size()); }

private:
  /*! \brief  Non-copying constructor. */
  Message(Type type,
          Weight weight,
          std::uint8_t const* data,
          data_size_type size);
  /*! \brief  Copying constructor. */
  Message(Type type,
          Weight weight,
          std::unique_ptr<std::uint8_t[]> data,
          data_size_type size);

  Header header;
  std::uint8_t const* data{nullptr};
  std::unique_ptr<std::uint8_t[]> owned_data;
};

/*! \brief  Serialize the given message to the byte buffer.
 *
 * The buffer must be of sufficient capacity.
 */
Buffer& operator<<(Buffer& dst, Message const& rhs);
/*! \brief  Print the given message in human-readable format to the stream. */
std::ostream& operator<<(std::ostream& os, Message const& msg);
}
