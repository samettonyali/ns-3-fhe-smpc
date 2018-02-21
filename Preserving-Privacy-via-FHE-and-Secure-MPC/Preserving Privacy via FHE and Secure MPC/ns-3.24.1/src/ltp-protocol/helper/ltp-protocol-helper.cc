/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Universitat Autnoma de Barcelona
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
 * Author: Rubn Martnez <rmartinez@deic.uab.cat>
 */

#include "ltp-protocol-helper.h"
#include "ns3/object.h"
#include "ns3/names.h"
#include "ns3/node.h"
#include "ns3/node-list.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4.h"
#include "ns3/ipv6.h"
#include "ns3/enum.h"
#include <ns3/log.h>

NS_LOG_COMPONENT_DEFINE ("LtpHelper");


namespace ns3 {
//namespace ltp {

LtpProtocolHelper::LtpProtocolHelper ()
  : m_startTime (Seconds (0)),
    m_ltpid (0),
    m_resolutionTable (0)
{
  NS_LOG_FUNCTION (this);
  m_claFactory.SetTypeId (LtpUdpConvergenceLayerAdapter::GetTypeId ());   // Default UDP
  m_ltpFactory.SetTypeId (LtpProtocol::GetTypeId ());
}

void
LtpProtocolHelper::SetAttributes (std::string n1, const AttributeValue &v1,
                                  std::string n2, const AttributeValue &v2,
                                  std::string n3, const AttributeValue &v3,
                                  std::string n4, const AttributeValue &v4,
                                  std::string n5, const AttributeValue &v5,
                                  std::string n6, const AttributeValue &v6,
                                  std::string n7, const AttributeValue &v7,
                                  std::string n8, const AttributeValue &v8,
                                  std::string n9, const AttributeValue &v9)
{

  m_ltpFactory.Set (n1, v1);
  m_ltpFactory.Set (n2, v2);
  m_ltpFactory.Set (n3, v3);
  m_ltpFactory.Set (n4, v4);
  m_ltpFactory.Set (n5, v5);
  m_ltpFactory.Set (n6, v6);
  m_ltpFactory.Set (n7, v7);
  m_ltpFactory.Set (n8, v8);
  m_ltpFactory.Set (n9, v9);
}


void
LtpProtocolHelper::SetConvergenceLayerAdapter (std::string type,
                                               std::string n1, const AttributeValue &v1,
                                               std::string n2, const AttributeValue &v2,
                                               std::string n3, const AttributeValue &v3,
                                               std::string n4, const AttributeValue &v4,
                                               std::string n5, const AttributeValue &v5,
                                               std::string n6, const AttributeValue &v6,
                                               std::string n7, const AttributeValue &v7,
                                               std::string n8, const AttributeValue &v8,
                                               std::string n9, const AttributeValue &v9)
{
  m_claFactory.SetTypeId (type);
  m_claFactory.Set (n1, v1);
  m_claFactory.Set (n2, v2);
  m_claFactory.Set (n3, v3);
  m_claFactory.Set (n4, v4);
  m_claFactory.Set (n5, v5);
  m_claFactory.Set (n6, v6);
  m_claFactory.Set (n7, v7);
  m_claFactory.Set (n8, v8);
  m_claFactory.Set (n9, v9);
}


void
LtpProtocolHelper::InstallAndLink (NodeContainer c)
{
  NS_LOG_FUNCTION (this);

  uint64_t base_addr = m_ltpid;
  uint32_t numNode = 0;

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      if (++numNode < c.GetN ())
        {
          m_claFactory.Set ("RemotePeer", UintegerValue (m_ltpid + 1));
        }
      else
        {
          m_claFactory.Set ("RemotePeer", UintegerValue (base_addr));
        }

      InstallAndLink (*i);
    }

}
void
LtpProtocolHelper::InstallAndLink (Ptr<Node> n)
{
  NS_LOG_FUNCTION (this);
  Ptr<LtpConvergenceLayerAdapter> link = m_claFactory.Create ()->GetObject<LtpConvergenceLayerAdapter> ();
  if (link == 0)
    {
      NS_FATAL_ERROR ("The requested cla does not exist: \"" <<
                      m_claFactory.GetTypeId ().GetName () << "\"");
    }

  Install (n);
  Ptr<LtpProtocol> ltpProtocol = n->GetObject<LtpProtocol> ();

  link->SetProtocol (ltpProtocol);
  ltpProtocol->AddConvergenceLayerAdapter (link);
  ltpProtocol->SetIpResolutionTable (m_resolutionTable);
  link->EnableReceive (ltpProtocol->GetLocalEngineId ());
  //link->EnableSend ();

  ltpProtocol->EnableLinkStateCues (link);

  EnumValue mode = 0;
  m_resolutionTable->GetAttribute ("Addressing",mode);

  UintegerValue port = 0;
  link->GetAttribute ("ServerPort",port);

  if (mode.Get () == LtpIpResolutionTable::Ipv4)
    {
      Ptr<Ipv4> ipv4 = n->GetObject<Ipv4> ();
      m_resolutionTable->AddBinding (/*m_ltpid*/n->GetId(),ipv4->GetAddress (1,0).GetLocal (),port.Get ());
    }
  else
    {
      Ptr<Ipv6> ipv6 = n->GetObject<Ipv6> ();
      m_resolutionTable->AddBinding (/*m_ltpid*/n->GetId(),ipv6->GetAddress (1,0).GetAddress (),port.Get ());
    }
  NS_LOG_INFO("LTP ID: " << m_ltpid);
  m_ltpid++;
  link->SetLinkUpCallback ( MakeCallback (&LtpProtocol::Send,ltpProtocol));

  Simulator::Schedule (m_startTime, &LtpUdpConvergenceLayerAdapter::SetLinkUp, link);

}


