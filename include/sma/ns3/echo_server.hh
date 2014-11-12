#ifndef ECHO_SERVER_H_
#define ECHO_SERVER_H_

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"

namespace sma {

  class EchoServer : public ns3::Application
  {
  public:
    static ns3::TypeId GetTypeId();
    EchoServer();
    virtual ~EchoServer();

  protected:
    virtual void DoDispose();

  private:
    virtual void StartApplication();
    virtual void StopApplication();

    void HandleRead(ns3::Ptr<ns3::Socket> socket);

    uint16_t port_;
    ns3::Ptr<ns3::Socket> socket_;
  };

}

#endif