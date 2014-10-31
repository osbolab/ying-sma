#pragma once

#include "Channel/SelectableChannel.hh"
#include "msg/Message.hh"


namespace sma
{

class BsdSocketChannel : SelectableChannel<Message>
{
};
}