void
LtpProtocolHelper::InstallAndLink (Ptr<Node> n, uint64_t remotePeer, bool isSensor)
{
  NS_LOG_FUNCTION (this);
  m_claFactory.Set ("RemotePeer", UintegerValue (remotePeer));
  Ptr<LtpConvergenceLayerAdapter> link = m_claFactory.Create ()->GetObject<LtpConvergenceLayerAdapter> ();
  if (link == 0)
    {
      NS_FATAL_ERROR ("The requested cla does not exist: \"" <<
                      m_claFactory.GetTypeId ().GetName () << "\"");
    }

  Install (n);
  Ptr<LtpProtocol> ltpProtocol = n->GetObject<LtpProtocol> ();

  link->SetProtocol (ltpProtocol);
  ltpProtocol->AddConvergenceLayerAdapter (link);
  ltpProtocol->SetIpResolutionTable (m_resolutionTable);
  if(!isSensor)
    link->EnableReceive (ltpProtocol->GetLocalEngineId ());
  //link->EnableSend ();

  ltpProtocol->EnableLinkStateCues (link);

  EnumValue mode = 0;
  m_resolutionTable->GetAttribute ("Addressing",mode);

  UintegerValue port = 0;
  link->GetAttribute ("ServerPort",port);

  if (mode.Get () == LtpIpResolutionTable::Ipv4)
    {
      Ptr<Ipv4> ipv4 = n->GetObject<Ipv4> ();
      m_resolutionTable->AddBinding (/*remotePeer,m_ltpid*/n->GetId(),ipv4->GetAddress (1,0).GetLocal (),port.Get ());
    }
  else
    {
      Ptr<Ipv6> ipv6 = n->GetObject<Ipv6> ();
      m_resolutionTable->AddBinding (/*remotePeer,m_ltpid*/n->GetId(),ipv6->GetAddress (1,0).GetAddress (),port.Get ());
    }

  NS_LOG_INFO("LTP ID: " << m_ltpid);
  link->SetLinkUpCallback ( MakeCallback (&LtpProtocol::Send,ltpProtocol));

  Simulator::Schedule (m_startTime, &LtpUdpConvergenceLayerAdapter::SetLinkUp, link);

}

void
LtpProtocolHelper::Install (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);
  if (node->GetObject<LtpProtocol> () != 0)
    {
      NS_FATAL_ERROR ("LtpProtocolHelper::Install (): Aggregating "
                      "an LtpProtocol to a node with an existing Ltp object");
      return;
    }

  Ptr<LtpProtocol> ltpProtocol =  m_ltpFactory.Create ()->GetObject<LtpProtocol> ();
  ltpProtocol->SetAttribute ("LocalEngineId", UintegerValue (m_ltpid));
  ltpProtocol->SetNode (node);


  if (m_resolutionTable == 0)
    {
      NS_FATAL_ERROR ("LtpProtocolHelper::Install (): "
                      "Routing protocol is not set");
    }

  node->AggregateObject (ltpProtocol);

}
void
LtpProtocolHelper::Install (NodeContainer c)
{
  NS_LOG_FUNCTION (this);
  uint64_t base_addr = m_ltpid;
  uint32_t numNode = 0;

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      if (++numNode < c.GetN ())
        {
          m_claFactory.Set ("RemotePeer", UintegerValue (m_ltpid + 1));
        }
      else
        {
          m_claFactory.Set ("RemotePeer", UintegerValue (base_addr));
        }

      Install (*i);
    }
}

void
LtpProtocolHelper::Install (std::string nodeName)
{
  NS_LOG_FUNCTION (this);
  Ptr<Node> node = Names::Find<Node> (nodeName);
  Install (node);
}

void
LtpProtocolHelper::SetBaseLtpEngineId (uint64_t id)
{
  NS_LOG_FUNCTION (this << id);
  m_ltpid = id;
}

void
LtpProtocolHelper::SetLtpEngineId (Ptr<Node> node, uint64_t id)
{
  NS_LOG_FUNCTION (this << node << id);
  Ptr<LtpProtocol> ltpProtocol = node->GetObject<LtpProtocol> ();
  ltpProtocol->SetAttribute ("LocalEngineId", UintegerValue (id));
}


void
LtpProtocolHelper::SetLtpIpResolutionTable (Ptr<LtpIpResolutionTable> rprot)
{
  NS_LOG_FUNCTION (this << rprot);
  m_resolutionTable = rprot;
}

bool
LtpProtocolHelper::AddBinding (uint64_t dstLtpEngineId, Ipv4Address dstAddr, uint16_t port)
{
  NS_LOG_FUNCTION (this << dstLtpEngineId << dstAddr << port);
  return m_resolutionTable->AddBinding (dstLtpEngineId,dstAddr,port);
}

bool
LtpProtocolHelper::AddBinding (uint64_t dstLtpEngineId, Ipv4Address dstAddr)
{
  NS_LOG_FUNCTION (this << dstLtpEngineId << dstAddr);
  return AddBinding (dstLtpEngineId,dstAddr,0);
}

void
LtpProtocolHelper::SetStartTransmissionTime (Time start)
{
  NS_LOG_FUNCTION (this << start);
  m_startTime = start;
}

//} //namespace ltp
} //namespace ns3

