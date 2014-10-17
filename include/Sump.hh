#pragma once

#include <vector>
#include <memory>
#include <utility>

#include "BlockingSource.hh"
#include "Sink.hh"


namespace sma
{

template<class T>
class Sump
{
public:
  Sump(std::shared_ptr<BlockingSource<T>> source, std::shared_ptr<Sink<T>> sink);
  Sump(std::shared_ptr<BlockingSource<T>> source, const std::vector<std::shared_ptr<Sink<T>>>& sinks);

  virtual void start();
  virtual void stop();

private:
  std::shared_ptr<BlockingSource<T>>    source;
  std::vector<std::shared_ptr<Sink<T>>> sinks;
  bool                                  running;
};


template<class T>
Sump<T>::Sump(std::shared_ptr<BlockingSource<T>> source, 
              std::shared_ptr<Sink<T>> sink)
  : source(source), 
  sinks(std::vector<std::shared_ptr<Sink<T>>>())
{
  sinks.push_back(std::move(sink));
}

template<class T>
Sump<T>::Sump(std::shared_ptr<BlockingSource<T>> source, 
              const std::vector<std::shared_ptr<Sink<T>>>& sinks)
  : source(source), 
  sinks(sinks)
{
}

template<class T>
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

template<class T>
void Sump<T>::stop()
{
  running = false;
}

}