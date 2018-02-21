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
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/lte-aggregator.h"
#include "ns3/seq-ts-header.h"
#include "ns3/uinteger.h"
#include "ns3/packet-socket-address.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"

#include "ns3/nstime.h"
#include "ns3/string.h"
#include "ns3/pointer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteAggregator");
NS_OBJECT_ENSURE_REGISTERED (LteAggregator);

TypeId 
LteAggregator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LteAggregator")
    .SetParent<Application> ()
    .AddConstructor<LteAggregator> ()
    .AddAttribute ("Local", "The Address on which to Bind the rx socket.",
                   AddressValue (),
                   MakeAddressAccessor (&LteAggregator::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("Protocol", "The type id of the protocol to use for the rx socket.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&LteAggregator::m_tid),
                   MakeTypeIdChecker ())
    .AddAttribute ("Child", "The number of child node.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&LteAggregator::m_child_node),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&LteAggregator::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("Mode", "Type of operation [0=forwarding], 1=aggregation", 
                    UintegerValue (0),
                   MakeUintegerAccessor (&LteAggregator::m_operation_type),
                   MakeUintegerChecker<uint8_t> ())
   .AddAttribute ("MaxFragment", 
                   "The max number of bytes to send in one segment.",
                   UintegerValue (1500),
                   MakeUintegerAccessor (&LteAggregator::m_maxFragment),
                   MakeUintegerChecker<uint32_t> ())
   .AddAttribute ("PacketSize", "The size of packets sent in on state",
                   UintegerValue (512),
                   MakeUintegerAccessor (&LteAggregator::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1))
   .AddAttribute ("DefaultRxSize", "The default size of packets received",
                   UintegerValue (6140),
                   MakeUintegerAccessor (&LteAggregator::m_defSize),
                   MakeUintegerChecker<uint32_t> ())
   .AddAttribute ("Interval", "the interval for sending data",
                    TimeValue (Seconds (30)),
                   MakeTimeAccessor (&LteAggregator::m_interval),
                   MakeTimeChecker ())
   .AddAttribute ("FirstTime", "the first time for sending data",
                    TimeValue (Seconds (20)),
                   MakeTimeAccessor (&LteAggregator::m_firstTime),
                   MakeTimeChecker ())
    .AddAttribute ("DelayBetweenFragmentedPacket", "the interval between two consecutive fragmented packet",
                    TimeValue (MilliSeconds (1)),
                   MakeTimeAccessor (&LteAggregator::m_timeBetweenFragmentedPacket),
                   MakeTimeChecker ())
    .AddAttribute ("HomomorphicOperationTime", "the time needed to perform homomorphic operations",
                    TimeValue (NanoSeconds (1000)),
                   MakeTimeAccessor (&LteAggregator::m_homomorphicTime),
                   MakeTimeChecker ())
    .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&LteAggregator::m_rxTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}


LteAggregator::LteAggregator ()
  : m_socket (0),
  m_child_node (0),
  m_pSocket (0),
  m_connected (false),
  m_seqnum (0),
  m_operation_type (0),
  m_maxFragment (1500),
  m_nextTime (0)
{
  NS_LOG_FUNCTION (this);
}

LteAggregator::~LteAggregator()
{
  NS_LOG_FUNCTION (this);
}

Ptr<Socket>
LteAggregator::GetListeningSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

std::list<Ptr<Socket> >
LteAggregator::GetAcceptedSockets (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socketList;
}

void LteAggregator::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Statistics : " << InetSocketAddress::ConvertFrom (m_local).GetIpv4 () <<  " " << m_txCount << " " << m_txTotBytes << " " << m_rxCount << " " << m_rxTotBytes << " " <<  m_child_node);
  m_socket = 0; 
  m_socketList.clear ();
 
  m_pSocket = 0;

  // chain up
  Application::DoDispose ();
}


