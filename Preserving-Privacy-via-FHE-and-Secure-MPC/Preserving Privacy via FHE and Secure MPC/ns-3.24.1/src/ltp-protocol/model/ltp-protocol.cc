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

#include "ltp-protocol.h"
#include "ns3/ltp-header.h"
#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/seq-ts-header.h"

#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>


namespace ns3 {
//namespace ltp {

NS_LOG_COMPONENT_DEFINE ("LtpProtocol");

ClientServiceStatus::ClientServiceStatus ()
  : m_activeSessions (),
    m_reportStatus ()
{
  NS_LOG_FUNCTION (this);
}

ClientServiceStatus::~ClientServiceStatus ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
ClientServiceStatus::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ClientServiceStatus")
    .SetParent<Object> ()
    .AddConstructor<ClientServiceStatus> ()
    .AddTraceSource ("SessionStatus",
                     "Trace used to report changes in session status",
                     MakeTraceSourceAccessor (&ClientServiceStatus::m_reportStatus),
                    "ns3::ClientServiceStatus::SessionStatusTracedCallback")

  ;
  return tid;
}


void ClientServiceStatus::ReportStatus (uint32_t seqNum,
                                        StatusNotificationCode code,
                                        std::vector<uint8_t> data = std::vector<uint8_t> (),
                                        uint32_t dataLength = 0,
                                        bool endFlag = false,
                                        uint64_t srcLtpEngine = 0,
                                        uint32_t offset = 0,
                                        Time timeStamp = Simulator::Now()
                                        )
{
  NS_LOG_FUNCTION (this);
  m_reportStatus (seqNum,code,data,dataLength,endFlag,srcLtpEngine,offset,timeStamp);
}

void ClientServiceStatus::AddSession (SessionId id)
{
  NS_LOG_FUNCTION (this);
  m_activeSessions.insert (m_activeSessions.begin (), id);
}

void ClientServiceStatus::ClearSessions ()
{
  NS_LOG_FUNCTION (this);
  m_activeSessions.clear ();
}

SessionId ClientServiceStatus::GetSession (uint32_t index)
{
  NS_LOG_FUNCTION (this);
  return m_activeSessions.at (index);
}

uint32_t ClientServiceStatus::GetNSessions () const
{
  NS_LOG_FUNCTION (this);
  return m_activeSessions.size ();
}

LtpProtocol::LtpProtocol ()
  : m_activeSessions (),
    m_activeClients (),
    m_clas (),
    m_localEngineId (0),
    m_cpRtxLimit (0),
    m_rpRtxLimit (0),
    m_rxProblemLimit (0),
    m_cxRtxLimit (0),
    m_rtxCycleLimit (0),
    m_version (0),
    m_localDelays (Seconds (1.0)),
    m_onewayLightTime (Seconds (0)),
    m_inactivityLimit (Seconds (0)),
    m_localOperatingSchedule ()
{
  NS_LOG_FUNCTION (this);
  // m_randomSession = CreateObject<UniformRandomVariable> ();
}

LtpProtocol::~LtpProtocol ()
{
  NS_LOG_FUNCTION (this);

}

