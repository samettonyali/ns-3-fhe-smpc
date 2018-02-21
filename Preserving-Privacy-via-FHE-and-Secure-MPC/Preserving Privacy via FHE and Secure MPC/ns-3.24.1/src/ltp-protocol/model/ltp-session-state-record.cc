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

#include "ltp-session-state-record.h"
#include "ns3/log.h"


NS_LOG_COMPONENT_DEFINE ("SessionStateRecord");

namespace ns3 {
//namespace ltp {

SessionStateRecord::SessionStateRecord ()
  : m_sessionId (),
    m_txQueue (),
    m_localLtpEngine (0),
    m_peerLtpEngine (0),
    m_localClientService (0),
    m_CpTimer (Timer::CANCEL_ON_DESTROY),
    m_RsTimer (Timer::CANCEL_ON_DESTROY),
    m_CxTimer (Timer::CANCEL_ON_DESTROY),
    m_firstCpSerialNumber (0),
    m_currentCpSerialNumber (0),
    m_firstRpSerialNumber (0),
    m_currentRpSerialNumber (0),
    m_rcvSegments (),
    m_redpartSucces (false),
    m_blockSuccess (false),
    m_fullRedData (false),
    m_fullGreenData (false),
    m_redPartLength (0),
    m_lowBound (0),
    m_highBound (0),
    m_rTxCnt (0),
    m_canceled (NOT_CANCELED),
    m_canceledReason (RESERVED),
    m_suspended (false)
{
  NS_LOG_FUNCTION (this);
}


SessionStateRecord::SessionStateRecord (uint64_t localLtpEngineId,
                                        uint64_t localClientServiceId,
                                        uint64_t peerLtpEngineId)
  : m_sessionId (),
    m_txQueue (),
    m_localLtpEngine (localLtpEngineId),
    m_peerLtpEngine (peerLtpEngineId),
    m_localClientService (localClientServiceId),
    m_CpTimer (Timer::CANCEL_ON_DESTROY),
    m_RsTimer (Timer::CANCEL_ON_DESTROY),
    m_CxTimer (Timer::CANCEL_ON_DESTROY),
    m_firstCpSerialNumber (0),
    m_currentCpSerialNumber (0),
    m_firstRpSerialNumber (0),
    m_currentRpSerialNumber (0),
    m_rcvSegments (),
    m_redpartSucces (false),
    m_blockSuccess (false),
    m_fullRedData (false),
    m_fullGreenData (false),
    m_redPartLength (0),
    m_lowBound (0),
    m_highBound (0),
    m_rTxCnt (0),
    m_canceled (NOT_CANCELED),
    m_canceledReason (RESERVED),
    m_suspended (false)

{
  NS_LOG_FUNCTION (this);
}


SenderSessionStateRecord::SenderSessionStateRecord ()
  : SessionStateRecord (),
    m_destinationClientServiceId (0),
    m_txData (),
    m_cpTxCnt (0),
    m_redpartAckSuccess (false)
{
  NS_LOG_FUNCTION (this);
}

SenderSessionStateRecord::SenderSessionStateRecord ( uint64_t localLtpEngineId,
                                                     uint64_t localClientServiceId,
                                                     uint64_t destinationClientService,
                                                     uint64_t destinationLtpEngine,
                                                     Ptr<RandomVariableStream> sessionNumber,
                                                     Ptr<RandomVariableStream> serialNumber
                                                     )
  : SessionStateRecord (localLtpEngineId,localClientServiceId,destinationLtpEngine),
    m_destinationClientServiceId (destinationClientService),
    m_txData (),
    m_cpTxCnt (0),
    m_redpartAckSuccess (false)
{
  NS_LOG_FUNCTION (this);
  m_sessionId = SessionId (m_localLtpEngine,
                           sessionNumber->GetInteger ()) ;
  NS_ASSERT_MSG
  ((m_sessionId.GetSessionNumber() > SessionId::MIN_SESSION_NUMBER) &&
  (m_sessionId.GetSessionNumber() < SessionId::MAX_SESSION_NUMBER),
  "The session number generated with the provided random variable is not valid");

  m_firstCpSerialNumber = serialNumber->GetInteger ();

  NS_ASSERT_MSG
  ((m_firstCpSerialNumber > MIN_INITIAL_SERIAL_NUMBER) &&
  (m_firstCpSerialNumber < MAX_INITIAL_SERIAL_NUMBER),
  "The serial number generated with the provided random variable is not valid: "
  << m_firstCpSerialNumber << " "
  << MIN_INITIAL_SERIAL_NUMBER << " "
  << MAX_INITIAL_SERIAL_NUMBER);

  m_currentCpSerialNumber = m_firstCpSerialNumber;
  m_firstRpSerialNumber = 0;
  m_currentRpSerialNumber = 0;
}
ReceiverSessionStateRecord::ReceiverSessionStateRecord ()
  : SessionStateRecord (),
    m_rpTxCnt (0),
    m_rxRedBuffer (),
    m_rxGreendBuffer ()
{
  NS_LOG_FUNCTION (this);
}

ReceiverSessionStateRecord::ReceiverSessionStateRecord (uint64_t localLtpEngineId,
                                                        uint64_t localClientServiceId,
                                                        SessionId id,
                                                        Ptr<RandomVariableStream> number )
  : SessionStateRecord (localLtpEngineId,localClientServiceId,id.GetSessionOriginator ()),
    m_rpTxCnt (0),
    m_rxRedBuffer (),
    m_rxGreendBuffer ()

{
  NS_LOG_FUNCTION (this);

  m_sessionId = id;

  m_firstCpSerialNumber = 0;
  m_currentCpSerialNumber = 0;
  m_firstRpSerialNumber = number->GetInteger ();

  NS_ASSERT_MSG
  ((m_firstRpSerialNumber > MIN_INITIAL_SERIAL_NUMBER) &&
  (m_firstRpSerialNumber < MAX_INITIAL_SERIAL_NUMBER),
  "The serial number generated with the provided random variable is not valid: "
  << m_firstCpSerialNumber << " "
  << MIN_INITIAL_SERIAL_NUMBER << " "
  << MAX_INITIAL_SERIAL_NUMBER);

  m_currentRpSerialNumber = m_firstRpSerialNumber;
}


SessionStateRecord::~SessionStateRecord ()
{
  NS_LOG_FUNCTION (this);
}
ReceiverSessionStateRecord::~ReceiverSessionStateRecord ()
{
  NS_LOG_FUNCTION (this);
}
SenderSessionStateRecord::~SenderSessionStateRecord ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SessionStateRecord::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SessionStateRecord")
    .SetParent<Object> ()
    .AddConstructor<SessionStateRecord> ()
  ;
  return tid;
}

