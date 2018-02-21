/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
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
 * Author:  Tom Henderson (tomhend@u.washington.edu)
 */
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/seq-ts-header.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/qos-utils.h"
#include "vanet-packet-sink.h"


#include <iostream>
#include <sstream>
#include <fstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("VanetPacketSink");
NS_OBJECT_ENSURE_REGISTERED (VanetPacketSink);

TypeId 
VanetPacketSink::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::VanetPacketSink")
    .SetParent<Application> ()
    .AddConstructor<VanetPacketSink> ()
    .AddAttribute ("Local", "The Address on which to Bind the rx socket.",
                   AddressValue (),
                   MakeAddressAccessor (&VanetPacketSink::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("Remote", "The Address of the destination.",
                   AddressValue (),
                   MakeAddressAccessor (&VanetPacketSink::m_target),
                   MakeAddressChecker ())
    .AddAttribute ("UDPTargetAddress", "The Address of the destination.",
                   AddressValue (),
                   MakeAddressAccessor (&VanetPacketSink::m_UDPTarget),
                   MakeAddressChecker ())
    .AddAttribute ("NumberOfNodes", "The default size of packets received",
                   UintegerValue (25),
                   MakeUintegerAccessor (&VanetPacketSink::m_nNodes),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("DCRLMode", "Whether distributed certificate revocation list mode on or off",
                   UintegerValue (0),
                   MakeUintegerAccessor (&VanetPacketSink::m_DCRLMode),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Protocol", "The type id of the protocol to use for the rx socket.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&VanetPacketSink::m_tid),
                   MakeTypeIdChecker ())
    .AddAttribute ("DefaultRxSize", "The default size of packets received",
                   UintegerValue (512),
                   MakeUintegerAccessor (&VanetPacketSink::m_defSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Mode", "The default size of packets received",
                   UintegerValue (1),
                   MakeUintegerAccessor (&VanetPacketSink::m_mode),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Scenario", "The default size of packets received",
                   UintegerValue (1),
                   MakeUintegerAccessor (&VanetPacketSink::m_scenario),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("PacketSize", "The default size of packets to be sent",
                   UintegerValue (512),
                   MakeUintegerAccessor (&VanetPacketSink::m_pktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MeterType", "[0]->GW, [1]->AGG, [2]->LEAF",
                   UintegerValue (1),
                   MakeUintegerAccessor (&VanetPacketSink::m_meterType),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Delay", "The default size of packets received",
                   UintegerValue (0),
                   MakeUintegerAccessor (&VanetPacketSink::m_procDelay),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Child", "The number of child meters of this meter",
                   UintegerValue (1),
                   MakeUintegerAccessor (&VanetPacketSink::m_childNum),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("LeafMeters", "The number of child meters of this meter",
                   UintegerValue (1),
                   MakeUintegerAccessor (&VanetPacketSink::m_leafMeters),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("FileName", "The output filename",
                   StringValue ("roundstat"),
                   MakeStringAccessor (&VanetPacketSink::m_outputFilename),
                   MakeStringChecker ())
    .AddAttribute ("OperationIdentifier", "The identifier for the operation",
                    UintegerValue (0),
                    MakeUintegerAccessor (&VanetPacketSink::m_operationId),
                    MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (10),
                   MakeUintegerAccessor (&VanetPacketSink::m_UDPPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("MultipleTargets", "Whether there are multiple targets.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&VanetPacketSink::m_multTargetFlag),
                   MakeUintegerChecker<uint8_t> ())
    .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&VanetPacketSink::m_rxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&VanetPacketSink::m_txTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

VanetPacketSink::VanetPacketSink ()
{
  NS_LOG_FUNCTION (this);
  m_targetSocket = 0;
  m_UDPTargetSocket = 0;
  m_connected = false;
  m_socket = 0;
  m_totalRx = 0;
  m_seqnum = 0;
  m_totBytes = 0;
  m_lastStartTime = Seconds (0);
  m_meterType = 0;
  m_mode = 0;
  m_scenario = 1;
  m_childNum = 0;
  m_nNodes = 25;
}

VanetPacketSink::VanetPacketSink (uint16_t port, Address local, uint32_t delay)
{
  NS_LOG_FUNCTION (this);
  m_targetSocket = 0;
  m_UDPTargetSocket = 0;
  m_connected = false;
  m_socket = 0;
  m_UDPPort = port;
  m_local = local;
  m_totalRx = 0;
  m_seqnum = 0;
  m_procDelay = delay;
  m_totBytes = 0;
  m_lastStartTime = Seconds (0);
  m_meterType = 0;
  m_mode = 0;
  m_scenario = 1;
  m_childNum = 0;
  m_nNodes = 25;
}

VanetPacketSink::~VanetPacketSink()
{
  NS_LOG_FUNCTION (this);
  m_UDPsocket = 0;
  m_UDPsocket6 = 0;
  StatPrint();
}

uint32_t VanetPacketSink::GetTotalRx () const
{
  NS_LOG_FUNCTION (this);
  return m_totalRx;
}

Ptr<Socket>
VanetPacketSink::GetListeningSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

std::list<Ptr<Socket> >
VanetPacketSink::GetAcceptedSockets (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socketList;
}

void VanetPacketSink::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_targetSocket = 0;
  m_UDPTargetSocket = 0;
  m_socketList.clear ();
  m_targetSockets.clear();

  // chain up
  Application::DoDispose ();
}

// Application Methods
void VanetPacketSink::StartApplication ()    // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);
  
  ///////////////////////////////UDP////////////////////////////////////
  if (m_UDPsocket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_UDPsocket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_UDPPort);
      m_UDPsocket->Bind (local);
      if (addressUtils::IsMulticast (m_UDPlocal))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_UDPsocket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_UDPlocal);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }

  if (m_UDPsocket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_UDPsocket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), m_UDPPort);
      m_UDPsocket6->Bind (local6);
      if (addressUtils::IsMulticast (local6))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_UDPsocket6);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, local6);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }

  m_UDPsocket->SetRecvCallback (MakeCallback (&VanetPacketSink::HandleReadUDP, this));
  m_UDPsocket6->SetRecvCallback (MakeCallback (&VanetPacketSink::HandleReadUDP, this));
  

////////////////////////////////////TCP/////////////////////////////////////////
  
    //[0]->GW, [1]->AGG, [2]->LEAF
  
    // Create the socket if not already
    //GW
    if(m_meterType == (uint32_t)0){
        if(!m_localTCPsocket){
            TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
            m_localTCPsocket = Socket::CreateSocket (GetNode (), tid);
            m_localTCPsocket->Bind (m_local);

            m_localTCPsocket->Listen ();

    //        m_localTCPsocket->ShutdownSend ();

            m_localTCPsocket->SetAcceptCallback (
                MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
                MakeCallback (&VanetPacketSink::HandleAccept, this));
            m_localTCPsocket->SetCloseCallbacks (
                MakeCallback (&VanetPacketSink::HandlePeerClose, this),
                MakeCallback (&VanetPacketSink::HandlePeerError, this));

            m_localTCPsocket->SetRecvCallback (MakeCallback (&VanetPacketSink::HandleReadTCP, this));
        }
    }
    else if(m_meterType == (uint32_t)2){    //meshToEV Meter
//        if(m_multTargetFlag){
//            int nTargets = m_targets.size();
//            TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
//            for(int i=0; i<nTargets; i++){
//                Ptr<Socket> socket = Socket::CreateSocket (GetNode (), tid);
//
//                if (Inet6SocketAddress::IsMatchingType (m_targets[i])){
//                    socket->Bind6 ();
//                }
//                else if (InetSocketAddress::IsMatchingType (m_targets[i]) ||
//                 PacketSocketAddress::IsMatchingType (m_targets[i])){
//                    socket->Bind ();
//                }
//
//                socket->Connect (m_targets[i]);
//                socket->SetAllowBroadcast (true);
//    //            socket->ShutdownRecv ();
//
//                socket->SetConnectCallback (
//                            MakeCallback (&VanetPacketSink::ConnectionSucceeded, this),
//                            MakeCallback (&VanetPacketSink::ConnectionFailed, this));
//
//                socket->SetRecvCallback (MakeCallback (&VanetPacketSink::HandleReadTCP, this));
//
//                m_targetSockets.push_back(socket);
//            }
//        }
//        else{
            if (!m_socket){
                TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
                m_socket = Socket::CreateSocket (GetNode (), tid);

                if (Inet6SocketAddress::IsMatchingType (m_target)){
                    m_socket->Bind6 ();
                }
                else if (InetSocketAddress::IsMatchingType (m_target) ||
                 PacketSocketAddress::IsMatchingType (m_target)){
                    m_socket->Bind ();
                }

                m_socket->Connect (m_target);
                m_socket->SetAllowBroadcast (true);
    //            m_socket->ShutdownRecv ();

                m_socket->SetConnectCallback (
                            MakeCallback (&VanetPacketSink::ConnectionSucceeded, this),
                            MakeCallback (&VanetPacketSink::ConnectionFailed, this));
                
                m_socket->SetRecvCallback (MakeCallback (&VanetPacketSink::HandleReadTCP, this));
            }
//        }
    }

//
//    m_socket->SetRecvCallback (MakeCallback (&VanetPacketSink::HandleReadTCP, this));
//    
//  if(m_target.GetLength() != 0){
//    if (!m_targetSocket)
//      {
//        m_targetSocket = Socket::CreateSocket (GetNode (), m_tid);
//        
//        if(m_tid.GetName() == "ns3::TcpSocketFactory"){
//            NS_LOG_INFO("Target-SMPCPacketSink::StartApplication: ns3::TcpSocketFactory");
//            if (Inet6SocketAddress::IsMatchingType (m_target))
//               {
//                    m_targetSocket->Bind6 ();
//               }
//            else if (InetSocketAddress::IsMatchingType (m_target) ||
//             PacketSocketAddress::IsMatchingType (m_target)){
//                    m_targetSocket->Bind ();
//            }
//            
//            m_targetSocket->SetAllowBroadcast (true);
//            m_targetSocket->ShutdownRecv ();
//
//            m_targetSocket->SetConnectCallback (
//                MakeCallback (&VanetPacketSink::ConnectionSucceeded, this),
//                MakeCallback (&VanetPacketSink::ConnectionFailed, this));
//        }
//        else if(m_tid.GetName() == "ns3::UdpSocketFactory"){
//        }
//        else{
//            NS_LOG_INFO("SMPCPacketSink::StartApplication: There is a problem!!!");
//        }
//
//        m_targetSocket->Connect (m_target);
//        
//        NS_LOG_INFO("Connection Establishment First");
//      }
//  }
//    
//    if(m_UDPTarget.GetLength() != 0){
//      if (!m_UDPTargetSocket)
//      {
//        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
//        m_UDPTargetSocket = Socket::CreateSocket (GetNode (), tid);
//        
//        NS_LOG_INFO("Target-SMPCPacketSink::StartApplication: ns3::TcpSocketFactory");
//        if (Inet6SocketAddress::IsMatchingType (m_UDPTarget))
//           {
//                m_UDPTargetSocket->Bind6 ();
//                m_UDPTargetSocket->Connect (m_UDPTarget);
//           }
//        else if (InetSocketAddress::IsMatchingType (m_UDPTarget) ||
//         PacketSocketAddress::IsMatchingType (m_UDPTarget)){
//                m_UDPTargetSocket->Bind ();
//                m_UDPTargetSocket->Connect (m_UDPTarget);
//        }
//      }
//        
//       m_UDPTargetSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
//       m_UDPTargetSocket->SetAllowBroadcast (true);
//  }
  
}

void VanetPacketSink::StopApplication ()     // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);
  while(!m_socketList.empty ()) //these are accepted sockets, close them
    {
      Ptr<Socket> acceptedSocket = m_socketList.front ();
      m_socketList.pop_front ();
      acceptedSocket->Close ();
    }
  while(!m_targetSockets.empty()){
      Ptr<Socket> targetSocket = m_targetSockets.front();
      m_targetSockets.pop_front();
      targetSocket->Close();
  }
  if (m_socket) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  
  CancelEvents ();
  if(m_targetSocket != 0)
    {
      m_targetSocket->Close ();
    }
  else
    {
      NS_LOG_WARN ("VanetPacketSink found null socket to close in StopApplication");
    }
  
  ///////////////////////////////UDP////////////////////////////////////
  if (m_UDPsocket != 0) 
    {
      m_UDPsocket->Close ();
      m_UDPsocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  if (m_UDPsocket6 != 0) 
    {
      m_UDPsocket6->Close ();
      m_UDPsocket6->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  
}

void VanetPacketSink::CancelEvents ()
{
  NS_LOG_FUNCTION (this);

  if (m_sendEvent.IsRunning ())
    { // Cancel the pending send packet event
      // Calculate residual bits since last packet sent
      Time delta (Simulator::Now () - m_lastStartTime);
    }
  Simulator::Cancel (m_sendEvent);
}

void VanetPacketSink::HandleReadTCP (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
  {
      if (packet->GetSize () == 0)
        { //EOF
          break;
      }
      
      //[0]->GW, [1]->AGG, [2]->LEAF
      
        if(m_meterType == (uint32_t)0){
            if (InetSocketAddress::IsMatchingType (from)){
                NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s GW received " << packet->GetSize () << " bytes from " <<
                InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                InetSocketAddress::ConvertFrom (from).GetPort ());
            }
            else if (Inet6SocketAddress::IsMatchingType (from)){
                NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s GW received " << packet->GetSize () << " bytes from " <<
                Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                Inet6SocketAddress::ConvertFrom (from).GetPort ());
            }
            
            packet->RemoveAllPacketTags ();
            packet->RemoveAllByteTags ();
            
            Simulator::Schedule (NanoSeconds (m_procDelay), &VanetPacketSink::SendToTargetSocket, this, packet, socket);
        }
        else if(m_meterType == (uint32_t)2){
            if (InetSocketAddress::IsMatchingType (from)){
                NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << 
                            "s Mesh-To-EV meter (Node " << GetNode()->GetId() << 
                            ") received " << packet->GetSize () << " bytes from GW " <<
                            InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                            InetSocketAddress::ConvertFrom (from).GetPort ());
            }
            else if (Inet6SocketAddress::IsMatchingType (from)){
                NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << 
                            "s Mesh-To-EV meter (Node " << GetNode()->GetId() << 
                            ") received " << packet->GetSize () << " bytes from GW " <<
                            Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                            Inet6SocketAddress::ConvertFrom (from).GetPort ());
            }
            
            packet->RemoveAllPacketTags ();
            packet->RemoveAllByteTags ();
            
            Simulator::ScheduleNow (&VanetPacketSink::EchoPacket, this, packet);
        }
        else{
            NS_LOG_INFO("We have a problem with meter type!");
        }
  }
}
 //     InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (from);
 //      NS_LOG_INFO("From : " << from << " Socket : " << iaddr.GetIpv4 () << " port: " << iaddr.GetPort () );


