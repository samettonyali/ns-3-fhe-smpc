/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Universitat Autònoma de Barcelona
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Rubén Martínez <rmartinez@deic.uab.cat>
 */

#include "ltp-udp-convergence-layer-adapter.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/socket.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/tcp-header.h"
#include "ns3/seq-ts-header.h"

#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"

namespace ns3 {
//namespace ltp {

NS_LOG_COMPONENT_DEFINE ("LtpUdpConvergenceLayerAdapter");

//NS_OBJECT_ENSURE_REGISTERED (LtpUdpConvergenceLayerAdapter);


LtpUdpConvergenceLayerAdapter::LtpUdpConvergenceLayerAdapter ()
  : 
    m_ltp (0),
    m_ltpRouting (0),
    m_serverPort (0),
    m_keepAliveValue (0),
    m_rcvSocket (0),
    m_rcvSocket6 (0),
    m_l4SendSockets (),
    m_l4ReceiveSockets ()
{
  NS_LOG_FUNCTION (this);

  m_linkUp = MakeNullCallback< void, Ptr<LtpConvergenceLayerAdapter> > ();
  m_linkDown = MakeNullCallback< void, Ptr<LtpConvergenceLayerAdapter>  > ();
  m_checkpointSent = MakeNullCallback< void, SessionId, RedSegmentInfo> ();
  m_reportSent = MakeNullCallback< void, SessionId, RedSegmentInfo> ();
  m_cancelSent = MakeNullCallback< void, SessionId > ();
  m_endOfBlockSent = MakeNullCallback< void, SessionId> ();

}

LtpUdpConvergenceLayerAdapter::~LtpUdpConvergenceLayerAdapter ()
{
  NS_LOG_FUNCTION (this);
}

bool LtpUdpConvergenceLayerAdapter::EnableReceive (const uint64_t &localLtpEngineId)
{
  NS_LOG_FUNCTION (this << localLtpEngineId);
  TypeId tid;
  //tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
  
  InetSocketAddress inetAddr = InetSocketAddress (Ipv4Address::GetAny (),m_serverPort);

  if (m_ltp)
    {
      m_rcvSocket = Socket::CreateSocket (m_ltp->GetNode (), tid);
      m_rcvSocket6 = Socket::CreateSocket (m_ltp->GetNode (), tid);

      m_rcvSocket->Bind (inetAddr);
      m_rcvSocket->Listen ();
      m_rcvSocket->ShutdownSend ();

      m_rcvSocket->SetRecvCallback (MakeCallback (&LtpUdpConvergenceLayerAdapter::Receive,  this));
      m_rcvSocket->SetAcceptCallback (
              MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
              MakeCallback (&LtpUdpConvergenceLayerAdapter::HandleAccept, this));
      
      m_rcvSocket->SetCloseCallbacks (
              MakeCallback (&LtpUdpConvergenceLayerAdapter::HandlePeerClose, this),
              MakeCallback (&LtpUdpConvergenceLayerAdapter::HandlePeerError, this));

      m_rcvSocket6->Bind6 ();
      m_rcvSocket6->Listen ();
      m_rcvSocket6->ShutdownSend ();
      m_rcvSocket6->SetRecvCallback (MakeCallback (&LtpUdpConvergenceLayerAdapter::Receive,  this));
      
      m_rcvSocket6->SetAcceptCallback (
              MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
              MakeCallback (&LtpUdpConvergenceLayerAdapter::HandleAccept, this));
      
      m_rcvSocket6->SetCloseCallbacks (
              MakeCallback (&LtpUdpConvergenceLayerAdapter::HandlePeerClose, this),
              MakeCallback (&LtpUdpConvergenceLayerAdapter::HandlePeerError, this));

      return true;
    }

  else
    {
      NS_LOG_DEBUG ("Protocol instance is not assigned");
      return false;
    }
}

bool LtpUdpConvergenceLayerAdapter::EnableSend (){
    NS_LOG_FUNCTION(this);
    
    Address addr = m_ltpRouting->GetRoute (m_peerLtpEngineId);
  
    NS_LOG_INFO("Local Engine ID: " << m_ltp->GetLocalEngineId() << " Peer Engine ID: " << m_peerLtpEngineId << " Peer Address: " << InetSocketAddress::ConvertFrom(addr).GetIpv4());

    std::map<uint64_t, Ptr<Socket> >::const_iterator it_sock;
    it_sock = m_l4SendSockets.find (m_peerLtpEngineId);

    if (it_sock == m_l4SendSockets.end ())
    {
        TypeId tid;
        tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
        current_socket = Socket::CreateSocket (m_ltp->GetNode (), tid);

        if (m_ltpRouting->GetAddressMode () == 0)
        {
            current_socket->Bind ();
        }
        else current_socket->Bind6();
      
        current_socket->Connect (addr);

        Ipv4Header iph;
        TcpHeader tcph;

//      NS_LOG_INFO("IP Header Size: " << iph.GetSerializedSize ());        
//      NS_LOG_INFO("TCP Header Size: " << tcph.GetSerializedSize ());        
//      NS_LOG_INFO("MTU: " << mtu);
      
        current_socket->SetAllowBroadcast (true);
        current_socket->ShutdownRecv ();
        current_socket->SetConnectCallback (
            MakeCallback (&LtpUdpConvergenceLayerAdapter::ConnectionSucceeded, this),
            MakeCallback (&LtpUdpConvergenceLayerAdapter::ConnectionFailed, this));
      
        m_l4SendSockets.insert (std::pair<uint64_t, Ptr<Socket> > (m_peerLtpEngineId,current_socket));
    }
    else
    {
        NS_LOG_DEBUG ("LtpUdpConvergenceLayerAdapter:: Reuse socket");
        current_socket = it_sock->second;
    }
    
    return true;
}

void LtpUdpConvergenceLayerAdapter::DisposeSensorApp(){
    current_socket = 0;
}

void LtpUdpConvergenceLayerAdapter::DisposeAggSensorApp(){
    DisposeSensorApp();
    m_rcvSocket = 0;
    m_rcvSocket6 = 0;
    
    m_l4ReceiveSockets.clear ();
}

void LtpUdpConvergenceLayerAdapter::StopSensorApp(){
    if(current_socket != 0)
    {
        current_socket->Close ();
    }
    else
    {
        NS_LOG_WARN ("current_socket found null socket to close in StopApplication");
    }
}

void LtpUdpConvergenceLayerAdapter::StopAggSensorApp(){
    StopSensorApp();
    
    while(!m_l4ReceiveSockets.empty ()) //these are accepted sockets, close them
    {
      Ptr<Socket> acceptedSocket = m_l4ReceiveSockets.front ();
      m_l4ReceiveSockets.pop_front ();
      acceptedSocket->Close ();
    }    
    
    if(m_rcvSocket != 0)
    {
        m_rcvSocket->Close ();
    }
    else
    {
        NS_LOG_WARN ("m_rcvSocket found null socket to close in StopApplication");
    }
    
    if(m_rcvSocket6 != 0)
    {
        m_rcvSocket6->Close ();
    }
    else
    {
        NS_LOG_WARN ("m_rcvSocket6 found null socket to close in StopApplication");
    }
}

void LtpUdpConvergenceLayerAdapter::HandlePeerClose (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 
void LtpUdpConvergenceLayerAdapter::HandlePeerError (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

void LtpUdpConvergenceLayerAdapter::HandleAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  s->SetRecvCallback (MakeCallback (&LtpUdpConvergenceLayerAdapter::Receive, this));
  m_l4ReceiveSockets.push_back (s);
}

TypeId
LtpUdpConvergenceLayerAdapter::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LtpUdpConvergenceLayerAdapter")
    .SetParent<LtpConvergenceLayerAdapter> ()
    .AddConstructor<LtpUdpConvergenceLayerAdapter> ()
    .AddAttribute ("ServerPort", "UDP port to listen for incoming transmissions",
                   UintegerValue (1113),
                   MakeUintegerAccessor (&LtpUdpConvergenceLayerAdapter::m_serverPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("KeepAlive", "Keep-Alive Timeout",
                   UintegerValue (15),
                   MakeUintegerAccessor (&LtpUdpConvergenceLayerAdapter::m_keepAliveValue),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&LtpUdpConvergenceLayerAdapter::m_peerAddress),
                   MakeAddressChecker ())
  ;
  return tid;
}


uint32_t LtpUdpConvergenceLayerAdapter::Send (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  NS_LOG_INFO("Send: Size of packet: " << p->GetSize());
  
  LtpHeader header;
  uint32_t headerBytes = p->RemoveHeader(header);
  SeqTsHeader seqTs;
  p->RemoveHeader(seqTs);
  
  NS_LOG_INFO("Sender meter in header: " << header.GetSmartMeterID());
  NS_LOG_INFO("Conveyor meter: " << GetProtocol()->GetNode()->GetId());
  NS_LOG_INFO("Receiver meter: " << GetRemoteEngineId());
  NS_LOG_INFO("Size of the header: " << headerBytes);
  NS_LOG_INFO("Fragment ID: " << uint32_t(header.GetFragmentID()));
  NS_LOG_INFO("Fragment Size: " << uint32_t(header.GetFragmentSize()));
  
  NS_LOG_INFO ("[node "  << GetProtocol()->GetNode()->GetId() << "]: Tx " << p->GetSize() 
                   << " " << InetSocketAddress::ConvertFrom(m_ltpRouting->GetRoute (m_peerLtpEngineId)).GetIpv4 ()
                   << ":" << InetSocketAddress::ConvertFrom(m_ltpRouting->GetRoute (m_peerLtpEngineId)).GetPort()
                   << " Sequence Number: " << seqTs.GetSeq () 
                   <<" Time " << (Simulator::Now ()).GetSeconds ());
  
  if(current_socket == NULL) NS_LOG_INFO("Bu null la!");
  
  p->AddHeader (seqTs);
  p->AddHeader (header);
  
  uint32_t bytes = current_socket->Send (p);
  NS_LOG_INFO("Sent bytes: " << bytes);

  return bytes;
}

void LtpUdpConvergenceLayerAdapter::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_INFO("Node " << GetProtocol()->GetNode()->GetId() << " successfully established a TCP connection at " << Simulator::Now().GetSeconds() << "s!!!");
  m_connected = true;
}

void LtpUdpConvergenceLayerAdapter::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

void LtpUdpConvergenceLayerAdapter::Receive (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << " " << socket);

