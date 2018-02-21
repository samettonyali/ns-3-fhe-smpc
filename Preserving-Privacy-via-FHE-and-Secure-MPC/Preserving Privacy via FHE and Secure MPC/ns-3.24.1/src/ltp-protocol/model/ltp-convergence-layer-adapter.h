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

#ifndef LTP_CONVERGENCE_LAYER_ADAPTER_H_
#define LTP_CONVERGENCE_LAYER_ADAPTER_H_

#include "ns3/object.h"
#include "ns3/socket.h"
#include "ltp-header.h"
#include "ltp-session-state-record.h"

class Packet;

namespace ns3 {
//namespace ltp {

class LtpProtocol;
class LtpIpResolutionTable;
class SessionId;


/**
 * \ingroup dtn
 *
 * \brief Convergence Layer Adapter abstract base class
 *
 * Abstract base class for CLAs used in the LTP protocol
 */
class LtpConvergenceLayerAdapter : public Object
{
public:
  static TypeId GetTypeId (void);

  /**
   * Default Constructor
   */
  LtpConvergenceLayerAdapter ();

  /**
   * Destructor
   */
  virtual ~LtpConvergenceLayerAdapter () = 0;

  /*
   * Send packet using the underlying layer.
   * \param p packet to send.
   * \return 0 if operation failed, size of sent data otherwise.
   */
  virtual uint32_t Send (Ptr<Packet> p) = 0;

  /* \brief Receive a data packet from the lower layer.
   * \return 0 if there are no available datagrams to deliver
   */
  virtual void Receive (Ptr<Socket> socket) = 0;

  /**
   * Get the maximum transmission unit (MTU) associated with this
   * destination Engine ID.
   * \return 0 if operation failed (e.g. binding is not there);
   * otherwise, return the MTU in bytes.
   */
  virtual uint16_t GetMtu () const = 0;

  /**
    * Enable the underlying layer to receive packets
    *
    * \param localLtpEngineId local ltp engine id
    * \return 1 on success, 0 otherwise
    */
  virtual  bool EnableReceive (const uint64_t &localLtpEngineId) = 0;
  
  virtual bool EnableSend () = 0;
  
  virtual void DisposeSensorApp() = 0;
  
  virtual void StopSensorApp() = 0;
  
  virtual void DisposeAggSensorApp() = 0;
  
  virtual void StopAggSensorApp() = 0;

  /* Getter functions */

  /**
   * Get LTP protocol associated to this CLA
   *
   * \return pointer to LtpProtocol instance
   */
  virtual Ptr<LtpProtocol> GetProtocol () const = 0;

  /*
   * \brief Get routing protocol instance
   *
   * \return pointer to routing protocol.
   */
  virtual Ptr<LtpIpResolutionTable> GetRoutingProtocol () const = 0;

  /*
   * \brief Get Ltp engine of remote engine to which this channel is linked
   */
  uint64_t GetRemoteEngineId () const;

  /*
   * \brief Get active session id that is using this channel.
   */
  SessionId GetSessionId () const;
  
  /*
   * Check link state
   * \return true if link is ready, false otherwhise
   */
  bool IsLinkUp();

  /* Setter functions */

  /**
   * Assign LTP protocol to CLA.
   *
   * \param prot LtpProtocol instance
   */
  virtual void SetProtocol (Ptr<LtpProtocol> prot) = 0;

  /*
   * Assign Routing protocol for resolution of LTP engine IDs
   * to IP addresses.
   *
   * \param prot pointer to routing protocol instance.
   */
  virtual void SetRoutingProtocol (Ptr<LtpIpResolutionTable> prot) = 0;

  /*
   * \brief Set ltp engine of remote engine to which this channel is linked.
   * \param id remote ltp engine id.
   */
  void SetRemoteEngineId (uint64_t id);

  /*
   * \brief Set active session id that is using this channel.
   * \param id Ltp session id.
   */
  void SetSessionId (SessionId id );

  /*
   * \brief switches link status to "up" and notifies upper layer, if already up it does nothing.
   */
  void SetLinkUp (void);

  /*
   * \brief switches link status to "down" and notifies upper layer, if already down it does nothing.
   */
  void SetLinkDown (void);


  /* Setters for Callbacks :
   * each callback represents a link state cue as defined in Section 5 of RFC 5327
   */

  void SetLinkUpCallback ( Callback<void,  Ptr<LtpConvergenceLayerAdapter> >);
  void SetLinkDownCallback ( Callback<void, Ptr<LtpConvergenceLayerAdapter> >);

  void SetCheckPointSentCallback ( Callback<void, SessionId,RedSegmentInfo>);
  void SetReportSentCallback ( Callback<void, SessionId,RedSegmentInfo>);

  void SetEndOfBlockSentCallback ( Callback<void, SessionId>);
  void SetCancellationCallback ( Callback<void, SessionId>);
  
  uint64_t m_peerLtpEngineId;                   //!< Remote Ltp engine to which this cla is linked.

protected:

  bool m_linkState;                             //!< Defines Link state: up/down.

  
  SessionId m_activeSessionId;                  //!< Ltp session id that is using this channel.

  /* Link status callbacks */

  Callback<void, Ptr<LtpConvergenceLayerAdapter> >  m_linkUp;   //!< Callback used to notify link ready status.
  Callback<void, Ptr<LtpConvergenceLayerAdapter> >  m_linkDown; //!< Callback used to notify link down status.

  /* Segment transmission callbacks */

  Callback<void, SessionId,RedSegmentInfo>        m_checkpointSent; //!< Notify transmission of a checkpoint segment
  Callback<void, SessionId,RedSegmentInfo>        m_reportSent; 	//!< Callback used to notify that Report has been transmitted
  Callback<void, SessionId>                   	  m_endOfBlockSent; //!< Callback used to notify that the EOB segment has been transmitted
  Callback<void, SessionId>                       m_cancelSent; 	//!< Callback used to notify that a Cancelation segment has been transmitted


};

//} // namespace ltp
} // namespace ns3


#endif /* LTP_CONVERGENCE_LAYER_ADAPTER_H_ */
