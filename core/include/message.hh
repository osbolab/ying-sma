#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <functional>


namespace sma
{

struct Message final {
  using Type = uint8_t;

  // Types defining the serialized size of dimension fields
  using field_nr_recipients_type = uint8_t;
  using field_body_len_type = uint16_t;
  static const size_t field_nr_recipients_max = UINT8_MAX;
  static const size_t field_body_len_max = UINT16_MAX;

  // Feel free to change the size of this; the serialization is flexible
  union Recipient {
    uint8_t uint8[2];
    uint16_t uint16;
    uint16_t hash;
  };

  Message(Type type,
          Recipient sender,
          std::vector<Recipient> recipients,
          const uint8_t* body,
          size_t len);

  // Parse the message structure from src
  Message(const uint8_t* src, size_t len);

  Message(Message&& m);
  Message& operator=(Message&& m);

  size_t serialized_size() const { return header_size() + body_len; }

  // return dst advanced to the end of the serialized message
  uint8_t* serialize_to(uint8_t* dst, size_t body_len) const;

  size_t body_size() const { return body_len; }
  const uint8_t* cbody() const { return body_data; }

  bool operator==(const Message& other) const;
  bool operator!=(const Message& other) const { return !(*this == other); }

private:
  size_t header_size() const;

  inline static uint8_t* write(const Recipient& in, uint8_t* dst);
  inline static uint8_t* write(const uint8_t& in, uint8_t* dst);
  inline static uint8_t* write(const uint16_t& in, uint8_t* dst);
  inline static uint8_t* write(const uint32_t& in, uint8_t* dst);
  inline static uint8_t* write(const uint64_t& in, uint8_t* dst);

  inline static const uint8_t* read(const uint8_t* src, Recipient& out);
  inline static const uint8_t* read(const uint8_t* src, uint8_t& out);
  inline static const uint8_t* read(const uint8_t* src, uint16_t& out);
  inline static const uint8_t* read(const uint8_t* src, uint32_t& out);
  inline static const uint8_t* read(const uint8_t* src, uint64_t& out);

  // In packet order
  Recipient sender{{0}};
  std::vector<Recipient> recipients;
  Type type{0};
  field_body_len_type body_len{0};
  const uint8_t* body_data{nullptr};
};
}

namespace std
{
template <>
struct hash<sma::Message::Recipient> {
  constexpr auto operator()(const sma::Message::Recipient& r) const
      -> decltype(sma::Message::Recipient::hash)
  {
    return r.hash;
  }
};

template <>
struct equal_to<sma::Message::Recipient> {
  constexpr bool operator()(const sma::Message::Recipient& lhs,
                            const sma::Message::Recipient& rhs) const
  {
    return lhs.hash == rhs.hash;
  }
};
}
