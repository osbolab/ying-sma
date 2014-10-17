#pragma once

namespace sma
{

class MessageBuilder
{
  friend class Messenger;


private:
  MessageBuilder(std::shared_ptr<const Messenger> messenger);

  std::shared_ptr<const Messenger> messenger;
};

}