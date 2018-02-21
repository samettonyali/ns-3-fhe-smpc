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
 * Author:  Tom Henderson (tomhend@u.washington.edu)
 */
#include <vector>

#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/agg-sensor.h"
#include "ns3/seq-ts-header.h"
#include "ns3/uinteger.h"
#include "ns3/packet-socket-address.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"

#include "ns3/nstime.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "src/ltp-protocol/model/ltp-protocol.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AggSensor");
NS_OBJECT_ENSURE_REGISTERED (AggSensor);

TypeId 
AggSensor::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::AggSensor")
    .SetParent<Application> ()
    .AddConstructor<AggSensor> ()
    .AddAttribute ("Local", "The Address on which to Bind the rx socket.",
                   AddressValue (),
                   MakeAddressAccessor (&AggSensor::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("Protocol", "The type id of the protocol to use for the rx socket.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&AggSensor::m_tid),
                   MakeTypeIdChecker ())
    .AddAttribute ("Child", "The number of child node.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&AggSensor::m_child_node),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("LeafMeters", "The number of leaf nodes.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&AggSensor::m_leaf_node),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Remote", "The address of the destination",
                   AddressValue (),
                   MakeAddressAccessor (&AggSensor::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("Mode", "Type of operation [0=forwarding], 1=aggregation", 
                    UintegerValue (0),
                   MakeUintegerAccessor (&AggSensor::m_operation_type),
                   MakeUintegerChecker<uint8_t> ())
   .AddAttribute ("MaxFragment", 
                   "The max number of bytes to send in one segment.",
                   UintegerValue (1500),
                   MakeUintegerAccessor (&AggSensor::m_maxFragment),
                   MakeUintegerChecker<uint32_t> ())
   .AddAttribute ("PacketSize", "The size of packets sent in on state",
                   UintegerValue (512),
                   MakeUintegerAccessor (&AggSensor::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1))
   .AddAttribute ("DefaultRxSize", "The default size of packets received",
                   UintegerValue (6140),
                   MakeUintegerAccessor (&AggSensor::m_defSize),
                   MakeUintegerChecker<uint32_t> ())
   .AddAttribute ("Interval", "the interval for sending data",
                    TimeValue (Seconds (30)),
                   MakeTimeAccessor (&AggSensor::m_interval),
                   MakeTimeChecker ())
   .AddAttribute ("FirstTime", "the first time for sending data",
                    TimeValue (Seconds (20)),
                   MakeTimeAccessor (&AggSensor::m_firstTime),
                   MakeTimeChecker ())
   .AddAttribute ("FileName", "The output filename",
                   StringValue ("roundstat"),
                   MakeStringAccessor (&AggSensor::m_outputFilename),
                   MakeStringChecker ())
    .AddAttribute ("DelayBetweenFragmentedPacket", "the interval between two consecutive fragmented packet",
                    TimeValue (MilliSeconds (1)),
                   MakeTimeAccessor (&AggSensor::m_timeBetweenFragmentedPacket),
                   MakeTimeChecker ())
    .AddAttribute ("HomomorphicOperationTime", "the time needed to perform homomorphic operations",
                    UintegerValue (1000),
                   MakeUintegerAccessor (&AggSensor::m_homomorphicTime),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Sink", "An indicator that shows whether the application is sink or not",
                   UintegerValue (0),
                   MakeUintegerAccessor (&AggSensor::m_isSink),
                   MakeUintegerChecker<uint32_t> ())
  /*.AddTraceSource ("SessionStatus",
                     "Trace used to report changes in session status",
                     MakeTraceSourceAccessor (&AggSensor::m_reportStatus),
                    "ns3::ClientServiceStatus::SessionStatusTracedCallback")*/
    .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&AggSensor::m_rxTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

TypeId
AggSensor::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

AggSensor::AggSensor ()
  : m_expectedPackets (),
  m_socket (0),
  m_child_node (0),
  m_pSocket (0),
  m_connected (false),
  m_operation_type (0),
  m_maxFragment (1482),
  m_nextTime (0)
{
  NS_LOG_FUNCTION (this);
}

AggSensor::AggSensor (Ptr<LtpProtocol> protocol, InetSocketAddress address, uint32_t child)
  : m_expectedPackets (),
  m_socket (0),
  m_local (address),
  m_child_node (child),
  m_pSocket (0),
  m_connected (false),
  m_protocol (protocol),
  m_operation_type (0),
  m_maxFragment (1482),
  m_nextTime (0),
  m_localClientServiceId (0),
  m_seqnum (0)
{
  NS_LOG_FUNCTION (this);
  CallbackBase cb = MakeCallback (&AggSensor::Receive, this);
  m_protocol->RegisterClientService (m_localClientServiceId, cb);
}

AggSensor::AggSensor (Ptr<LtpProtocol> protocol, InetSocketAddress address, uint32_t child, 
             bool isSender, uint64_t localClientId)
  : m_expectedPackets (),
  m_socket (0),
  m_local (address),
  m_child_node (child),
  m_pSocket (0),
  m_connected (false),
  m_protocol (protocol),
  m_operation_type (0),
  m_maxFragment (1482),
  m_nextTime (0),
  m_isSender(isSender),
  m_localClientServiceId (localClientId),
  m_seqnum (0)
{
  NS_LOG_FUNCTION (this);
  CallbackBase cb = MakeCallback (&AggSensor::Receive, this);
  m_protocol->RegisterClientService (m_localClientServiceId, cb);
}

AggSensor::AggSensor (Ptr<LtpProtocol> protocol, InetSocketAddress address, 
             InetSocketAddress parentAddress, uint32_t child, bool isSender, 
             uint64_t localClientId, uint64_t destinationClient, 
             uint64_t destinationLtpEngine, uint32_t bytesToSend, 
             uint32_t blockSize, uint32_t redPartSize)
  : m_expectedPackets (),
  m_socket (0),
  m_local (address),
  m_child_node (child),
  m_pSocket (0),    
  m_peerAddress (parentAddress),
  m_connected (false),
  m_protocol (protocol),
  m_operation_type (0),
  m_maxFragment (1482),
  m_nextTime (0),
  m_isSender(isSender),
  m_localClientServiceId(localClientId),
  m_seqnum (0),
  m_destinationClientServiceId (destinationClient),
  m_destinationLtpId(destinationLtpEngine),
  m_bytesToSend(bytesToSend),
  m_blockSize(blockSize),
  m_redPartSize(redPartSize)
{
  NS_LOG_FUNCTION (this);
  CallbackBase cb = MakeCallback (&AggSensor::Receive, this);
  m_protocol->RegisterClientService (m_localClientServiceId, cb);
}

AggSensor::AggSensor (Ptr<LtpProtocol> protocol, InetSocketAddress address, 
                      InetSocketAddress parentAddress, uint32_t processingDelay,
                      uint64_t localClientId, uint64_t destinationClientId)
: m_expectedPackets (),
  m_socket (0),
  m_local (address),
  m_pSocket (0),    
  m_peerAddress (parentAddress),
  m_connected (false),
  m_protocol (protocol),
  m_operation_type (0),
  m_maxFragment (536),
  m_nextTime (0),
  m_localClientServiceId(localClientId),
  m_seqnum (0),
  m_destinationClientServiceId (destinationClientId),
  m_destinationLtpId (destinationClientId),
  m_procDelay (processingDelay)
{
    NS_LOG_FUNCTION (this);
    CallbackBase cb = MakeCallback (&AggSensor::Receive, this);
    m_protocol->RegisterClientService (m_localClientServiceId, cb);
}

AggSensor::~AggSensor()
{
  NS_LOG_FUNCTION (this);
  
  StatPrint();
}

void AggSensor::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_protocol->GetConvergenceLayerAdapter(m_destinationLtpId)->DisposeAggSensorApp();
  m_protocol = 0;

  // chain up
  Application::DoDispose ();
}


// Application Methods
void AggSensor::StartApplication ()    // Called at time specified by Start
{
  NS_LOG_FUNCTION (this << m_destinationLtpId << m_localClientServiceId << m_isSink);
  
//  Ptr<LtpConvergenceLayerAdapter> mpro = m_protocol->GetConvergenceLayerAdapter(m_destinationLtpId);
//  
//  if(mpro == 0)
//      NS_LOG_INFO("Sikinti buyuk gardasim!");
//  
//  mpro->EnableReceive(m_localClientServiceId);
  
  if(!m_isSink){
      NS_LOG_INFO("Local: " << m_localClientServiceId << " Peer: " << m_destinationClientServiceId);
    if (!m_peerAddress.IsInvalid () ) {
        m_protocol->GetConvergenceLayerAdapter(m_destinationLtpId)->EnableSend();
    }
  }
}

void AggSensor::StopApplication ()     // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);
  
  CancelEvents ();
  
  m_protocol->GetConvergenceLayerAdapter(m_destinationLtpId)->StopAggSensorApp();
}

void AggSensor::CancelEvents ()
{
  NS_LOG_FUNCTION (this);

  Simulator::Cancel (m_sendEvent);
}

// Event handlers
void AggSensor::StartSending ()
{
  NS_LOG_FUNCTION (this);
  ScheduleNextTx ();  // Schedule the send packet event
  ScheduleStopEvent ();
}

void AggSensor::StopSending ()
{
  NS_LOG_FUNCTION (this);
  CancelEvents ();

  ScheduleStartEvent ();
}

// Private helpers
void AggSensor::ScheduleNextTx ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO("AggSensor:m_firstTime = " << m_firstTime);
   if(m_nextTime==0) m_nextTime = m_firstTime;
   else m_nextTime = m_interval;
   NS_LOG_LOGIC ("nextTime = " << m_nextTime);
   NS_LOG_LOGIC ("m_interval = " << m_interval);
    m_sendEvent = Simulator::Schedule (m_nextTime, &AggSensor::SendPacket, this);
}