void VanetPacketSink::HandleReadUDP (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);

    m_EVsocket = socket;
    
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom (from))){
        if (InetSocketAddress::IsMatchingType (from))
        {
            NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << 
                        "s Mesh-To-EV meter (Node " << GetNode()->GetId() << 
                        ") received " << packet->GetSize () << " bytes from " <<
                        InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                        InetSocketAddress::ConvertFrom (from).GetPort ());
        }
        else if (Inet6SocketAddress::IsMatchingType (from))
        {
            NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << 
                        "s Mesh-To-EV meter (Node " << GetNode()->GetId() << 
                        ") received " << packet->GetSize () << " bytes from " <<
                        Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                        Inet6SocketAddress::ConvertFrom (from).GetPort ());
        }
        
        m_EVfrom = from;

        packet->RemoveAllPacketTags ();
        packet->RemoveAllByteTags ();
        
        if(m_DCRLMode){
            NS_LOG_INFO("DCRL Mode: " << m_DCRLMode);
            Simulator::Schedule (NanoSeconds (m_procDelay), &VanetPacketSink::EchoPacket, this, packet);
        }
        else{
            Simulator::Schedule (NanoSeconds (m_procDelay), &VanetPacketSink::SendToTarget, this, packet);
        }

//        Simulator::Schedule (Seconds(2.0), &VanetPacketSink::EchoPacket, this, packet);
        
    }
    