// Application Methods
void LteAggregator::StartApplication ()    // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);
  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      m_socket->Bind (m_local);
      m_socket->Listen ();
      m_socket->ShutdownSend ();
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: joining multicast on a non-UDP socket");
            }
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&LteAggregator::HandleRead, this));
  m_socket->SetAcceptCallback (
    MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
    MakeCallback (&LteAggregator::HandleAccept, this));
  m_socket->SetCloseCallbacks (
    MakeCallback (&LteAggregator::HandlePeerClose, this),
    MakeCallback (&LteAggregator::HandlePeerError, this));
  //
  if (!m_peerAddress.IsInvalid () ) {
      // Create the parent socket if not already
     // NS_LOG_INFO("Connect to parent node");
      if (!m_pSocket)
      {
         m_pSocket = Socket::CreateSocket (GetNode (), m_tid);
         if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
           {
             m_pSocket->Bind6 ();
           }
         else if (InetSocketAddress::IsMatchingType (m_peerAddress) ||
                 PacketSocketAddress::IsMatchingType (m_peerAddress))
           {
             m_pSocket->Bind ();
           }
           m_pSocket->Connect (m_peerAddress);
           m_pSocket->SetAllowBroadcast (true);
           m_pSocket->ShutdownRecv ();

           m_pSocket->SetConnectCallback (
              MakeCallback (&LteAggregator::ConnectionSucceeded, this),
              MakeCallback (&LteAggregator::ConnectionFailed, this));
       }
   //CancelEvents ();
   //ScheduleStartEvent ();
   // if forwarding operation, set the schedule
   if(m_operation_type==0) ScheduleNextTx();
   }
   ResetStatistic();
}

void LteAggregator::StopApplication ()     // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);
  

  while(!m_socketList.empty ()) //these are accepted sockets, close them
    {
      Ptr<Socket> acceptedSocket = m_socketList.front ();
      m_socketList.pop_front ();
      acceptedSocket->Close ();
    }
  if (m_socket) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  if (m_pSocket != 0) { // close connection to parent
      m_pSocket->Close ();
  }
}

void LteAggregator::CancelEvents ()
{
  NS_LOG_FUNCTION (this);

  Simulator::Cancel (m_sendEvent);
  Simulator::Cancel (m_startStopEvent);
}

// Event handlers
void LteAggregator::StartSending ()
{
  NS_LOG_FUNCTION (this);
  ScheduleNextTx ();  // Schedule the send packet event
  ScheduleStopEvent ();
}

void LteAggregator::StopSending ()
{
  NS_LOG_FUNCTION (this);
  CancelEvents ();

  ScheduleStartEvent ();
}

// Private helpers
void LteAggregator::ScheduleNextTx ()
{
  NS_LOG_FUNCTION (this);
   if(m_nextTime==0) m_nextTime = m_firstTime;
   else m_nextTime = m_interval;
   NS_LOG_LOGIC ("nextTime = " << m_nextTime);
    m_sendEvent = Simulator::Schedule (m_nextTime,
                                         &LteAggregator::SendPacket, this);
}

void LteAggregator::ScheduleStartEvent ()
{  // Schedules the event to start sending data (switch to the "On" state)
  NS_LOG_FUNCTION (this);
 
}

void LteAggregator::ScheduleStopEvent ()
{  // Schedules the event to stop sending data (switch to "Off" state)
  NS_LOG_FUNCTION (this);
}

