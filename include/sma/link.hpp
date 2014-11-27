#pragma once

#include <sma/link/linkmanager.hpp>

#include <cstdint>
#include <utility>


namespace sma
{
class LinkManager;

class Link
{
  friend class LinkManager;

public:
  /*
  Link(Link&& r)
    : manager(r.manager)
  {
    r.manager = nullptr;
  }
  Link& operator=(Link&& r)
  {
    std::swap(manager, r.manager);
    return *this;
  }
  */
  virtual ~Link() {}

  bool readable();

  virtual std::size_t read(void* dst, std::size_t size) = 0;
  virtual std::size_t write(void const* src, std::size_t size) = 0;

  virtual void close() = 0;

protected:
  void readable(bool r);

private:
  LinkManager* manager{nullptr};
  bool is_readable{false};
};
}
