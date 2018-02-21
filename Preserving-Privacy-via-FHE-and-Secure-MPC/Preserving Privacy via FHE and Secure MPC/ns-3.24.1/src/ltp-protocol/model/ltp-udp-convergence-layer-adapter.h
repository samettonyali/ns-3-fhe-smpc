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
 * Authors:  Tom Henderson (tomhend@u.washington.edu)
 *           Rubén Martínez <rmartinez@deic.uab.cat>
 */


#ifndef LTP_UDP_CLA_H_
#define LTP_UDP_CLA_H_

#include "ns3/object.h"
#include "ns3/address.h"
#include "ns3/socket.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/node.h"

#include "ltp-convergence-layer-adapter.h"
#include "ltp-protocol.h"
#include "ltp-ip-resolution-table.h"


#include <map>

namespace ns3 {
//namespace ltp {

/**
 * \ingroup dtn
 *
 * \brief Convergence layer adapter between LTP and UDP
 *
 * The main purpose of this class is to allow LTP to operate
 * over ns-3's UDP implementation (and DCCP if, in the future, it
 * is added to ns-3.  This object implements the CLA defined in
 * <a href="http://tools.ietf.org/html/rfc7122">RFC 7122 </a>.
 *
 * This class is responsible for the following:
 * 1) mapping LTP segments into UDP socket send() operations
 * 2) mapping received UDP segments to the LTP receiver
 * 3) opening a port on the IANA-assigned listening port of 1113
 * 4) allowing the LTP implementation to determine the MTU that will
 *    avoid fragmentation
 * 5) support the generation and disposal of keep-alive packets
 *    according to RFC 7122 section 3.4
 * 6) provide link-state cues and link-up/link-down notifications
 *
 * Since UDP is connectionless and not flow controlled, the transmission
 * of a segment will typically cause the link state cue to immediately
 * trigger, and link notifications will just correspond to the local
 * state of interfaces and routes.
 *
 * LTP authentication is not used by this model.
 *
 * Multicast is not supported, as LTP is intended to be point-to-point.
 */
class LtpUdpConvergenceLayerAdapter : public LtpConvergenceLayerAdapter
{

public:
  /**
   * Default Constructor
   */
  LtpUdpConvergenceLayerAdapter ();

  /**
   * Destructor
   */
  ~LtpUdpConvergenceLayerAdapter ();

  static TypeId GetTypeId (void);

  /* public API related to the interface with LTP */

  /*
   * Send packet using the underlying layer.
   * \param p packet to send.
   * \return 0 if operation failed, size of sent data otherwise.
   */
  virtual uint32_t Send (Ptr<Packet> p);
  
  void ConnectionSucceeded (Ptr<Socket> socket);
  void ConnectionFailed (Ptr<Socket> socket);

  /* \brief Receive a data packet from the lower layer.
   * \return 0 if there are no available datagrams to deliver
   */
  void Receive (Ptr<Socket> socket);

  /* public API to get the MTU */

  /**
   * Get the maximum transmission unit (MTU) associated with this
   * destination Engine ID. This is implemented
   * by resolving the Engine ID to an IP address, then opening a
   * connected UDP socket to the destination IP address and
   * checking it for its MTU.
   *
   * \return 0 if operation failed (e.g. binding is not there); otherwise, return the MTU in bytes.   *
   */
  virtual uint16_t GetMtu () const;

  /**
    * Enable server socket to listen for incoming connections to defined server port
    *
    * \param localLtpEngineId local ltp engine id
    * \return 1 on success, 0 otherwise
    */
  virtual bool EnableReceive (const uint64_t &localLtpEngineId);
  
  virtual bool EnableSend ();
  
  virtual void DisposeSensorApp();
  
  virtual void DisposeAggSensorApp();
  
  virtual void StopSensorApp();
  
  virtual void StopAggSensorApp();

  /**
   * Assign LTP protocol to CLA.
   *
   * \param prot LtpProtocol instance
   */
  virtual void SetProtocol (Ptr<LtpProtocol> prot);

  /**
   * Get LTP protocol associated to this CLA
   *
   * \return pointer to LtpProtocol instance
   */
  virtual Ptr<LtpProtocol> GetProtocol () const;

  /*
   * Assign Routing protocol for resolution of LTP engine IDs
   * to IP addresses.
   *
   * \param pointer to routing protocol instance.
   */
  virtual void SetRoutingProtocol (Ptr<LtpIpResolutionTable> prot);

  /* \brief  Get routing protocol instance
   * \return pointer to routing protocol.
   */
  virtual Ptr<LtpIpResolutionTable> GetRoutingProtocol () const;
  
  Ptr<LtpProtocol> m_ltp;                 //!< LTP protocol
  Ptr<LtpIpResolutionTable> m_ltpRouting; //!< LTP routing protocol
  Ptr<Socket> current_socket;

private:

  /* Attribute values */

  uint16_t m_serverPort; 	 //!< Port to listen for incoming connections
  uint16_t m_keepAliveValue; //!< Keep Alive
  
  bool m_connected;    // True if connected

  /* Sockets */

  Ptr<Socket> m_rcvSocket;                          //!< Receiving socket
  Ptr<Socket> m_rcvSocket6;                         //!< Receiving socket for Ipv6
  std::map<uint64_t, Ptr<Socket> > m_l4SendSockets; //!< Socket for sending data to remote LTP egnines.
  std::list<Ptr<Socket> > m_l4ReceiveSockets; //!< Socket for sending data to remote LTP egnines.
  Address         m_peerAddress; // peer address

  /* Pointers  */

  
  
  void HandleAccept (Ptr<Socket>, const Address& from);
  void HandlePeerClose (Ptr<Socket>);
  void HandlePeerError (Ptr<Socket>);

};


//} //namespace ltp
} //namespace ns3

#endif /* LTP_UDP_CLA_H_ */