//  NS_LOG_FUNCTION (this << socket);
//  Ptr<Packet> packet;
//  Address from;
//  while ((packet = socket->RecvFrom (from)))
//    {
//      if (packet->GetSize () == 0)
//        { //EOF
//          break;
//        }
// //     InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (from);
// //      NS_LOG_INFO("From : " << from << " Socket : " << iaddr.GetIpv4 () << " port: " << iaddr.GetPort () );
//
//    std::stringstream ss;
//    ss << m_nNodes;
//    std::string opIdStr;
//    ss >> opIdStr;
//    std::ostringstream os;
//    os << "10.1.1." << opIdStr;
//    Ipv4Address controlIP = Ipv4Address (os.str().c_str());
//    
//    Ipv4Address senderIP =  InetSocketAddress::ConvertFrom (from).GetIpv4 ();
//    if(senderIP.IsEqual(controlIP)){
//       m_defSize = (uint16_t)1084;
//    }
//    else if(senderIP.IsEqual(Ipv4Address ("10.1.2.1"))){
//       m_defSize = (uint16_t)80;
//    }
//    else if(senderIP.IsEqual(Ipv4Address ("10.1.1.1"))){
//       m_defSize = (uint16_t)80;
//    }
//      
//      uint16_t m_pktsize = packet->GetSize();
//      m_totalRx += packet->GetSize ();
//      m_rxTrace (packet, from);
//      // check whether there is a previous save packet first
//      Ptr<Packet> m_prevPacket = packet;
//      
////==============================================================================      
////      if(packet->GetSize () == 80)
////        HandleReport(m_prevPacket,from);
////==============================================================================
//      
//      uint16_t m_savePktSize = 0;
//      if (IsThereAnyPreviousStoredPacket(from)) {
//         for(std::vector<DataWaitingPacket>::iterator it = m_waitingPacket.begin(); it != m_waitingPacket.end(); ++it) {
//	    if(it->from == from) {
//	          m_prevPacket = it->pkt;
//	          m_savePktSize = m_prevPacket->GetSize();
//	          break;
//	    }
//         }   
//      }
//      if (m_savePktSize > 0) {  // there is a previously saved packet, m_prevPacket is concatenation of previous and received
//         // concatenate 
//         NS_LOG_INFO("Concatenate previous stored packet and the received packet " << m_savePktSize << " " << m_pktsize );
//         m_prevPacket->AddAtEnd (packet);
//         m_pktsize = m_prevPacket->GetSize ();    // new pkt size from concatenation
//         // delete the old record
//         NS_LOG_INFO("Delete previous stored packet ! " << from << " " << m_savePktSize);
//         if (m_waitingPacket.size() > 1) {
//            std::vector<DataWaitingPacket> tmp (m_waitingPacket); // copy to temp
//            m_waitingPacket.clear();
//            
//            for(std::vector<DataWaitingPacket>::iterator itw = tmp.begin() ; itw != tmp.end(); ++itw) {
//	        if(itw->from != from) { // keep maintain in the waiting list
//	            DataWaitingPacket keep;
//	            keep.from = itw->from;
//	            keep.pkt   = itw->pkt;
//	            m_waitingPacket.push_back(keep);
//	            NS_LOG_INFO("Keep waiting packet " << keep.from << " " << keep.pkt->GetSize() << " " << m_waitingPacket.size () );
//	        }
//             }
//         } else m_waitingPacket.clear();
//      } else m_prevPacket = packet; // there were saved packets, but none was from this address
//      
//      if (m_pktsize == m_defSize) {
//          HandleReport(m_prevPacket,from);
//      } else {
//        // two cases, > and <, if higher, split them
//      	if (m_pktsize > m_defSize) {
//      	    uint16_t m_begin = 0;
//      	    uint16_t m_length = m_defSize;
//      	    while (m_pktsize >= m_defSize) {
//      	        NS_LOG_INFO("Split packet : " << m_pktsize << " from : " << m_begin << " length " << m_length);
//      	        Ptr<Packet> frag = m_prevPacket->CreateFragment(m_begin, m_length);
//      	        HandleReport(frag, from);
//      	        m_begin += (m_length);
//      	        m_pktsize -= m_defSize;
//      	        if (m_pktsize >= m_defSize) m_length = m_defSize;
//      	        else {
//      	          m_length = m_pktsize;
//      	        }
//      	    }
//      	    if (m_pktsize > 0) {
//               DataWaitingPacket tmp;
//               tmp.from = from;
//               tmp.pkt  = m_prevPacket->CreateFragment(m_begin, m_length);
//               m_waitingPacket.push_back(tmp);
//	       NS_LOG_INFO("add the rest of the packet in the waiting packet " << tmp.from << " " << tmp.pkt->GetSize() << " " << m_waitingPacket.size () );               
//      	    }
//      	} else {
//           DataWaitingPacket tmp;
//           tmp.from = from;
//           tmp.pkt  = m_prevPacket;
//           m_waitingPacket.push_back(tmp);
//           NS_LOG_INFO("add waiting packet " << tmp.from << " " << tmp.pkt->GetSize() << " " << m_waitingPacket.size () );               
//      	} // end of else m_pktsize > m_defSize
//      } // end else m_pktsize == m_defSize	  
//    } // end while
}

