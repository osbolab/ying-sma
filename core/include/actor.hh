#pragma once

#include <cstdint>
#include <cstring>


namespace sma
{

struct ActorId {
  const std::uint8_t id[4];

  ActorId(std::uint8_t* id) { std::memcpy(this->id, id, sizeof id); }

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

  Actor& operator=(Actor&& rhs) { std::swap(id, rhs.id); }

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
struct hash<ActorId> {
  std::size_t operator()(const ActorId& o) const
  {
    // clang-format off
    return  std::uint32_t{o.id[0]} << 24
          | std::uint32_t{o.id[1]} << 16
          | std::uint32_t{o.id[2]} << 8
          | o.id[3];
    // clang-format on
  }
};
}
