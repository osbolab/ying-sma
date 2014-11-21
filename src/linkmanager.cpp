#include <sma/link/linkmanager.hpp>

#include <sma/message.hpp>


namespace sma
{
LinkManager::LinkManager(std::vector<Link*> links)
  : links(std::move(links))
{
  assert(!this->links.empty());
  for (auto& link : this->links)
    link->manager = this;
}
LinkManager(LinkManager&& r)
  : links(std::move(r.links))
  , ibx(r.ibx)
{
  r.ibx = nullptr;
}
LinkManager& operator=(LinkManager&& r)
{
  std::swap(links, r.links);
  std::swap(ibx, r.ibx);
  return *this;
}

void LinkManager::accept(Message const& msg)
{
  std::uint8_t buf[1024];
  std::size_t size = BinaryMessageWriter(buf, sizeof buf).write(msg);

  std::size_t wrote = 0;
  for (auto& link : links)
    while (!(wrote = link->write(buf, size)));
}

void LinkManager::on_link_readable(Link* link)
{
  if (!ibx)
    return;

  std::uint8_t buf[1024];
  std::size_t read = 0;
  while ((read = link->read(buf, sizeof buf)))
    ibx->accept(BinaryMessageReader::read(buf, read));
}
}
