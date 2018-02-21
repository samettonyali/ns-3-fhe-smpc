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
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/integer.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/lte-packet-source.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/qos-tag.h"
#include "ns3/qos-id-seq-ts-header.h"



namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LtePacketSource");
NS_OBJECT_ENSURE_REGISTERED (LtePacketSource);

TypeId
LtePacketSource::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LtePacketSource")
    .SetParent<Application> ()
    .AddConstructor<LtePacketSource> ()
    .AddAttribute ("PacketSize", "The size of packets sent in on state",
                   UintegerValue (512),
                   MakeUintegerAccessor (&LtePacketSource::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&LtePacketSource::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("FirstSendingTime", "The first time packet is sent",
                   TimeValue (Seconds (1)),
                    MakeTimeAccessor (&LtePacketSource::m_initialSendingTime),
                    MakeTimeChecker ())
    .AddAttribute ("Interval", "The interval between packet",
                   TimeValue (Seconds (1)),
                   MakeTimeAccessor (&LtePacketSource::m_packetInterval),
                   MakeTimeChecker () )
    .AddAttribute ("ConnectionType", "The connection type",
                   IntegerValue (0),
                   MakeIntegerAccessor (&LtePacketSource::m_connectionType),
                   MakeIntegerChecker<int> ())
    .AddAttribute ("Protocol", "The type of protocol to use.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&LtePacketSource::m_tid),
                   MakeTypeIdChecker ())
    .AddAttribute ("Pseudonym", "the customer id of smart meter",
                    UintegerValue (0),
                    MakeUintegerAccessor (&LtePacketSource::m_pseudonym),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("OperationIdentifier", "The identifier for the operation",
                    UintegerValue (0),
                    MakeUintegerAccessor (&LtePacketSource::m_operationId),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("AccessClass", "access category",
                    UintegerValue (0),
                    MakeUintegerAccessor (&LtePacketSource::m_qosId),
                    MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("Privacy", "privacy information [0], 0=no privacy, 1=privacy",
                    UintegerValue (0),
                    MakeUintegerAccessor (&LtePacketSource::m_privacy),
                    MakeUintegerChecker<uint8_t> ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&LtePacketSource::m_txTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}


LtePacketSource::LtePacketSource ()
  : m_socket (0),
    m_connected (false),
    m_lastStartTime (Seconds (0)),
    m_totBytes (0),
    m_seqnum (0),
    m_pseudonym (0),
    m_operationId (0),
    m_initialSendingTime (Seconds (1)),
    m_packetInterval (Seconds (1)),
    m_connectionType (0),
    m_qosId (0),
    m_privacy (0)
{
  NS_LOG_FUNCTION (this);
}

LtePacketSource::~LtePacketSource()
{
  NS_LOG_FUNCTION (this);
}

Ptr<Socket>
LtePacketSource::GetSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

int64_t 
LtePacketSource::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
 // m_onTime->SetStream (stream);
 // m_offTime->SetStream (stream + 1);
  return 2;
}

void
LtePacketSource::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

// Application Methods
void LtePacketSource::StartApplication () // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);
  CancelEvents ();
  if (m_connectionType==0) {
        NS_LOG_INFO ("run connection " << m_connectionType);
	 if (!m_socket)
    	    {	
      		m_socket = Socket::CreateSocket (GetNode (), m_tid);
      		if (Inet6SocketAddress::IsMatchingType (m_peer))
        	   {
          		m_socket->Bind6 ();
        	   }
      		else if (InetSocketAddress::IsMatchingType (m_peer) ||
              	 PacketSocketAddress::IsMatchingType (m_peer))
        	   {
          		m_socket->Bind ();
        	   }	
      		m_socket->Connect (m_peer);
      		m_socket->SetAllowBroadcast (true);
      		m_socket->ShutdownRecv ();

      		m_socket->SetConnectCallback (
        		MakeCallback (&LtePacketSource::ConnectionSucceeded, this),
        		MakeCallback (&LtePacketSource::ConnectionFailed, this));
              NS_LOG_INFO("Connection Establishment First");
    	    }
  } else {
    NS_LOG_INFO ("unable " << m_connectionType);
  }
  //if (!m_connected) return;
  m_sendEvent = Simulator::Schedule (m_initialSendingTime, &LtePacketSource::SendPacket, this);
}

void LtePacketSource::StopApplication () // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  CancelEvents ();
  if(m_socket != 0)
    {
      m_socket->Close ();
    }
  else
    {
      NS_LOG_WARN ("LtePacketSource found null socket to close in StopApplication");
    }
}

void LtePacketSource::CancelEvents ()
{
  NS_LOG_FUNCTION (this);

  if (m_sendEvent.IsRunning ())
    { // Cancel the pending send packet event
      // Calculate residual bits since last packet sent
      Time delta (Simulator::Now () - m_lastStartTime);
    }
  Simulator::Cancel (m_sendEvent);
}


// Private helpers
void LtePacketSource::ScheduleNextTx ()
{
  NS_LOG_FUNCTION (this);
  m_sendEvent = Simulator::Schedule (m_packetInterval,&LtePacketSource::SendPacket, this);
}



void LtePacketSource::SendPacket ()
{
  NS_LOG_FUNCTION (this);
 // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      if (Inet6SocketAddress::IsMatchingType (m_peer))
        {
          m_socket->Bind6 ();
        }
      else if (InetSocketAddress::IsMatchingType (m_peer) ||
               PacketSocketAddress::IsMatchingType (m_peer))
        {
          m_socket->Bind ();
        }
      m_socket->Connect (m_peer);
      m_socket->SetAllowBroadcast (true);
      m_socket->ShutdownRecv ();

      m_socket->SetConnectCallback (
        MakeCallback (&LtePacketSource::ConnectionSucceeded, this),
        MakeCallback (&LtePacketSource::ConnectionFailed, this));
      NS_LOG_INFO("Connection Establishment WHEN THE FIRST PACKET SENT");
    }

  NS_ASSERT (m_sendEvent.IsExpired ());
  QosIdSeqTsHeader seqTs;
  seqTs.SetPrivacy (m_privacy);
  seqTs.SetQosId (m_qosId);
  seqTs.SetCustId (m_pseudonym);
  seqTs.SetOpId (m_operationId);
  seqTs.SetSeq (m_seqnum);
  NS_LOG_INFO ("Size of seqTs: " << seqTs.GetSerializedSize());
  Ptr<Packet> packet = Create<Packet> (m_pktSize-(seqTs.GetSerializedSize())); // 8+4+4+1+1 : the size of the qosidseqTs header
//  Ptr<Packet> packet = Create<Packet> (m_pktSize-(8+4+4+4+1+1)); // 8+4+4+1+1 : the size of the qosidseqTs header
  packet->AddHeader (seqTs);
  //
  QosTag m_tag;
  m_tag.SetTid(m_qosId);
  packet->AddPacketTag (m_tag);
  //
  m_txTrace (packet);
  m_socket->Send (packet);
  m_totBytes += m_pktSize;
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      ++m_seqnum;
      NS_LOG_INFO (" Tx " << packet->GetSize() 
                   << " " << InetSocketAddress::ConvertFrom(m_peer).GetIpv4 ()
                   <<" Uid " << packet->GetUid () 
                   << " Sequence Number: " << seqTs.GetSeq () 
                   << " Consumer Id : " << seqTs.GetCustId ()
                   << " Operation ID : " << seqTs.GetOpId()
                   << " QoS ID : " << (u_int32_t)seqTs.GetQosId()
                   <<" Time " << (Simulator::Now ()).GetSeconds ());
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peer))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s on-off application sent "
                   <<  packet->GetSize () << " bytes to "
                   << Inet6SocketAddress::ConvertFrom(m_peer).GetIpv6 ()
                   << " port " << Inet6SocketAddress::ConvertFrom (m_peer).GetPort ()
                   << " total Tx " << m_totBytes << " bytes");
    }
  m_lastStartTime = Simulator::Now ();
  ScheduleNextTx ();
}


void LtePacketSource::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_INFO("Connection Succeed");
  m_connected = true;
}

void LtePacketSource::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_INFO("Connection Failed");
}

void LtePacketSource::SetPseudonym (uint32_t ps)
{
  m_pseudonym = ps;
}

uint32_t LtePacketSource::GetPseudonym (void) const
{
  return m_pseudonym;
}

void LtePacketSource::SetQosId (uint8_t ps)
{
  m_qosId = ps;
}

uint8_t LtePacketSource::GetQosId (void) const
{
  return m_qosId;
}
} // Namespace ns3
