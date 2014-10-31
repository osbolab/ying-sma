#pragma once

#include "selectablechannel.hh"
#include "message.hh"


namespace sma
{

class BsdSocketChannel : SelectableChannel<Message>
{
};
}
