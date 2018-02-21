/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include "ns3/qos-tag.h"
#include "ns3/uinteger.h"
#include "qos-id-seq-ts-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("QosIdSeqTsHeader");

NS_OBJECT_ENSURE_REGISTERED (QosIdSeqTsHeader);



QosIdSeqTsHeader::QosIdSeqTsHeader ()
  : m_qosId (0),
    m_privacy (0),
    m_custId (0), 
    m_operationId (0),
    m_seq (0),
    m_ts (Simulator::Now ().GetTimeStep ())
{
  NS_LOG_FUNCTION (this);
}
void
QosIdSeqTsHeader::SetPrivacy (uint8_t id)
{
  NS_LOG_FUNCTION (this << id);
  m_privacy = id;
}
uint8_t
QosIdSeqTsHeader::GetPrivacy (void) const
{
  NS_LOG_FUNCTION (this);
  return m_privacy;
}


void
QosIdSeqTsHeader::SetQosId (uint8_t id)
{
  NS_LOG_FUNCTION (this << id);
  m_qosId = id;
}
uint8_t
QosIdSeqTsHeader::GetQosId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_qosId;
}

void
QosIdSeqTsHeader::SetOpId (uint32_t id)
{
  NS_LOG_FUNCTION (this << id);
  m_operationId = id;
}
uint32_t
QosIdSeqTsHeader::GetOpId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_operationId;
}

void
QosIdSeqTsHeader::SetCustId (uint32_t id)
{
  NS_LOG_FUNCTION (this << id);
  m_custId = id;
}
uint32_t
QosIdSeqTsHeader::GetCustId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_custId;
}

void
QosIdSeqTsHeader::SetSeq (uint32_t seq)
{
  NS_LOG_FUNCTION (this << seq);
  m_seq = seq;
}
uint32_t
QosIdSeqTsHeader::GetSeq (void) const
{
  NS_LOG_FUNCTION (this);
  return m_seq;
}

Time
QosIdSeqTsHeader::GetTs (void) const
{
  NS_LOG_FUNCTION (this);
  return TimeStep (m_ts);
}

TypeId
QosIdSeqTsHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QosIdSeqTsHeader")
    .SetParent<Header> ()
    .AddConstructor<QosIdSeqTsHeader> ()
    .AddAttribute ("tid", "The tid that indicates AC which packet belongs",
                   UintegerValue (0),
                   MakeUintegerAccessor (&QosIdSeqTsHeader::m_qosId),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("Privacy", "privacy information [0], 0=no privacy, 1=privacy",
                   UintegerValue (0),
                   MakeUintegerAccessor (&QosIdSeqTsHeader::m_privacy),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}
TypeId
QosIdSeqTsHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void
QosIdSeqTsHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << " (Operation Id= " << m_operationId << " QoS ID=" << m_qosId << " Privacy id = " << m_privacy << " custId=" << m_custId << " seq=" << m_seq << " time=" << TimeStep (m_ts).GetSeconds () << ")";
}
uint32_t
QosIdSeqTsHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return 1+1+4+4+4+8;
}

void
QosIdSeqTsHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteU8 (m_qosId);
  i.WriteU8 (m_privacy);
  i.WriteHtonU32 (m_custId);
  i.WriteHtonU32 (m_operationId);
  i.WriteHtonU32 (m_seq);
  i.WriteHtonU64 (m_ts);
}
uint32_t
QosIdSeqTsHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  m_qosId = (UserPriority) i.ReadU8 ();
//  m_qosId = i.ReadNtohU16 ();
  m_privacy = i.ReadU8 ();
  m_custId = i.ReadNtohU32 ();
  m_operationId = i.ReadNtohU32 ();
  m_seq = i.ReadNtohU32 ();
  m_ts = i.ReadNtohU64 ();
  return GetSerializedSize ();
}

} // namespace ns3