void VanetPacketSink::EchoPacket(Ptr<Packet> packet){
    NS_LOG_LOGIC ("Echoing packet");
    m_EVsocket->SendTo (packet, 0, m_EVfrom);

//    if (InetSocketAddress::IsMatchingType (m_EVfrom))
//    {
//      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
//                   InetSocketAddress::ConvertFrom (m_EVfrom).GetIpv4 () << " port " <<
//                   InetSocketAddress::ConvertFrom (m_EVfrom).GetPort ());
//    }
//    else if (Inet6SocketAddress::IsMatchingType (m_EVfrom))
//    {
//      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
//                   Inet6SocketAddress::ConvertFrom (m_EVfrom).GetIpv6 () << " port " <<
//                   Inet6SocketAddress::ConvertFrom (m_EVfrom).GetPort ());
//    }
}

void VanetPacketSink::SendToTarget(Ptr<Packet> packet){
    NS_LOG_LOGIC ("Send the Request to the target");
    
    if (!m_socket){
        TypeId tid = TypeId::LookupByName ("ns3::TcpSocketFactory");
        m_socket = Socket::CreateSocket (GetNode (), tid);
        
        if (Inet6SocketAddress::IsMatchingType (m_target)){
            m_socket->Bind6 ();
        }
        else if (InetSocketAddress::IsMatchingType (m_target) ||
         PacketSocketAddress::IsMatchingType (m_target)){
            m_socket->Bind ();
        }

        m_socket->SetAllowBroadcast (true);
        m_socket->ShutdownRecv ();

        m_socket->SetConnectCallback (
            MakeCallback (&VanetPacketSink::ConnectionSucceeded, this),
            MakeCallback (&VanetPacketSink::ConnectionFailed, this));

        m_socket->Connect (m_target);
        NS_LOG_INFO("Connection Establishment WHEN THE FIRST PACKET SENT");
    }
    
    m_txTrace (packet);
    m_socket->Send (packet);
    
//    if (InetSocketAddress::IsMatchingType (m_target)){
//        NS_LOG_INFO (" Tx " << packet->GetSize() 
//                   << " " << InetSocketAddress::ConvertFrom(m_target).GetIpv4 ()
//                   << ":" << InetSocketAddress::ConvertFrom(m_target).GetPort()
//                   <<" Uid " << packet->GetUid () 
//                   <<" Time " << (Simulator::Now ()).GetSeconds ());
//    }
//    else if (Inet6SocketAddress::IsMatchingType (m_target)){
//        NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
//                    << "s on-off application sent "
//                    <<  packet->GetSize () << " bytes to "
//                    << Inet6SocketAddress::ConvertFrom(m_target).GetIpv6 ()
//                    << " port " << Inet6SocketAddress::ConvertFrom (m_target).GetPort ());
//    }
}

