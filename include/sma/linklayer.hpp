#pragma once

#include <sma/link.hpp>
#include <sma/messageheader.hpp>
#include <sma/util/binarystreamreader.hpp>
#include <sma/util/binarystreamwriter.hpp>

#include <vector>
#include <memory>

#include <istream>
#include <ostream>
#include <sstream>

namespace sma
{
class Link;

class LinkLayer final
{
  friend class Link;

public:
  LinkLayer(std::vector<std::unique_ptr<Link>> links);

  LinkLayer(LinkLayer const& r) = delete;
  LinkLayer& operator=(LinkLayer const& r) = delete;

  void receive_to(Node* node) { this->node = node; }

  template <typename M>
  void forward(MessageHeader header, M&& msg);

private:
  void on_link_readable(Link* link);

  std::vector<std::unique_ptr<Link>> links;
  Node* node{nullptr};

  // Sending and receiving are mutually thread-safe
  std::stringbuf::char_type send_buf[1024];
  std::stringbuf::char_type recv_buf[1024];
  std::stringbuf send_sbuf;
  std::stringbuf recv_sbuf;
  // Don't reorder the below without changing the initializer.
  std::ostream send_os;
  std::istream recv_is;
  BinaryStreamWriter writer;
  BinaryStreamReader reader;
};


template <typename M>
void LinkLayer::forward(MessageHeader header, M&& msg)
{
  // lock buffer
  send_sbuf.pubseekpos(0);

  writer << header;
  writer << msg;

  std::size_t const size = send_os.tellp();

  std::size_t wrote = 0;
  for (auto& link : links)
    assert((wrote = link->write(send_buf, size)));
  // unlock buffer
}
}
