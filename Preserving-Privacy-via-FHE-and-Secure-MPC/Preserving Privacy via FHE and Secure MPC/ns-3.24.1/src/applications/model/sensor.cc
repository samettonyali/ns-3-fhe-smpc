/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//
// Copyright (c) 2006 Georgia Tech Research Corporation
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: George F. Riley<riley@ece.gatech.edu>
//

// ns3 - On/Off Data Source Application class
// George F. Riley, Georgia Tech, Spring 2007
// Adapted from ApplicationOnOff in GTNetS.

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/sensor.h"
#include "ns3/applications-module.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/seq-ts-header.h"

namespace ns3 {
    
    //using namespace ltp;
NS_LOG_COMPONENT_DEFINE ("Sensor");
NS_OBJECT_ENSURE_REGISTERED (Sensor);

TypeId
Sensor::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Sensor")
    .SetParent<Application> ()
    .AddConstructor<Sensor> ()
    .AddAttribute ("PacketSize", "The size of packets sent in on state",
                   UintegerValue (512),
                   MakeUintegerAccessor (&Sensor::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1))
//    .AddAttribute ("Remote", "The address of the destination",
//                   AddressValue (),
//                   MakeAddressAccessor (&Sensor::m_peer),
//                   MakeAddressChecker ())
//    .AddAttribute ("OnTime", "A RandomVariableStream used to pick the duration of the 'On' state.",
//                   StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
//                   MakePointerAccessor (&Sensor::m_onTime),
//                   MakePointerChecker <RandomVariableStream>())
//    .AddAttribute ("OffTime", "A RandomVariableStream used to pick the duration of the 'Off' state.",
//                   StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"),
//                   MakePointerAccessor (&Sensor::m_offTime),
//                   MakePointerChecker <RandomVariableStream>())
    .AddAttribute ("Mode", "Type of operation [0=forwarding], 1=aggregation", 
                    UintegerValue (0),
                   MakeUintegerAccessor (&Sensor::m_operation_type),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("MaxFragment", 
                   "The max number of bytes to send in one segment.",
                   UintegerValue (1500),
                   MakeUintegerAccessor (&Sensor::m_maxFragment),
                   MakeUintegerChecker<uint32_t> ())
  .AddAttribute ("FirstTime", "the first time for sending data",
                    TimeValue (Seconds (15)),
                   MakeTimeAccessor (&Sensor::m_firstTime),
                   MakeTimeChecker ())
    .AddAttribute ("Protocol", "The type of protocol to use.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&Sensor::m_tid),
                   MakeTypeIdChecker ())
    .AddAttribute ("Interval", "the interval for sending data",
                    TimeValue (Seconds (30)),
                   MakeTimeAccessor (&Sensor::m_interval),
                   MakeTimeChecker ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&Sensor::m_txTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

TypeId
Sensor::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}


Sensor::Sensor ()
  :// m_socket (0),
   // m_connected (false),
    m_lastStartTime (Seconds (0)),
    m_totBytes (0),
    m_seqnum (0),
    m_maxFragment (1500),
    m_nextTime (0)
{
  NS_LOG_FUNCTION (this);
}

Sensor::Sensor (Ptr <LtpProtocol> protocol, Address address, bool isSender, uint64_t localClientId)
  :// m_socket (0),
    //m_peer (address),
   // m_connected (false),
    m_protocol(protocol),
    m_lastStartTime (Seconds (0)),
    m_totBytes (0),
    m_seqnum (0),
    m_maxFragment (1500),
    m_nextTime (0),
    m_isSender(isSender),
    m_localClientServiceId (localClientId),
    m_bytesSent (0)
{
  NS_LOG_FUNCTION (this);
  CallbackBase cb = MakeCallback (&Sensor::Receive, this);
  m_protocol->RegisterClientService (m_localClientServiceId, cb);
}

Sensor::Sensor (Ptr<LtpProtocol> protocol, Address address, bool isSender, 
          uint64_t localClientId, uint64_t destinationClient, 
          uint64_t destinationLtpEngine, uint32_t bytesToSend, 
          uint32_t blockSize, uint32_t redPartSize)
  : //m_socket (0),
   //m_peer (address),
   // m_connected (false),
    m_protocol(protocol),
    m_lastStartTime (Seconds (0)),
    m_totBytes (0),
    m_seqnum (0),
    m_maxFragment (1500),
    m_nextTime (0),
    m_firstTime (Seconds (20)),
    m_isSender(isSender),
    m_localClientServiceId (localClientId),
    m_destinationClientServiceId (destinationClient),
    m_destinationLtpId (destinationLtpEngine),
    m_bytesToSend (bytesToSend),
    m_bytesSent (0),
    m_blockSize (blockSize),
    m_redPartSize (redPartSize)
{
  NS_LOG_FUNCTION (this);
  CallbackBase cb = MakeCallback (&Sensor::Receive, this);
  m_protocol->RegisterClientService (m_localClientServiceId, cb);
}

Sensor::Sensor (Ptr<LtpProtocol> protocol, Address address, bool isSender, 
          uint64_t localClientId, uint64_t destinationClient, 
          uint64_t destinationLtpEngine)
  : //m_socket (0),
   // m_peer (address),
   // m_connected (false),
    m_protocol(protocol),
    m_lastStartTime (Seconds (0)),
    m_totBytes (0),
    m_seqnum (0),
    m_maxFragment (1500),
    m_nextTime (0),
    m_firstTime (Seconds (20)),
    m_isSender(isSender),
    m_localClientServiceId (localClientId),
    m_destinationClientServiceId (destinationClient),
    m_destinationLtpId (destinationLtpEngine),
    m_bytesSent (0)
{
  NS_LOG_FUNCTION (this);
  CallbackBase cb = MakeCallback (&Sensor::Receive, this);
  m_protocol->RegisterClientService (m_localClientServiceId, cb);
}

Sensor::~Sensor()
{
  NS_LOG_FUNCTION (this);
}

int64_t 
Sensor::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  //m_onTime->SetStream (stream);
  //m_offTime->SetStream (stream + 1);
  return 2;
}

void
Sensor::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_protocol->GetConvergenceLayerAdapter(m_destinationLtpId)->DisposeSensorApp();
  m_protocol = 0;
  // chain up
  Application::DoDispose ();
}