TypeId
LtpProtocol::GetTypeId (void)
{
  std::stringstream session;
  session << "ns3::UniformRandomVariable" <<
  "[Min="  << SessionId::MIN_SESSION_NUMBER  <<
  "|Max="  << SessionId::MAX_SESSION_NUMBER - 1 << "]";

  std::stringstream serial;
  serial << "ns3::UniformRandomVariable" <<
  "[Min="  << SessionStateRecord::MIN_INITIAL_SERIAL_NUMBER  <<
  "|Max="  << SessionStateRecord::MAX_INITIAL_SERIAL_NUMBER - 1 << "]";

  static TypeId tid = TypeId ("ns3::LtpProtocol")
    .SetParent<Object> ()
    .AddConstructor<LtpProtocol> ()
    .AddAttribute ("RandomSessionNum",
                   "The random variable used to generate session numbers.",
                   StringValue (session.str ()),
                   MakePointerAccessor (&LtpProtocol::m_randomSession),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("RandomSerialNum",
                   "The random variable used to generate serial numbers.",
                   StringValue (serial.str ()),
                   MakePointerAccessor (&LtpProtocol::m_randomSerial),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("LocalEngineId", "Identification of the local LTP engine",
                   UintegerValue (0),
                   MakeUintegerAccessor (&LtpProtocol::m_localEngineId),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Mode", "Type of operation [0=forwarding], 1=aggregation", 
                   UintegerValue (0),
                   MakeUintegerAccessor (&LtpProtocol::m_operation_type),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("CheckPointRtxLimit", "Maximum number of checkpoints retransmissions allowed",
                   UintegerValue (20),
                   MakeUintegerAccessor (&LtpProtocol::m_cpRtxLimit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("ReportSegmentRtxLimit", "Maximum number of report segment retransmissions allowed",
                   UintegerValue (20),
                   MakeUintegerAccessor (&LtpProtocol::m_rpRtxLimit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("ReceptionProblemLimit", "Maximum number of reception failures allowed",
                   UintegerValue (20),
                   MakeUintegerAccessor (&LtpProtocol::m_rxProblemLimit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CancelationRtxLimit", "Maximum number of cancellation request retransmissions allowed",
                   UintegerValue (20),
                   MakeUintegerAccessor (&LtpProtocol::m_cxRtxLimit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("RetransCyclelimit", "Maximum number of cancellation cycle retransmissions allowed",
                   UintegerValue (20),
                   MakeUintegerAccessor (&LtpProtocol::m_rtxCycleLimit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("SegmentSize", "Maximum size of a packet to be received",
                   UintegerValue (1500),
                   MakeUintegerAccessor (&LtpProtocol::m_sSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("LocalProcessingDelays", "Queue Processing Times (for use in timers)",
                   TimeValue (Seconds (0.3)),
                   MakeTimeAccessor (&LtpProtocol::m_localDelays),
                   MakeTimeChecker ())
    .AddAttribute ("OneWayLightTime", "Time to reach destination (for use in timers)",
                   TimeValue (Seconds (10.0)),
                   MakeTimeAccessor (&LtpProtocol::m_onewayLightTime),
                   MakeTimeChecker ())
    .AddAttribute ("SessionInactivityLimit", "Time to maintain an inactive session",
                   TimeValue (Seconds (2000.0)),
                   MakeTimeAccessor (&LtpProtocol::m_inactivityLimit),
                   MakeTimeChecker ())
  ;
  return tid;
}

TypeId
LtpProtocol::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}


uint32_t LtpProtocol::GetCheckPointRetransLimit () const
{
  NS_LOG_FUNCTION (this);
  return m_cpRtxLimit;
}
uint32_t LtpProtocol::GetReportRetransLimit () const
{
  NS_LOG_FUNCTION (this);
  return m_rpRtxLimit;
}

uint32_t LtpProtocol::GetReceptionProblemLimit () const
{
  NS_LOG_FUNCTION (this);
  return m_rxProblemLimit;
}

uint32_t LtpProtocol::GetCancellationRetransLimit () const
{
  NS_LOG_FUNCTION (this);
  return m_cxRtxLimit;
}

uint32_t LtpProtocol::GetRetransCycleLimit () const
{
  NS_LOG_FUNCTION (this);
  return m_rtxCycleLimit;
}

bool LtpProtocol::RegisterClientService (uint64_t id,  const CallbackBase & cb )
{
  NS_LOG_FUNCTION (this << id);
  std::pair<uint64_t,Ptr<ClientServiceStatus> > entry;
  
  Ptr<ClientServiceStatus> notifications =  CreateObject<ClientServiceStatus> ();
  notifications->TraceConnectWithoutContext ("SessionStatus", cb);
  entry = std::make_pair (id, notifications);

  std::pair<ClientServiceInstances::iterator,bool> ret;
  ret =  m_activeClients.insert (entry);
  
  return ret.second;
}

void LtpProtocol::UnregisterClientService (uint64_t id)
{
  NS_LOG_FUNCTION (this << id);
  ClientServiceInstances::iterator it = m_activeClients.find (id);

  for (uint32_t i = 0; i < it->second->GetNSessions (); ++i)
    {
      SessionId id = it->second->GetSession (i);
      CancelSession (id);
    }

  it->second->ClearSessions ();
  m_activeClients.erase (it);
}

uint32_t LtpProtocol::StartTransmission ( uint64_t sourceId, 
                                          uint64_t dstClientService,
                                          uint64_t dstLtpEngine, 
                                          std::vector<uint8_t> data, 
                                          uint16_t fragmentSize,
                                          uint8_t operationType, uint32_t seqnum)
{

  NS_LOG_FUNCTION (this << dstClientService << dstLtpEngine << data.size() << seqnum );


  Ptr<SenderSessionStateRecord> ssr = CreateObject<SenderSessionStateRecord> (m_localEngineId, sourceId, dstClientService, dstLtpEngine,m_randomSession,m_randomSerial);
  ssr->SetInactiveSessionCallback (MakeCallback (&LtpProtocol::CloseSession, this),
                                 m_inactivityLimit);

  SessionId id = ssr->GetSessionId ();

  //NS_LOG_DEBUG ("New Session id " << id);

  /* Report Session Start to Client Service Instance */
  ClientServiceInstances::iterator it;
  it = m_activeClients.find (sourceId);
  it->second->ReportStatus (uint32_t(-1),SESSION_START);
  it->second->AddSession (id);

  /* Keep Track of new session */
  std::pair<SessionId,Ptr<SessionStateRecord> > entry;
  entry = std::make_pair (id, DynamicCast< SessionStateRecord > (ssr) );
  m_activeSessions.insert (m_activeSessions.begin (),entry);

  Ptr<LtpConvergenceLayerAdapter> link = GetConvergenceLayerAdapter (dstLtpEngine);
  //NS_LOG_INFO("Destination LTP Engine: " << dstLtpEngine);
  NS_ASSERT_MSG (link != 0, "No available link for destination LTP engine");
  link->SetSessionId (id);
  EncapsulateBlockData (dstClientService, ssr, data, fragmentSize, 0, 0, 0, seqnum);

  m_dstEngineId = dstLtpEngine;
  
  ConvergenceLayerAdapters::iterator itCla = m_clas.find (dstLtpEngine);
  Ptr<LtpConvergenceLayerAdapter> cla = itCla->second;
  
  //NS_LOG_INFO("Before Scheduling...");

  if (cla->IsLinkUp ())
    {
      Simulator::Schedule (m_localDelays, &LtpProtocol::Send, this, cla);
      //NS_LOG_INFO("After Scheduling...");
    }

  return ssr->GetNPackets ();

}

void LtpProtocol::CancelSession (SessionId id)
{
  NS_LOG_FUNCTION (this << id);
  SessionStateRecords::iterator it = m_activeSessions.find (id);

  if (it == m_activeSessions.end ())
     {
       NS_FATAL_ERROR ("Can't Cancel session no active session found");
     }


  it->second->Cancel (LOCAL_CANCEL,USR_CNCLD);

  ClientServiceInstances::iterator itCls = m_activeClients.find (it->second->GetLocalClientServiceId ());
  itCls->second->ReportStatus (uint32_t(-2),RX_SESSION_CANCEL);

  m_activeSessions.erase (it);
}

void LtpProtocol::SignifyRedPartReception (SessionId id)
{
  NS_LOG_FUNCTION (this << id);
  SessionStateRecords::iterator it = m_activeSessions.find (id);

  if (it != m_activeSessions.end ())
    {

      ClientServiceInstances::iterator itCls = m_activeClients.find (it->second->GetLocalClientServiceId ());

      std::vector<uint8_t> blockData;
      bool EOB = false;
      uint64_t remoteLtp = it->second->GetPeerLtpEngineId ();

      if (it->second->GetInstanceTypeId () == ReceiverSessionStateRecord::GetTypeId ())
        {
          Ptr<ReceiverSessionStateRecord> ssr = DynamicCast<ReceiverSessionStateRecord> (it->second);

          Ptr<Packet> p = 0;
          LtpHeader header;
          LtpContentHeader contentHeader;

          while ((p = ssr->RemoveRedDataSegment ()))
            {
              p->RemoveHeader (header);
              contentHeader.SetSegmentType (header.GetSegmentType ());
              p->RemoveHeader (contentHeader);

              uint32_t size = p->GetSize ();
              uint8_t *raw_data = new uint8_t[size];
              p->CopyData (raw_data, size);

              std::vector<uint8_t> packetData ( raw_data, raw_data + size);

              blockData.insert ( blockData.end (), packetData.begin (), packetData.end () );
              delete raw_data;
            }

//          if (header.GetSegmentType () == LTPTYPE_RD_CP_EORP_EOB)
//            {
//              EOB = true;
//            }
        }

      itCls->second->ReportStatus (uint32_t(-3),RED_PART_RCV, blockData, blockData.size (), EOB,  remoteLtp);
    }

}

void LtpProtocol::SignifyGreenPartSegmentArrival (SessionId id)
{
  NS_LOG_FUNCTION (this << id);

  SessionStateRecords::iterator it = m_activeSessions.find (id);

  if (it == m_activeSessions.end ())
     {
       NS_FATAL_ERROR ("Active session not found");
     }

  ClientServiceInstances::iterator itCls = m_activeClients.find (it->second->GetLocalClientServiceId ());

  if (it->second->GetInstanceTypeId () == ReceiverSessionStateRecord::GetTypeId ())
    {
      Ptr<ReceiverSessionStateRecord> ssr = DynamicCast<ReceiverSessionStateRecord> (it->second);

      std::vector<uint8_t> packetData;
      Ptr<Packet> p = 0;
      bool EOB = false;
      uint32_t offset = 0;
      uint64_t remoteLtp = it->second->GetPeerLtpEngineId ();

      if ((p = ssr->RemoveGreenDataSegment ()))
        {
          LtpHeader header;
          LtpContentHeader contentHeader;

          p->RemoveHeader (header);
          contentHeader.SetSegmentType (header.GetSegmentType ());
          p->RemoveHeader (contentHeader);

          offset = contentHeader.GetOffset ();

          // No better way to determine if this is a full Green block on the receiver side
          if (offset == 0)
            {
              ssr->SetFullGreen ();
            }

//          if (header.GetSegmentType () == LTPTYPE_GD_EOB)
//            {
//              EOB = true;
//            }

          uint32_t size = p->GetSize ();
          uint8_t *raw_data = new uint8_t[size];
          p->CopyData (raw_data, size);

          packetData.insert (packetData.end (), raw_data, raw_data + size);
          delete raw_data;

        }

      itCls->second->ReportStatus (uint32_t(-4),GP_SEGMENT_RCV, packetData, packetData.size (), EOB,  remoteLtp, offset);

    }

}


void LtpProtocol::CloseSession (SessionId id)
{
  NS_LOG_FUNCTION (this << id);

  SessionStateRecords::iterator it = m_activeSessions.find (id);

  if (it == m_activeSessions.end ())
     {
       NS_FATAL_ERROR ("Active session not found");
     }


  if (it != m_activeSessions.end ())
    {
      Ptr<SessionStateRecord> ssr = it->second;
      ssr->SetBlockFinished ();

      if (it->second->GetNPackets ())
        {
          // Buffer is not empty , schedule again to allow the transmission of buffered segments and exit
          //NS_LOG_DEBUG ("Close session" << id << " buffer is not empty");
          Simulator::Schedule (m_localDelays, &LtpProtocol::CloseSession, this, id);
          return;
        }

      ssr->CancelTimer (CHECKPOINT);
      ssr->CancelTimer (REPORT);
      ssr->Close ();

      ConvergenceLayerAdapters::iterator itCla = m_clas.find (it->second->GetPeerLtpEngineId ());

      Ptr<LtpConvergenceLayerAdapter> cla = itCla->second;
      //NS_LOG_DEBUG ("Active CLA session " << cla->GetSessionId () << " closing session " << id);

      /* This should be done later, send a CX request first*/
      //m_activeSessions.erase (it); -- Do not remove for now.

      ClientServiceInstances::iterator itCls = m_activeClients.find (ssr->GetLocalClientServiceId ());
      itCls->second->ReportStatus (uint32_t(-5), SESSION_END);

    }

}

void LtpProtocol::SetCheckPointTransmissionTimer (SessionId id, RedSegmentInfo info)
{
  NS_LOG_FUNCTION (this << id << info.CpserialNum << info.low_bound << info.high_bound << info.claims.size ());

  SessionStateRecords::iterator it = m_activeSessions.find (id);

  Ptr<SessionStateRecord> ssr = 0;

  if (it != m_activeSessions.end ())
    {
      // Round trip = time to reach peer * 2 ( one way and back ) + processing times at both sides + margin
      double rtt = m_onewayLightTime.GetSeconds () * 2 + m_localDelays.GetSeconds () * 2 + 1.0;

      ssr = it->second;
      ssr->SetTimerFunction (&LtpProtocol::RetransmitSegment, this, id, info, Seconds (rtt), CHECKPOINT);
      ssr->StartTimer (CHECKPOINT);
    }
}

void LtpProtocol::SetReportReTransmissionTimer (SessionId id, RedSegmentInfo info)
{
  NS_LOG_FUNCTION (this << id);

  SessionStateRecords::iterator it = m_activeSessions.find (id);
  Ptr<SessionStateRecord> ssr = 0;

  if (it != m_activeSessions.end ())
    {
      // Round trip = time to reach peer * 2 ( one way and back ) + processing times at both sides + margin
      double rtt = m_onewayLightTime.GetSeconds () * 2 + m_localDelays.GetSeconds () * 2 + 1.0;

      ssr = it->second;
      ssr->SetTimerFunction (&LtpProtocol::RetransmitReport, this, id, info, Seconds (rtt), REPORT);
      ssr->StartTimer (REPORT);
    }
}


/* Should be called on signal from the LinkStateCue: linkUp */
void LtpProtocol::Send (Ptr<LtpConvergenceLayerAdapter> cla)
{
  NS_LOG_FUNCTION (this);
  //NS_LOG_INFO("Node: " << cla->GetProtocol()->m_node->GetId());
  SessionId id =  cla->GetSessionId ();
  SessionStateRecords::iterator it = m_activeSessions.find (id);

  // Dequeue all segments from SSR
  if (it != m_activeSessions.end ())
    {
      //NS_LOG_INFO("Node " << cla->GetProtocol()->m_node->GetId() << " has an active session!");
      //NS_LOG_INFO("There are " << it->second->GetNPackets() << " packets in the queue!");
      Ptr<Packet> packet = it->second->Dequeue ();

      if (packet)
        {
          cla->Send (packet);
          Simulator::Schedule (m_localDelays, &LtpProtocol::Send, this, cla);
        }
    }
}

void LtpProtocol::DirectSend (Ptr<Packet> packet, Ptr<LtpConvergenceLayerAdapter> cla)
{
  NS_LOG_FUNCTION (this);
  //NS_LOG_INFO("Node: " << cla->GetProtocol()->m_node->GetId());
  
  SessionId id =  cla->GetSessionId ();
  //NS_LOG_INFO("Girdi1");
  SessionStateRecords::iterator it = m_activeSessions.find (id);
  //NS_LOG_INFO("Girdi2");

  // Dequeue all segments from SSR
  if (it != m_activeSessions.end ())
    {
    //  NS_LOG_INFO("Girdi3");
      if (packet)
        {
      //    NS_LOG_INFO("Girdi4");
          cla->Send (packet);
        }
    }
  //NS_LOG_INFO("Girdi5");
}

bool LtpProtocol::IsThereAnyPreviousStoredPacket(Address from) {
    bool lfoundPacket = false;
    for(std::vector<DataWaitingPacket>::iterator it = m_waitingPacket.begin(); it != m_waitingPacket.end(); ++it) {
       if(it->from == from) {
          lfoundPacket=true;
          break;
       }
    }     
    return lfoundPacket;
}

/* Should be called from lower layer */
void LtpProtocol::Receive (Ptr<Packet> packet, Ptr<LtpConvergenceLayerAdapter> cla, 
        Address from)
{
  NS_LOG_FUNCTION (this);
  
  LtpHeader header;
  SeqTsHeader seqTs;

  Ptr<Packet> p = packet->Copy ();
  
  NS_LOG_DEBUG(p->GetSize() << "-byte packet was received from " << 
          InetSocketAddress::ConvertFrom (from).GetIpv4 ());
  
    uint8_t *packetBuffer, *fragmentBuffer, *remnantBuffer;

    RemnantByteMap::iterator remnantBytesEntry;
    remnantBytesEntry = m_remnantBytesMap.find(from);
  
    if(remnantBytesEntry != m_remnantBytesMap.end()){
        std::vector< uint8_t > remnantBytes = remnantBytesEntry->second;

        NS_LOG_DEBUG("There are remnant bytes!!!");
        //unify the packet buffer with the remnant bytes then clear remnant bytes
        remnantBuffer = new uint8_t[remnantBytes.size()];
        for(uint32_t i=0; i<remnantBytes.size(); i++)
            remnantBuffer[i] = remnantBytes[i];

        Ptr<Packet> remnantPacket = NULL;
        remnantPacket = new Packet(remnantBuffer, remnantBytes.size());

        remnantPacket->AddAtEnd (p);

        p = remnantPacket;
        
        if(p->GetSize() < 18)
            return;
    }
    else{
        if(p->GetSize() < 18){
            remnantBuffer = new uint8_t[p->GetSize()];
            p->CopyData(remnantBuffer, p->GetSize());
            
            std::vector< uint8_t > remnantBytes;
            
            for(uint32_t i=0; i<p->GetSize(); i++)
                remnantBytes.push_back(remnantBuffer[i]);
            
            m_remnantBytesMap.insert(std::make_pair(from, remnantBytes));
            
            return;
        }
    }
  
    packetBuffer = new uint8_t[p->GetSize()];
    p->CopyData(packetBuffer, p->GetSize());
  
  
  uint32_t totalPacketSize = 0;

  uint32_t sizeOfRest = p->GetSize();
  
  do{
      fragmentBuffer = new uint8_t[18];
      for(uint32_t i=0; i<18; i++){
          fragmentBuffer[i] = packetBuffer[i+totalPacketSize];
      }
      
//      totalPacketSize += 18;
      
      Ptr<Packet> fragmentHeaders = NULL;
      fragmentHeaders = new Packet(fragmentBuffer, 18);
      packet = fragmentHeaders;
      
      uint32_t bytes = packet->RemoveHeader (header);
      NS_ASSERT (bytes == header.GetSerializedSize ());

      bytes = packet->RemoveHeader(seqTs);
      NS_ASSERT (bytes == seqTs.GetSerializedSize ());
      
      uint8_t fType = header.GetFragmentType();
      
      uint32_t seqNum = seqTs.GetSeq();
      
      NS_LOG_INFO ("[node " << GetNode()->GetId() << "]: RX " << p->GetSize() 
                      << " " << InetSocketAddress::ConvertFrom (from).GetIpv4 () 
                      << ":" << InetSocketAddress::ConvertFrom (from).GetPort()
                      << " " << seqNum
                      << " " << packet->GetUid () 
                      << " TXtime: " << seqTs.GetTs () 
                      << " RXtime: " << Simulator::Now() );
      
      //NS_LOG_INFO("We have headers!");
      uint16_t fSize = header.GetFragmentSize();
      NS_LOG_DEBUG("LtpProtocol-Receive: Fragment Owner: " << uint32_t(header.GetSmartMeterID()));
      NS_LOG_DEBUG("LtpProtocol-Receive: Fragment ID: " << uint32_t(header.GetFragmentID()));
      NS_LOG_DEBUG("LtpProtocol-Receive: Fragment Type: " << uint32_t(header.GetFragmentType()));                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
      NS_LOG_DEBUG("LtpProtocol-Receive: Fragment Size: " << uint32_t(fSize));
      NS_LOG_DEBUG("LtpProtocol-Receive: Fragment Sequence Number: " << seqTs.GetSeq());
      NS_LOG_DEBUG("LtpProtocol-Receive: Fragment Timestamp: " << seqTs.GetTs());
      
      if(fType == SINGLE){
          //Forward packet up to Application Layer
          ClientServiceInstances::iterator it;                                                                                                                                                                                                  
          it = m_activeClients.find (GetNode()->GetId());
          std::vector<uint8_t> data = std::vector<uint8_t> (fSize-18);
          
          NS_LOG_INFO("LtpProtocol-Receive: Fragment Type is Single.");
                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
          it->second->ReportStatus (seqNum, GP_SEGMENT_RCV, data, 
                                    uint32_t(header.GetFragmentID()), true, 
                                    uint64_t(header.GetFragmentSize()), 
                                    uint32_t(header.GetSmartMeterID()),
                                    seqTs.GetTs());
          
          sizeOfRest -= fSize;
          totalPacketSize += fSize;
      }
      else{
          NS_LOG_INFO("LtpProtocol-Receive: Fragment Type is Multiple says " << GetNode()->GetId() << ".");
          
          PacketEntries::iterator packetRecords;
          packetRecords = m_packetEntries.find(std::make_pair(seqNum, from));
          
          if(packetRecords == m_packetEntries.end()){   //This is the first segment from this sender in this round
              PacketRecord packRec;
              packRec.seqNum = seqNum;
              packRec.from = from;
              packRec.packetSize = (uint16_t)6420;
              packRec.accFragmentsSize = 0;
              if(header.GetFragmentID() == 0){  // This is the first fragment of the complete packet.
                  NS_LOG_INFO("1 - LtpProtocol-Receive: The first fragment of the " <<
                    "packet received from the meter " << uint32_t(header.GetSmartMeterID()) << 
                    " in sequence " << seqNum << ".");
                  packRec.packetSize = header.GetFragmentSize();
                  packRec.accFragmentsSize += 536;

                  if(packRec.accFragmentsSize == packRec.packetSize){
                      ClientServiceInstances::iterator it;
                      it = m_activeClients.find (GetNode()->GetId());
                      std::vector<uint8_t> data = std::vector<uint8_t> (6204);
                      
                      NS_LOG_INFO("1 - LtpProtocol-Receive: A packet from the meter " <<
                              uint32_t(header.GetSmartMeterID()) << " was received completely in sequence " 
                              << seqNum << ".");
          
                      it->second->ReportStatus (seqNum, GP_SEGMENT_RCV, data, 
                                    uint32_t(header.GetFragmentID()), true, 
                                    uint64_t(header.GetFragmentSize()), 
                                    uint32_t(header.GetSmartMeterID()),
                                    seqTs.GetTs());
                  }
                  
                  if(p->GetSize() < 536)
                      sizeOfRest -= p->GetSize();
                  else
                    sizeOfRest -= 536;
                  
                  totalPacketSize += 536;
              }
              else{
                  packRec.accFragmentsSize += header.GetFragmentSize();
                  
                  NS_LOG_INFO("1 - LtpProtocol-Receive: A packet fragment received from the meter " << 
                          uint32_t(header.GetSmartMeterID()) << " in sequence " << seqNum << ".");
                  
                  if(packRec.accFragmentsSize == packRec.packetSize){
                      ClientServiceInstances::iterator it;
                      it = m_activeClients.find (GetNode()->GetId());
                      std::vector<uint8_t> data = std::vector<uint8_t> (6204);
                      
                      NS_LOG_INFO("1 - LtpProtocol-Receive: A packet from the meter " <<
                              uint32_t(header.GetSmartMeterID()) << " was received completely in sequence " 
                              << seqNum << ".");
          
                      it->second->ReportStatus (seqNum, GP_SEGMENT_RCV, data, 
                                    uint32_t(header.GetFragmentID()), true, 
                                    uint64_t(header.GetFragmentSize()), 
                                    uint32_t(header.GetSmartMeterID()),
                                    seqTs.GetTs());
                  }
                      
                  sizeOfRest -= header.GetFragmentSize();
                  totalPacketSize += header.GetFragmentSize();
              }
              m_packetEntries.insert(std::make_pair(std::make_pair(seqNum, from), packRec));
          }
          else{
              PacketRecord record = packetRecords->second;
              
              if(header.GetFragmentID() == 0){  // This is the first fragment of the complete packet.
                  NS_LOG_INFO("2 - LtpProtocol-Receive: The first fragment of the " <<
                    "packet received from the meter " << uint32_t(header.GetSmartMeterID()) << 
                    " in sequence " << seqNum << ".");
                  record.packetSize = header.GetFragmentSize();
                  record.accFragmentsSize += 536;
                  
                  NS_LOG_INFO("Packet Size: " << record.packetSize << " The Size of Accumulated Fragments: " << record.accFragmentsSize);
                  
                  if(record.accFragmentsSize == record.packetSize){
                    ClientServiceInstances::iterator it;
                    it = m_activeClients.find (GetNode()->GetId());
                    std::vector<uint8_t> data = std::vector<uint8_t> (6204);
                    
                    NS_LOG_INFO("2 - LtpProtocol-Receive: A packet from the meter " <<
                              uint32_t(header.GetSmartMeterID()) << " was received completely in sequence " 
                              << seqNum << ".");

                    it->second->ReportStatus (seqNum, GP_SEGMENT_RCV, data, 
                                  uint32_t(header.GetFragmentID()), true, 
                                  uint64_t(header.GetFragmentSize()), 
                                  uint32_t(header.GetSmartMeterID()),
                                  seqTs.GetTs());
                  }
                  
                  
                  if(p->GetSize() < 536)
                      sizeOfRest -= p->GetSize();
                  else
                    sizeOfRest -= 536;
                  
                  totalPacketSize += 536;
              }
              else{
                  NS_LOG_INFO("2 - LtpProtocol-Receive: A packet fragment received from the meter " << 
                          uint32_t(header.GetSmartMeterID()) << " in sequence " << seqNum << ".");
                  
                  record.accFragmentsSize += header.GetFragmentSize();
                  
                  NS_LOG_INFO("Packet Size: " << record.packetSize << " The Size of Accumulated Fragments: " << record.accFragmentsSize);
                  
                  if(record.accFragmentsSize == record.packetSize){
                      ClientServiceInstances::iterator it;
                      it = m_activeClients.find (GetNode()->GetId());
                      std::vector<uint8_t> data = std::vector<uint8_t> (6204);
                      
                      NS_LOG_INFO("2 - LtpProtocol-Receive: A packet from the meter " <<
                              uint32_t(header.GetSmartMeterID()) << " was received completely in sequence " 
                              << seqNum << ".");
          
                      it->second->ReportStatus (seqNum, GP_SEGMENT_RCV, data, 
                                    uint32_t(header.GetFragmentID()), true, 
                                    uint64_t(header.GetFragmentSize()), 
                                    uint32_t(header.GetSmartMeterID()),
                                    seqTs.GetTs());
                  }
                      
                  sizeOfRest -= header.GetFragmentSize();
                  totalPacketSize += header.GetFragmentSize();
              }
              
              m_packetEntries.erase(std::make_pair(seqNum, from));
              m_packetEntries.insert(std::make_pair(std::make_pair(seqNum, from), record));
          }
      }

      NS_LOG_DEBUG("Size of rest: " << sizeOfRest);
      
      if(sizeOfRest == (uint32_t)0){
          if(remnantBytesEntry != m_remnantBytesMap.end()){
              m_remnantBytesMap.erase(from);
          }
      }
      
      if(sizeOfRest < (uint32_t)524 && sizeOfRest > (uint32_t)0){
          if(remnantBytesEntry != m_remnantBytesMap.end()){
              m_remnantBytesMap.erase(from);
          }
          
          std::vector < uint8_t > remnantBytes;
          for(uint32_t i=0; i<sizeOfRest; i++)
              remnantBytes.push_back(packetBuffer[p->GetSize()-sizeOfRest+i]);
          
          m_remnantBytesMap.insert(std::make_pair(from, remnantBytes));
          
          break;
      }
  }while(sizeOfRest > 0);

//  NS_LOG_INFO("Total bytes read in total: " << totalPacketSize);
  
}

void LtpProtocol::ReportSegmentTransmission (SessionId id, uint64_t cpSerialNum, uint64_t lower, uint64_t upper)
{
  NS_LOG_FUNCTION (this << id << cpSerialNum);

  SessionStateRecords::iterator it = m_activeSessions.find (id);

  if (it == m_activeSessions.end ())
     {
       NS_FATAL_ERROR ("Active session not found");
     }

  ConvergenceLayerAdapters::iterator itCla = m_clas.find (it->second->GetPeerLtpEngineId ());

  Ptr<LtpConvergenceLayerAdapter> cla = itCla->second;
  Ptr<ReceiverSessionStateRecord> srecv = DynamicCast<ReceiverSessionStateRecord> (it->second);

  Ptr<Packet> p = Create<Packet> ();
  LtpHeader header;
  LtpContentHeader contentHeader;

//  header.SetSegmentType (LTPTYPE_RS);
  header.SetVersion (m_version);
  header.SetSessionId (id);

  uint32_t RpSerial = it->second->GetRpCurrentSerialNumber ();
  uint32_t CpSerial = cpSerialNum;
  uint32_t upperBound = (upper) ? upper : srecv->GetHighBound ();
  uint32_t lowerBound = (lower) ? lower : srecv->GetLowBound ();


//  contentHeader.SetSegmentType (LTPTYPE_RS);
  contentHeader.SetRpSerialNumber (RpSerial);
  contentHeader.SetCpSerialNumber (CpSerial);
  contentHeader.SetUpperBound (upperBound);       // Size of red part to which this report pertains.
  contentHeader.SetLowerBound (lowerBound);      // Size of the previous red part.


  std::set<LtpContentHeader::ReceptionClaim> claims = srecv->GetClaims (RpSerial);

  for (std::set<LtpContentHeader::ReceptionClaim>::iterator it = claims.begin (); it != claims.end (); ++it)
    {
      contentHeader.AddReceptionClaim (*it);
    }

  p->AddHeader (contentHeader);
  p->AddHeader (header);

  srecv->StoreClaims (contentHeader);

  srecv->Enqueue (p);

  if (cla->IsLinkUp ())
    {
      Simulator::Schedule (m_localDelays, &LtpProtocol::Send, this, cla);
    }
}

void
LtpProtocol::ReportSegmentAckTransmission (SessionId id, uint64_t rpSerialNum, Ptr<LtpConvergenceLayerAdapter>cla )
{
  NS_LOG_FUNCTION (this << id << rpSerialNum << cla);

  SessionStateRecords::iterator it = m_activeSessions.find (id);

  if (it == m_activeSessions.end ())
     {
       NS_FATAL_ERROR ("Can't send ReportAckSegment because active session not found");
     }


  Ptr<SenderSessionStateRecord> ssr = DynamicCast<SenderSessionStateRecord> (it->second);

  Ptr<Packet> p = Create<Packet> ();
  LtpHeader header;
  LtpContentHeader contentHeader;

//  header.SetSegmentType (LTPTYPE_RAS);
  header.SetVersion (m_version);
  header.SetSessionId (id);

//  contentHeader.SetSegmentType (LTPTYPE_RAS);
  contentHeader.SetRpSerialNumber (rpSerialNum);

  p->AddHeader (contentHeader);
  p->AddHeader (header);
  if (ssr)
    {
      ssr->Enqueue (p);

      if (cla->IsLinkUp ())
        {
          Simulator::Schedule (m_localDelays, &LtpProtocol::Send, this, cla);
        }
    }
  else
    {
      // A RAS is received after the session has already been closed
      if (cla->IsLinkUp ())
        {
          Simulator::Schedule (m_localDelays, &LtpConvergenceLayerAdapter::Send, cla, p);
        }
    }
}


void
LtpProtocol::CheckRedPartReceived (SessionId id)
{
  NS_LOG_FUNCTION (this << id);

  SessionStateRecords::iterator it = m_activeSessions.find (id);

  if (it == m_activeSessions.end ())
     {
       NS_FATAL_ERROR ("Active session not found");
     }

  Ptr<ReceiverSessionStateRecord> ssr = DynamicCast<ReceiverSessionStateRecord> (it->second);

  RedSegmentInfo info = ssr->FindMissingClaims (ssr->GetRpCurrentSerialNumber ());

  if (info.claims.size () == 0)
    {
      ssr->SetRedPartFinished ();
    }

  info = ssr->FindMissingClaims (ssr->GetRpCurrentSerialNumber ());

}

void LtpProtocol::RetransmitSegment (SessionId id, RedSegmentInfo info)
{
  NS_LOG_FUNCTION (this << id << info.RpserialNum << info.low_bound << info.high_bound << info.claims.size ());

  SessionStateRecords::iterator it = m_activeSessions.find (id);

  if (it == m_activeSessions.end ())
  {
       NS_FATAL_ERROR ("Can't send ReportSegment because active session not found");
  }

  Ptr<SenderSessionStateRecord> ssr = DynamicCast<SenderSessionStateRecord> (it->second);

  ConvergenceLayerAdapters::iterator itCla = m_clas.find (it->second->GetPeerLtpEngineId ());

  Ptr<LtpConvergenceLayerAdapter> cla = itCla->second;

  if (m_cpRtxLimit  > ssr->GetCpRtxNumber ())
    {
      std::vector<uint8_t> rdData = ssr->GetBlockData ();
      uint32_t claimSz = 1;

      for (std::set<LtpContentHeader::ReceptionClaim>::iterator it = info.claims.begin (); it != info.claims.end (); ++it)
        {
          if (claimSz++ < info.claims.size ())
            {
              EncapsulateBlockData (ssr->GetDestination (), ssr, rdData, rdData.size (), it->offset, it->length);
            }
          else
            {
              EncapsulateBlockData (ssr->GetDestination (), ssr, rdData, rdData.size (), it->offset, it->length, info.RpserialNum);
            }
        }

      if (cla->IsLinkUp ())
        {
          ssr->IncrementCpRtxNumber ();
          Simulator::Schedule (m_localDelays, &LtpProtocol::Send, this, cla);
        }
    }

}

void LtpProtocol::RetransmitReport (SessionId id, RedSegmentInfo info)
{
  NS_LOG_FUNCTION (this << id);
  SessionStateRecords::iterator it = m_activeSessions.find (id);

  if (it == m_activeSessions.end ())
  {
       NS_FATAL_ERROR ("Can't send ReportSegment because active session not found");
  }

  Ptr<ReceiverSessionStateRecord> srecv =  DynamicCast<ReceiverSessionStateRecord> (it->second);

  if (m_rpRtxLimit > srecv->GetRpRtxNumber ())
    {
      ReportSegmentTransmission (id,info.CpserialNum);
      srecv->IncrementRpRtxNumber ();
    }

}


void LtpProtocol::RetransmitCheckpoint (SessionId id, RedSegmentInfo info)
{
  NS_LOG_FUNCTION (this << id);

  SessionStateRecords::iterator it = m_activeSessions.find (id);
}

uint64_t LtpProtocol::GetLocalEngineId () const
{
  NS_LOG_FUNCTION (this);
  return m_localEngineId;
}

void LtpProtocol::SetLocalEngineId (uint64_t engineID)
{
  NS_LOG_FUNCTION (this);
  m_localEngineId = engineID;
}

Ptr<Node>
LtpProtocol::GetNode () const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}

void LtpProtocol::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;

}

void LtpProtocol::SetIpResolutionTable (Ptr<LtpIpResolutionTable> rprot)
{
  NS_LOG_FUNCTION (this << rprot);

  ConvergenceLayerAdapters::iterator it;

  for (it = m_clas.begin (); it != m_clas.end (); ++it)
    {
      it->second->SetRoutingProtocol (rprot);
    }
}

bool LtpProtocol::AddConvergenceLayerAdapter (Ptr<LtpConvergenceLayerAdapter> link)
{
  NS_LOG_FUNCTION (this << link);

  std::pair<uint64_t,Ptr<LtpConvergenceLayerAdapter> > entry;
  entry = std::make_pair (link->GetRemoteEngineId (), link);
  //NS_LOG_INFO("Link->GetRemoteEngineId(): " << link->GetRemoteEngineId ());
  std::pair< std::map<uint64_t,Ptr<LtpConvergenceLayerAdapter> >::iterator,bool> ret;
  ret =  m_clas.insert (entry);
  
  //NS_LOG_INFO("There are " << m_clas.size() << " in the map!");
  
  return ret.second;

}

Ptr<LtpConvergenceLayerAdapter> LtpProtocol::GetConvergenceLayerAdapter (uint64_t engineId)
{
  NS_LOG_FUNCTION (this << engineId);
  ConvergenceLayerAdapters::iterator it = m_clas.find (engineId);
  if (it != m_clas.end ())
    {
      return it->second;
    }
  else
    {
      return 0;
    }
}

void
LtpProtocol::EnableLinkStateCues (Ptr<LtpConvergenceLayerAdapter> link)
{
  NS_LOG_FUNCTION (this << link);

  link->SetCheckPointSentCallback ( MakeCallback (&LtpProtocol::SetCheckPointTransmissionTimer, this));
  link->SetReportSentCallback ( MakeCallback (&LtpProtocol::SetReportReTransmissionTimer, this));
  link->SetEndOfBlockSentCallback ( MakeCallback (&LtpProtocol::SetEndOfBlockTransmission, this));
}

Ptr<Packet> LtpProtocol::EncapsulateSegment (uint64_t dstClientService,  
        SessionId id, std::vector<uint8_t> data, uint64_t offset, 
        uint64_t length, SegmentType type, uint32_t cpSerialNum, 
        uint32_t rpSerialNum, u_int8_t fragmentType, uint8_t fragmentID, 
        uint16_t fragmentSize, uint32_t seqNum){
    fragmentSize += 18;
  NS_LOG_FUNCTION (this << uint32_t(fragmentID) << fragmentSize << seqNum);
  
  SeqTsHeader seqTs;
  seqTs.SetSeq (seqNum);

  LtpHeader header;
  
  uint16_t meterID = uint16_t(this->m_node->GetId());
  
  NS_LOG_INFO("Meter ID: " << uint32_t(meterID));
  NS_LOG_INFO("Fragment Type: " << uint32_t(fragmentType));
  NS_LOG_INFO("Fragment ID: " << uint32_t(fragmentID));
  NS_LOG_INFO("Fragment Size: " << uint32_t(fragmentSize));
  NS_LOG_INFO("Real Data Size: " << data.size());
  
  header.SetSmartMeterID(meterID);
  header.SetFragmentType(fragmentType);
  header.SetFragmentID(fragmentID);
  header.SetFragmentSize(fragmentSize);

  /* Create packet of MTU size */
  std::vector<uint8_t>::const_iterator start = data.begin(); //data.begin () + offset;
  std::vector<uint8_t>::const_iterator end = data.end();//(offset + length > data.size ()) ? data.end () : data.begin () + offset + length;
  std::vector<uint8_t> segmentData (start,end);

  Ptr<Packet> packet = Create<Packet> ((uint8_t *) segmentData.data (),(uint32_t) segmentData.size ());
  packet->AddHeader (seqTs);
  packet->AddHeader (header);
  
  //NS_LOG_INFO("Size of the fragment is " << packet->GetSize());
 
  return packet;
}


void
LtpProtocol::EncapsulateBlockData (uint64_t dstClientService, Ptr<SessionStateRecord> ssr, std::vector<uint8_t> data, uint16_t frgSize, uint64_t claimOffset, uint64_t claimLength, uint32_t claimSerialNum, uint32_t seqNum)
{
  NS_LOG_FUNCTION (this << dstClientService << ssr << frgSize << claimOffset << claimLength << claimSerialNum << seqNum);

  SessionId id = ssr->GetSessionId ();

//  bool endOfRedPart = false;

  //SegmentType type = MULTIPLE;
  uint64_t offset = claimOffset;
  uint64_t length = claimLength;
  uint32_t cpSerialNum = ssr->GetCpCurrentSerialNumber ();
  uint32_t rpSerialNum = claimSerialNum;

  ConvergenceLayerAdapters::iterator itCla = m_clas.find (ssr->GetPeerLtpEngineId ());
//  uint16_t mtu = itCla->second->GetMtu ();
//
//  uint64_t dataSize = (claimLength) ? claimLength : data.size ();
//  dataSize += (claimOffset) ? claimOffset : 0;
//
//  NS_LOG_DEBUG ("mtu: " << mtu << " dataSize: " << dataSize);
//  
////////////////////////////////////////////////////////////////////////////////
  
  std::vector<std::vector <uint8_t> > dataFragments;
  
  SegmentType sType = SINGLE;
  uint8_t headersSize = 18;
  uint16_t pureFragmentSize = (uint16_t)(frgSize-(uint16_t)headersSize);   // 536 - 18 = 518 bytes
  uint64_t dataSize = data.size();
  uint16_t fragmentSize; 
  
  NS_LOG_INFO("Data size: " << dataSize);
  NS_LOG_INFO("Pure fragment size: " << pureFragmentSize);
  uint8_t numOfFragments = uint8_t(dataSize/pureFragmentSize);  // 6204/518 = 11
  if(dataSize%pureFragmentSize != 0) numOfFragments++;  // 11 + 1 = 12
  
  NS_LOG_INFO("The number of fragments: " << (uint32_t)numOfFragments);
  
  int remnantDataSize = dataSize%pureFragmentSize;
  
  NS_LOG_INFO("Remnant data size: " << remnantDataSize);
 
  if(numOfFragments > 1)
      sType = MULTIPLE;
  
  for(uint8_t i=0; i<numOfFragments; i++)
      dataFragments.push_back( std::vector < uint8_t > () );
  
  //NS_LOG_INFO("# of Fragments: " << uint32_t(numOfFragments));
  
  int dataPosition = 0;
  uint64_t blockSize = 0;
  
  for(uint8_t i=0; i<numOfFragments; i++){
      if(numOfFragments == 1){
          fragmentSize = data.size();
          blockSize = fragmentSize;
      }
      else{
          if(i == 0){
              fragmentSize = data.size() + (numOfFragments*headersSize) - headersSize;
              blockSize = pureFragmentSize;
          }
          else if(i == numOfFragments-1){
              if(remnantDataSize == 0)
                  fragmentSize = pureFragmentSize;
              else
                  fragmentSize = remnantDataSize;
              
              blockSize = fragmentSize;
          }
          else{
             fragmentSize = pureFragmentSize;
             blockSize = fragmentSize;
          }
      }
      
      NS_LOG_INFO("Fragment size: " << fragmentSize);
      NS_LOG_INFO("Block size: " << blockSize);

      //////////////////////////////////////////////////////////////////////////

      for(uint64_t j=0; j<blockSize; j++){
          dataFragments[i].push_back(data[dataPosition]);
          dataPosition++;
      }
      
      //////////////////////////////////////////////////////////////////////////
      
      /* Create packet of MTU size */
      Ptr<Packet> packet = EncapsulateSegment (dstClientService, id, dataFragments[i], 
                                               offset, length, sType, cpSerialNum, 
                                               rpSerialNum, sType, i, fragmentSize, 
                                               seqNum);
      
      /* Enqueue for transmission */
      ssr->Enqueue (packet);
  }
  std::cout << "Data Position: " << dataPosition << std::endl;
}


void
LtpProtocol::SetEndOfBlockTransmission (SessionId id)
{
  NS_LOG_FUNCTION (this << id);

  SessionStateRecords::iterator it = m_activeSessions.find (id);

  if (it == m_activeSessions.end ())
     {
       NS_FATAL_ERROR ("Active session not found");
     }

  Ptr<SenderSessionStateRecord> ssend = DynamicCast<SenderSessionStateRecord> (it->second);
  ssend->SetBlockFinished ();

  if (ssend->IsRedPartFinished () && ssend->IsBlockFinished ())
    {
      CloseSession (id);        // Stop Transmission procedure
    }
}


//} //namespace ltp
} //namespace ns3


