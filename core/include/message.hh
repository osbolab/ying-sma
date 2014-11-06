#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <functional>


namespace sma
{

struct Message final {
  using Type = std::uint8_t;

  // Types defining the serialized size of dimension fields
  using field_nr_recipients_type = std::uint8_t;
  static const std::size_t field_nr_recipients_max = UINT8_MAX;
  using field_body_len_type = std::uint16_t;
  static const std::size_t field_body_len_max = UINT16_MAX;

  // Feel free to change the size of this; the serialization is flexible
  union Recipient {
    std::uint8_t uint8[2];
    std::uint16_t uint16;
    std::uint16_t hash;
  };

  Message(Type type,
          Recipient sender,
          std::vector<Recipient> recipients,
          const std::uint8_t* body,
          std::size_t len);

  // Parse the message structure from src
  Message(const std::uint8_t* src, std::size_t len);

  Message(Message&& m);
  Message& operator=(Message&& m);

  size_t serialized_size() const { return header_size() + body_len; }

  // return dst advanced to the end of the serialized message
  std::uint8_t* serialize_to(std::uint8_t* dst, std::size_t body_len) const;

  std::size_t body_size() const { return body_len; }
  const std::uint8_t* body() const { return body_data; }

  bool operator==(const Message& other) const;
  bool operator!=(const Message& other) const { return !(*this == other); }

  bool operator==(const Message& other) const;
  bool operator!=(const Message& other) const { return !(*this == other); }

private:
  std::size_t header_size() const;

  inline static std::uint8_t* write(const Recipient& in, std::uint8_t* dst);
  inline static std::uint8_t* write(const std::uint8_t& in, std::uint8_t* dst);
  inline static std::uint8_t* write(const std::uint16_t& in, std::uint8_t* dst);
  inline static std::uint8_t* write(const std::uint32_t& in, std::uint8_t* dst);
  inline static std::uint8_t* write(const std::uint64_t& in, std::uint8_t* dst);

  inline static const std::uint8_t* read(const std::uint8_t* src,
                                         Recipient& out);
  inline static const std::uint8_t* read(const std::uint8_t* src,
                                         std::uint8_t& out);
  inline static const std::uint8_t* read(const std::uint8_t* src,
                                         std::uint16_t& out);
  inline static const std::uint8_t* read(const std::uint8_t* src,
                                         std::uint32_t& out);
  inline static const std::uint8_t* read(const std::uint8_t* src,
                                         std::uint64_t& out);

  // In packet order

  // In packet order
  Recipient sender{{0}};
  std::vector<Recipient> recipients;
  Type type{0};
  field_body_len_type body_len{0};
  const std::uint8_t* body_data{nullptr};
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
