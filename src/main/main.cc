#include <iostream>
#include <string>
#include <thread>

#include "Sump.hh"
#include "BlockingQueue.hh"
#include "channel/Channel.hh"

using std::string;

class PrintSink : public sma::Sink<string>
{
public:
  bool sink(string s) override
  {
    if (s.empty()) return false;
    std::cout << "Sunk: " << s << std::endl << std::flush;
    return true;
  }
};

class MyChannel : public sma::Channel<string>
{
public:
  bool sink(string item) override
  {
    this->item = string(std::move(item));
    return true;
  }

  string item;
};

int main(int argc, const char** argv)
{
  std::cout << "Hello, world!" << std::endl;

  MyChannel chan;

  std::shared_ptr<sma::BlockingQueue<string>> messages(new sma::BlockingQueue<string>());
  std::shared_ptr<sma::Sink<string>> Sink(new PrintSink());

  sma::Sump<string> pump(messages, std::move(Sink));

  std::thread th([&] {
    for (int i = 0; i < 3; ++i)
    {
      messages->offer(string("Hello, queue!"));
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    pump.stop();
    messages->offer(string());
  });

  pump.start();

  th.join();

  return 0;
}