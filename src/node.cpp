#include <sma/node.hpp>
#include <sma/message.hpp>
#include <sma/neighbormessage.hpp>

#include <sma/bytearrayreader.hpp>
#include <sma/binaryformatter.hpp>

namespace sma
{
Node::Node(NodeId id, Context* ctx)
  : Actor(ctx)
  , id_(id)
{
}

Node::~Node() {}

void Node::dispose() {}

void Node::receive(Message const& msg)
{
  if (msg.type() == NeighborMessage::TYPE) {
    auto reader
        = ByteArrayReader(msg.cdata(), msg.size()).format<BinaryFormatter>();
    auto nm = reader.template get<NeighborMessage>();
    if (neighbors.update(nm.sender).is_new())
      log.i("discovered neighbor: %v", nm.sender);
  }
}
}
