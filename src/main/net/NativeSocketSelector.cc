#include "net/NativeSocket.hh"
#include "net/NativeSocketSelector.hh"

namespace sma
{

NativeSocketSelector::NativeSocketSelector(std::vector<std::shared_ptr<NativeSocket>> sockets)
  : sockets(sockets) 
{

}


int NativeSocketSelector::select(std::shared_ptr<NativeSocket>& selected)
{

}

}