TypeId
SessionStateRecord::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

TypeId
SenderSessionStateRecord::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::SenderSessionStateRecord")
    .SetParent<SessionStateRecord> ()
    .AddConstructor<SenderSessionStateRecord> ()
  ;
  return tid;
}

TypeId
SenderSessionStateRecord::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

TypeId
ReceiverSessionStateRecord::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::ReceiverSessionStateRecord")
    .SetParent<SessionStateRecord> ()
    .AddConstructor<ReceiverSessionStateRecord> ()
  ;
  return tid;
}

TypeId
ReceiverSessionStateRecord::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}


SessionId
SessionStateRecord::GetSessionId () const
{
  NS_LOG_FUNCTION (this);
  return m_sessionId;
}

void
SessionStateRecord::StartTimer (TimerCode type)
{
  NS_LOG_FUNCTION (this << type);
  switch (type)
    {
    case CHECKPOINT:
      m_CpTimer.Cancel ();
      m_CpTimer.Schedule ();
      break;
    case REPORT:
      m_RsTimer.Cancel ();
      m_RsTimer.Schedule ();
      break;
    case CANCEL:
      m_CxTimer.Cancel ();
      m_CxTimer.Schedule ();
      break;
    default:
      break;
    }
}

void
SessionStateRecord::CancelTimer (TimerCode type)
{
  NS_LOG_FUNCTION (this << type);

  switch (type)
    {
    case CHECKPOINT:

      m_CpTimer.Cancel ();
      break;
    case REPORT:
      m_RsTimer.Cancel ();
      break;
    case CANCEL:
      m_CxTimer.Cancel ();
      break;
    default:
      break;
    }
}

