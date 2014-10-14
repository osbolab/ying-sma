#ifndef BSDSOCKETCHANNEL_H_
#define BSDSOCKETCHANNEL_H_

#include "channel/SelectableChannel.hh"
#include "msg/Message.hh"


namespace sma
{

class BsdSocketChannel : SelectableChannel<Message>
{
};

}

#endif