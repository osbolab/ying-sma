#pragma once

#include <cstdint>
#include <vector>
#include <memory>

namespace sma {
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
  static Message wrap(Type type, Weight weight, std::uint8_t const* data,
                      data_size_type size);
  /*! \brief  Create a message copying the given byte array as its data.
   */
  static Message copy(Type type, Weight weight, std::uint8_t const* data,
                      data_size_type size);

  Message(Message&& rhs);
  Message& operator=(Message&& rhs);
  /*! \brief  Allocate a new message and copy this message's content into it.
   *
   * This is distinct from the message's copy constructor to avoid unintentional
   * allocations when the message is wrapping an existing data array.
   */
  Message duplicate() const;

  // Serialization

  template <typename Reader>
  Message(Reader* in);

  template <typename Writer>
  void write_fields(Writer* out) const;

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
  Message(Header header, std::unique_ptr<std::uint8_t[]> owned_data,
          Weight weight);
  /*! \brief  Non-copying constructor. */
  Message(Header header, std::uint8_t const* data, Weight weight);

  Header header;
  std::uint8_t* data{nullptr};
  std::unique_ptr<std::uint8_t[]> owned_data;
};

template <typename Reader>
Message::Message(Reader* in) {
  *in >> header.type;
  *in >> header.data_size;
  owned_data = std::make_unique<std::uint8_t[]>(header.data_size);
  data = owned_data.get();
  in->read(data, header.data_size);
}

template <typename Writer>
void Message::write_fields(Writer* out) const {
  *out << header.type;
  *out << header.data_size;
  out->write(data, header.data_size);
}
}
