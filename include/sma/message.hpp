#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <iostream>


namespace sma
{
template <typename Formatter>
class ObjectDataIn;
template <typename Formatter>
class ObjectDataOut;

struct Message final {
  enum Weight { LIGHT, HEAVY };

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
  static Message
  wrap(Type type, Weight weight, std::uint8_t const* data, data_size_type size);
  /*! \brief  Create a message copying the given byte array as its data.
   */
  static Message
  copy(Type type, Weight weight, std::uint8_t const* data, data_size_type size);


  // Serialization

  template <typename Formatter>
  Message(ObjectDataIn<Formatter> in);
  template <typename Formatter>
  void write_fields(ObjectDataOut<Formatter> out);

  Message(Message&& rhs);
  Message& operator=(Message&& rhs);
  /*! \brief  Allocate a new message and copy this message's content into it.
   *
   * This is distinct from the message's copy constructor to avoid unintentional
   * allocations when the message is wrapping an existing data array.
   */
  Message duplicate() const;

  /*! \brief  Get the message type used for forwarding and dispatching this
   *          message.
   */
  Type type() const { return header.type; }

  /*! \brief  Get the immutable message contents. */
  std::uint8_t const* cdata() const noexcept { return data; }
  /*! \brief  Get the size in octets of the message contents. */
  std::size_t size() const noexcept { return std::size_t{header.data_size}; }

private:
  Message(Message const& r);
  /*! \brief  Non-copying constructor. */
  Message(Header header, std::uint8_t const* data, Weight weight);
  /*! \brief  Copying constructor. */
  Message(Header header,
          std::unique_ptr<std::uint8_t[]> owned_data,
          Weight weight);

  Header header;
  std::uint8_t const* data{nullptr};
  std::unique_ptr<std::uint8_t[]> owned_data;
};
}
