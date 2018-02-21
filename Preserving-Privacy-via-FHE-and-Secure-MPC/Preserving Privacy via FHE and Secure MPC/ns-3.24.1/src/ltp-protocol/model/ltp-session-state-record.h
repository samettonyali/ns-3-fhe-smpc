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

#ifndef LTP_SESSION_STATE_RECORD_H
#define LTP_SESSION_STATE_RECORD_H

#include "ltp-header.h"
#include "ltp-queue-set.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/object.h"
#include "ns3/timer.h"
#include <set>
#include <map>

namespace ns3 {
//namespace ltp {

/**
 * \enum TimerCode
 * \brief Defines the several timer types
 */
enum TimerCode
{
  CHECKPOINT = 0,  //!< Checkpoint Timer code.
  REPORT = 1,      //!< Report Timer code.
  CANCEL = 2       //!< Cancel timer code.
};

/**
 * \enum CancellationState
 * \brief Defines the status and reason of session cancellation
 */
enum CancellationState
{
  NOT_CANCELED = 0,  //!< Session Active
  REMOTE_CANCEL = 1,  //!< Remote LTP engine canceled the session
  LOCAL_CANCEL = 2,   //!< Remote LTP engine canceled the session
};

/*
 * \brief struct that stores specific values from report or checkpoint
 * segments, used for retransmission.
 */
struct RedSegmentInfo
{
  uint32_t CpserialNum;
  uint32_t RpserialNum;
  uint32_t low_bound;
  uint32_t high_bound;
  std::set<LtpContentHeader::ReceptionClaim> claims;
};

/**
 * \ingroup dtn
 *
 * \brief Abstract class to keep track of LTP session state, contains shared
 * flags, counters, timers and lists used in both sender and receiver sessions
 */
class SessionStateRecord : public Object
{

public:
  /**
   * \brief Default Constructor
   */
  SessionStateRecord ();
  /**
   * \brief Parametized Constructor Session State Record segment
   * \param localLtpEngineId local ltp Engine id.
   * \param localClientServiceId local client service id.
   * \param peerLtpEngineId remote ltp engine id.
   */
  SessionStateRecord (uint64_t localLtpEngineId,
                      uint64_t localClientServiceId,
                      uint64_t peerLtpEngineId);
  /**
   * \brief Pure virtual destructor this class cannot be instantiated.
   */
  ~SessionStateRecord ();


  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /* Timer management functions */

  /**
   * \brief Start the timer based on specified parameters
   * \param fn Function to call on timer expiration.
   * \param delay Time to wait.
   * \param type Type of timer to schedule.
   */
  template <typename FN>
  void SetTimerFunction (FN fn,const Time delay,TimerCode type);

  template <typename MEM_PTR, typename OBJ_PTR>
  void SetTimerFunction (MEM_PTR memPtr, OBJ_PTR objPtr, const Time delay,TimerCode type);

  template <typename MEM_PTR, typename OBJ_PTR, typename T1>
  void SetTimerFunction (MEM_PTR memPtr, OBJ_PTR objPtr, T1 param, const Time delay,TimerCode type);

  template <typename MEM_PTR, typename OBJ_PTR, typename T1, typename T2>
  void SetTimerFunction (MEM_PTR memPtr, OBJ_PTR objPtr, T1 param,T2 param2, const Time delay, TimerCode type);

  /**
   * \brief Start timer specified by parameter, if already running, it is stopped and restarted.
   * \param type Code of timer to start.
   */
  void StartTimer (TimerCode type);

  /**
   * \brief Cancel timer
   * \param type Code of timer to cancel.
   */

  void CancelTimer (TimerCode type);

  /**
   * \brief Suspend timer
   * \param type Code of timer to suspend.
   */
  void SuspendTimer (TimerCode type);

  /*
   * \brief Resume timer.
   * \param type Code of timer to resume.
   */
  void ResumeTimer (TimerCode type);

  /* Data management functions */

  /*
   * \brief Enqueue a packet for transmission
   * \param p Packet to Enqueue.
   */
  bool Enqueue (Ptr<Packet> p);

  /*
   * \brief Dequeue a packet to transmit
   * \return Dequeued Packet.
   */
  Ptr<Packet> Dequeue (void);

  /*
   * \brief Get number of queued packets
   * \return Number of packets.
   */
  uint32_t GetNPackets (void) const;