void LteAggregator::HandleRead (Ptr<Socket> socket)
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
      m_rxTotBytes += packet->GetSize ();
      m_rxCount++;
      uint32_t m_rxBytes = packet->GetSize ();
      m_rxTrace (packet, from);
      SeqTsHeader seqTs;
      packet->PeekHeader (seqTs);
      Time now = Simulator::Now ();
      Ipv4Address ipAdd = InetSocketAddress::ConvertFrom (from).GetIpv4 ();
      if (InetSocketAddress::IsMatchingType (from))
        {     
              NS_LOG_INFO (" RX " << m_rxBytes 
                           << " " << InetSocketAddress::ConvertFrom (from).GetIpv4 () 
                           << " " << seqTs.GetSeq () 
                           << " " << packet->GetUid () 
                           << " TXtime: " << seqTs.GetTs () 
                           << " RXtime: " << now 
                           << " Child " << m_child_node << " nodes" ); 
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("TraceDelay: RX " << m_rxBytes <<
                           " bytes from "<< Inet6SocketAddress::ConvertFrom (from).GetIpv6 () <<
                           " Sequence Number: " << seqTs.GetSeq() <<
                           " Uid: " << packet->GetUid () <<
                           " TXtime: " << seqTs.GetTs () <<
                           " RXtime: " << now <<
                           " Delay: " << now - seqTs.GetTs ());
         }    
         switch (m_operation_type) {
             case 0:   // forwarding only
                if (!m_peerAddress.IsInvalid () ) {
                   ScheduledForwardPacket(packet);
                } else { 
                   // for every m_defSize, transfer it to m_AggregationReadyList, without knowing the
                   // and ready for aggregation when it is = m_child_node;

                   CheckAggregateDataAtTheSink(ipAdd, now, m_rxBytes);
		     if(m_AggregationReadyList.size () >= m_child_node) {
      			m_AggregationReadyList.clear();
       		NS_LOG_INFO("Sink received all packets !  " << m_child_node);  
   		     }
                }
                break;
             case 1:  // aggregation
                 if(PacketIsReadyForAggregation(ipAdd,now, m_rxBytes)){
                     // remove from the readylist and then send it to the parent node
                     if (!m_peerAddress.IsInvalid () ) {
    	                  NS_LOG_INFO("received all packets !  " << m_child_node);  
          		     m_sendEvent = Simulator::Schedule (m_homomorphicTime,
                            		    &LteAggregator::SendPacket, this);
                     } else {
    	                  NS_LOG_INFO("Sink received all packets !  " << m_child_node);  
                     }
                  }
                 CheckForWaitingListPackets();
                 // check again the ready list after waiting list checking
                  if(m_AggregationReadyList.size () >= m_child_node) {
  		       m_AggregationReadyList.clear();
                     if (!m_peerAddress.IsInvalid () ) {
    	                  NS_LOG_INFO("received all packets !  " << m_child_node);  
          		     m_sendEvent = Simulator::Schedule (m_homomorphicTime,
                            		    &LteAggregator::SendPacket, this);
                     } else {
    	                  NS_LOG_INFO("Sink received all packets !  " << m_child_node);  
                     }
                     CheckForWaitingListPackets();
   		    }
                 break;
         }
    }
}

