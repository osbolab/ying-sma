#include <sma/android/nodecontainer.hpp>

#include <sma/io/log.hpp>


namespace sma
{
NodeContainer::NodeContainer()
{
  Logger("NodeContainer").d("Hello, world!");
}

NodeContainer::~NodeContainer()
{
  Logger("NodeContainer").d("Goodbye, cruel world!");
}
}
