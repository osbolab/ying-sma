#pragma once

#include <map>
#include <vector>
#include <memory>

#ifdef WIN32
#include "net/Winsock.hh"
#else
#include <sys/select.h>
#endif


namespace sma
{

class Socket;
class NativeSocket;

class NativeSocketSelector final
{
public:
  NativeSocketSelector(std::vector<std::shared_ptr<NativeSocket>> sockets);

  int select(std::shared_ptr<NativeSocket>& selected);

protected:
  //std::map<SOCKETstd::shared_ptr<NativeSocket>
  const std::vector<std::shared_ptr<NativeSocket>> sockets;
  const fd_set sockets;
};

}