bool LteAggregator::PacketIsReadyForAggregation(Ipv4Address ip, Time tx, uint32_t ds ) {
   bool lComplete = false;
  // m_defSize = FindDataSizeInformation(ip,ds);
  // uint32_t tmp = ds - m_defSize;
   CheckItInTheWaitingList(ip, tx, ds);
 /*  if (tmp == 0) {
           // check whether readlylist has the data from that address,
           if (IsInTheReadyList(ip, tx, ds)) {
              SaveItInTheWaitingList(ip, tx, ds);
           } 
   } else {
       if (tmp < 0) {
              CheckItInTheWaitingList(ip, tx, ds);
       } else { // greater than 
            // check in the readylist, if none, add it and the rest goes to the waiting list
            if (IsInTheReadyList(ip,tx, m_defSize)) {
               SaveItInTheWaitingList(ip, tx, ds); // save all in the waiting list if the ready list hast it
            } else {   
               SaveItInTheWaitingList(ip, tx, tmp);  // save the rest to the waiting list
            }               
       } 
   } */
   if(m_AggregationReadyList.size () >= m_child_node) {
      m_AggregationReadyList.clear();
      lComplete = true;
   }
   return lComplete;
}
void LteAggregator::CheckAggregateDataAtTheSink(Ipv4Address ip, Time t, uint32_t d) 
{
   bool lNewRecord = true;
   bool lNeedtoTransfer = false;
 
   for(std::vector<DataTable>::iterator it = m_AggregationWaitingList.begin(); it != m_AggregationWaitingList.end(); ++it) {
      if (it->from == ip) {
         NS_LOG_INFO("Previously stored data size " << it->dsReceived << " " << it->dsUsed << " "  << d  );
         it->dsReceived +=  d; // add to the waiting list 
         it->at = t; // update to current time
    
         uint32_t tmp = it->dsReceived - it->dsUsed;
         NS_LOG_INFO("Accepted from : " << ip << " " << d << " bytes, total available " << it->dsReceived << " " << it->dsUsed << " " << tmp );
         lNewRecord = false;
         if(tmp >= m_defSize) {
             NS_LOG_INFO("Full size data accepted, transfer to ready list !" << ip << " " << t << " " <<  tmp << " " << m_defSize);
             lNeedtoTransfer = true;
             it->dsUsed += m_defSize;
         }
         break;
      }
   }
   if (lNewRecord) {
      // if newrecord, but not equal to m_defSize, then add to waiting list, else
      // put it directly to the ready list
      DataTable dt;
      dt.from = ip;
      dt.dsReceived = d;
      dt.dsUsed = 0;
      dt.at = t;
      dt.complete = false;

      if(d==m_defSize) {
          m_AggregationReadyList.push_back(dt);
          NS_LOG_INFO("Add to the ready list ! " << ip << " " << t << " " << d << " " << m_AggregationReadyList.size() << " " << m_child_node);
      }  else { 
          if (d >= m_defSize) {
             uint32_t tmp = d;
             uint32_t tmpUsed = 0;
             while (tmp >= m_defSize) {
                dt.dsReceived = m_defSize;
                m_AggregationReadyList.push_back(dt);
                NS_LOG_INFO("Add to the ready list ! " << ip << " " << t << " " << d << " " << m_AggregationReadyList.size() << " " << m_child_node);
                tmp -= m_defSize;
                tmpUsed += m_defSize;
             }
             if (tmp > 0) {
                dt.dsReceived = d;
                dt.dsUsed = tmpUsed;
                m_AggregationWaitingList.push_back(dt);
                NS_LOG_INFO("Add to waiting list " << ip << " " << t << " " << d << " " << dt.dsUsed << " " << m_AggregationWaitingList.size() );
             }
          } else { 
            NS_LOG_INFO("Add to waiting list " << ip << " " << t << " " << d << " " << m_AggregationWaitingList.size() );
            m_AggregationWaitingList.push_back(dt);
          }
      }
   }
   if (lNeedtoTransfer) {
       Time curtime = Simulator::Now ();
       DataTable dtab;
       dtab.from = ip;
       dtab.at   = curtime;
       dtab.complete = false;
       dtab.dsReceived = m_defSize;
       dtab.dsUsed = 0;
       m_AggregationReadyList.push_back(dtab);
       NS_LOG_INFO("Add to the ready list ! " << ip << " " << curtime << " " << m_defSize << " " << m_AggregationReadyList.size() << " " << m_child_node);
   } 
}
void LteAggregator::CheckItInTheWaitingList(Ipv4Address ip, Time t, uint32_t d) 
{
   bool lNewRecord = true;
   bool lPurge = false;
   for(std::vector<DataTable>::iterator it = m_AggregationWaitingList.begin(); it != m_AggregationWaitingList.end(); ++it) {
      if (it->from == ip) {
         NS_LOG_INFO("Previously stored data size " << it->dsReceived << " " << d << " " << it->dsUsed );
         it->dsReceived +=  d; // add to the waiting list 
         it->at = t; // update to current time
    
         uint32_t tmp = it->dsReceived - it->dsUsed;
         NS_LOG_INFO("Accepted from : " << ip << " " << d << " bytes, total available " << it->dsReceived << " " << it->dsUsed << " " << tmp );
         lNewRecord = false;
         if(tmp >= m_defSize) {
             NS_LOG_INFO("meet the required size, check for available transfer to ready list !" << ip << " " << t << " " <<  tmp << " " << m_defSize);
             if (!IsInTheReadyList(ip, t, m_defSize)) {
                  it->dsUsed += m_defSize;  // record in the usage            
                  if (tmp==m_defSize) { // all have been used, delete the record
                     it->complete=true;
                     lPurge = true;
                  }
             }             
         } else {
            // less than the required size, do nothing
         }
         break;
      }
   }
   if (lNewRecord) {
      // if newrecord, but not equal to m_defSize, then add to waiting list, else
      // put it directly to the ready list
      DataTable dt;
      dt.from = ip;
      dt.dsReceived = d;
      dt.dsUsed = 0;
      dt.at = t;
      dt.complete = false;
      if (d < m_defSize) {
         m_AggregationWaitingList.push_back(dt);
         NS_LOG_INFO("Add to waiting list " << ip << " " << t << " " << d << " " << m_AggregationWaitingList.size() << " " << m_child_node << " " << m_defSize );
      } else {
          if (!IsInTheReadyList(ip, t, m_defSize)) {
             if (d > m_defSize) {
                 dt.dsUsed += m_defSize;
                 m_AggregationWaitingList.push_back(dt);
                 NS_LOG_INFO("Add to waiting list " << ip << " " << t << " " << d << " " << m_AggregationWaitingList.size() << " " << m_child_node << " " << m_defSize );
             }
          } else {
             m_AggregationWaitingList.push_back(dt);
             NS_LOG_INFO("Found in Ready List, store it in waiting list " << ip << " " << t << " " << d << " " << m_AggregationWaitingList.size() << " " << m_child_node << " " << m_defSize );
          }
      }
   }
   if (lPurge) {
      NS_LOG_INFO("Delete Waiting list entry for : " << ip );
      std::vector<DataTable> tmp (m_AggregationWaitingList); // copy to temp
      m_AggregationWaitingList.clear();
      for(std::vector<DataTable>::iterator itw = tmp.begin() ; itw != tmp.end(); ++itw) {
          if(itw->complete==false) { // keep maintain in the waiting list
              DataTable keep;
              keep.from = itw->from;
              keep.at   = itw->at;
              keep.complete = false;
              keep.dsReceived = itw->dsReceived;
              keep.dsUsed   = itw->dsUsed;
              m_AggregationWaitingList.push_back(keep);
              NS_LOG_INFO("Keep it in the waiting list " << keep.from << " " << keep.at << " " << (keep.dsReceived-keep.dsUsed) << " " << m_AggregationWaitingList.size () );
          }
      }
   }
}
void LteAggregator::CheckForWaitingListPackets() {
  if(m_AggregationWaitingList.size() > 0) {
        bool lPurge = false;
        for (std::vector<DataTable>::iterator it = m_AggregationWaitingList.begin() ; it != m_AggregationWaitingList.end(); ++it) {
           //Ipv4Address ip = it->from;
           uint32_t temp = it->dsReceived - it->dsUsed;
           if (temp >= m_defSize) {
               // search in the ready list, add it in the ready list in not found, and reduce the dsreceived
              if (!IsInTheReadyList(it->from, it->at, m_defSize)) {
                  it->dsUsed += m_defSize;
                  if((it->dsReceived - it->dsUsed)<= 0){
                      it->complete=true;
                      lPurge = true;
                  }   
              }
           }
       }
      if (lPurge) { // delete some entry of the waiting list
         NS_LOG_INFO("Current Waiting List : " << m_AggregationWaitingList.size() );
         std::vector<DataTable> tmp (m_AggregationWaitingList); // copy to temp
         m_AggregationWaitingList.clear();
         for(std::vector<DataTable>::iterator itw = tmp.begin() ; itw != tmp.end(); ++itw) {
             if(itw->complete==false) { // keep maintain in the waiting list
                DataTable keep;
                  keep.from = itw->from;
                  keep.at   = itw->at;
                  keep.complete = false;
                  keep.dsReceived = itw->dsReceived;
                  m_AggregationWaitingList.push_back(keep);
                  NS_LOG_INFO("Keep it in the waiting list " << keep.from << " " << keep.at << " " << keep.dsReceived << " " << m_AggregationWaitingList.size () );
             }
         }
         NS_LOG_INFO("The Waiting List after transfer to ready list : " << m_AggregationWaitingList.size() );
      }
  }
}
bool LteAggregator::IsInTheReadyList(Ipv4Address ip, Time tx, uint32_t ds) 
{
    bool lisInTheList = false;
    for (std::vector<DataTable>::iterator it = m_AggregationReadyList.begin() ; it != m_AggregationReadyList.end(); ++it) {
        if (it->from == ip) {
              lisInTheList = true;
              break;           
        }
    }
    if (!lisInTheList) { // save it to the ready list
       DataTable dt;
       dt.from = ip;
       dt.at   = tx;
       dt.complete = false;
       dt.dsReceived = ds;
       m_AggregationReadyList.push_back(dt);
       NS_LOG_INFO("Add to the ready list ! " << ip << " " << tx << " " << ds << " " << m_AggregationReadyList.size() << " " << m_child_node);
    }
    return lisInTheList;
}

