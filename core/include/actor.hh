#pragma once

#include <cstdint>
#include <cstring>
#include <utility>
#include <functional>


namespace sma
{

struct Message;

struct ActorId {
  std::uint8_t id[4];

  ActorId() { std::memset(id, 0, sizeof id); }

  ActorId(std::uint8_t* id) { std::memcpy(this->id, id, sizeof this->id); }

  bool operator==(const ActorId& rhs) const
  {
    for (std::size_t i = 0; i < sizeof id; ++i)
      if (id[i] != rhs.id[i])
        return false;
    return true;
  }
  bool operator!=(const ActorId& rhs) const { return !(*this == rhs); }

  bool operator>(const ActorId& rhs) const
  {
    for (std::size_t i = 0; i < sizeof id; ++i)
      if (id[i] != rhs.id[i])
        return id[i] > rhs.id[i];
    return false;
  }
  bool operator>=(const ActorId& rhs) const
  {
    for (std::size_t i = 0; i < sizeof id; ++i)
      if (id[i] != rhs.id[i])
        return id[i] > rhs.id[i];
    return true;
  }
  bool operator<(const ActorId& rhs) const
  {
    for (std::size_t i = 0; i < sizeof id; ++i)
      if (id[i] != rhs.id[i])
        return id[i] < rhs.id[i];
    return false;
  }
  bool operator<=(const ActorId& rhs) const
  {
    for (std::size_t i = 0; i < sizeof id; ++i)
      if (id[i] != rhs.id[i])
        return id[i] < rhs.id[i];
    return true;
  }
};


class Actor
{
public:
  Actor(Actor&& rhs)
    : id(std::move(rhs.id))
  {
  }

  virtual ~Actor() {}

  virtual void receive(Message msg) = 0;

  Actor& operator=(Actor&& rhs)
  {
    std::swap(id, rhs.id);
    return *this;
  }

protected:
  Actor(ActorId id)
    : id(std::move(id))
  {
  }

  ActorId id;
};
}


namespace std
{
template <>
struct hash<sma::ActorId> {
  std::size_t operator()(const sma::ActorId& o) const
  {
    return std::uint32_t{o.id[0]} << 24 | std::uint32_t{o.id[1]} << 16
           | std::uint32_t{o.id[2]} << 8 | o.id[3];
    // clang-format on
  }
};
}
