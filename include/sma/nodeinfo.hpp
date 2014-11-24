#pragma once

#include <ostream>

namespace sma
{
// clang-format off
struct NodeId {
private:
  using id_type = std::uint32_t;

public:
  NodeId(id_type id)
    : id(id) {}

  template <typename Reader>
  NodeId(Reader* r)
    : id(r->template get<id_type>()) {}

  template <typename Writer>
  void write_fields(Writer* w) const
  { *w << id; }


  operator std::uint32_t() const { return id; }
  operator std::string() const { return std::to_string(id); }

  id_type id;
};
// clang-format on

struct NodeInfo {
  using id_type = NodeId;

public:
  NodeInfo(id_type id)
    : id_(id)
  {
  }

  id_type id() const { return id_; }

private:
  id_type id_;
};

std::ostream& operator<<(std::ostream& os, NodeId const& id);
std::ostream& operator<<(std::ostream& os, NodeInfo const& node);
}