void LteAggregator::SaveItInTheWaitingList(Ipv4Address ip, Time tx, uint32_t ds) 
{  
  // add new or modify the time and data size
  bool lNewRecord = true;
  if (m_AggregationWaitingList.size () > 0) {
       for (std::vector<DataTable>::iterator it = m_AggregationWaitingList.begin() ; it != m_AggregationWaitingList.end(); ++it) {
          if (it->from == ip) {
              it->at = tx;
              it->dsReceived += ds;
              lNewRecord = false;
              break;           
          }
       }
  }
  if (lNewRecord) {
     DataTable dt;
     dt.from = ip;
     dt.at   = tx;
     dt.dsReceived = ds;
     dt.complete = false;
     m_AggregationWaitingList.push_back(dt);
     NS_LOG_INFO("Add to the waiting list ! " << ip << " " << tx << " " << ds << " " << m_AggregationWaitingList.size() << " " << m_child_node);
  }
}

uint32_t LteAggregator::FindDataSizeInformation(Ipv4Address ip, uint32_t ds) 
{
   uint32_t temp = ds;
   bool lfound = false;
   for (std::vector<DataPacketSize>::iterator it = m_childInfoDataSizeList.begin() ; it != m_childInfoDataSizeList.end(); ++it) {
         if(it->first==ip) {
              //temp = it->second;
              lfound = true;
              NS_LOG_INFO("Acceptable data size from " << ip << " is " << temp << " and the received packet is " << ds);
              break;
         }
   }
   if (!lfound) { 
       DataPacketSize dp = std::make_pair(ip, ds);
       m_childInfoDataSizeList.push_back(dp);
       NS_LOG_INFO("Registering new data size " << ip << " " << ds << " " << m_childInfoDataSizeList.size() << " " << m_child_node);
   }
   return temp;
}