void
SessionStateRecord::SuspendTimer (TimerCode type)
{
  NS_LOG_FUNCTION (this << type);
  switch (type)
    {
    case CHECKPOINT:
      m_CpTimer.Suspend ();
      break;
    case REPORT:
      m_RsTimer.Suspend ();
      break;
    case CANCEL:
      m_CxTimer.Suspend ();
      break;
    default:
      break;
    }
}
void
SessionStateRecord::ResumeTimer (TimerCode type)
{
  NS_LOG_FUNCTION (this << type);
  switch (type)
    {
    case CHECKPOINT:
      m_CpTimer.Resume ();
      break;
    case REPORT:
      m_RsTimer.Resume ();
      break;
    case CANCEL:
      m_CxTimer.Resume ();
      break;
    default:
      break;
    }
}

uint64_t
SessionStateRecord::GetCpStartSerialNumber () const
{
  NS_LOG_FUNCTION (this);
  return m_firstCpSerialNumber;
}

uint64_t
SessionStateRecord::GetRpStartSerialNumber () const
{
  NS_LOG_FUNCTION (this);
  return m_firstRpSerialNumber;
}

uint64_t
SessionStateRecord::GetCpCurrentSerialNumber () const
{
  NS_LOG_FUNCTION (this);
  return m_currentCpSerialNumber;
}

uint64_t
SessionStateRecord::GetRpCurrentSerialNumber () const
{
  NS_LOG_FUNCTION (this);
  return m_currentRpSerialNumber;
}

uint64_t
SessionStateRecord::GetPeerLtpEngineId () const
{
  NS_LOG_FUNCTION (this);
  return m_peerLtpEngine;
}

uint64_t
SessionStateRecord::GetLocalClientServiceId () const
{
  NS_LOG_FUNCTION (this);
  return m_localClientService;
}

uint64_t
SessionStateRecord::IncrementCpCurrentSerialNumber ()
{
  NS_LOG_FUNCTION (this);
  return m_currentCpSerialNumber++;
}

uint64_t
SessionStateRecord::IncrementRpCurrentSerialNumber ()
{
  NS_LOG_FUNCTION (this);

  /* Copy claim count from previous RP */
  std::map< uint64_t, RedSegmentInfo >::iterator it = m_rcvSegments.find(m_currentRpSerialNumber++);

  RedSegmentInfo info = it->second;
  std::pair<uint64_t, RedSegmentInfo > entry;
  entry = std::make_pair (m_currentRpSerialNumber,info);
  m_rcvSegments.insert (entry).second;

  return m_currentRpSerialNumber;
}

bool
SessionStateRecord::Enqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  return m_txQueue.Enqueue (p);
}

Ptr<Packet>
SessionStateRecord::Dequeue (void)
{
  NS_LOG_FUNCTION (this);
  return m_txQueue.Dequeue ();
}
uint32_t
SessionStateRecord::GetNPackets () const
{
  NS_LOG_FUNCTION (this);
  return m_txQueue.GetNPackets ();
}
bool
SessionStateRecord::IsRedPartFinished () const
{
  NS_LOG_FUNCTION (this);
  return m_redpartSucces;
}

void
SessionStateRecord::SetRedPartFinished ()
{
  NS_LOG_FUNCTION (this);
  m_redpartSucces = true;
}

bool
SessionStateRecord::IsBlockFinished () const
{
  NS_LOG_FUNCTION (this);
  return m_blockSuccess;
}

void
SessionStateRecord::SetBlockFinished ()
{
  NS_LOG_FUNCTION (this);
  m_blockSuccess = true;
}

uint64_t
SessionStateRecord::GetRTxNumber () const
{
  NS_LOG_FUNCTION (this);
  return m_rTxCnt;
}

void
SessionStateRecord::IncrementRtxNumber ()
{
  NS_LOG_FUNCTION (this);
  m_rTxCnt++;
}

bool
SessionStateRecord::IsCanceled () const
{
  NS_LOG_FUNCTION (this);
  return ((m_canceled == NOT_CANCELED) ? false : true);
}

void
SessionStateRecord::Cancel (CancellationState s, CxReasonCode r)
{
  NS_LOG_FUNCTION (this << r << s);
  m_canceled = s;
  m_canceledReason = r;
}

void
SessionStateRecord::Close ()
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_sessionActivityEvent);
  m_suspended = true;

}

CxReasonCode
SessionStateRecord::GetCancellationReason () const
{
  NS_LOG_FUNCTION (this);
  return m_canceledReason;
}

