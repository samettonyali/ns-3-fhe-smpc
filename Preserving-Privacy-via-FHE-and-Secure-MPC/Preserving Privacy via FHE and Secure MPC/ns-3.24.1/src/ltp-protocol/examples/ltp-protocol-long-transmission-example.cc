/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*-
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
 * Author: Rubén Martínez <rmartinez@deic.uab.cat>
 */

//        Network topology
//
//      Earth		Mars
//       n0              n1
//       |               |
//       =================
//          PointToPoint
//
//  This example defines a long haul transmission between two nodes,  both nodes are linked by a point a point channel with a high end-to-end delay.
//  The transmission is performed using the LTP protocol. Each node runs an application represented by class ClientServiceInstance which implementents the public
//  API required to interact with the LTP implementation. The data is sent through succesive transmissions of blocks of size 10KB, from which 5KB are guaranteed
//  to be transmitted reliably.

// - Data is sent end-to-end through a LtpProtcol <-> UdpLayerAdapter <-> PointToPointLink <-> UdpLayerAdapter <-> LtpProtcol.

#include "ns3/core-module.h"
#include "ns3/ltp-protocol-helper.h"
#include "ns3/ltp-protocol.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"
#include <sstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LtpProtocolSimpleExample");



class ClientServiceInstance : public Application
{
public:
  ClientServiceInstance ();
  ClientServiceInstance (
    bool isSender,
    uint64_t localClientId);
  ClientServiceInstance (
    bool isSender,
    uint64_t localClientId,
    uint64_t destinationClient,
    uint64_t destinationLtpEngine,
    uint32_t bytesToSend,
    uint32_t blockSize,
    uint32_t redPartSize
    );

  virtual ~ClientServiceInstance ();
  uint32_t GetBytesReceived();


  void SetProtocol (Ptr<LtpProtocol>);
protected:
  virtual void DoDispose (void);
private:
  virtual void StartApplication (void);           // Called at time specified by Start
  virtual void StopApplication (void);            // Called at time specified by Stop
  void Send ();
  void Receive (SessionId id,
                StatusNotificationCode code,
                std::vector<uint8_t> data,
                uint32_t dataLength,
                bool endFlag,
                uint64_t srcLtpEngine,
                uint32_t offset );

  bool m_isSender;
  uint64_t m_localClientServiceId;


  uint64_t m_destinationClientServiceId;
  uint64_t m_destinationLtpId;


  // Used by sender
  uint32_t m_bytesToSend;
  uint32_t m_bytesSent;
  uint32_t m_blockSize;
  uint32_t m_blocksSent;
  uint32_t m_redPartSize;


  // User by receiver
  uint32_t m_bytesReceived;
  uint32_t m_lastBlockSize;
  uint32_t m_numBlocks;

  Ptr<LtpProtocol> m_protocol;

};

ClientServiceInstance::~ClientServiceInstance ()
{
}

ClientServiceInstance::ClientServiceInstance ()
  :         m_isSender (0),
    m_localClientServiceId (0),
    m_destinationClientServiceId (0),
    m_destinationLtpId (0),
    m_bytesToSend (0),
    m_bytesSent (0),
    m_blockSize (0),
    m_blocksSent (0),
    m_redPartSize (0),
    m_bytesReceived (0),
    m_lastBlockSize (0),
    m_numBlocks (0),
    m_protocol (0)
{
}


ClientServiceInstance::ClientServiceInstance (
  bool isSender,
  uint64_t localClientId)
  : m_isSender (isSender),
    m_localClientServiceId (localClientId),
    m_destinationClientServiceId (0),
    m_destinationLtpId (0),
    m_bytesToSend (0),
    m_bytesSent (0),
    m_blockSize (0),
    m_blocksSent (0),
    m_redPartSize (0),
    m_bytesReceived (0),
    m_lastBlockSize (0),
    m_numBlocks (0),
    m_protocol (0)
{

}

ClientServiceInstance::ClientServiceInstance (
  bool isSender,
  uint64_t localClientId,
  uint64_t destinationClient,
  uint64_t destinationLtpEngine,
  uint32_t bytesToSend,
  uint32_t blockSize,
  uint32_t redPartSize
  )
  :      m_isSender (isSender),
    m_localClientServiceId (localClientId),
    m_destinationClientServiceId (destinationClient),
    m_destinationLtpId (destinationLtpEngine),
    m_bytesToSend (bytesToSend),
    m_bytesSent (0),
    m_blockSize (blockSize),
    m_blocksSent (0),
    m_redPartSize (redPartSize),
    m_bytesReceived (0),
    m_lastBlockSize (0),
    m_numBlocks (0),
    m_protocol (0)
{

}