  /*
   * \brief Insert a reception claim. Should be called upon reception of a data
   * segment if used by the receiver, or upon reception of a report segment if
   * used by the sender.
   * \param serialNum Serial Number Identifier to locate claims.
   * \param low Lower Bound of stored claims.
   * \param high Higher bound of store claims.
   * \param claim Reception claim including offset and length of the data.
   */
  bool InsertClaim (uint32_t serialNum,uint32_t low, uint32_t high, LtpContentHeader::ReceptionClaim claim);

  /*
   * \brief Store claims for a given serial number.
   * \param reportHeader packet corresponding to a report (contains claims, bounds and serial number).
   */
  bool StoreClaims (LtpContentHeader reportHeader);

  bool CompactClaims(uint32_t serialNum);

  /*
   * \brief Get the number of claims stored for a given serial number.
   * \param serialNum Serial Number Identifier to locate claims.
   * \return Number of claims
   */
  uint32_t GetNClaims (uint32_t serialNum) const;
  /*
   * \brief Get list of claims stored for a given serial number.
   * \param serialNum Serial Number Identifier to locate claims.
   * \return list of claims.
   */
  std::set<LtpContentHeader::ReceptionClaim> GetClaims (uint64_t reportSerialNumber);

  /*
   * \brief Find missing claims between the upper and lower bound of a given serial number.
   * \param serialNum Serial Number Identifier to locate claims.
   * \return report information about missing claims.
   */
  RedSegmentInfo FindMissingClaims (uint32_t serialNum);

  /* Setter Methods */

  /*
   * \brief Increment current Checkpoint serial number.
   * \return Value before increase.
   */
  uint64_t IncrementCpCurrentSerialNumber ();
  /*
   * \brief Increment current Report serial number.
   * \return Value before increase.
   */
  uint64_t IncrementRpCurrentSerialNumber ();

  /*
   * \brief Increment retransmission counter.
   */
  void IncrementRtxNumber ();

  /*
   * \brief Signal the successful transmission of the red part data.
   */
  void SetRedPartFinished ();

  /*
   * \brief Signal the successful transmission of the full block of data.
   */
  void SetBlockFinished ();

  /*
   * \brief Signal that the block only contained red data.
   */
  void SetFullRed ();
  /*
   * \brief Signal that the block only contained green data.
   */
  void SetFullGreen ();
  /*
   * \brief Set length of the red part of the block.
   * \param len Length of red data.
   */
  void SetRedPartLength (uint32_t len);


  /*
   * \brief Set starting checkpoint serial number.
   * \return serialNum before increase.
   */
  void SetCpStartSerialNumber (uint64_t serialNum);

  /*
   * \brief Set starting report serial number.
   * \return serialNum before increase.
   */
  void SetRpStartSerialNumber (uint64_t serialNum);

  /*
   * \brief Cancel Transmission Session
   * \param s Indicates which peer requested the cancellation.
   * \param r Indicates the reason for cancellation.
   */
  void Cancel (CancellationState s,CxReasonCode r);

  /*
   * \brief Close Transmission Session
   */
  void Close ();

  /*
   * \brief Suspend Transmission Session
   */
  void Suspend ();

  /*
   * \brief Resume Transmission Session
   */
  void Resume ();

  /* Getter Methods */

  /*
   * \return Checkpoint Starting serial number
   */
  uint64_t GetCpStartSerialNumber () const;
  /*
   * \return Checkpoint current serial number
   */
  uint64_t GetCpCurrentSerialNumber () const;
  /*
   * \return Report Starting serial number
   */
  uint64_t GetRpStartSerialNumber () const;
  /*
   * \return Report current serial number
   */
  uint64_t GetRpCurrentSerialNumber () const;
  /*
   * \return Remote LTP engine Id
   */
  uint64_t GetPeerLtpEngineId () const;
  /*
   * \return Local Client Service Instance id
   */
  uint64_t GetLocalClientServiceId () const;

  /*
   * \return true if Red part transmitted successfully, false otherwise
   */
  bool IsRedPartFinished () const;
  /*
   * \return true if whole data block transmitted successfully, false otherwise
   */
  bool IsBlockFinished () const;
  /*
   * \return true if session has been canceled, false otherwise
   */
  bool IsCanceled () const;
  /*
   * \return true if session is suspended, false if active.
   */
  bool IsSuspended () const;
  /*
   * \return true if the block only contains red data, false if active.
   */
  bool IsFullRed () const;
  /*
   * \return true if the block only contains green data, false if active.
   */
  bool IsFullGreen () const;

