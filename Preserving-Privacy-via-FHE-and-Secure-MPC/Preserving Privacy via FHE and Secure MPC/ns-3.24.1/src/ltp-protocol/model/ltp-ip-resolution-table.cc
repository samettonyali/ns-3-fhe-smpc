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

#include "ltp-ip-resolution-table.h"
#include "ns3/enum.h"
#include "ns3/log.h"

namespace ns3 {
//namespace ltp {

NS_LOG_COMPONENT_DEFINE ("LtpIpResolutionTable");

LtpIpResolutionTable::LtpIpResolutionTable ()
{
  NS_LOG_FUNCTION (this);
}

LtpIpResolutionTable::~LtpIpResolutionTable ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
LtpIpResolutionTable::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LtpIpResolutionTable")
    .SetParent<Object> ()
    .AddConstructor<LtpIpResolutionTable> ()
    .AddAttribute ("Addressing", "Ipv4 or Ipv6",
                   EnumValue (Ipv4),
                   MakeEnumAccessor (&LtpIpResolutionTable::m_addressMode),
                   MakeEnumChecker (Ipv4,"Ipv4",
                                    Ipv6,"Ipv6"))
  ;
  return tid;
}


bool LtpIpResolutionTable::AddBinding (uint64_t dstLtpEngineId, Ipv4Address dstAddr)
{
  NS_LOG_FUNCTION (this << " " << dstLtpEngineId << " " << dstAddr);
  return (m_ltpToIpv4Addr.insert ( std::pair<uint64_t, InetSocketAddress> (dstLtpEngineId,InetSocketAddress (dstAddr)))).second;
}

bool LtpIpResolutionTable::AddBinding (uint64_t dstLtpEngineId, Ipv4Address dstAddr, uint16_t dstPort)
{
  NS_LOG_FUNCTION (this << " " << dstLtpEngineId << " " << dstAddr << " " << dstPort );
  return (m_ltpToIpv4Addr.insert ( std::pair<uint64_t, InetSocketAddress> (dstLtpEngineId,InetSocketAddress (dstAddr,dstPort)))).second;
}

bool LtpIpResolutionTable::AddBinding (uint64_t dstLtpEngineId, Ipv6Address dstAddr)
{
  NS_LOG_FUNCTION (this << " " << dstLtpEngineId << " " << dstAddr);
  return (m_ltpToIpv6Addr.insert ( std::pair<uint64_t, Inet6SocketAddress> (dstLtpEngineId,Inet6SocketAddress (dstAddr)))).second;
}

bool LtpIpResolutionTable::AddBinding (uint64_t dstLtpEngineId, Ipv6Address dstAddr, uint16_t dstPort)
{
  NS_LOG_FUNCTION (this << " " << dstLtpEngineId << " " << dstAddr << " " << dstPort );
  return (m_ltpToIpv6Addr.insert ( std::pair<uint64_t, Inet6SocketAddress> (dstLtpEngineId,Inet6SocketAddress (dstAddr,dstPort)))).second;
}

bool LtpIpResolutionTable::RemoveBinding (uint64_t dstLtpEngineId, Ipv4Address dstAddr)
{
  NS_LOG_FUNCTION (this << " " << dstLtpEngineId << " " << dstAddr);
  bool ret = (m_ltpToIpv4Addr.erase (dstLtpEngineId) == 1) ? true : false;
  return ret;
}

bool LtpIpResolutionTable::RemoveBinding (uint64_t dstLtpEngineId, Ipv4Address dstAddr, uint16_t dstPort)
{
  NS_LOG_FUNCTION (this << " " << dstLtpEngineId << " " << dstAddr << " " << dstPort );
  return RemoveBinding (dstLtpEngineId,dstAddr);
}

bool LtpIpResolutionTable::RemoveBinding (uint64_t dstLtpEngineId, Ipv6Address dstAddr)
{
  NS_LOG_FUNCTION (this << " " << dstLtpEngineId << " " << dstAddr);
  bool ret = (m_ltpToIpv6Addr.erase (dstLtpEngineId) == 1) ? true : false;
  return ret;
}

bool LtpIpResolutionTable::RemoveBinding (uint64_t dstLtpEngineId, Ipv6Address dstAddr, uint16_t dstPort)
{
  NS_LOG_FUNCTION (this << " " << dstLtpEngineId << " " << dstAddr << " " << dstPort );
  return RemoveBinding (dstLtpEngineId,dstAddr);
}

InetSocketAddress LtpIpResolutionTable::GetIpv4Route (uint64_t dstLtpEngineId)
{
  NS_LOG_FUNCTION (this << " " << dstLtpEngineId);

  std::map <uint64_t, InetSocketAddress>::iterator it = m_ltpToIpv4Addr.end ();
  it = m_ltpToIpv4Addr.find (dstLtpEngineId);
  if (it == m_ltpToIpv4Addr.end ())
    {
      InetSocketAddress address ("127.0.0.1", 0);
      return address;
    }
  else
    {
      return it->second;
    }
}

Inet6SocketAddress LtpIpResolutionTable::GetIpv6Route (uint64_t dstLtpEngineId)
{
  NS_LOG_FUNCTION (this << " " << dstLtpEngineId);
  std::map <uint64_t, Inet6SocketAddress>::iterator it = m_ltpToIpv6Addr.end ();
  it = m_ltpToIpv6Addr.find (dstLtpEngineId);
  if (it == m_ltpToIpv6Addr.end ())
    {
      Inet6SocketAddress address ("::1", 0);
      return address;
    }
  else
    {
      return it->second;
    }
}

void LtpIpResolutionTable::PrintIpv4Bindings (Ptr<OutputStreamWrapper> stream) const
{
  NS_LOG_FUNCTION (this << stream);
  std::ostream* os = stream->GetStream ();
  if ((m_ltpToIpv4Addr.size () > 0))
    {
      *os << "LtpEngineId  |   Ipv4   -   Port   " << std::endl;
      std::map <uint64_t, InetSocketAddress>::const_iterator it = m_ltpToIpv4Addr.begin ();

      while (it != m_ltpToIpv4Addr.end ())
        {
          *os << it->first << " " << it->second << std::endl;
          it++;
        }
    }
}


void LtpIpResolutionTable::PrintIpv6Bindings (Ptr<OutputStreamWrapper> stream) const
{
  NS_LOG_FUNCTION (this << stream);
  std::ostream* os = stream->GetStream ();
  if ((m_ltpToIpv6Addr.size () > 0))
    {
      *os << "LtpEngineId  |   Ipv6   -   Port   " << std::endl;
      std::map <uint64_t, Inet6SocketAddress>::const_iterator it = m_ltpToIpv6Addr.begin ();

      while (it != m_ltpToIpv6Addr.end ())
        {
          *os << it->first << " " << it->second << std::endl;
          it++;
        }
    }
}


Address LtpIpResolutionTable::GetRoute (uint64_t LtpEngineId)
{
  NS_LOG_FUNCTION (this << LtpEngineId);

  if (m_addressMode == Ipv4)
    {
      return GetIpv4Route (LtpEngineId);
    }
  else
    {
      return GetIpv6Route (LtpEngineId);
    }
}

LtpIpResolutionTable::AddressMode LtpIpResolutionTable::GetAddressMode () const
{
  NS_LOG_FUNCTION (this);
  return m_addressMode;
}

//} //namespace ltp
} //namespace ns3