void LteAggregator::HandlePeerClose (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 
void LteAggregator::HandlePeerError (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 

void LteAggregator::HandleAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  s->SetRecvCallback (MakeCallback (&LteAggregator::HandleRead, this));
  m_socketList.push_back (s);
}

void LteAggregator::SetNumberOfChildNode (uint32_t x) {
   NS_LOG_FUNCTION (this << x);
   m_child_node = x;
}

uint8_t LteAggregator::GetNumberOfChildNode() const 
{
   NS_LOG_FUNCTION (this);
   return m_child_node;
}

void LteAggregator::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_connected = true;
}

void LteAggregator::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}

void LteAggregator::ForwardPacketFragmentation (uint32_t pktSize, SeqTsHeader ts)
{
     NS_LOG_INFO ("Forwarding packet greater than MTU : " << pktSize << " " << m_maxFragment);
     int numOfPacket = pktSize / m_maxFragment;
     uint32_t remainder = pktSize % m_maxFragment;
     for (int i=0; i < numOfPacket; i++) {
        Ptr<Packet> pk = Create<Packet> (m_maxFragment); // 12 is the size of header seqts
      //     pk->AddHeader (ts);
           SendForwardPacketFragmentation(pk);   
     } 
     if (remainder > 0) {
        uint32_t psize = remainder;
        
     //   if (remainder > 12) psize = remainder - 12;
        Ptr<Packet> pkt = Create<Packet> (psize);
     //   pkt->AddHeader (ts);           
        SendForwardPacketFragmentation(pkt);
     } 
}

