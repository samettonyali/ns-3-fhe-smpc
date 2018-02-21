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

#include "ns3/log.h"
#include "ltp-queue-set.h"

NS_LOG_COMPONENT_DEFINE ("LtpQueueSet");

namespace ns3 {
//namespace ltp {


TypeId
LtpQueueSet::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LtpQueueSet")
    .SetParent<Queue> ()
    .AddConstructor<LtpQueueSet> ()
  ;

  return tid;
}

LtpQueueSet::LtpQueueSet ()
  : Queue (),
    m_internalOps (),
    m_appData ()
{
  NS_LOG_FUNCTION (this);
}

LtpQueueSet::~LtpQueueSet ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<Packet>
LtpQueueSet::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);
  Ptr<Packet> p;

  if (!m_internalOps.empty ())
    {
      p = m_internalOps.front ();
      m_internalOps.pop ();
    }
  else if (!m_appData.empty ())
    {
      p = m_appData.front ();
      m_appData.pop ();
    }
  else
    {
      NS_LOG_LOGIC ("QueueSet is empty");
      return 0;
    }
  return p;
}


bool
LtpQueueSet::DoEnqueue (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
  /*LtpHeader header;
  p->PeekHeader (header);
  SegmentType type = header.GetSegmentType ();

  switch (type)
    {
    case LTPTYPE_RD:
    case LTPTYPE_RD_CP:
    case LTPTYPE_RD_CP_EORP:
    case LTPTYPE_RD_CP_EORP_EOB:
    case LTPTYPE_GD:
    case LTPTYPE_GD_EOB:
      m_appData.push (p);
      break;
    case LTPTYPE_RS:
    case LTPTYPE_RAS:
    case LTPTYPE_CS:
    case LTPTYPE_CAS:
    case LTPTYPE_CR:
    case LTPTYPE_CAR:
      m_internalOps.push (p);
      break;
    default:
      NS_LOG_LOGIC ("Unexpected Segment type");
      return false;
      break;
    }*/
  
  m_appData.push (p);
  
  NS_LOG_INFO(p->GetSize() << "-byte packet was pushed to the queue!");

  return true;
}

Ptr<const Packet>
LtpQueueSet::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);
  Ptr<Packet> p;

  if (!m_internalOps.empty ())
    {
      p = m_internalOps.front ();

    }
  else if (!m_appData.empty ())
    {
      p = m_appData.front ();

    }
  else
    {
      NS_LOG_LOGIC ("QueueSet is empty");
      return 0;
    }
  return p;

  NS_LOG_LOGIC ("Number packets " << m_internalOps.size () + m_appData.size ());

  return p;
}

//} // namespace ltp
} // namespace ns3
