/* 
 * File:   vanet-packet-sink.h
 * Author: samet
 *
 * Created on August 17, 2016, 2:53 PM
 */

#ifndef VANET_PACKET_SINK_H
#define	VANET_PACKET_SINK_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/string.h"

#include <map>

namespace ns3 {

class Address;
class Socket;
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
class VanetPacketSink : public Application
{
public:
  static TypeId GetTypeId (void);
  VanetPacketSink ();
  VanetPacketSink (uint16_t port, Address local, uint32_t delay);

  virtual ~VanetPacketSink ();

  void ReportStat (std::ostream & os);

  /**
   * \return the total bytes received in this sink app
   */
  uint32_t GetTotalRx () const;

  /**
   * \return pointer to listening socket
   */
  Ptr<Socket> GetListeningSocket (void) const;

  /**
   * \return list of pointers to accepted sockets
   */
  std::list<Ptr<Socket> > GetAcceptedSockets (void) const;

protected:
  virtual void DoDispose (void);
private:
  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop
  
  void Send (void);

  void HandleReadUDP (Ptr<Socket>);
  void HandleReadTCP (Ptr<Socket> socket);
  void EchoPacket(Ptr<Packet>);
  void SendToTarget(Ptr<Packet>);
  void SendToTargetSocket(Ptr<Packet> packet, Ptr<Socket> socket);
  void HandleAccept (Ptr<Socket>, const Address& from);
  void HandlePeerClose (Ptr<Socket>);
  void HandlePeerError (Ptr<Socket>);
  void HandleReport (Ptr<Packet> pkt, Address from);
  bool IsThereAnyPreviousStoredPacket(Address from);
  void StatPrint ();
  
  void SendPacket (uint32_t seqNum);
  void SendUDPPacket ();
  void ConnectionSucceeded (Ptr<Socket> socket);
  void ConnectionFailed (Ptr<Socket> socket);
  
  void CancelEvents ();

  struct DataWaitingPacket {
          Address from;
          Ptr<Packet> pkt;
  };

  struct StatRecord {
	  uint16_t round;
	  uint32_t rxCount;
	  uint32_t rxBytes;
	  uint32_t totDelay;
	  Time     firstRxTime;
	  Time     lastRxTime;
	  Time     minTxTime;
  };

  // In the case of TCP, each socket accept returns a new socket, so the
  // listening socket is stored seperately from the accepted sockets
  Ptr<Socket>     m_socket;       // Listening socket
  Ptr<Socket>     m_localTCPsocket;
  Ptr<Socket>     m_EVsocket;
  Address         m_EVfrom;
  uint16_t m_UDPPort; //!< Port on which we listen for incoming packets.
  Ptr<Socket> m_UDPsocket; //!< IPv4 Socket
  Ptr<Socket> m_UDPsocket6; //!< IPv6 Socket
  Address m_UDPlocal; //!< local multicast address
  std::list<Ptr<Socket> > m_socketList; //the accepted sockets
  std::list<Ptr<Socket> > m_targetSockets; //the accepted sockets
  std::vector<DataWaitingPacket> m_waitingPacket;
  std::vector<StatRecord> m_stat;
  std::vector<Address> m_targets;
  
  typedef std::map<uint32_t, uint32_t> MeterSeqNumMap;
  MeterSeqNumMap m_meterSeqNumMap;

  Address         m_local;        // Local address to bind to
  Address         m_target;        // Local address to bind to
  Address         m_UDPTarget;
  bool            m_connected;    // True if connected
  Ptr<Socket>     m_targetSocket;       // Associated socket
  Ptr<Socket>     m_UDPTargetSocket;       // Associated socket
  uint32_t        m_totalRx;      // Total bytes received
  uint32_t        m_totBytes;     // Total bytes sent so far
  EventId         m_sendEvent;    // Eventid of pending "send packet" event
  TypeId          m_tid;          // Protocol TypeId
  Time            m_lastStartTime; // Time last packet sent
  Time            m_firstSendingTime;
  uint32_t        m_serviceMessageSize; //!< Size of the sent packet (including the SeqTsHeader)
  Time            m_serviceMessageInterval; //!< Packet inter-send time
  TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;
  TracedCallback<Ptr<const Packet> > m_txTrace;
  uint16_t        m_defSize ; // default size of receiving packet
  uint32_t        m_pktSize;
  uint32_t        m_seqnum;
  uint32_t        m_procDelay;
  uint32_t        m_childNum;
  uint32_t        m_leafMeters;
  uint32_t        m_meterType;
  uint32_t        m_mode;
  uint32_t        m_scenario;
  uint32_t        m_operationId;
  uint32_t        m_nNodes;
  uint32_t        m_DCRLMode;
  std::string     m_outputFilename;
  uint8_t         m_multTargetFlag;

};

} // namespace ns3

#endif	/* VANET_PACKET_SINK_H */