void VanetPacketSink::SendToTargetSocket(Ptr<Packet> packet, Ptr<Socket> socket){
    NS_LOG_LOGIC ("Send the Request to the target");
    
    m_txTrace (packet);
    socket->Send (packet);
}

void VanetPacketSink::HandleReport(Ptr<Packet> packet, Address from) 
{
   SeqTsHeader seqTs;
   uint32_t m_rxBytes;
   m_rxBytes = packet->GetSize (); 
   packet->PeekHeader (seqTs);
   uint32_t seqNum;
   m_seqnum = seqNum = seqTs.GetSeq ();
 
   Time now = Simulator::Now ();
   Time txtime = seqTs.GetTs ();
   
    if(m_meterType == (uint32_t)0){
       bool lNewRecord = true;
       for(std::vector<StatRecord>::iterator it = m_stat.begin(); it != m_stat.end(); ++it) {
          if(it->round == seqNum) {
            it->rxCount++;
            it->rxBytes += m_rxBytes;
            it->totDelay += (now.ToInteger (Time::US) - txtime.ToInteger (Time::US));
            it->lastRxTime = now;
            if (it->minTxTime > txtime) it->minTxTime = txtime;
            lNewRecord = false;
            break;
          }      
       }

        if (lNewRecord) {
           StatRecord tmp;
           tmp.round = seqNum;
           tmp.rxCount = 1;
           tmp.rxBytes = m_rxBytes;
           tmp.totDelay = (now.ToInteger (Time::US) - txtime.ToInteger (Time::US) );
           tmp.firstRxTime = now;
           tmp.lastRxTime = now;
           tmp.minTxTime = txtime;
           m_stat.push_back(tmp);
        }
    }
  
   if (InetSocketAddress::IsMatchingType (from))
      {
        std::stringstream ss;
        ss << m_nNodes;
        std::string opIdStr;
        ss >> opIdStr;
        std::ostringstream os;
        os << "10.1.1." << opIdStr;
        Ipv4Address controlIP = Ipv4Address (os.str().c_str());
       
       Ipv4Address senderIP =  InetSocketAddress::ConvertFrom (from).GetIpv4 ();
       if(senderIP.IsEqual(Ipv4Address ("10.1.2.1"))){
           NS_LOG_INFO ("XXX" << " SeqNum: " << seqNum << " TXtime: " << seqTs.GetTs ());
       }
       else if(senderIP.IsEqual(controlIP)){
           NS_LOG_INFO ("YYY" << " SeqNum: " << seqNum << " RXtime: " << now);
       }
       else{
         NS_LOG_INFO (InetSocketAddress::ConvertFrom (m_local).GetIpv4 () 
                      << ":" << InetSocketAddress::ConvertFrom (m_local).GetPort()
                      <<" RX " << m_rxBytes 
                      << " " << InetSocketAddress::ConvertFrom (from).GetIpv4 () 
                      << ":" << InetSocketAddress::ConvertFrom (from).GetPort()
                      << " " << seqTs.GetSeq () 
                      << " SeqNum: " << seqNum 
                      << " TXtime: " << seqTs.GetTs () 
                      << " RXtime: " << now );
       }
      }
   else if (Inet6SocketAddress::IsMatchingType (from))
      {
         NS_LOG_INFO ("TraceDelay: RX " << m_rxBytes <<
                      " bytes from "<< Inet6SocketAddress::ConvertFrom (from).GetIpv6 () <<
                      " Sequence Number: " << seqTs.GetSeq() <<
                      " Uid: " << packet->GetUid () <<
                      " TXtime: " << seqTs.GetTs () <<
                      " RXtime: " << Simulator::Now () <<
                      " Delay: " << Simulator::Now () - seqTs.GetTs ());
       }
   
   Ipv4Address senderIP =  InetSocketAddress::ConvertFrom (from).GetIpv4 ();
   if(senderIP.IsEqual(Ipv4Address ("10.1.2.1"))){
       if(m_scenario == (uint32_t)1)
           m_sendEvent = Simulator::Schedule (NanoSeconds(0.0), &VanetPacketSink::SendPacket, this, seqNum);
   }
   else if(senderIP.IsEqual(Ipv4Address ("10.1.1.1"))){
       if(m_scenario == (uint32_t)1)
           m_sendEvent = Simulator::Schedule (NanoSeconds(116828508.0), &VanetPacketSink::SendPacket, this, seqNum);
   }
   
//    //Round-trip communication delay over LTE = 116828508 ns
       
   
//   uint16_t port = InetSocketAddress::ConvertFrom (m_local).GetPort();

//    if(m_meterType == (uint32_t)0){ //gateway meter
//        MeterSeqNumMap::iterator itSeq;
//        itSeq = m_meterSeqNumMap.find(seqNum);
//        
//        NS_LOG_INFO("The gateway has received a packet!!! # children: " << m_childNum);
//        
//        if(itSeq == m_meterSeqNumMap.end()){
//            m_meterSeqNumMap.insert(std::make_pair(seqNum, (uint32_t)1));
//            
//            if(m_childNum == 1){
//                NS_LOG_INFO("Gateway Sequence " << seqNum << " was completed!");
//            }
//        }
//        else{
//            uint32_t seqNumCounter = itSeq->second;
//            seqNumCounter++;
//            if(seqNumCounter == m_childNum){
//                NS_LOG_INFO("Gateway Sequence " << seqNum << " was completed!");
//            }
//            else{
//                NS_LOG_INFO("Gateway Counter is " << seqNumCounter << " for Sequence " << seqNum);
//                m_meterSeqNumMap.erase(seqNum);
//                m_meterSeqNumMap.insert(std::make_pair(seqNum, seqNumCounter));
//            }
//        }
//    }
//    else if(m_meterType == (uint32_t)1){    //aggregator meter
//        if(port == (uint16_t)7000){
//            NS_LOG_INFO("Aggregator " << GetNode()->GetId() << " has received a packet from the gateway!!!!");
//        }
//        else{
//            MeterSeqNumMap::iterator itSeq;
//            itSeq = m_meterSeqNumMap.find(seqNum);
//
//            NS_LOG_INFO("Aggregator " << GetNode()->GetId() << " has received a packet!!! # children: " << m_childNum);
//
//            if(itSeq == m_meterSeqNumMap.end()){
//                m_meterSeqNumMap.insert(std::make_pair(seqNum, (uint32_t)1));
//                if(m_childNum == (uint32_t)1){
//                    NS_LOG_INFO("Aggregator " << GetNode()->GetId() << " Sequence " << seqNum << " was completed!");
//                    NS_LOG_INFO("A send operation is scheduled after " << m_procDelay << " nanoseconds.");
//                    m_sendEvent = Simulator::Schedule (NanoSeconds(m_procDelay), &VanetPacketSink::SendPacket, this);
//                }
//            }
//            else{
//                uint32_t seqNumCounter = itSeq->second;
//                seqNumCounter++;
//                if(seqNumCounter == m_childNum){
//                    NS_LOG_INFO("Aggregator " << GetNode()->GetId() << " Sequence " << seqNum << " was completed!");
//                    NS_LOG_INFO("A send operation is scheduled after " << m_procDelay << " nanoseconds.");
//                    m_sendEvent = Simulator::Schedule (NanoSeconds(m_procDelay), &VanetPacketSink::SendPacket, this);
//                }
//                else{
//                    NS_LOG_INFO("Aggregator Counter is " << seqNumCounter << " for Sequence " << seqNum);
//                    m_meterSeqNumMap.erase(seqNum);
//                    m_meterSeqNumMap.insert(std::make_pair(seqNum, seqNumCounter));
//                }
//            }
//        }
//    }
//    else if(m_meterType == (uint32_t)2){    //leaf meter
//        NS_LOG_INFO("Leaf " << GetNode()->GetId() << " has received a packet!!! Sequence # = " << seqNum);
//        NS_LOG_INFO("A send operation is scheduled after " << m_procDelay << " nanoseconds.");
//        m_sendEvent = Simulator::Schedule (NanoSeconds(m_procDelay), &VanetPacketSink::SendPacket, this);
//    }
//    else{    //error!!!
//       NS_LOG_INFO("There is a problem here!!!");
//    }
}

