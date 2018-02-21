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
#include "id-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("IdHeader");

NS_OBJECT_ENSURE_REGISTERED (IdHeader);

IdHeader::IdHeader ()
  : m_id (0)
{
  NS_LOG_FUNCTION (this);
}

void
IdHeader::SetId (uint32_t id)
{
  NS_LOG_FUNCTION (this << id);
  m_id = id;
}
uint32_t
IdHeader::GetId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_id;
}

TypeId
IdHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::IdHeader")
    .SetParent<Header> ()
    .AddConstructor<IdHeader> ()
  ;
  return tid;
}
TypeId
IdHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void
IdHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "(consumer id=" << m_id << ")";
}
uint32_t
IdHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return 4;
}

void
IdHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteHtonU32 (m_id);
}
uint32_t
IdHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  m_id = i.ReadNtohU32 ();
  return GetSerializedSize ();
}

} // namespace ns3
