/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
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
 * Author:  Nico Saputro, Florida International University
 */

#include "ns3/log.h"
#include "ns3/core-module.h"  // activate booleanvalue, makebooleanaccessor etc
#include "ns3/hwmp-tcp-interface.h"


NS_LOG_COMPONENT_DEFINE ("HwmpTcpInterface");
namespace ns3 
{
NS_OBJECT_ENSURE_REGISTERED (HwmpTcpInterface);

TypeId 
HwmpTcpInterface::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::HwmpTcpInterface")
    .SetParent<Object> ()
    .AddConstructor<HwmpTcpInterface> ()
   
  ;
  return tid;
}
HwmpTcpInterface::HwmpTcpInterface()
{
//     NS_LOG_FUNCTION (this);
  m_total=0;
}

HwmpTcpInterface::~HwmpTcpInterface()
{
  m_timePerrIssued.clear();
  //   NS_LOG_FUNCTION (this);
}
void
HwmpTcpInterface::ReceivedPerrInfoFromHwmp(uint32_t count, Time ts)
{
   m_timePerrIssued.push_back(ts);  
   NS_LOG_INFO("Recording Path Error Information " << ts << " " << m_timePerrIssued.size() << " " << count );
}
// if found, we do also the deletion of the old history to save make a faster search
bool
HwmpTcpInterface::CheckHwmpForPerrInfo(Time beginTs, Time endTs)
{
  bool lfoundPathErrorEvent = false;
  NS_LOG_INFO("Checking between " << beginTs << " and " << endTs << " " << m_timePerrIssued.size() );
  for (std::vector<Time>::iterator it = m_timePerrIssued.begin() ; it != m_timePerrIssued.end (); ++it)
  {
      if ( (*it >= beginTs) && (*it <= endTs) ) {       
          m_total++;
          lfoundPathErrorEvent = true;
          NS_LOG_INFO("Found Path Error, no doubling RTO " << *it << " " << m_total);
          break;
      } 
  }
  if (!lfoundPathErrorEvent) 
      m_timePerrIssued.clear();
  return lfoundPathErrorEvent;
}
uint32_t
HwmpTcpInterface::GetReportRto()
{
   return m_total;
}
}