  Ptr<Packet> packet;
  Address peer;
  while ((packet = socket->RecvFrom (peer)))
    {
      NS_LOG_INFO("LtpUdpConvergenceLayerAdapter::Receive: " << packet->GetSize() << "-byte packet received from " << InetSocketAddress::ConvertFrom(peer).GetIpv4 ());
	Simulator::ScheduleNow ( &LtpProtocol::Receive, m_ltp, packet, this, peer);
    }
}

uint16_t LtpUdpConvergenceLayerAdapter::GetMtu () const
{
  NS_LOG_FUNCTION (this);

  Address addr = m_ltpRouting->GetRoute (m_peerLtpEngineId);

  Ptr<Socket> socket = Socket::CreateSocket (m_ltp->GetNode (), TcpSocketFactory::GetTypeId ()/*UdpSocketFactory::GetTypeId ()*/);
  socket->Bind ();
  socket->Connect (addr);
  /*socket->SetAllowBroadcast (true);
  socket->ShutdownRecv ();
  socket->SetConnectCallback (
    MakeCallback (&LtpUdpConvergenceLayerAdapter::ConnectionSucceeded, this),
    MakeCallback (&LtpUdpConvergenceLayerAdapter::ConnectionFailed, this));*/

  Ipv4Header iph;
  //UdpHeader udph;
  TcpHeader udph;

  uint16_t  mtu =  socket->GetTxAvailable () - iph.GetSerializedSize () - udph.GetSerializedSize ();

  return (uint16_t) mtu;

}


void LtpUdpConvergenceLayerAdapter::SetProtocol (Ptr<LtpProtocol> prot)
{
  NS_LOG_FUNCTION (this << " " << prot);
  m_ltp = prot;
}

Ptr<LtpProtocol> LtpUdpConvergenceLayerAdapter::GetProtocol () const
{
  NS_LOG_FUNCTION (this);
  return m_ltp;
}

void LtpUdpConvergenceLayerAdapter::SetRoutingProtocol (Ptr<LtpIpResolutionTable> prot)
{
  NS_LOG_FUNCTION (this << " " << prot);
  m_ltpRouting = prot;
}

Ptr<LtpIpResolutionTable> LtpUdpConvergenceLayerAdapter::GetRoutingProtocol () const
{
  NS_LOG_FUNCTION (this);
  return m_ltpRouting;
}




//} //namespace ltp
} //namespace ns3