void VanetPacketSink::SendPacket (uint32_t seqNum)
{
  NS_LOG_FUNCTION (this);
 // Create the socket if not already

  //NS_ASSERT (m_sendEvent.IsExpired ());
  SeqTsHeader seqTs;
  seqTs.SetSeq (seqNum);
  NS_LOG_INFO ("PacketSink: Size of seqTs: " << seqTs.GetSerializedSize());
  Ptr<Packet> packet = Create<Packet> (m_pktSize-(seqTs.GetSerializedSize()));
  packet->AddHeader (seqTs);
  
  m_txTrace (packet);
  m_targetSocket->Send (packet);
  m_totBytes += m_pktSize;
  if (InetSocketAddress::IsMatchingType (m_target))
    {
      //++m_seqnum;
      NS_LOG_INFO ("PacketSink: Tx " << packet->GetSize() 
                   << " " << InetSocketAddress::ConvertFrom(m_target).GetIpv4 ()
                   << ":" << InetSocketAddress::ConvertFrom(m_target).GetPort()
                   <<" Uid " << packet->GetUid () 
                   << " Sequence Number: " << seqTs.GetSeq () 
                   <<" Time " << (Simulator::Now ()).GetSeconds ());
    }
  else if (Inet6SocketAddress::IsMatchingType (m_target))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s on-off application sent "
                   <<  packet->GetSize () << " bytes to "
                   << Inet6SocketAddress::ConvertFrom(m_target).GetIpv6 ()
                   << " port " << Inet6SocketAddress::ConvertFrom (m_target).GetPort ()
                   << " total Tx " << m_totBytes << " bytes");
    }
  m_lastStartTime = Simulator::Now ();
}

