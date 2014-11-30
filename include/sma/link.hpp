#pragma once

#include <cstdint>
#include <utility>


namespace sma
{
class LinkLayer;

class Link
{
public:
  virtual ~Link() {}

  void receive_to(LinkLayer& ll) { linklayer = &ll; }

  bool readable();

  virtual std::size_t read(void* dst, std::size_t size) = 0;
  virtual std::size_t write(void const* src, std::size_t size) = 0;

  virtual void close() = 0;

protected:
  void readable(bool r);

private:
  LinkLayer* linklayer{nullptr};
  bool is_readable{false};
};
}