void 
LteAggregator::SendForwardPacketFragmentation(Ptr<Packet> p) 
{
  
  SeqTsHeader seqTs;
 // p->PeekHeader (seqTs);
  m_pSocket->Send (p);
 // m_totBytes += m_pktSize;
  m_txCount++;
  m_txTotBytes += p->GetSize();
  if (InetSocketAddress::IsMatchingType (m_peerAddress))
    {
     
      NS_LOG_INFO (" Tx " << p->GetSize() 
                   << " " << InetSocketAddress::ConvertFrom(m_peerAddress).GetIpv4 ()
                   <<" Uid " << p->GetUid () 
                   << " Sequence Number: " << seqTs.GetSeq () 
                   <<" Time " << (Simulator::Now ()).GetSeconds () );
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                   << "s on-off application sent "
                   <<  p->GetSize () << " bytes to "
                   << Inet6SocketAddress::ConvertFrom(m_peerAddress).GetIpv6 ()
                   << " port " << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetPort ());
                  // << " total Tx " << m_totBytes << " bytes");
    }
}

void
LteAggregator::ScheduledForwardPacket(Ptr<Packet> pkt)
{
   NS_LOG_FUNCTION (this);
  SeqTsHeader seqTs;
 // pkt->PeekHeader (seqTs);
  if (pkt->GetSize() > m_maxFragment) {
       uint32_t pksize = pkt->GetSize();
   //    if (pkt->GetSize () > 12) {
   //       pksize = pkt->GetSize() - 12;
   //    }
       ForwardPacketFragmentation (pksize, seqTs);
     }
   else {
     SendForwardPacketFragmentation(pkt);
   }
}

void LteAggregator::SendNewPacket (uint32_t packetSize)
{
     SeqTsHeader seqTs;
     seqTs.SetSeq (m_seqnum);
     uint32_t calculateSize = packetSize;
     if (packetSize > 12) {
         calculateSize = packetSize - (8+4); 
     } 
     Ptr<Packet> packet = Create<Packet> (calculateSize); // 8+4 : the size of the seqTs header
     packet->AddHeader (seqTs);
     m_pSocket->Send (packet);
     m_txTotBytes += packet->GetSize();
     m_txCount++;
     if (InetSocketAddress::IsMatchingType (m_peerAddress))
       {
         ++m_seqnum;
         NS_LOG_INFO (" Tx " << packet->GetSize() 
                      << " " << InetSocketAddress::ConvertFrom(m_peerAddress).GetIpv4 ()
                      <<" Uid " << packet->GetUid () 
                      << " Sequence Number: " << seqTs.GetSeq () 
                      <<" Time " << (Simulator::Now ()).GetSeconds ());

       }
     else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
       {
         NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                      << "s on-off application sent "
                      <<  packet->GetSize () << " bytes to "
                      << Inet6SocketAddress::ConvertFrom(m_peerAddress).GetIpv6 ()
                      << " port " << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetPort ());
                   //   << " total Tx " << m_totBytes << " bytes");
       }
}

void LteAggregator::SendPacket ()
{
  NS_LOG_FUNCTION (this);

  if (m_pktSize > m_maxFragment) {
     NS_LOG_INFO (m_pktSize << " " << m_maxFragment);
     int numOfPacket = m_pktSize / m_maxFragment;
     uint32_t remainder = m_pktSize % m_maxFragment;
     for (int i=0; i < numOfPacket; i++) {
        SendNewPacket(m_maxFragment);   
     } 
     SendNewPacket(remainder); 
  } else {
     SendNewPacket(m_pktSize);
  }
  if(m_operation_type==0) ScheduleNextTx ();
}


uint32_t LteAggregator::GetTotalRx () const
{
  return m_txCount;
}

uint32_t LteAggregator::GetTotalTx () const
{
  return m_txCount;
}

uint32_t LteAggregator::GetBytesRx () const
{
  return m_rxTotBytes;
}
 
uint32_t LteAggregator::GetBytesTx () const
{
  return m_txTotBytes;
}
void LteAggregator::ResetStatistic () 
{
  m_txCount = 0;
  m_txTotBytes = 0;
  m_rxCount = 0;
  m_rxTotBytes = 0;

}
} // Namespace ns3
