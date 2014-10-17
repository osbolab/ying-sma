#pragma once

#include "channel/SelectableChannel.hh"
#include "msg/Message.hh"


namespace sma
{

class BsdSocketChannel : SelectableChannel<Message>
{
};

}