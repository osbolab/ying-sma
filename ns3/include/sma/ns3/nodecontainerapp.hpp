#pragma once

#include <sma/linklayer.hpp>
#include <sma/component.hpp>
#include <sma/ns3/async.hpp>
#include <sma/node.hpp>

#include <ns3/application.h>

#include <cstdint>
#include <memory>
#include <vector>


namespace sma
{

class Ns3NodeContainerApp final : public ns3::Application
{
public:
  // ns3 constructs our object while injecting config parameters into it.
  // We use this to look up the class when giving ns3 applications.
  static ns3::TypeId TypeId();

  Ns3NodeContainerApp();
  Ns3NodeContainerApp(Ns3NodeContainerApp&& rhs);
  Ns3NodeContainerApp& operator=(Ns3NodeContainerApp&& rhs);
  ~Ns3NodeContainerApp();

  void add_component(std::unique_ptr<Component> c);

protected:
  virtual void DoDispose() override;
  virtual void StartApplication() override;
  virtual void StopApplication() override;

private:
  std::uint16_t prop_id;

  Ns3Async async;
  std::unique_ptr<Context> ctx;
  std::unique_ptr<LinkManager> linkmgr;
  std::vector<std::unique_ptr<Component>> components;
  std::unique_ptr<Node> node;
};
}