  /*
   * \return Reason for cancellation.
   */
  CxReasonCode GetCancellationReason () const;

  /**
  * \return SessionId object
  */
  SessionId GetSessionId () const;

  /**
  * \return Number of retransmissions.
  */
  uint64_t GetRTxNumber () const;

  /**
  * \return Length of the red part of the data block.
  */
  uint32_t GetRedPartLength () const;

  void SessionKeepAlive();

  void SetInactiveSessionCallback( Callback<void,  SessionId > cb, Time keepAlive);

  /* Constants */
  /*
  * CCSDS 734.1-R-2: 3.4.3-3.4.7
  * The initial checkpoint serial number values should be chosen at
  * random from the range [1, 2^14-1]
. */
  static const uint32_t MIN_INITIAL_SERIAL_NUMBER = 1;
  static const uint32_t MAX_INITIAL_SERIAL_NUMBER = 16383;

  /*
  * CCSDS 734.1-R-2: 3.4.5
  * If the value of the checkpoint serial number for a given session
  * exceeds 2^32, the session sender must cancel the session with
  * reason code SYS_CNCLD.
  */
  static const uint64_t MAX_SERIAL_NUMBER = 4294967296LLU;


protected:

  void SignalSessionInactive();

  EventId m_sessionActivityEvent;
  Callback<void,SessionId> m_inactiveSession;
  Time m_keepAlive;

  SessionId m_sessionId;                    //!< Session Identifier
  LtpQueueSet m_txQueue;                    //!< Outgoing Packet Queue

  uint64_t m_localLtpEngine;                //!< Id of the local Ltp Engine
  uint64_t m_peerLtpEngine;                 //!< Id of the peer Ltp Engine
  uint64_t m_localClientService;            //!< Local Client Service Id


  /* Timers */
  Timer m_CpTimer;      //!< Checkpoint timer.
  Timer m_RsTimer;      //!< Report timer.
  Timer m_CxTimer;      //!< Cancel timer.

  /* Checkpoints*/
  uint64_t m_firstCpSerialNumber;       //!< First checkpoint serial number chosen(sender)/received (received)
  uint64_t m_currentCpSerialNumber;     //!< Next checkpoint serial number (sender) / Last checkpoint number received (receiver)

  /* Reception Reports */
  uint64_t m_firstRpSerialNumber;       //!< First report serial number chosen(receiver)/received (sender)
  uint64_t m_currentRpSerialNumber;     //!< Next report serial number (receiver) / Last report serial number received (sender)


  std::map< uint64_t, RedSegmentInfo >
  m_rcvSegments;                        //!< Track Received (receiver) or ACKed (sender) segments - First : Serial Number , Second: ReceptionClaims

  bool m_redpartSucces;                 //!< Red part Transmitted/Received successfully
  bool m_blockSuccess;                  //!< Full block Transmitted/Received successfully
  bool m_fullRedData;                   //!< True if the block only contains red data
  bool m_fullGreenData;                 //!< True if the block only contains green data

  uint32_t m_redPartLength;             //!< Length of the red part
  uint32_t m_lowBound;                  //!< Smallest non acknowledged offset
  uint32_t m_highBound;                 //!< Biggest non acknowledged offset

  /* Retransmission */
  uint64_t m_rTxCnt;                    //!< Count Number of Retransmissions.

  /* Session state */
  CancellationState m_canceled;         //!< Session Canceled
  CxReasonCode m_canceledReason;        //!< Reason of Cancellation
  bool m_suspended;                     //!< Session Suspended

private:
  
