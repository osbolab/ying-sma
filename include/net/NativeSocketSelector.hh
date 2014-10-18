#pragma once

#include <map>
#include <vector>
#include <memory>


namespace sma
{

class Socket;
class NativeSocket;

struct fd_set;

class NativeSocketSelector final
{
public:
  NativeSocketSelector(std::vector<std::shared_ptr<NativeSocket>> sockets);

  int select(std::shared_ptr<NativeSocket>& selected);

protected:
  std::map<SOCKETstd::shared_ptr<NativeSocket>
  const std::vector<std::shared_ptr<NativeSocket>> sockets;
  const fd_set sockets;
};

}