void
SessionStateRecord::SetCpStartSerialNumber ( uint64_t serialNum)
{
  NS_LOG_FUNCTION (this << serialNum);
  if (m_firstCpSerialNumber == 0)
    {
      m_firstCpSerialNumber = serialNum;

      if (m_currentCpSerialNumber == 0)
        {
          m_currentCpSerialNumber = serialNum;
        }
    }
}

void
SessionStateRecord::SetRpStartSerialNumber ( uint64_t serialNum)
{
  NS_LOG_FUNCTION (this << serialNum);
  if (m_firstCpSerialNumber == 0)
    {
      m_firstRpSerialNumber = serialNum;
    }
}


bool
SessionStateRecord::IsSuspended () const
{
  NS_LOG_FUNCTION (this);
  return m_suspended;
}
void
SessionStateRecord::Suspend ()
{
  NS_LOG_FUNCTION (this);
  m_suspended = true;
}

void
SessionStateRecord::Resume ()
{
  NS_LOG_FUNCTION (this);
  m_suspended = false;
}


bool 
SessionStateRecord::InClaims(LtpContentHeader::ReceptionClaim claim, RedSegmentInfo info) 
{
  NS_LOG_FUNCTION (this << claim.offset << claim.length);

  uint32_t lower = claim.offset; 
  uint32_t upper = claim.offset + claim.length;
  
  for (std::set<LtpContentHeader::ReceptionClaim>::iterator it=info.claims.begin(); it!=info.claims.end(); ++it)
  {
    uint32_t upperbound = it->offset + it->length;
    bool low_found = false;
    bool high_found = false;
    
    if ((lower >= it->offset) && (lower <= upperbound))
	low_found = true;
    
    if ((upper >= it->offset) && (upper <= upperbound))
	high_found = true;     

    if (high_found && low_found)
      return true;
  }    
  return false;
}


bool SessionStateRecord::CompactClaims(uint32_t serialNum)
{
	 std::map< uint64_t, RedSegmentInfo >::iterator it = m_rcvSegments.find (serialNum);
	 if (it != m_rcvSegments.end ())
	    {
		 	 std::set<LtpContentHeader::ReceptionClaim>::iterator previous,current = it->second.claims.begin();
		 	 previous = current;
		 	 current++;

		 	 while (current != it->second.claims.end())
		 	 {
		 		 uint32_t upper = previous->offset+previous->length;
				 if (current->offset == upper) // Can the claims be merged?
				 {
					 LtpContentHeader::ReceptionClaim mergedClaim;
					 mergedClaim.offset = previous->offset;
					 mergedClaim.length = previous->length + current->length;
					 it->second.claims.erase(previous);
					 it->second.claims.erase(current);
					 it->second.claims.insert (mergedClaim).second;
					 // break and set iterators again.
					 return true;
				 }
				 //Otherwise advance
		 		 previous = current;
		 		 current++;
		 	 }
		 	 return false;
    }
	return false;
}

bool
SessionStateRecord::InsertClaim (uint32_t serialNum,uint32_t low, uint32_t high, LtpContentHeader::ReceptionClaim claim)
{
  NS_LOG_FUNCTION (this << serialNum << claim.offset << claim.length);

  std::map< uint64_t, RedSegmentInfo >::iterator it = m_rcvSegments.find (serialNum);
  bool ret = false;

  if (it == m_rcvSegments.end ())
    {
      RedSegmentInfo info;
      info.low_bound = 0;
      info.high_bound = 0;
      info.claims.insert (claim);

      std::pair<uint64_t, RedSegmentInfo > entry;

      entry = std::make_pair (serialNum,info);
      ret = m_rcvSegments.insert (entry).second;
    }
  else
    {
	  if (!InClaims(claim,it->second)) // If already claimed skip all the process
	  {

	  	  it->second.low_bound = low;
	   	  it->second.high_bound = high;

    	  LtpContentHeader::ReceptionClaim expandedClaim;

    	  bool expanded = false;
    	  std::set<LtpContentHeader::ReceptionClaim>::iterator itcl = it->second.claims.begin();

    	  /* Traverse all claims */
    	  while ( itcl!=it->second.claims.end() )
    	   {
    		  uint32_t upper = itcl->offset + itcl->length;
    		  /* Expand previous claim if this is the following one */
    		  if ( claim.offset == upper )
    		  {
    			  expandedClaim.offset = itcl->offset;
    			  expandedClaim.length = itcl->length + claim.length;
    			  expanded = true;
    			  break;
    		  }
    		  ++itcl;
    	   }

    	  /* If not expanded insert new one as it is */
    	  if (!expanded)
    	  {
    		  ret = it->second.claims.insert (claim).second;
    	  }
    	  else /* erase old one and replace with expanded one*/
    	  {
    		  it->second.claims.erase(itcl);
    		  ret = it->second.claims.insert (expandedClaim).second;
    	  }
	  }

    }

  while(CompactClaims(serialNum));

  return ret;

}