void VanetPacketSink::SendUDPPacket ()
{
  NS_LOG_FUNCTION (this);
 // Create the socket if not already

  //NS_ASSERT (m_sendEvent.IsExpired ());
  SeqTsHeader seqTs;
  seqTs.SetSeq (m_seqnum);
  NS_LOG_INFO ("PacketSink: Size of seqTs: " << seqTs.GetSerializedSize());
  Ptr<Packet> packet = Create<Packet> (m_pktSize-(seqTs.GetSerializedSize()));
  packet->AddHeader (seqTs);
  
  m_txTrace (packet);
  m_UDPTargetSocket->Send (packet);
  m_totBytes += m_pktSize;
  if (InetSocketAddress::IsMatchingType (m_UDPTarget))
    {
      ++m_seqnum;
      NS_LOG_INFO ("PacketSink: Tx " << packet->GetSize() 
                   << " " << InetSocketAddress::ConvertFrom(m_UDPTarget).GetIpv4 ()
                   << ":" << InetSocketAddress::ConvertFrom(m_UDPTarget).GetPort()
                   <<" Uid " << packet->GetUid () 
                   << " Sequence Number: " << seqTs.GetSeq () 
                   <<" Time " << (Simulator::Now ()).GetSeconds ());
    }
  else if (Inet6SocketAddress::IsMatchingType (m_UDPTarget))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s on-off application sent "
                   <<  packet->GetSize () << " bytes to "
                   << Inet6SocketAddress::ConvertFrom(m_UDPTarget).GetIpv6 ()
                   << " port " << Inet6SocketAddress::ConvertFrom (m_UDPTarget).GetPort ()
                   << " total Tx " << m_totBytes << " bytes");
    }
  m_lastStartTime = Simulator::Now ();
}

void VanetPacketSink::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_INFO("Node " << GetNode()->GetId() << 
              " successfully established a TCP connection at " << Simulator::Now().GetSeconds() << " s!!!");
  m_connected = true;
}

void VanetPacketSink::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_INFO("Connection Failed");
}

bool VanetPacketSink::IsThereAnyPreviousStoredPacket(Address from) {
    bool lfoundPacket = false;
    for(std::vector<DataWaitingPacket>::iterator it = m_waitingPacket.begin(); it != m_waitingPacket.end(); ++it) {
       if(it->from == from) {
          lfoundPacket=true;
          break;
       }
    }     
    return lfoundPacket;
}

