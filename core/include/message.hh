#pragma once

#include <cstdlib>
#include <cstdint>
#include <vector>


namespace sma
{

class Messenger;

struct Message final {
  friend class Messenger;

public:
  using Type = std::uint8_t;

  union Address {
    std::uint8_t data[2];
    std::uint16_t s;
  };

  Message(Message&& m);
  Message& operator=(Message&& m);

  std::size_t serialized_size() const;
  std::uint8_t* serialize_to(std::uint8_t* dst, std::size_t body_len) const;

  std::size_t body_size() const;
  const std::uint8_t* body() const;

  bool operator==(const Message& other) const;
  bool operator!=(const Message& other) const;

private:
  Message(Type type,
          Address sender,
          std::vector<Address> recipients,
          const std::uint8_t* body,
          std::size_t len);

  Message(const std::uint8_t* src, std::size_t len);

  std::size_t header_size() const;

  // In packet order
  Address sender{{0}};
  std::vector<Address> recipients;
  Type type{0};
  std::size_t body_len{0};
  const std::uint8_t* body_data{nullptr};
};
}