uint32_t
SessionStateRecord::GetNClaims (uint32_t serialNum) const
{
  NS_LOG_FUNCTION (this << serialNum);

  std::map< uint64_t, RedSegmentInfo >::const_iterator it =
    m_rcvSegments.find (serialNum);

  if (it != m_rcvSegments.end ())
    {
      return it->second.claims.size ();
    }

  return 0;
}

std::set<LtpContentHeader::ReceptionClaim>
SessionStateRecord::GetClaims (uint64_t reportSerialNumber)
{
  NS_LOG_FUNCTION (this << reportSerialNumber);


  std::map< uint64_t, RedSegmentInfo >::const_iterator it =
    m_rcvSegments.find (reportSerialNumber);

  if (it != m_rcvSegments.end ())
    {

      return it->second.claims;
    }
  else
    {

      return std::set<LtpContentHeader::ReceptionClaim> ();
    }

}

bool
SessionStateRecord::StoreClaims (LtpContentHeader reportHeader)
{
  NS_LOG_FUNCTION (this);

  bool ret = false;

  for (uint32_t i = 0; i < reportHeader.GetRxClaimCnt (); i++)
    {
      ret = InsertClaim (reportHeader.GetRpSerialNumber (),
                         reportHeader.GetLowerBound (),
                         reportHeader.GetUpperBound (),
                         reportHeader.GetReceptionClaim (i));

      if (!ret)
        {
          break;
        }
    }

  return ret;
}

RedSegmentInfo
SessionStateRecord::FindMissingClaims (uint32_t serialNum)
{
  NS_LOG_FUNCTION (this << serialNum);

  uint32_t last_upper = 0;

  std::set<LtpContentHeader::ReceptionClaim> report_claims = GetClaims (serialNum);

  RedSegmentInfo missing_claims;
  missing_claims.high_bound = GetRedPartLength ();
  missing_claims.low_bound = 0;

  for (std::set<LtpContentHeader::ReceptionClaim>::iterator it = report_claims.begin (); it != report_claims.end (); ++it)
    {
      /* Assumes that reception claims are ordered */
      LtpContentHeader::ReceptionClaim claim = *it;

      if (last_upper != claim.offset)
        {
          LtpContentHeader::ReceptionClaim missing;
          missing.offset = last_upper;
          missing.length = claim.offset - last_upper;

          missing_claims.claims.insert (missing);
        }

      last_upper = claim.offset + claim.length;
    }
  return missing_claims;
}

void
SessionStateRecord::SetRedPartLength (uint32_t len)
{
  NS_LOG_FUNCTION (this << len);
  m_redPartLength = len;
}

uint32_t
SessionStateRecord::GetRedPartLength () const
{
  NS_LOG_FUNCTION (this);
  return m_redPartLength;
}

void
SessionStateRecord::SetFullRed ()
{
  NS_LOG_FUNCTION (this);
  m_fullRedData = true;
}
bool
SessionStateRecord::IsFullRed () const
{
  NS_LOG_FUNCTION (this);
  return m_fullRedData;
}

void
SessionStateRecord::SetFullGreen ()
{
  NS_LOG_FUNCTION (this);
  m_fullGreenData = true;
}

bool
SessionStateRecord::IsFullGreen () const
{
  NS_LOG_FUNCTION (this);
  return m_fullGreenData;
}

void
SessionStateRecord::SessionKeepAlive()
{
	  NS_LOG_FUNCTION (this);
	  Simulator::Cancel (m_sessionActivityEvent);
	  m_sessionActivityEvent = Simulator::Schedule (m_keepAlive, &SessionStateRecord::SignalSessionInactive, this);
}

void
SessionStateRecord::SignalSessionInactive()
{
	  NS_LOG_FUNCTION (this);
	  m_inactiveSession(m_sessionId);
}