void VanetPacketSink::StatPrint () 
{
   std::ostringstream os;
   os << m_outputFilename+".sta";
   std::ofstream osf (os.str().c_str(), std::ios::out | std::ios::app);

   double totrxCount = 0;
   double totrxBytes = 0;
   double toteteDelay = 0;
   double maxCount = 0;
   double totCT = 0;
   Time minfirstRx;
   Time maxLastRx;
   uint32_t counter = 0;
   uint32_t roundCounter = 0;
   bool lfirst = true;
   for(std::vector<StatRecord>::iterator it = m_stat.begin(); it != m_stat.end(); ++it) {
      totrxCount += it->rxCount;
      totrxBytes += it->rxBytes;
      if (maxCount < it->rxCount) maxCount = it->rxCount;
      if (lfirst) {
           minfirstRx = it->firstRxTime;
           maxLastRx = it->lastRxTime;
           lfirst = false;
      } else {
          if (minfirstRx > it->firstRxTime) minfirstRx = it->firstRxTime;
          if (maxLastRx < it->lastRxTime) maxLastRx = it->lastRxTime;
      }
      toteteDelay += it->totDelay;
      double ct = ((it->lastRxTime).ToInteger (Time::US) - (it->minTxTime).ToInteger (Time::US));
      if(it->rxCount == m_childNum){
        totCT += ct;
        roundCounter++;
      }
      counter++;
      NS_LOG_INFO (it->round << " " << it->rxCount << " " << it->rxBytes << " " << it->totDelay << " " << 
            it->firstRxTime << " " << it->lastRxTime << " " << it->minTxTime << " " << ct);
      osf << it->round << " " << it->rxCount << " " << it->rxBytes << " " << it->totDelay << " " << it->firstRxTime << " " << it->lastRxTime << " " << it->minTxTime << " " << ct << std::endl;
      
   }
   osf.close();
   
   double pdr = 0.0;
   double delta = (maxLastRx.ToInteger (Time::US) - minfirstRx.ToInteger (Time::US))/1000000; 
   if(m_mode == (uint32_t)0)
       pdr =  100*totrxCount/(counter*m_leafMeters);
   else
       pdr =  100*totrxCount/(maxCount*counter);
   double tp = (totrxBytes*8)/delta/1024;
   double ete = toteteDelay/totrxCount/1000000;  
   double avgCT = totCT/roundCounter/1000000; // in seconds
   avgCT += (((double)m_procDelay)/1000000000.0);
//   NS_LOG_INFO ("Statistic : " << totrxCount << " " << maxCount*counter << " " << totrxBytes << " " << maxLastRx << " " << minfirstRx << " "
//                << delta << " " << toteteDelay << " " << totCT
//                << " PDR " << pdr
//                << " TP " << tp
//                << " ETE Delay " << ete << " seconds "
//                << " CT " << avgCT );
   std::ostringstream os1;
   os1 << m_outputFilename+".rcp";
   std::ofstream osf1 (os1.str().c_str(), std::ios::out | std::ios::app);
   osf1 << totrxCount << " " << maxCount*counter << " " << totrxBytes << " " << maxLastRx << " " << minfirstRx << " " << delta << " " << toteteDelay << " " << totCT << " PDR " << pdr << " TP " << tp << " ETE Delay " << ete << " seconds " << " CT " << avgCT << std::endl ;
   osf1.close();
}

void VanetPacketSink::ReportStat (std::ostream & os)  
{
   NS_LOG_INFO(m_stat.size() );
 /*  double totrxCount = 0;
   double totrxBytes = 0;
   double toteteDelay = 0;
   double maxCount = 0;
   double totCT = 0;
   Time minfirstRx;
   Time maxLastRx;
   uint32_t counter = 0;
   bool lfirst = true;
   
   for(std::vector<StatRecord>::iterator it = m_stat.begin(); it != m_stat.end(); ++it) {
    NS_LOG_INFO("Report Statistic to file test !");
      totrxCount += it->rxCount;
      totrxBytes += it->rxBytes;
      if (maxCount < it->rxCount) maxCount = it->rxCount;
      if (lfirst) {
           minfirstRx = it->firstRxTime;
           maxLastRx = it->lastRxTime;
           lfirst = false;
      } else {
          if (minfirstRx > it->firstRxTime) minfirstRx = it->firstRxTime;
          if (maxLastRx < it->lastRxTime) maxLastRx = it->lastRxTime;
      }
      toteteDelay += it->totDelay;
      double ct = ((it->lastRxTime).ToInteger (Time::US) - (it->minTxTime).ToInteger (Time::US)); 
      totCT += ct;
      counter++;
      NS_LOG_INFO (it->round << " " << it->rxCount << " " << it->rxBytes << " " << it->totDelay << " " << 
            it->firstRxTime << " " << it->lastRxTime << " " << it->minTxTime << " " << ct);
            
   }
   double delta = (maxLastRx.ToInteger (Time::US) - minfirstRx.ToInteger (Time::US))/1000000;  
   double pdr = 100*totrxCount/(maxCount*counter);
   double tp = (totrxBytes*8)/delta/1024;
   double ete = toteteDelay/totrxCount/1000000;  
   double avgCT = totCT/counter/1000000; // in seconds
   os << totrxCount << " " << maxCount*counter << " " << totrxBytes << " " << maxLastRx << " " << minfirstRx << " " << delta << " " << toteteDelay << " " << totCT << " PDR " << pdr << " TP " << tp << " ETE Delay " << ete << " seconds " << " CT " << avgCT << std::endl ;
   */
   
}

void VanetPacketSink::HandlePeerClose (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 
void VanetPacketSink::HandlePeerError (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 

void VanetPacketSink::HandleAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  s->SetRecvCallback (MakeCallback (&VanetPacketSink::HandleReadTCP, this));
  m_socketList.push_back (s);
}

} // Namespace ns3