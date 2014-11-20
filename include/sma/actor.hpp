#pragma once

namespace sma
{
  struct message;

  class actor
  {
  public:
    virtual void on_message(message const& msg) = 0;
  };
}