// Application Methods
void Sensor::StartApplication () // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);
  
  m_protocol->GetConvergenceLayerAdapter(m_destinationLtpId)->EnableSend();
  m_sendEvent = Simulator::Schedule (m_firstTime, &Sensor::SendPacket, this);
}

void Sensor::StopApplication () // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  CancelEvents ();
  
  m_protocol->GetConvergenceLayerAdapter(m_destinationLtpId)->StopSensorApp();
}

void Sensor::CancelEvents ()
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_sendEvent);
}

// Private helpers
void Sensor::ScheduleNextTx ()
{
  NS_LOG_FUNCTION (this);
  m_sendEvent = Simulator::Schedule (m_interval, &Sensor::SendPacket, this);
}

void Sensor::Receive (uint32_t seqNum,
                      StatusNotificationCode code,
                      std::vector<uint8_t> data,
                      uint32_t dataLength,
                      bool endFlag,
                      uint64_t srcLtpEngine,
                      uint32_t offset, Time timeStamp ){
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("Sensor: A packet arrived! Code: " << code);
}

void Sensor::SendPacket ()
{
  NS_LOG_FUNCTION (this);
  
  NS_LOG_INFO ("Sensor::SendPacket - Seq#: " << m_seqnum << " From: " << 
               m_localClientServiceId << " To: " << m_destinationLtpId << " " <<
               m_pktSize << " bytes " << "TXtime: " << Simulator::Now() );
  
  std::vector<uint8_t> data (m_pktSize, 0);
  m_protocol->StartTransmission(m_localClientServiceId,
    m_destinationClientServiceId,
    m_destinationLtpId,
    data,
    m_maxFragment, // 536 bytes (default segment size) - 18 bytes (headers)
    m_operation_type,
    m_seqnum);
  
  ++m_seqnum;
  
  m_bytesSent += m_pktSize;
  m_lastStartTime = Simulator::Now ();
  ScheduleNextTx ();
}

ApplicationContainer Sensor::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

Ptr<Application> Sensor::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application> ();
  node->AddApplication (app);

  return app;
}

} // Namespace ns3
