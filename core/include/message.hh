#pragma once

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <type_traits>


namespace sma
{

class Messenger;

struct Message {
  friend class Messenger;

public:
  using Type = std::uint8_t;

  Message(Message&& m);
  Message& operator=(Message&& m);

  template <typename T>
  static T read(const std::uint8_t* src, std::size_t len)
  {
    static_assert(std::is_base_of<Message, T>::value);
    T t;
    t.read_fields(src, len);
    return t;
  }

  virtual std::uint8_t* write_fields(std::uint8_t* dst, std::size_t len) const;
  virtual std::size_t size() const
  {
    return sizeof(ActorId::id) * (recipients.size() + 1) + sizeof Type;
  }

  bool operator==(const Message& other) const;
  bool operator!=(const Message& other) const { return !(*this == other); }

protected:
  virtual const std::uint8_t* read_fields(const std::uint8_t* src,
                                          std::size_t len);

private:
  Message(Type type, ActorId sender, std::vector<ActorId> recipients);

  // In packet order
  ActorId sender{{0}};
  std::vector<ActorId> recipients;
  Type type{0};
};
}
