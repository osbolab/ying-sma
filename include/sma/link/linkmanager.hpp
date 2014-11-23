#pragma once

#include <sma/link/link.hpp>
#include <sma/sink.hpp>
#include <sma/message.hpp>
#include <sma/binaryformatter.hpp>

#include <vector>
#include <memory>
#include <istream>
#include <ostream>
#include <sstream>

namespace sma
{
class Link;

class LinkManager final : public Sink<Message const&>
{
  friend class Link;

public:
  LinkManager(std::vector<std::unique_ptr<Link>> links);
  /*
  LinkManager(LinkManager&& r);
  LinkManager& operator=(LinkManager&& r);
  */

  void inbox(Sink<Message const&>* ibx) { this->ibx = ibx; }

  void accept(Message const& msg) override;

private:
  void on_link_readable(Link* link);

  std::vector<std::unique_ptr<Link>> links;
  Sink<Message const&>* ibx{nullptr};

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
