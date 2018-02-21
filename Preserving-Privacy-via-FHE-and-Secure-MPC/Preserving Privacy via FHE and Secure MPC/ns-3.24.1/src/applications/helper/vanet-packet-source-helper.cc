/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
#include "vanet-packet-source-helper.h"
#include "ns3/vanet-packet-source.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

VanetPacketSourceHelper::VanetPacketSourceHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (VanetPacketSource::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

VanetPacketSourceHelper::VanetPacketSourceHelper (Ipv4Address address, uint16_t port)
{
  m_factory.SetTypeId (VanetPacketSource::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (Address(address)));
  SetAttribute ("RemotePort", UintegerValue (port));
}

VanetPacketSourceHelper::VanetPacketSourceHelper (Ipv6Address address, uint16_t port)
{
  m_factory.SetTypeId (VanetPacketSource::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (Address(address)));
  SetAttribute ("RemotePort", UintegerValue (port));
}

void 
VanetPacketSourceHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

void
VanetPacketSourceHelper::SetFill (Ptr<Application> app, std::string fill)
{
  app->GetObject<VanetPacketSource>()->SetFill (fill);
}

void
VanetPacketSourceHelper::SetFill (Ptr<Application> app, uint8_t fill, uint32_t dataLength)
{
  app->GetObject<VanetPacketSource>()->SetFill (fill, dataLength);
}

void
VanetPacketSourceHelper::SetFill (Ptr<Application> app, uint8_t *fill, uint32_t fillLength, uint32_t dataLength)
{
  app->GetObject<VanetPacketSource>()->SetFill (fill, fillLength, dataLength);
}

ApplicationContainer
VanetPacketSourceHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
VanetPacketSourceHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
VanetPacketSourceHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
VanetPacketSourceHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<VanetPacketSource> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3

