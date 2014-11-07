#pragma once

#include <cstdint>
#include <functional>
#include <unordered_set>


namespace sma
{
}

namespace sma
{

class Messenger;


struct Message final {
  friend class Messenger;

  using Address = std::uint16_t;
  using Type = std::uint8_t;

  static const std::size_t field_nr_recipients_max = UINT8_MAX;
  static const std::size_t field_body_len_max = UINT16_MAX;

  /*******************************
   * Message::Builder
   */
  struct Builder final {
    friend struct Message;

    Builder(Type type, const std::uint8_t* body, std::size_t len)
      : type(type)
      , body(body)
      , len(len)
    {
    }

    void add(Address recipient) { recipients.insert(std::move(recipient)); }

  private:
    Type type;
    std::unordered_set<Address> recipients;
    const std::uint8_t* body;
    std::size_t len;
  };
  /*
   * Message::Builder
   *******************************/

  static Builder build(Type type, const std::uint8_t* body, std::size_t len)
  {
    return Builder(type, body, len);
  }

  Message(Builder&& builder, Address sender);
  Message(Type type,
          Address sender,
          std::unordered_set<Address>&& recipients,
          const std::uint8_t* body,
          std::size_t len);

  Message(const std::uint8_t* src, std::size_t len);

  // return dst advanced to the end of the serialized message
  std::uint8_t* put_in(std::uint8_t* dst, std::size_t len) const;

  std::size_t serialized_size() const { return header_size() + body_len; }
  std::size_t body_size() const { return body_len; }

  Type type() const { return type_; }
  Address sender() const { return sender_; }
  const std::unordered_set<Address> recipients() const { return recipients_; }
  const std::uint8_t* body() const { return body_data; }

  bool operator==(const Message& other) const;
  bool operator!=(const Message& other) const { return !(*this == other); }

private:
  inline static std::uint8_t* write(const std::uint8_t& in, std::uint8_t* dst);
  inline static std::uint8_t* write(const std::uint16_t& in, std::uint8_t* dst);
  inline static std::uint8_t* write(const std::uint32_t& in, std::uint8_t* dst);
  inline static std::uint8_t* write(const std::uint64_t& in, std::uint8_t* dst);

  inline static const std::uint8_t* read(const std::uint8_t* src,
                                         std::uint8_t& out);
  inline static const std::uint8_t* read(const std::uint8_t* src,
                                         std::uint16_t& out);
  inline static const std::uint8_t* read(const std::uint8_t* src,
                                         std::uint32_t& out);
  inline static const std::uint8_t* read(const std::uint8_t* src,
                                         std::uint64_t& out);

  // Types of dimension fields
  using field_nr_recipients_type = std::uint8_t;
  using field_body_len_type = std::uint16_t;

  // Invariant header size as if variable fields are empty
  static constexpr std::size_t static_header_size()
  {
    return sizeof(Type) + sizeof(Address) + sizeof(field_nr_recipients_type)
           + sizeof(field_body_len_type);
  }

  // Actual header size including variable fields
  std::size_t header_size() const
  {
    return static_header_size() + sizeof(Address) * recipients_.size();
  }

  // In packet order
  Address sender_;
  std::unordered_set<Address> recipients_;
  Type type_;
  field_body_len_type body_len{0};
  const std::uint8_t* body_data{nullptr};
};
}
