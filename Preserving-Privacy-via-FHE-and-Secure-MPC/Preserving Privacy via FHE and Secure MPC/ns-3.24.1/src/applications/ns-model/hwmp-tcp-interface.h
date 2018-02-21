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

#ifndef HWMP_TCP_INTERFACE_H
#define HWMP_TCP_INTERFACE_H

#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"

#include <string>
#include <iostream>
#include <sstream>

namespace ns3 {

   class HwmpTcpInterface : public Object
   {
     public:
        static TypeId GetTypeId (void);

  	 HwmpTcpInterface();
  	 virtual ~HwmpTcpInterface();

  	 void ReceivedPerrInfoFromHwmp(uint32_t count, Time ts);
        bool CheckHwmpForPerrInfo(Time beginTs, Time endTs);
        uint32_t GetReportRto();

 	
	private:
 	   std::vector<Time> m_timePerrIssued;
          uint32_t m_total;
   };

} // namespace


#endif	/* HWMP_TCP_INTERFACE_H */