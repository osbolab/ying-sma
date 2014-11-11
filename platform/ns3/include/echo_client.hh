#ifndef ECHO_CLIENT_H_
#define ECHO_CLIENT_H_

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"

namespace sma {

  class EchoClient : public ns3::Application
  {
  public:
    static ns3::TypeId GetTypeId();
    EchoClient();
    virtual ~EchoClient();

  protected:
    virtual void DoDispose();

  private:
    virtual void StartApplication();
    virtual void StopApplication();

    void ScheduleTransmit();
    void Send(ns3::Ptr<ns3::Packet> p);
    void HandleRead(ns3::Ptr<ns3::Socket> socket);

    uint16_t remote_port_;
    ns3::Address remote_addr_;
    ns3::Ptr<ns3::Socket> socket_;

    int num_sent_;
    static const int num_to_send_ = 2;

    ns3::Ptr<ns3::Packet> packet;
  };
}

#endif
