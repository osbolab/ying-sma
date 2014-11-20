#pragma once

//  ns3 drives executable components by installing Applications into
// simulated nodes; the Application is the subject of events and callbacks
// as the simulation progresses.
//  The container ns3::Application is created, hosted, and driven by the ns3
// simulator and is our source of events from and access to the simulation.
//
//  From this point, any references to that simulator-owned application will
// be as ns3::Application and any references to "the application" refer
// instead to the high-level cross-platform logical application. References
// to nodes refer to the high-level logical nodes defined by the application,
// not to the physical node abstractions defined by ns3; to those I'll refer
// by ns3::Node.
//
//  The container constructs the hosted application (the logical node) from
// the bottom up:
// --------------------------- NETWORKING -----------------------------------
//  1. Create whatever ns3 sockets interface with the simulated topology.
//  2. Create sma::ns3_socket proxies to abstract those to sma types.
//     These wrappers take ownership of the ns3 sockets so the application
//     has the same control over them as with real sockets.
//  3. Create a single-threaded channel that subscribes to callbacks from the
//     ns3 sockets and broadcasts messages to them. The real analog uses
//     posix select() to read from sockets and may implement multiple threads
//     to handle successive reads.
//  4. Create a single-threaded instance of the messenger and give it
//     the channel. The messenger abstracts "receiving a message" away from
//     the network; the high-level application only knows of sending and
//     receiving messages from other logical nodes, which can also be
//     considered instances of itself.
//      Ultimately I would like the application to be unaware of the idea of
//     nodes and instead think in terms of passing messages between actors,
//     but until the networking requirements are more clear that's abstract
//     enough to be limiting.
// ----------------------------- TIMING -------------------------------------
// The application may like to run asynchronous or recurring tasks; normally
// it would do this by manually or with a threadpool scheduling concurrent
// threads, but the simulator is not just single-threaded--it also lives in
// virtual simulation time. Therefore the application must have some way to
// schedule tasks it believes to be concurrent or delayed by some period
// without actually worrying about if either of those things is true.
//  This is accomplished by implementing an abstract task scheduler that, in
// ns3, registers callbacks in the simulated future to execute tasks in the
// application. In a native implementation this may use a threadpool and
// actually run real-time-delayed tasks concurrently, but in ns3 it just
// gives the simulator a chance to do everything that should happen before
// that future time is simulated. If the application gets the right messages
// in the right sequences and its tasks execute with the right frequency it
// shouldn't need to know what the absolute time is.
//  The container is thus ready when a scheduler implementation is proxying
// to and from the simulator.
// ---------------------------- APPLICATION ---------------------------------
//  Once the messenger and scheduling stacks are build we just construct the
// application launcher and feed it the abstract interfaces.
//  For good cross-platformability the application launcher is in total
// control of building and launching its components. It can share the service
// interfaces with its components as long as none doesn't try to access native
// resources like threads.
/*
 *                             ns3::Simulator
 * ---------------------------------------------------------------------------
 * |                             ns3::Node                                   |
 * | ----------------------------------------------------------------------- |
 * | |                     ns3::ApplicationContainer                       | |
 * | | ------------------------------------------------------------------- | |
 * | | |               app_container --> ns3::Application              | | | |
 * | | | --------------------------------------------------------------| | | |
 * | | | |                                                             | | | |
 * | | | |    ------------------      -------------------------------  | | | |
 * | | | |    | -------------- |      |       ns3::Simulator*       |  | | | |
 * | | | |    | |ns3::Socket*| |      -------------------------------  | | | |
 * | | | |    | -------------- |              ns3::scheduler           | | | |
 * | | | |    |   ns3_socket   |                     |                 | | | |
 * | | | |    ------------------                     |                 | | | |
 * | | | |        ns3_channel                        |                 | | | |
 * | | | |            |                              |                 | | | |
 * | | | |            v                              v                 | | | |
 * | | | |        <channel>                     <scheduler>            | | | |
 * | | | |            ^                            ^   |               | | | |
 * | | | |            |                            |   |               | | | |
 * | | | | ----------------------                  |   |               | | | |
 * | | | | |      messenger     |                  |   |               | | | |
 * | | | | ----------------------                  |   |               | | | |
 * | | | |         ^     |                         |   |               | | | |
 * | | | |         |     v                         |   v               | | | |
 * | | | | ----------------------------------------------------------- | | | |
 * | | | | |                      application                        | | | | |
 * | | | | ----------------------------------------------------------- | | | |
 * | | | |                                                             | | | |
 * | | | --------------------------------------------------------------- | | |
 * | | ------------------------------------------------------------------- | |
 * | ----------------------------------------------------------------------- |
 * ---------------------------------------------------------------------------
 */

#include <sma/device.hpp>
#include <sma/message_dispatch.hpp>
#include <sma/app/application.hpp>

#include <sma/ns3/ns3_socket.hpp>
#include <sma/ns3/ns3_channel.hpp>

#include <ns3/ptr.h>
#include <ns3/application.h>

#include <cstdint>
#include <memory>


namespace sma
{

class app_container final : public ns3::Application
{
  using Myt = app_container;

public:
  // ns3 constructs our object while injecting config parameters into it.
  // We use this to look up the class when giving ns3 applications.
  static ns3::TypeId TypeId();

  app_container();
  app_container(Myt&& rhs);
  Myt& operator=(Myt&& rhs);
  virtual ~app_container();

protected:
  virtual void DoDispose() override;
  virtual void StartApplication() override;
  virtual void StopApplication() override;

private:
  void this_device(std::unique_ptr<device> d)
  {
    dev = std::move(d);
  }


  std::unique_ptr<device> dev;
  std::unique_ptr<ns3_socket> sock;
  std::unique_ptr<ns3_channel> chan;
  std::unique_ptr<message_dispatch> msgr;
  std::unique_ptr<application> app;
};
}