uint32_t ClientServiceInstance::GetBytesReceived() {
  return m_bytesReceived;  
}

void ClientServiceInstance::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

void ClientServiceInstance::StartApplication ()
{
  NS_LOG_FUNCTION (this);
  if (m_protocol)
    {

      if (m_isSender)
        {
          Send ();
        }
    }
}

void ClientServiceInstance::StopApplication ()
{
  NS_LOG_FUNCTION (this);
}

void
ClientServiceInstance::SetProtocol (Ptr<LtpProtocol> prot)
{
  NS_LOG_FUNCTION (this);
  m_protocol = prot;
  CallbackBase cb = MakeCallback (&ClientServiceInstance::Receive, this);
  m_protocol->RegisterClientService (m_localClientServiceId, cb);
}

void
ClientServiceInstance::Send ()
{
  NS_LOG_FUNCTION (this);
  // Create a block of dummy data
  std::vector<uint8_t> data ( m_blockSize, 0);
  m_protocol->StartTransmission (
    m_localClientServiceId,
    m_destinationClientServiceId,
    m_destinationLtpId,
    data,
    m_redPartSize);


  m_bytesSent += m_blockSize;

}

void
ClientServiceInstance::Receive (SessionId id,
                                StatusNotificationCode code,
                                std::vector<uint8_t> data,
                                uint32_t dataLength,
                                bool endFlag,
                                uint64_t srcLtpEngine,
                                uint32_t offset )
{
  NS_LOG_FUNCTION (this);
  if (m_isSender)
    {

      if (code == ns3::SESSION_END)
        {
          m_blocksSent++;
          NS_LOG_INFO("ClientServiceNotification - Sender Session - Sent a full block of data with size: ( " << m_blockSize << ") - total data sent " <<  m_bytesSent);
          if (m_bytesSent < m_bytesToSend)
            {
              Send ();
            }
        }
    }
  else
    {

      m_bytesReceived += dataLength;
      m_lastBlockSize += dataLength;

      if (code == ns3::GP_SEGMENT_RCV)
        {

          NS_LOG_INFO( "ClientServiceNotification - Receiver Session - Received a Green Data Segment of Size: ( " << dataLength << ")" );
        }

      if (code == ns3::RED_PART_RCV)
        {
          std::stringstream ss;

          for ( std::vector<uint8_t>::const_iterator i = data.begin (); i != data.end (); ++i)
            {
              ss << *i;
            }

          NS_ASSERT (ss.str ().length () == dataLength);

          NS_LOG_INFO( "ClientServiceNotification - Receiver Session - Received Full Red Part of Size: ( " << dataLength << ")" );

        }
      if (code == ns3::SESSION_END)
        {
          m_numBlocks++;
          NS_LOG_INFO( "ClientServiceNotification - Receiver Session - Received block of data with size: (" << m_lastBlockSize << ") - Source LtpEngine: (" << srcLtpEngine << ") - Blocks received: (" << m_numBlocks  << ") - Total data received: " << m_bytesReceived << " bytes" );
          m_lastBlockSize = 0;
        }
    }
}