  bool InClaims(LtpContentHeader::ReceptionClaim claim, RedSegmentInfo info);


};

/**
 * \ingroup dtn
 *
 * \brief The Sender Session State Record class to
 *  store sender specific variables and methods.
 */
class SenderSessionStateRecord : public SessionStateRecord
{
public:
  /**
    * \brief Default Constructor
    */
  SenderSessionStateRecord ();
  /**
    * \brief Constructor a Sender session state record for the given destination
    * \param Destination Client Service Instance
    * \param Config parameters.
    */
  SenderSessionStateRecord (uint64_t localLtpEngineId,
                            uint64_t localClientServiceId,
                            uint64_t destinationClientService,
                            uint64_t destinationLtpEngine,
                            Ptr<RandomVariableStream> sessionNumber,
                            Ptr<RandomVariableStream> serialNumber );
  /**
    * \brief Default Destructor
    */
  ~SenderSessionStateRecord ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /*
   * \return Destination Client Service Instance
   */
  uint64_t GetDestination () const;
  /*
  * \return Number of checkpoint retransmissions during this session.
  */
  uint64_t GetCpRtxNumber () const;
  /*
   * \return true if Red part acknowledged successfully, false otherwise
   */
  bool IsRedPartAck () const;
  /*
   * \brief Get the block of data to be transmitted
   * \return block data to be transmited.
   */
  std::vector<uint8_t> GetBlockData ();
  /*
   * \brief Get the block of data to be transmitted within the bounds specified
   * by offset and length.
   * \param offset starting index of the data.
   * \param length length of the data.
   * \return block data to be transmited.
   */
  std::vector<uint8_t> GetBlockData (uint32_t offset, uint32_t length);

  /*
  * \brief Store a copy of the block data to be transmitted (
  * it may be required for retransmissions)
  * \param data vector containing the data.
  */
  void CopyBlockData (std::vector<uint8_t> data);
  /*
   * \brief Signal the successful acknowledgment of the red part data.
   */
  void SetRedPartAck ();
  /*
   * \brief Increment Checkpoint retransmission counter.
   */
  void IncrementCpRtxNumber ();


private:
  uint64_t m_destinationClientServiceId;       //!<  Destination Client Service instance
  std::vector<uint8_t> m_txData;               //!< Block Data to transmit

  uint64_t m_cpTxCnt;                          //!< Count Number of retransmitted checkpoints

  bool m_redpartAckSuccess;                        //!< Red part Acknowledged successfully
};
/**
 * \ingroup dtn
 *
 * \brief The Receiver Session State Record class to store
 *  receiver specific variables and methods.
 */
class ReceiverSessionStateRecord : public SessionStateRecord
{
public:
  /**
   * \brief Default Constructor
   */
  ReceiverSessionStateRecord ();
  /**
   * \brief Constructor a Receiver session state record for the given destination
   * \param Session ID obtained from the sender LTP engine.
   * \param Config parameters.
   */
  ReceiverSessionStateRecord (uint64_t localLtpEngineId,
                              uint64_t localClientServiceId,
                              SessionId session,
                              Ptr<RandomVariableStream> number );
  /**
   * \brief Default Destructor
   */
  ~ReceiverSessionStateRecord ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;


  /* Data Storage */

  /*
   * \brief Store a received red data segment in the inbound traffic queue.
   * \param p received packet.
   */
  void StoreRedDataSegment (Ptr<Packet> p);
  /*
  * \brief Store a received green data segment in the inbound traffic queue.
  * \param p received packet.
  */
  void StoreGreenDataSegment (Ptr<Packet> p);

  /*
   * \brief remove a received red data segment from the inbound traffic queue.
   * \return p received packet.
   */
  Ptr<Packet> RemoveRedDataSegment ();
  /*
   * \brief remove a received green data segment from the inbound traffic queue.
   * \return p received packet.
   */
  Ptr<Packet> RemoveGreenDataSegment ();

  /* Setter functions */

  /*
   * \brief Set lower bound of received data
   * \param offset Lower bound.
   */
  void SetLowBound (uint32_t offset);

  /*
   * \brief Set higher bound of received data
   * \param offset higher bound.
   */
  void SetHighBound (uint32_t offset);
  /*
   * \brief Increment Checkpoint retransmission counter.
   */
  void IncrementRpRtxNumber ();


  /* Getter functions */

  /*
  * \brief Get lower bound of received data
  * \return Lower bound.
  */
  uint32_t GetLowBound () const;
  /*
  * \brief Get higher bound of received data
  * \return High bound.
  */
  uint32_t GetHighBound () const;
  /*
  * \return Number of checkpoint retransmissions during this session.
  */
  uint64_t GetRpRtxNumber () const;

private:
  uint64_t m_rpTxCnt;                             //!< Count Number of retransmitted reports.

  std::map<uint32_t, Ptr<Packet> > m_rxRedBuffer; //!< Storage for received red segments
  std::queue<Ptr<Packet> > m_rxGreendBuffer;      //!<  Storage for received green segments
};

//} // namespace ltp
} // namespace ns3

#include "ltp-session-state-record-impl.h"

#endif /* LTP_SESSION_STATE_RECORD_H */