void
SessionStateRecord::SetInactiveSessionCallback( Callback<void,  SessionId > cb, Time keepAlive)
{
	  NS_LOG_FUNCTION (this << &cb);
	  m_inactiveSession = cb;
	  m_keepAlive = keepAlive;

}

uint64_t
SenderSessionStateRecord::GetDestination () const
{
  NS_LOG_FUNCTION (this);
  return m_destinationClientServiceId;
}

void
SenderSessionStateRecord::CopyBlockData (std::vector<uint8_t> data)
{
  NS_LOG_FUNCTION (this << data.size ());
  m_txData = data;
}

std::vector<uint8_t>
SenderSessionStateRecord::GetBlockData (uint32_t offset, uint32_t length)
{
  NS_LOG_FUNCTION (this << offset << length);
  std::vector<uint8_t> ret;
  std::copy ( m_txData.begin () + offset, m_txData.begin () + offset + length, ret.begin () );
  return ret;
}

std::vector<uint8_t>
SenderSessionStateRecord::GetBlockData ()
{
  NS_LOG_FUNCTION (this);
  return m_txData;
}

bool
SenderSessionStateRecord::IsRedPartAck () const
{
  NS_LOG_FUNCTION (this);
  return m_redpartAckSuccess;
}

void
SenderSessionStateRecord::SetRedPartAck ()
{
  NS_LOG_FUNCTION (this);
  m_redpartAckSuccess = true;
}

uint64_t
SenderSessionStateRecord::GetCpRtxNumber () const
{
  NS_LOG_FUNCTION (this);
  return m_cpTxCnt;
}

void
SenderSessionStateRecord::IncrementCpRtxNumber ()
{
  NS_LOG_FUNCTION (this);
  m_cpTxCnt++;
}

uint64_t
ReceiverSessionStateRecord::GetRpRtxNumber () const
{
  NS_LOG_FUNCTION (this);
  return m_rpTxCnt;
}

void
ReceiverSessionStateRecord::IncrementRpRtxNumber ()
{
  NS_LOG_FUNCTION (this);
  m_rpTxCnt++;
}

void
ReceiverSessionStateRecord::StoreRedDataSegment (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);

  LtpHeader header;
  LtpContentHeader contentHeader;

  Ptr<Packet> packet = p->Copy ();

  packet->RemoveHeader (header);
  contentHeader.SetSegmentType (header.GetSegmentType ());
  packet->RemoveHeader (contentHeader);

  std::pair<uint32_t, Ptr<Packet> > entry;
  entry = std::make_pair (contentHeader.GetOffset (),p);

  m_rxRedBuffer.insert (entry);
}

Ptr<Packet>
ReceiverSessionStateRecord::RemoveRedDataSegment ()
{
  NS_LOG_FUNCTION (this);
  Ptr<Packet> p = 0;
  if (m_rxRedBuffer.size () > 0)
    {
      std::map<uint32_t, Ptr<Packet> >::iterator it = m_rxRedBuffer.begin ();
      p = it->second;
      m_rxRedBuffer.erase (it);
    }
  return p;
}

void
ReceiverSessionStateRecord::StoreGreenDataSegment (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  m_rxGreendBuffer.push (p);
}

Ptr<Packet>
ReceiverSessionStateRecord::RemoveGreenDataSegment ()
{
  NS_LOG_FUNCTION (this);
  Ptr<Packet> p = 0;
  if (m_rxGreendBuffer.size () > 0)
    {
      p = m_rxGreendBuffer.front ();
      m_rxGreendBuffer.pop ();
    }
  return p;
}

void
ReceiverSessionStateRecord::SetLowBound (uint32_t len)
{
  NS_LOG_FUNCTION (this << len);
  m_lowBound = len;
}
uint32_t
ReceiverSessionStateRecord::GetLowBound () const
{
  NS_LOG_FUNCTION (this);
  return m_lowBound;
}

void
ReceiverSessionStateRecord::SetHighBound (uint32_t len)
{
  NS_LOG_FUNCTION (this << len);
  if (len > m_highBound)
    {
      m_highBound = len;
    }
}
uint32_t
ReceiverSessionStateRecord::GetHighBound () const
{
  NS_LOG_FUNCTION (this);
  return m_highBound;
}



//} // namespace ltp
} // namespace ns3
