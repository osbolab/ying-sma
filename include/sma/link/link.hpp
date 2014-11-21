#pragma once

namespace sma
{
class Link
{
  friend class LinkManager;

public:
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
  virtual ~Link() { close(); }

  virtual std::size_t write(void const* src, std::size_t size) = 0;
  virtual std::size_t read(void* dst, std::size_t size) = 0;

  virtual void close() = 0;

protected:
  void readable(bool r)
  {
    if ((is_readable = r) && manager)
      manager->on_link_readable(this);
  }

private:
  LinkManager* manager{nullptr};
  bool is_readable{false};
};
}
