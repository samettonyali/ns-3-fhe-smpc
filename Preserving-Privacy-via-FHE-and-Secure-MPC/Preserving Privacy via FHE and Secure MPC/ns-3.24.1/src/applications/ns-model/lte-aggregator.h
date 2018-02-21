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

#ifndef LTE_AGGREGATOR_H
#define LTE_AGGREGATOR_H

#include "ns3/core-module.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/data-rate.h"
#include "ns3/seq-ts-header.h"

namespace ns3 {

class Address;
class Socket;
class RandomVariableStream;
class Packet;

/**
 * \ingroup applications 
 * \defgroup packetsink PacketSink
 *
 * This application was written to complement OnOffApplication, but it
 * is more general so a PacketSink name was selected.  Functionally it is
 * important to use in multicast situations, so that reception of the layer-2
 * multicast frames of interest are enabled, but it is also useful for
 * unicast as an example of how you can write something simple to receive
 * packets at the application layer.  Also, if an IP stack generates 
 * ICMP Port Unreachable errors, receiving applications will be needed.
 */

/**
 * \ingroup packetsink
 *
 * \brief Receive and consume traffic generated to an IP address and port
 *
 * This application was written to complement OnOffApplication, but it
 * is more general so a PacketSink name was selected.  Functionally it is
 * important to use in multicast situations, so that reception of the layer-2
 * multicast frames of interest are enabled, but it is also useful for
 * unicast as an example of how you can write something simple to receive
 * packets at the application layer.  Also, if an IP stack generates 
 * ICMP Port Unreachable errors, receiving applications will be needed.
 *
 * The constructor specifies the Address (IP address and port) and the 
 * transport protocol to use.   A virtual Receive () method is installed 
 * as a callback on the receiving socket.  By default, when logging is
 * enabled, it prints out the size of packets and their address, but
 * we intend to also add a tracing source to Receive() at a later date.
 */
class LteAggregator : public Application 
{
public:
  static TypeId GetTypeId (void);
  LteAggregator ();

  virtual ~LteAggregator ();

  /**
   * \return the total bytes received in this sink app
   */
  void SetNumberOfChildNode (uint32_t x);
  uint8_t GetNumberOfChildNode () const;

  uint32_t GetTotalRx () const;

  uint32_t GetTotalTx () const;

  uint32_t GetBytesRx () const;
 
  uint32_t GetBytesTx () const;
  void ResetStatistic ();


  /**
   * \return pointer to listening socket
   */
  Ptr<Socket> GetListeningSocket (void) const;

  /**
   * \return list of pointers to accepted sockets
   */
  std::list<Ptr<Socket> > GetAcceptedSockets (void) const;

 //  void Report(std::ostream & os) const;

 uint32_t        m_txCount;
  uint32_t        m_txTotBytes;
  uint32_t        m_rxCount;
  uint32_t        m_rxTotBytes;

 
protected:
  virtual void DoDispose (void);
private:
  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  void HandleRead (Ptr<Socket>);
  void HandleAccept (Ptr<Socket>, const Address& from);
  void HandlePeerClose (Ptr<Socket>);
  void HandlePeerError (Ptr<Socket>);

 //helpers
  void CancelEvents ();

// Event handlers
  void StartSending ();
  void StopSending ();

  struct DataTable {
         Ipv4Address from;
         Time        at;
         bool        complete;
         uint32_t    dsReceived;    
         uint32_t    dsUsed;     
  };

  void ScheduledForwardPacket (Ptr<Packet> pkt);

  void SendPacket ();
  void SendNewPacket (uint32_t packetSize);
  void ForwardPacketFragmentation (uint32_t packetSize, SeqTsHeader ts);
  void SendForwardPacketFragmentation(Ptr<Packet> p);
  

  bool PacketIsReadyForAggregation (Ipv4Address ip, Time tx, uint32_t dsize );
  void CheckForWaitingListPackets();
  bool IsInTheReadyList(Ipv4Address ip, Time tx, uint32_t dsize );
  void SaveItInTheWaitingList(Ipv4Address ip, Time tx, uint32_t dsize );
  void CheckItInTheWaitingList(Ipv4Address ip, Time tx, uint32_t dsize);
  void CheckAggregateDataAtTheSink (Ipv4Address ip, Time tx, uint32_t dsize);

 // bool findInTable (Ipv4Address from, Time tx);
 // void addTable (Ipv4Address from, Time tx, uint32_t dsize);
   
 // void RegisteringChildDataSize(Ipv4Address ip, uint32_t s);
  uint32_t FindDataSizeInformation(Ipv4Address ip, uint32_t s);
 
  // In the case of TCP, each socket accept returns a new socket, so the 
  // listening socket is stored seperately from the accepted sockets
  Ptr<Socket>     m_socket;       // Listening socket
  std::list<Ptr<Socket> > m_socketList; //the accepted sockets

  Address         m_local;        // Local address to bind to
  uint32_t        m_pktSize;      // Size of packets
  uint32_t        m_defSize;      // default size of receiving packets
  TypeId          m_tid;          // Protocol TypeId
  TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;
  typedef std::pair<Ipv4Address, uint32_t > DataPacketSize;
//  typedef std::pair< Ipv4Address, Time >  DataInfoPacket;
 
  std::vector<DataTable> m_AggregationReadyList;     // to store the packet that  ready to be aggregated
  std::vector<DataTable> m_AggregationWaitingList;   // packet waiting when the packet has been found in the ready list
                                                     // and the aggregation of previous data is not done yet.
  std::vector<DataPacketSize> m_childInfoDataSizeList;
//  std::vector<ReceivedPacket> m_bufferPackets;
  uint32_t         m_child_node; // number of child nodes
  //
  Ptr<Socket>     m_pSocket; //  Parent Socket
  Address         m_peerAddress; // peer address
  bool            m_connected; // true if connected
  uint32_t        m_seqnum;
  uint8_t         m_operation_type; // 0 = forwarding, 1 aggregation
  uint32_t        m_maxFragment;
 
  //
  EventId         m_startStopEvent;     // Event id for next start or stop event
  EventId         m_sendEvent;    // Eventid of pending "send packet" event
  bool            m_sending;      // True if currently in sending state
  //
  Time            m_firstTime;
  Time            m_interval;
  Time            m_nextTime;
  Time            m_timeBetweenFragmentedPacket;
  Time            m_homomorphicTime;


  ///\}


private:
  void ScheduleNextTx ();
  void ScheduleStartEvent ();
  void ScheduleStopEvent ();
  void ConnectionSucceeded (Ptr<Socket> socket);
  void ConnectionFailed (Ptr<Socket> socket);
};

} // namespace ns3

#endif /* LTE_AGGREGATOR_H */

