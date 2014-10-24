#pragma once

#include <vector>
#include <memory>
#include <utility>

#include "concurrent/BlockingSource.hh"
#include "util/Sink.hh"


namespace sma
{

template<typename T>
class Sump
{
public:
  Sump(std::shared_ptr<BlockingSource<T>> src, std::shared_ptr<Sink<T>> dst);
  Sump(std::shared_ptr<BlockingSource<T>> src,
       const std::vector<std::shared_ptr<Sink<T>>>& dsts);

  virtual void start();
  virtual void stop();

private:
  std::shared_ptr<BlockingSource<T>>    source;
  std::vector<std::shared_ptr<Sink<T>>> sinks;
  bool                                  running;
};


template<typename T>
Sump<T>::Sump(std::shared_ptr<BlockingSource<T>> src,
              std::shared_ptr<Sink<T>> dst)
  : source(src),
    sinks(std::vector<std::shared_ptr<Sink<T>>>())
{
  sinks.push_back(std::move(dst));
}

template<typename T>
Sump<T>::Sump(std::shared_ptr<BlockingSource<T>> src,
              const std::vector<std::shared_ptr<Sink<T>>>& dsts)
  : source(src),
    sinks(dsts)
{
}

template<typename T>
void Sump<T>::start()
{
  running = true;
  while (running) {
    const T item = std::move(source->take());
    for (auto iter = sinks.begin(); iter != sinks.end();) {
      if ((*iter)->sink(item)) {
        ++iter;
      } else {
        iter = sinks.erase(iter);
      }
    }
    if (sinks.empty()) return;
  }
}

template<typename T>
void Sump<T>::stop()
{
  running = false;
}

}