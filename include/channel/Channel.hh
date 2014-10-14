#ifndef CHANNEL_H_
#define CHANNEL_H_

#include "Sink.hh"
#include "BlockingSource.hh"


namespace sma
{

template<class T>
class Channel : public Sink<T>, BlockingSource<T>
{
};

}

#endif