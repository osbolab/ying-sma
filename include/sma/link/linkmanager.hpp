#pragma once

#include <sma/link/link.hpp>
#include <sma/channel.hpp>
#include <sma/sink.hpp>

#include <vector>
#include <memory>


namespace sma
{
struct Message;

class LinkManager final : public Sink<Message const&>
{
  friend class Link;

public:
  LinkManager(std::vector<std::unique_ptr<Link>> links);
  LinkManager(LinkManager&& r);
  LinkManager& operator=(LinkManager&& r);

  void inbox(Sink<Message const&>* ibx) { this->ibx = ibx; }

  // Sink

  void accept(Message const& msg) override;

private:
  void on_link_readable(Link* link);

  std::vector<std::unique_ptr<Link>> links;
  Sink<Message const&>* ibx{nullptr};
};
}
