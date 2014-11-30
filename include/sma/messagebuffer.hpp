#pragma once

namespace sma
{
class MessageBuffer
{
  public:
    virtual ~MessageBuffer() {}

    bool try_push();
};
}