void AggSensor::ScheduleStartEvent ()
{  // Schedules the event to start sending data (switch to the "On" state)
  NS_LOG_FUNCTION (this);
 
}

void AggSensor::ScheduleStopEvent ()
{  // Schedules the event to stop sending data (switch to "Off" state)
  NS_LOG_FUNCTION (this);
}

void AggSensor::Receive (uint32_t seqNum,
                         StatusNotificationCode code,
                         std::vector<uint8_t> data,
                         uint32_t fragmentID,
                         bool endFlag,
                         uint64_t fragmentSize,
                         uint32_t meterID,
                         Time timeStamp)
{
    NS_LOG_FUNCTION (this);
    
    if(code == 0)
        return;
    
    NS_LOG_INFO("AggSensor::Current Time: " << Simulator::Now());
    NS_LOG_INFO("AggSensor::Timestamp of the Packet: " << timeStamp);
    NS_LOG_INFO("AggSensor::Sequence Number: " << seqNum);
    NS_LOG_INFO("AggSensor::Size of the received packet: " << fragmentSize);
    NS_LOG_INFO("AggSensor::Fragment ID: " << fragmentID);
    NS_LOG_INFO("AggSensor::Sender Smart Meter ID: " << meterID);
    NS_LOG_INFO("AggSensor::m_isSink variable is " << m_isSink << ".");
    
    uint32_t m_rxBytes = fragmentSize;
    Time now = Simulator::Now ();
    Time txtime = timeStamp;
    
    if(m_isSink){
        bool lNewRecord = true;
       for(std::vector<StatRecord>::iterator it = m_stat.begin(); it != m_stat.end(); ++it) {
          if(it->round == seqNum) {
            it->rxCount++;
            it->rxBytes += m_rxBytes;
            it->totDelay += (now.ToInteger (Time::US) - txtime.ToInteger (Time::US));
            it->lastRxTime = now;
            if (it->minTxTime > txtime) it->minTxTime = txtime;
            lNewRecord = false;
            break;
          }      
       }

        if (lNewRecord) {
           StatRecord tmp;
           tmp.round = seqNum;
           tmp.rxCount = 1;
           tmp.rxBytes = m_rxBytes;
           tmp.totDelay = (now.ToInteger (Time::US) - txtime.ToInteger (Time::US) );
           tmp.firstRxTime = now;
           tmp.lastRxTime = now;
           tmp.minTxTime = txtime;
           m_stat.push_back(tmp);
        }
    }
    
    if(m_isSink){ //gateway meter
        MeterSeqNumMap::iterator itSeq;
        itSeq = m_meterSeqNumMap.find(seqNum);
        
        NS_LOG_INFO("The gateway has received a packet!!! # children: " << m_child_node);
        
        if(itSeq == m_meterSeqNumMap.end()){
            m_meterSeqNumMap.insert(std::make_pair(seqNum, (uint32_t)1));
        }
        else{
            uint32_t seqNumCounter = itSeq->second;
            seqNumCounter++;
            if(seqNumCounter == m_child_node){
                NS_LOG_INFO("Gateway Sequence " << seqNum << " was completed!");
            }
            else{
                NS_LOG_INFO("Gateway Counter is " << seqNumCounter << " for Sequence " << seqNum);
                m_meterSeqNumMap.erase(seqNum);
                m_meterSeqNumMap.insert(std::make_pair(seqNum, seqNumCounter));
            }
        }
    }
    else{    //aggregator meter
        MeterSeqNumMap::iterator itSeq;
        itSeq = m_meterSeqNumMap.find(seqNum);

        NS_LOG_INFO("Aggregator " << GetNode()->GetId() << " has received a packet!!! # children: " << m_child_node);

        if(itSeq == m_meterSeqNumMap.end()){
            m_meterSeqNumMap.insert(std::make_pair(seqNum, (uint32_t)1));
            if(m_child_node == (uint32_t)1){
                NS_LOG_INFO("Aggregator " << GetNode()->GetId() << " Sequence " << seqNum << " was completed!");
                NS_LOG_INFO("An aggregated data packet will be sent to the parent meter after homomorphic addition operation(s)!");
                m_sendEvent = Simulator::Schedule (MilliSeconds(m_homomorphicTime), &AggSensor::SendPacket, this);
            }
        }
        else{
            uint32_t seqNumCounter = itSeq->second;
            seqNumCounter++;
            if(seqNumCounter == m_child_node){
                NS_LOG_INFO("Aggregator " << GetNode()->GetId() << " Sequence " << seqNum << " was completed!");
                NS_LOG_INFO("An aggregated data packet will be sent to the parent meter after homomorphic addition operation(s)!");
                m_sendEvent = Simulator::Schedule (MilliSeconds(m_homomorphicTime), &AggSensor::SendPacket, this);
            }
            else{
                NS_LOG_INFO("Aggregator Counter is " << seqNumCounter << " for Sequence " << seqNum);
                m_meterSeqNumMap.erase(seqNum);
                m_meterSeqNumMap.insert(std::make_pair(seqNum, seqNumCounter));
            }
        }
    }
}

