#pragma once

#include <sma/ns3/ns3async.hpp>
#include <sma/link/linkmanager.hpp>
#include <sma/messagedispatch.hpp>
#include <sma/component.hpp>
#include <sma/ccn/ccnapplication.hpp>

#include <ns3/ptr.h>
#include <ns3/application.h>

#include <cstdint>
#include <memory>


namespace sma
{

class Ns3AppContainer final : public ns3::Application
{
public:
  // ns3 constructs our object while injecting config parameters into it.
  // We use this to look up the class when giving ns3 applications.
  static ns3::TypeId TypeId();

  Ns3AppContainer();
  Ns3AppContainer(Ns3AppContainer&& rhs);
  Ns3AppContainer& operator=(Ns3AppContainer&& rhs);
  ~Ns3AppContainer();

  void add_component(std::unique_ptr<Component> c);

protected:
  virtual void DoDispose() override;
  virtual void StartApplication() override;
  virtual void StopApplication() override;

private:
  Ns3Async async;
  std::unique_ptr<Context> ctx;
  std::unique_ptr<LinkManager> linkmgr;
  std::unique_ptr<MessageDispatch> msgr;
  std::unique_ptr<CcnApplication> app;
};
}
