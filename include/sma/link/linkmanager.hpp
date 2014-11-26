#pragma once

#include <sma/link/link.hpp>
#include <sma/util/sink.hpp>
#include <sma/serial/binaryformatter.hpp>

#include <vector>
#include <memory>

#include <istream>
#include <ostream>
#include <sstream>

namespace sma
{
class Link;
struct Message;

class LinkManager final : public Sink<Message&&>
{
  friend class Link;

public:
  LinkManager(std::vector<std::unique_ptr<Link>> links);
  LinkManager(LinkManager const& r) = delete;
  LinkManager& operator=(LinkManager const& r) = delete;

  void inbox(Sink<Message&&>* ibx) { this->ibx = ibx; }

  void accept(Message&& msg) override;

private:
  void on_link_readable(Link* link);

  std::vector<std::unique_ptr<Link>> links;
  Sink<Message&&>* ibx{nullptr};

  std::stringbuf::char_type send_buf[1024];
  std::stringbuf::char_type recv_buf[1024];
  std::stringbuf send_sbuf;
  std::stringbuf recv_sbuf;
  // The order of these is important for
  std::ostream send_os;
  std::istream recv_is;
  BinaryFormatter<std::ostream> serializer;
  BinaryFormatter<std::istream> deserializer;
};
}
