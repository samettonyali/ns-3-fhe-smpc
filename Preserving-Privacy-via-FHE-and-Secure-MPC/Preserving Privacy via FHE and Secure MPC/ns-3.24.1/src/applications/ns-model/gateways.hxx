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
 */

#ifndef GATEWAYS_H
#define GATEWAYS_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include <iostream>

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup requestresponse
 * \brief A Request Response aggregator
 *
 * Every packet sent should be returned by the server and received here.
 */
class Gateways : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  Gateways ();

  virtual ~Gateways ();


protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \brief Schedule the next packet transmission
   * \param dt time interval between packets.
   */
  void ScheduleTransmit (Time dt);
  /**
   * \brief Send a packet
   */
  void Send (void);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);
  void HandleAccept (Ptr<Socket>, const Address& from);
  void HandlePeerClose (Ptr<Socket>);
  void HandlePeerError (Ptr<Socket>);

  void SendDown(Ptr<Packet> packet);

 struct DataWaitingPacket {
          Address from;
          Ptr<Packet> pkt;
  };

  bool m_randStartTime;
  bool m_randIntervalTime;
  double m_interval; //!< Packet inter-send time
  uint16_t m_eventSize;
  uint16_t m_packetSize;
  uint8_t ipLocal[4];
  uint8_t ipRemote[4];
  uint8_t ipTemp[4];

  uint32_t m_sent; //!< Counter for sent packets
  uint32_t m_evented;
  Ptr<Socket> m_socket; //!< Socket
  Ptr<Socket> m_outSocket; // out going socket to remotehost
  
  Address m_peerAddress; //!< Remote peer address
  uint16_t m_peerPort; //!< Remote peer port
  std::list<Ptr<Socket> > m_socketList; //the accepted sockets
  std::list<Ptr<Socket> > m_outSocketList;
  std::vector<DataWaitingPacket> m_waitingPacket;

  bool stoped;
  std::string m_receivedFilename;
  std::string m_sentFilename;
  std::string m_aggFilename;
  std::string m_protocol;
  double m_aggregationTime;
  bool firstReceived;
  bool m_meshAggregationEnable;
  uint16_t m_aggDataSize;
  uint16_t m_totPacketReceived;
  TypeId          m_tid;


  /// Callbacks for tracing the packet Tx events
  TracedCallback<Ptr<const Packet> > m_txTrace;
};

} // namespace ns3

#endif /* GATEWAYS_H */
