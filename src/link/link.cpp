#include <sma/link/link.hpp>
#include <sma/link/linkmanager.hpp>


namespace sma
{
  void Link::readable(bool r)
  {
    if ((is_readable = r) && manager)
      manager->on_link_readable(this);
  }
}