void AggSensor::SendPacket ()
{
  NS_LOG_FUNCTION (this);
  
  std::vector<uint8_t> data (m_pktSize, 0);
  
  m_protocol->StartTransmission(m_localClientServiceId,
    m_destinationClientServiceId,
    m_destinationLtpId,
    data,
    m_maxFragment,//m_redPartSize,
    m_operation_type,
    m_seqnum);
  
  ++m_seqnum;
  
  m_bytesSent += m_pktSize;
  m_lastStartTime = Simulator::Now ();
//  if(m_operation_type==0) 
//       ScheduleNextTx();
  //ScheduleNextTx ();
  
  //if(m_operation_type==0) ScheduleNextTx ();
}

void AggSensor::StatPrint () 
{
   std::ostringstream os;
   os << m_outputFilename+".sta";
   std::ofstream osf (os.str().c_str(), std::ios::out | std::ios::app);

   double totrxCount = 0;
   double totrxBytes = 0;
   double toteteDelay = 0;
   double maxCount = 0;
   double totCT = 0;
   Time minfirstRx;
   Time maxLastRx;
   uint32_t counter = 0;
   uint32_t roundCounter = 0;
   bool lfirst = true;
   for(std::vector<StatRecord>::iterator it = m_stat.begin(); it != m_stat.end(); ++it) {
      totrxCount += it->rxCount;
      totrxBytes += it->rxBytes;
      if (maxCount < it->rxCount) maxCount = it->rxCount;
      if (lfirst) {
           minfirstRx = it->firstRxTime;
           maxLastRx = it->lastRxTime;
           lfirst = false;
      } else {
          if (minfirstRx > it->firstRxTime) minfirstRx = it->firstRxTime;
          if (maxLastRx < it->lastRxTime) maxLastRx = it->lastRxTime;
      }
      toteteDelay += it->totDelay;
      double ct = ((it->lastRxTime).ToInteger (Time::US) - (it->minTxTime).ToInteger (Time::US)); 
      if(it->rxCount == m_child_node){
        totCT += ct;
        roundCounter++;
      }
      counter++;
      NS_LOG_INFO (it->round << " " << it->rxCount << " " << it->rxBytes << " " << it->totDelay << " " << 
            it->firstRxTime << " " << it->lastRxTime << " " << it->minTxTime << " " << ct);
      osf << it->round << " " << it->rxCount << " " << it->rxBytes << " " << it->totDelay << " " << it->firstRxTime << " " << it->lastRxTime << " " << it->minTxTime << " " << ct << std::endl;
      
   }
   osf.close();

   double pdr = 0.0;
   double delta = (maxLastRx.ToInteger (Time::US) - minfirstRx.ToInteger (Time::US))/1000000;  
   if(m_operation_type == (uint32_t)1)
       pdr =  100*totrxCount/(counter*m_leaf_node);
   else
       pdr =  100*totrxCount/(maxCount*counter);
   double tp = (totrxBytes*8)/delta/1024;
   double ete = toteteDelay/totrxCount/1000000;  
   double avgCT = totCT/roundCounter/1000000; // in seconds
   avgCT += (((double)m_homomorphicTime)/1000000.0);
   NS_LOG_INFO ("Aggregation time at the gateway: " << (((double)m_homomorphicTime)/1000000.0) << " seconds.");
   NS_LOG_INFO ("Statistic : " << totrxCount << " " << maxCount*counter << " " << totrxBytes << " " << maxLastRx << " " << minfirstRx << " "
                << delta << " " << toteteDelay << " " << totCT
                << " PDR " << pdr
                << " TP " << tp
                << " ETE Delay " << ete << " seconds "
                << " CT " << avgCT );
   std::ostringstream os1;
   os1 << m_outputFilename+".rcp";
   std::ofstream osf1 (os1.str().c_str(), std::ios::out | std::ios::app);
   osf1 << totrxCount << " " << maxCount*counter << " " << totrxBytes << " " << maxLastRx << " " << minfirstRx << " " << delta << " " << toteteDelay << " " << totCT << " PDR " << pdr << " TP " << tp << " ETE Delay " << ete << " seconds " << " CT " << avgCT << std::endl ;
   osf1.close();
   
}

ApplicationContainer
AggSensor::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

Ptr<Application>
AggSensor::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application> ();
  node->AddApplication (app);

  return app;
}
} // Namespace ns3
