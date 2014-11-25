#pragma once

namespace sma
{
struct Context;

class CcnNode
{
public:
  CcnNode(Context* ctx);

  void dispose();

private:
  Context* ctx;
};
}