int
main (int argc, char *argv[])
{
  
  bool verbose = true;
  std::string errorModelType = "ns3::RateErrorModel";
  double errorRate = 0.00001;
  double delay = 750;
  double expnum = 0;
  double inacuracy = 1;
  std::string dataRate = "500Kbps";
  
  uint32_t bytesToSend = 1024 * 1024 * 10; // 10 MB
  uint32_t blockSize = 1024 * 100; // 100 KB
  
  CommandLine cmd;
  cmd.AddValue ("verbose", "Tell application to log if true", verbose);
  cmd.AddValue("errorModelType", "TypeId of the error model to use", errorModelType);
  cmd.AddValue("BER", "Error rate", errorRate);
  cmd.AddValue("inRTT", "Percentage of RTT inaccuracy", inacuracy);
  cmd.AddValue("totalBytes", "Total number of bytes to send", bytesToSend);
  cmd.AddValue("blockSize", "Size of the block", blockSize);
  cmd.AddValue("dataRate", "Channel data rate", dataRate);
  cmd.AddValue("channelDelay", "channelDelay", delay);
  cmd.AddValue("expNum", "experiment number", expnum);
  cmd.Parse (argc,argv);
  
  if (verbose) 
  {
     LogComponentEnable ("LtpProtocolSimpleExample", LOG_LEVEL_ALL);  
     LogComponentEnable ("LtpProtocol", LOG_LEVEL_ALL);
     LogComponentEnable ("LtpUdpConvergenceLayerAdapter", LOG_LEVEL_ALL);
     LogComponentEnable ("SessionStateRecord", LOG_LEVEL_ALL);
     LogComponentEnable ("UdpSocketImpl", LOG_LEVEL_ALL);
     LogComponentEnable ("LtpHelper", LOG_LEVEL_ALL);
     LogComponentEnable ("TcpSocketBase", LOG_LEVEL_ALL);
    // LogComponentEnable ("ErrorModel", LOG_LEVEL_ALL);
  }
  
  uint32_t dataSentReliably = blockSize; // 100 KB
    
  Config::SetDefault ("ns3::RateErrorModel::ErrorRate", DoubleValue (errorRate));
  Config::SetDefault ("ns3::RateErrorModel::ErrorUnit", StringValue ("ERROR_UNIT_BYTE"));
  
  // Create the nodes required by the topology (shown above).
  NodeContainer nodes;
  nodes.Create (2);
  
  TimeValue channelDelay = TimeValue(Seconds(delay));  // Earth to Mars - Average
  
  // Create point to point links and instell them on the nodes
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue (dataRate)); 
  pointToPoint.SetChannelAttribute ("Delay", channelDelay);
  
  pointToPoint.EnablePcapAll("ltp");

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);
  
  /* Install error model */
  ObjectFactory factory;
  factory.SetTypeId (errorModelType);
  Ptr<ErrorModel> em = factory.Create<ErrorModel> ();
  devices.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
  
  // Install the internet stack on the nodes
  InternetStackHelper internet;
  internet.Install (nodes);

  // Assign IPv4 addresses.
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  // Create a LtpIpResolution table to perform mappings between Ipv4 adresses and LtpEngineIDs
  Ptr<LtpIpResolutionTable> routing =  CreateObjectWithAttributes<LtpIpResolutionTable> ("Addressing", StringValue ("Ipv4"));

  
  TimeValue OnewayRttEstimation = TimeValue(Seconds(delay*inacuracy));  
  // Use a helper to create and install Ltp Protocol instances in the nodes.
  LtpProtocolHelper ltpHelper;
  ltpHelper.SetAttributes ("CheckPointRtxLimit",  UintegerValue (30),
                           "ReportSegmentRtxLimit", UintegerValue (30),
                           "RetransCyclelimit",  UintegerValue (30),
                           "OneWayLightTime", OnewayRttEstimation,
                           "SessionInactivityLimit", TimeValue (Seconds (10000.0)),
                           "RandomSerialNum", StringValue("ns3::UniformRandomVariable[Min=1.0|Max=16383]")
			  );
  ltpHelper.SetLtpIpResolutionTable (routing);
  ltpHelper.SetBaseLtpEngineId (0);
  ltpHelper.SetStartTransmissionTime (Seconds (1));
  ltpHelper.InstallAndLink (nodes);

  // Define the ClientService ID Code of the Client Service Instance that will be using the Ltp protocol.
  uint64_t ClientId = 0;  // Bundle

  // Create a client service instance that will act as receiver
  Ptr<ClientServiceInstance> receiver = CreateObject<ClientServiceInstance> (false, ClientId);
  receiver->SetProtocol (  nodes.Get (1)->GetObject<LtpProtocol> () );
  receiver->SetStartTime (Seconds (0));
  receiver->SetNode (  nodes.Get (1));
  nodes.Get (1)->AddApplication (receiver);

  // Define parameters used for the sender
  uint64_t receiverLtpId = nodes.Get (1)->GetObject<LtpProtocol> ()->GetLocalEngineId ();

  // Create a client service instance that will act as a sender.
  Ptr<ClientServiceInstance> sender = CreateObject<ClientServiceInstance> (true, ClientId, ClientId, receiverLtpId, bytesToSend, blockSize, dataSentReliably );
  sender->SetProtocol (  nodes.Get (0)->GetObject<LtpProtocol> () );
  sender->SetStartTime (Seconds (0));
  sender->SetNode (  nodes.Get (0));
  nodes.Get (0)->AddApplication (sender);
  
  FlowMonitorHelper flowMonitorHelper;
  flowMonitorHelper.Install(nodes);

  Ptr<FlowMonitor> monitor = flowMonitorHelper.GetMonitor();

  monitor->SetAttribute("DelayBinWidth", DoubleValue (0.001));
  monitor->SetAttribute("JitterBinWidth", DoubleValue (0.001));
  monitor->SetAttribute("PacketSizeBinWidth", DoubleValue (20));
  monitor->SetAttribute("MaxPerHopDelay", TimeValue(Seconds(1000))); 
  
  Simulator::Stop (Seconds (600*3600));
  Simulator::Run ();


  monitor->CheckForLostPackets (); 
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowMonitorHelper.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  
  double end_to_end_delay = 0;
  double delivery_ratio = 0;
  double packet_loss = 0;
  double throughput = 0;
  double goodput = 0;
  double txpackets = 0;

  uint32_t samples = 0;
  uint32_t delivery_skip = 0;

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
  {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);
   
    if (t.destinationPort != 1113) continue; // Skip flows not generated by LTP

    double end_to_end_delay_flow = (double) (iter->second.delaySum).GetSeconds() / iter->second.rxPackets;
    double delivery_ratio_flow = (double) iter->second.rxPackets / iter->second.txPackets;
    double packet_loss_flow = (double) iter->second.lostPackets /  (iter->second.rxPackets + iter->second.lostPackets);
    double throughput_flow = (double) iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024;
    double goodput_flow = (double) receiver->GetBytesReceived() * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024;
    txpackets += (double) iter->second.txPackets;
    
    NS_LOG_INFO("Total bytes by IP Layer: " << (double) iter->second.rxBytes);
    NS_LOG_INFO("Total bytes by Application Layer: " << (double) receiver->GetBytesReceived());
    
    Ipv4Address rcv_addr("10.1.1.2");        
    if(t.destinationAddress == rcv_addr) // Pick only the flow going to the receiver side which is the one that contains data segments
    {    
      delivery_ratio += delivery_ratio_flow;
      if(iter->second.rxPackets > 0) end_to_end_delay += end_to_end_delay_flow;
      else delivery_skip++;
      packet_loss += packet_loss_flow;
      throughput += throughput_flow;
      goodput += goodput_flow;    
      samples++;
    }

    if (verbose)
    {
      NS_LOG_UNCOND("Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress);
      NS_LOG_UNCOND("Tx Packets = " << iter->second.txPackets);
      NS_LOG_UNCOND("Rx Packets = " << iter->second.rxPackets);
      NS_LOG_UNCOND("End-To-End Delay = " << end_to_end_delay_flow);
      NS_LOG_UNCOND("Delivery Ratio = " << delivery_ratio_flow);
      NS_LOG_UNCOND("Packet Loss = " << packet_loss_flow);
      NS_LOG_UNCOND("Throughput: " <<  throughput_flow << " Kbps");
      NS_LOG_UNCOND("Goodput: " << goodput_flow << " Kbps");
    }
  }

  
  end_to_end_delay = (double) end_to_end_delay/ (samples - delivery_skip);
  delivery_ratio = delivery_ratio/samples;
  packet_loss = packet_loss/samples;
  throughput = throughput/samples;
  

  NS_LOG_UNCOND(   "============= Performance Summary =============" << std::endl
                << "===============================================" << std::endl  
                << "End to End Delay (hours) : " << end_to_end_delay / 3600 << std::endl
                << "Delivery Ratio ( P(X) over 1) :"  << delivery_ratio << std::endl
                << "Packet Loss ( 1 - P(X) ):" << packet_loss << std::endl
                << "Throughput (kbps) :" << throughput << std::endl
                << "Goodput (kbps) :" << goodput << std::endl                
                << "Tx packets :" << txpackets << std::endl 
                << "===============================================");

  AsciiTraceHelper ascii;  
  
  std::stringstream ss;
  
  ss << "LTP-ErrorRate-" << errorRate << "-RTT-" << inacuracy << "-" << expnum << ".dat";
  Ptr<OutputStreamWrapper> logfile = ascii.CreateFileStream (ss.str(),std::ios_base::app);
  
  *logfile->GetStream() << channelDelay.Get().GetSeconds() << " " 
			<< errorRate << " " 
			<< throughput << " " 
			<< goodput;
			
  if (expnum == 4)
     *logfile->GetStream() << " " << txpackets;
  
  *logfile->GetStream()  << std::endl;
 
  Simulator::Destroy ();
  

  return 0;
}



