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
#include "id-seq-ts-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("IdSeqTsHeader");

NS_OBJECT_ENSURE_REGISTERED (IdSeqTsHeader);

IdSeqTsHeader::IdSeqTsHeader ()
  : m_custId (0), 
    m_opID (0),
    m_seq (0),
    m_ts (Simulator::Now ().GetTimeStep ())
{
  NS_LOG_FUNCTION (this);
}

void
IdSeqTsHeader::SetCustId (uint32_t id)
{
  NS_LOG_FUNCTION (this << id);
  m_custId = id;
}
uint32_t
IdSeqTsHeader::GetCustId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_custId;
}

void
IdSeqTsHeader::SetOpId (uint32_t id)
{
  NS_LOG_FUNCTION (this << id);
  m_opID = id;
}
uint32_t
IdSeqTsHeader::GetOpId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_opID;
}

void
IdSeqTsHeader::SetSeq (uint32_t seq)
{
  NS_LOG_FUNCTION (this << seq);
  m_seq = seq;
}
uint32_t
IdSeqTsHeader::GetSeq (void) const
{
  NS_LOG_FUNCTION (this);
  return m_seq;
}

Time
IdSeqTsHeader::GetTs (void) const
{
  NS_LOG_FUNCTION (this);
  return TimeStep (m_ts);
}

TypeId
IdSeqTsHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::IdSeqTsHeader")
    .SetParent<Header> ()
    .AddConstructor<IdSeqTsHeader> ()
  ;
  return tid;
}
TypeId
IdSeqTsHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void
IdSeqTsHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "(custId=" << m_custId << " opID=" << m_opID << " seq=" << m_seq << " time=" << TimeStep (m_ts).GetSeconds () << ")";
}
uint32_t
IdSeqTsHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return (sizeof(m_custId) + sizeof(m_seq) + sizeof(m_ts) + sizeof(m_opID));//4+4+8+4;
}

void
IdSeqTsHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteHtonU32 (m_custId);
  i.WriteHtonU32 (m_opID);
  i.WriteHtonU32 (m_seq);
  i.WriteHtonU64 (m_ts);
}
uint32_t
IdSeqTsHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  m_custId = i.ReadNtohU32 ();
  m_opID = i.ReadNtohU32 ();
  m_seq = i.ReadNtohU32 ();
  m_ts = i.ReadNtohU64 ();
  return GetSerializedSize ();
}

} // namespace ns3
