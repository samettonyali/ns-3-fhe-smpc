/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008,2009 IITP RAS
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
 * */

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mesh-module.h"
#include "ns3/mobility-module.h"
#include "ns3/mesh-helper.h"
#include "ns3/mesh-module.h"
#include "ns3/wifi-phy.h"
#include <ns3/point-to-point-helper.h>
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/hwmp-protocol.h"
#include "ns3/arp-l3-protocol.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/hwmp-tcp-interface.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/packet.h"

#include "src/ltp-protocol/model/ltp-protocol.h"

#include "ns3/ltp-protocol-helper.h"
#include "ns3/ltp-protocol.h"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>

#include "n_eq_coord.h"
#include "n_eq_25.h"
#include "n_eq_36.h"
#include "n_eq_49.h"
#include "n_eq_64.h"
#include "n_eq_81.h"
#include "n_eq_100.h"
#include "n_eq_121.h"
#include "n_eq_144.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("EtoEAgg_PP_FHE_PRP_ProtocolScript");

class EtoEAgg_PP_FHE_PRP_Protocol{
    public:
        /// Init test
        EtoEAgg_PP_FHE_PRP_Protocol ();
        /// Configure test from command line arguments
        void Configure (int argc, char ** argv);
        void ReadMST();
        void ReadFHEAggTimes();
        /// Run test
        int Run ();
        
    private:
        int m_xSize;
        int m_ySize;
        double m_step;
        double m_randomStart;
        double m_totalTime;
        double m_firstSendingTime;
        double m_packetInterval;
        uint16_t m_FxSize;
        uint32_t m_numOfLeafMeters;
        uint32_t m_nIfaces;
        bool m_chan;
        bool m_pcap;
        std::string m_stack;
        std::string m_root;
        std::string m_txrate;
        std::string m_input;
        int m_node_num;
        int m_conn;
        int m_shuffle;
        double m_initstart;
        int m_sink;
        std::string m_sinkIpAddress;
        bool m_ActivateSecurityModule;
        std::string m_filename;
        bool m_gridtopology;
        std::string m_UdpTcpMode;
        int m_arpOp;
        double m_arpwait;
        bool m_randomAppStart;
        int m_maxDepth;
        int m_strategy;
        double m_delta;
        bool m_peartoActivated;
        uint32_t m_consumerId;
        uint32_t m_opID;
        int m_connectionType;
        int m_portId;
        int m_sensor, m_aggregator;
        uint32_t child_count;

        vector< coordinates > nodeCoords;
        vector< int > child;
        vector< int > parent;
        vector< int > aggnode;
        vector< int > parent2;
        vector< int > count;
        vector< int > psize;
        vector< int > fheAggTimes;

        //to calculate the length of the simulation
        float m_timeTotal, m_timeStart, m_timeEnd;
        // List of network nodes
        NodeContainer nodes;
        // List of all mesh point devices
        NetDeviceContainer meshDevices;
        //Addresses of interfaces:
        Ipv4InterfaceContainer interfaces;
        // MeshHelper. Report is not static methods
        MeshHelper mesh;
        
        LtpProtocolHelper ltpHelper;
  
    private:
        // Create nodes and setup their mobility
        void CreateNodes ();
        // Install internet m_stack on nodes
        void InstallInternetStack ();
        
        void InstallLTP();
        
        // Install applications
        void InstallApplication (double m_pktInterval, uint32_t opId);

        // Print mesh devices diagnostics
        void Report ();
  
        // interface between Hwmp and ArpL3Protocol
        void InstallSecureArp ();

        void InstallHwmpTcpInterface ();
        void CreateCustId();
};

EtoEAgg_PP_FHE_PRP_Protocol::EtoEAgg_PP_FHE_PRP_Protocol () :
    m_xSize (3),
    m_ySize (3),
    m_step (100.0),
    m_randomStart (0.1),
    m_totalTime (100.0),
    m_firstSendingTime (15.0),
    m_FxSize (32),
    m_nIfaces (1),
    m_chan (true),
    m_pcap (false),
    m_stack ("ns3::Dot11sStack"),
    m_root ("00:00:00:00:00:01"),
    m_txrate ("150kbps"),
    m_node_num (0),
    m_conn (0),
    m_shuffle (2),
    m_initstart (15.4),
    m_sink (0),
    m_sinkIpAddress ("10.1.1.1"),
    m_ActivateSecurityModule (false),
    m_gridtopology (true),
    m_UdpTcpMode ("tcp"),
    m_arpOp (1),
    m_arpwait (4), // default 1 s, 4s better since no failed node
    m_randomAppStart (false),
    m_maxDepth (0),
    m_strategy (3),
    m_delta (0.03),
    m_peartoActivated (false),
    m_consumerId (15),
    m_connectionType (0),
    m_portId (8100)
{
}

void EtoEAgg_PP_FHE_PRP_Protocol::Configure (int argc, char *argv[]){
    CommandLine cmd;
    cmd.AddValue ("x-size", "Number of nodes in a row grid. [6]", m_xSize);
    cmd.AddValue ("y-size", "Number of rows in a grid. [6]", m_ySize);
    cmd.AddValue ("step",   "Size of edge in our grid, meters. [100 m]", m_step);
    /*
     * As soon as starting node means that it sends a beacon,
     * simultaneous start is not good.
     */
    cmd.AddValue ("start",  "Maximum random start delay, seconds. [0.1 s]", m_randomStart);
    cmd.AddValue ("time",  "Simulation time, seconds [100 s]", m_totalTime);
    cmd.AddValue ("opID",  "Simulation time, seconds [100 s]", m_opID);
    cmd.AddValue ("firstSending",  "Simulation time, seconds [100 s]", m_firstSendingTime);
    cmd.AddValue ("packetInterval",  "Interval between packets in UDP ping, seconds [0.001 s]", m_packetInterval);
    cmd.AddValue ("Fx-size", "Size of the second packets", m_FxSize);
    cmd.AddValue ("interfaces", "Number of radio interfaces used by each mesh point. [1]", m_nIfaces);
    cmd.AddValue ("channels",   "Use different frequency channels for different interfaces. [0]", m_chan);
    cmd.AddValue ("pcap",   "Enable PCAP traces on interfaces. [0]", m_pcap);
    cmd.AddValue ("stack",  "Type of protocol stack. ns3::Dot11sStack by default", m_stack);
    cmd.AddValue ("root", "Mac address of root mesh point in HWMP", m_root);
    cmd.AddValue ("node", "Node sink", m_node_num);
    cmd.AddValue ("conn", "Number of sending nodes [1]", m_conn); 
    cmd.AddValue ("shuffle", "Number of random shuffle [2]", m_shuffle);
    cmd.AddValue ("init", "Initial Starting time [5.4]", m_initstart);
    cmd.AddValue ("input", "Topology file to read in node positions", m_input);
    cmd.AddValue ("sink", "Sink node ID [0]", m_sink);
    cmd.AddValue ("sink-ip", "IP address of the default entry in ARP table", m_sinkIpAddress);
    cmd.AddValue ("security","Activate Security Module [false]", m_ActivateSecurityModule);
    cmd.AddValue ("grid", "Choice whether grid or random topology [true]", m_gridtopology);
    cmd.AddValue ("UdpTcp", "UDP or TCP mode [udp]", m_UdpTcpMode);
    cmd.AddValue ("arp-op", "ARP operations : 1. Normal [default], 2. Creation only, 3. Maintenance ony, 4. All pre-install arp table", m_arpOp);
    cmd.AddValue ("wait-arp", "When this timeout expires, the cache entries will be scanned and entries in WaitReply state will resend ArpRequest unless MaxRetries has been exceeded, in which case the entry is marked dead [1s]", m_arpwait);
    cmd.AddValue ("random-start", "Random start of the application [false]", m_randomAppStart);
    cmd.AddValue ("strategy", "Type of scheduling strategies. 1. Nearest nodes first, 2. Farthest node first, 3. Random [3]", m_strategy);
    cmd.AddValue ("delta", "The additional time for scheduling strategy [0.03]", m_delta);
    cmd.AddValue ("pearto", "PEA RTO Activated ? [false]", m_peartoActivated);
    cmd.AddValue ("connection-type", "Type of connection for connection establishment [0] = created first, 1 = created when data is sent", m_connectionType);
    cmd.AddValue ("port-num", "The port number of the remote host, it can determine the QoS support, 8100 = tcp_default, 9100 = gbr_gamming", m_portId);

    cmd.Parse (argc, argv);
    NS_LOG_DEBUG ("Grid:" << m_xSize << "*" << m_ySize);
    NS_LOG_DEBUG ("Simulation time: " << m_totalTime << " s");
}

void EtoEAgg_PP_FHE_PRP_Protocol::ReadFHEAggTimes(){
    std::ifstream input;
    input.open("fhe_agg_delays.txt");
    
    std::string line;
    
    if (input.is_open()){
        for(int i=0; i<99; i++){
            getline (input,line);
            fheAggTimes.push_back(atoi(line.c_str()));
            std::cout << "pheAggTimes[" << i << "] = " << fheAggTimes[i] << "\n";
        }
    }
    else {
        std::cerr << "Error: Can't open file " << m_input << "\n";
        exit (EXIT_FAILURE);    
    }
}

void EtoEAgg_PP_FHE_PRP_Protocol::ReadMST (){
    std::ifstream input;
    NS_LOG_INFO("Input File: " << m_input);
    input.open(("MSTs/" + m_input).c_str());

    if (input.is_open()){
        //int j = 0;
        std::string line;

        getline (input,line);
        getline (input,line);

        int counter = atoi(line.c_str());
        
        m_numOfLeafMeters = counter;

        getline (input,line);

        int readValue = 0;

        for(int i=0; i<counter; ++i){
            //input >> child[i];
            getline (input,line);
            readValue = atoi(line.c_str());
            child.push_back(readValue);
        }

        m_sensor = child.size();

        getline (input,line);
        getline (input,line);
        getline (input,line);

        counter = atoi(line.c_str());

        getline (input,line);

        for(int i=0; i<counter; ++i){
            getline (input,line);
            readValue = atoi(line.c_str());
            parent.push_back(readValue);
        }

        getline (input,line);
        getline (input,line);
        getline (input,line);

        counter = atoi(line.c_str());

        getline (input,line);

        for(int i=0; i<counter; ++i){
            getline (input,line);
            readValue = atoi(line.c_str());
            aggnode.push_back(readValue);
        }

        m_aggregator = aggnode.size();

        getline (input,line);
        getline (input,line);
        getline (input,line);

        counter = atoi(line.c_str());

        getline (input,line);

        for(int i=0; i<counter; ++i){
            getline (input,line);
            readValue = atoi(line.c_str());
            parent2.push_back(readValue);
        }

        getline (input,line);
        getline (input,line);
        getline (input,line);

        child_count = atoi(line.c_str());

        getline (input,line);
        getline (input,line);
        getline (input,line);

        counter = atoi(line.c_str());

        getline (input,line);

        for(int i=0; i<counter; ++i){
            getline (input,line);
            readValue = atoi(line.c_str());
            count.push_back(readValue);
        }

        getline (input,line);
        getline (input,line);
        getline (input,line);

        counter = atoi(line.c_str());

        getline (input,line);

        for(int i=0; i<counter; ++i){
            getline (input,line);
            readValue = atoi(line.c_str());
            psize.push_back(readValue);
        }
        input.close();
    } else {
        std::cerr << "Error: Can't open file " << m_input << "\n";
        exit (EXIT_FAILURE);    
    }
}

void EtoEAgg_PP_FHE_PRP_Protocol::CreateNodes (){
    double m_txpower = 18.0; // dbm
    /*
    * Create m_ySize*m_xSize stations to form a grid topology
    */
    nodes.Create (m_ySize*m_xSize);
    
    // Configure YansWifiChannel
    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();

    wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue (-89.0) );
    wifiPhy.Set ("CcaMode1Threshold", DoubleValue (-62.0) );
    wifiPhy.Set ("TxGain", DoubleValue (1.0) );
    wifiPhy.Set ("RxGain", DoubleValue (1.0) );
    wifiPhy.Set ("TxPowerLevels", UintegerValue (1) );
    wifiPhy.Set ("TxPowerEnd", DoubleValue (m_txpower) );
    wifiPhy.Set ("TxPowerStart", DoubleValue (m_txpower) );
    wifiPhy.Set ("RxNoiseFigure", DoubleValue (7.0) );

    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    wifiPhy.SetChannel (wifiChannel.Create ());

    // Configure the parameters of the Peer Link
    Config::SetDefault ("ns3::dot11s::PeerLink::MaxBeaconLoss", UintegerValue (20));
    Config::SetDefault ("ns3::dot11s::PeerLink::MaxRetries", UintegerValue (4));
    Config::SetDefault ("ns3::dot11s::PeerLink::MaxPacketFailure", UintegerValue (5));

    // Configure the parameters of the HWMP
    Config::SetDefault ("ns3::dot11s::HwmpProtocol::Dot11MeshHWMPnetDiameterTraversalTime", TimeValue (Seconds (2)));
    Config::SetDefault ("ns3::dot11s::HwmpProtocol::Dot11MeshHWMPactivePathTimeout", TimeValue (Seconds (100)));
    Config::SetDefault ("ns3::dot11s::HwmpProtocol::Dot11MeshHWMPactiveRootTimeout", TimeValue (Seconds (100)));
    Config::SetDefault ("ns3::dot11s::HwmpProtocol::Dot11MeshHWMPmaxPREQretries", UintegerValue (5));
    Config::SetDefault ("ns3::dot11s::HwmpProtocol::UnicastPreqThreshold",UintegerValue (10));
    Config::SetDefault ("ns3::dot11s::HwmpProtocol::UnicastDataThreshold",UintegerValue (5));
    Config::SetDefault ("ns3::dot11s::HwmpProtocol::DoFlag", BooleanValue (true));
    Config::SetDefault ("ns3::dot11s::HwmpProtocol::RfFlag", BooleanValue (false));

    if (m_arpwait != 1.0) {
        Config::SetDefault ("ns3::ArpCache::WaitReplyTimeout", TimeValue (Seconds (m_arpwait)));
    }
   
    /*
     * Create mesh helper and set stack installer to it
     * Stack installer creates all needed protocols and install them to
     * mesh point device
     */
  
    mesh = MeshHelper::Default ();
    if (!Mac48Address (m_root.c_str ()).IsBroadcast ()){
        mesh.SetStackInstaller (m_stack, "Root", Mac48AddressValue (Mac48Address (m_root.c_str ())));
    }
    
    else{
        //If root is not set, we do not use "Root" attribute, because it
        //is specified only for 11s
        mesh.SetStackInstaller (m_stack);
    }
    
    if (m_chan){
        mesh.SetSpreadInterfaceChannels (MeshHelper::SPREAD_CHANNELS);
    }
    else{
        mesh.SetSpreadInterfaceChannels (MeshHelper::ZERO_CHANNEL);
    }
    
    mesh.SetStandard (WIFI_PHY_STANDARD_80211g);
    mesh.SetMacType ("RandomStart", TimeValue (Seconds(m_randomStart)));
    mesh.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("ErpOfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (2500));
 
    // Set number of interfaces - default is single-interface mesh point
    mesh.SetNumberOfInterfaces (m_nIfaces);
  
    // Install protocols and return container if MeshPointDevices
    meshDevices = mesh.Install (wifiPhy, nodes);

    // Setup mobility - static grid topology
    MobilityHelper mobility;
    if (m_gridtopology) {
        mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                       "MinX", DoubleValue (0.0),
                                       "MinY", DoubleValue (0.0),
                                       "DeltaX", DoubleValue (m_step),
                                       "DeltaY", DoubleValue (m_step),
                                       "GridWidth", UintegerValue (m_xSize),
                                       "LayoutType", StringValue ("RowFirst"));
									
	for (int i = 0; i < m_xSize*m_ySize; i++){
            //case ROW_FIRST:
            coordinates position;
            position.X = m_step * (i % m_xSize);
            position.Y = m_step * (i / m_xSize);
            nodeCoords.push_back ( position );
        } 							
    }
    else { // random topology
        int topoId = m_shuffle; 
        switch (m_xSize) {
            case 5:
                //  for (unsigned int i = 0; i < m_xSize*m_ySize; i++)
                for (unsigned int i = 0; i < sizeof array(n_eq_25[topoId]); i++)           
                    nodeCoords.push_back (n_eq_25[topoId][i]);
                break;
            case 6:
                for (unsigned int i = 0; i < sizeof array(n_eq_36[topoId]); i++)           
                    nodeCoords.push_back (n_eq_36[topoId][i]);
                break;
            case 7:
                for (unsigned int i = 0; i < sizeof array(n_eq_49[topoId]); i++)           
                    nodeCoords.push_back (n_eq_49[topoId][i]);
                break;
            case 8:
                for (unsigned int i = 0; i < sizeof array(n_eq_64[topoId]); i++)           
                    nodeCoords.push_back (n_eq_64[topoId][i]);
                break;
            case 9:
                for (unsigned int i = 0; i < sizeof array(n_eq_81[topoId]); i++)           
                    nodeCoords.push_back (n_eq_81[topoId][i]);
                break;
            case 10:
                for (unsigned int i = 0; i < sizeof array(n_eq_100[topoId]); i++)           
                    nodeCoords.push_back (n_eq_100[topoId][i]);
                break;
            case 11:
                for (unsigned int i = 0; i < sizeof array(n_eq_121[topoId]); i++)           
                    nodeCoords.push_back (n_eq_121[topoId][i]);
                break;
            case 12:
                for (unsigned int i = 0; i < sizeof array(n_eq_144[topoId]); i++)           
                    nodeCoords.push_back (n_eq_144[topoId][i]);
                break;
        }
        
        Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
        for (vector< coordinates >::iterator j = nodeCoords.begin (); j != nodeCoords.end (); j++){
            positionAlloc->Add (Vector ((*j).X, (*j).Y, 0.0));
        }
        mobility.SetPositionAllocator (positionAlloc);
    }
    
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (nodes);
    
    if (m_pcap)
        wifiPhy.EnablePcapAll (std::string ("mp-"));
}

void EtoEAgg_PP_FHE_PRP_Protocol::InstallInternetStack (){
    //Config::SetDefault ("ns3::TcpSocketBase::OutputFilename", StringValue (m_filename));
    //Config::SetDefault ("ns3::TcpSocketBase::PeaRto", BooleanValue (m_peartoActivated));

    InternetStackHelper internetStack;
    internetStack.Install (nodes);
    Ipv4AddressHelper address;
    address.SetBase ("7.1.0.0", "255.255.255.0");
    interfaces = address.Assign (meshDevices);
    
    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (536));
}

void EtoEAgg_PP_FHE_PRP_Protocol::InstallLTP(){
    double delay = 0.0;
    double inaccuracy = 1;

    // Create a LtpIpResolution table to perform mappings between Ipv4 adresses and LtpEngineIDs
    Ptr<LtpIpResolutionTable> routing =  CreateObjectWithAttributes<LtpIpResolutionTable> ("Addressing", StringValue ("Ipv4"));

    TimeValue OnewayRttEstimation = TimeValue(Seconds(delay*inaccuracy));
    // Use a helper to create and install Ltp Protocol instances in the nodes.
    
    ltpHelper.SetAttributes ("CheckPointRtxLimit",  UintegerValue (30),
                             "ReportSegmentRtxLimit", UintegerValue (30),
                             "RetransCyclelimit",  UintegerValue (30),
                             "OneWayLightTime", OnewayRttEstimation,
                             "SessionInactivityLimit", TimeValue (Seconds (10000.0)),
                             "RandomSerialNum", StringValue("ns3::UniformRandomVariable[Min=1.0|Max=16383]")
    );
    
    ltpHelper.SetLtpIpResolutionTable (routing);
    ltpHelper.SetBaseLtpEngineId (0);
    ltpHelper.SetStartTransmissionTime (Seconds (0.1));
}

void EtoEAgg_PP_FHE_PRP_Protocol::InstallHwmpTcpInterface (){
    for (NetDeviceContainer::Iterator i = meshDevices.Begin (); i != meshDevices.End (); ++i){
        Ptr<MeshPointDevice> mp = (*i)->GetObject<MeshPointDevice> ();
        Ptr<Node> node = mp->GetNode () ;
        Ptr<HwmpTcpInterface> hti = CreateObject <HwmpTcpInterface> ();
        node->AggregateObject (hti);

        Ptr<ns3::dot11s::HwmpProtocol> hwmp = mp->GetObject<ns3::dot11s::HwmpProtocol> ();
        hwmp->SetReportHwmpTcpInterfaceCallback (MakeCallback (&HwmpTcpInterface::ReceivedPerrInfoFromHwmp, PeekPointer (hti)));
    }
}

//TODO: The leaf meters send their reading at every 60 sec. Once an aggregator 
//      meter receives all readings from its child meters, it aggregates them
//      and send the aggregated meter reading to its own parent meter.
//      Packet size: 256 bytes encrypted data + 64 bytes signature
//      Delay introduced: Aggregation operations + signature verifications
void EtoEAgg_PP_FHE_PRP_Protocol::InstallApplication (double m_pktInterval, uint32_t opId){       
    int displacement = 0;
    int *array = new int[m_ySize*m_xSize];
    for (int i = 0; i < m_ySize*m_xSize-1; i++) {
        if (i == m_sink) {
            displacement++;
        }
        array[i] = i+displacement;
    }
    
    uint32_t processingDelay;    //in milliseconds
    
    double duration;
    
    // shuffle twice, to make it more random
    for (int i = 0; i < m_shuffle; i++) {
        std::random_shuffle(array,array+(m_ySize*m_xSize-1));
    }
    
    char num [3];
    //char onoff [8];
    ApplicationContainer apps [m_ySize*m_xSize-1];
    ApplicationContainer receiver;
    
    Ptr<UniformRandomVariable> rand_start = CreateObject<UniformRandomVariable> ();
    rand_start->SetAttribute ("Min", DoubleValue (0.001));
    rand_start->SetAttribute ("Max", DoubleValue (0.009));

    Ptr<UniformRandomVariable> rand_port = CreateObject<UniformRandomVariable> ();
    rand_port->SetAttribute ("Min", DoubleValue (9000));
    rand_port->SetAttribute ("Max", DoubleValue (9100));

    std::stringstream ss;
    ss << opId;
    std::string opIdStr, iStr;
    ss >> opIdStr;

    std::ostringstream os;
    os << m_filename << "-" << opIdStr <<"-time.txt";

    std::ofstream of (os.str().c_str(), std::ios::out | std::ios::app);
    std::string m_protocol;
    if (m_UdpTcpMode=="udp") m_protocol = "ns3::UdpSocketFactory";
    else m_protocol = "ns3::TcpSocketFactory";
    
    // Sink for the Gateway: Listens at port 8000
    ltpHelper.InstallAndLink (nodes.Get (m_sink));
    Ptr<LtpProtocol> protocol = nodes.Get (m_sink)->GetObject<LtpProtocol> ();

    protocol->SetLocalEngineId(uint64_t (m_sink));
    protocol->SetAttribute("Mode", UintegerValue (1));
    
    Ptr<AggSensor> sink = 
        CreateObject<AggSensor> (protocol, InetSocketAddress (
                                 interfaces.GetAddress (m_sink), 8000),
                                 (m_ySize*m_xSize-1), false, m_sink);
    
    processingDelay = fheAggTimes[m_ySize*m_xSize-1];
    
    sink->SetAttribute("HomomorphicOperationTime", UintegerValue(processingDelay));
    sink->SetAttribute("Sink", UintegerValue(1));
    sink->SetAttribute("Child", UintegerValue(m_ySize*m_xSize-1));
    sink->SetAttribute("LeafMeters", UintegerValue(m_ySize*m_xSize-1));
    sink->SetAttribute("Mode", UintegerValue (0));
    sink->SetAttribute ("FileName", StringValue (m_filename+"-"+opIdStr));
    sink->SetNode(nodes.Get(m_sink));
    
    sink->SetStartTime(Seconds (0.1));
    sink->SetStopTime(Seconds (m_totalTime+20));
    nodes.Get (m_sink)->AddApplication(sink);
    
    std::cout << "IPv4 Address of the gateway: " << interfaces.GetAddress (m_sink) << std::endl;
    
    // Static Routing from non-gateway nodes to the gateway
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    
    char node [8];
    int shiftCounter = 0;

    int m_SensorNum = 0;
    int m_source;

    for (int sourceIndex=0; sourceIndex<(m_ySize*m_xSize-1); sourceIndex++){
        strcpy(node,"node");
        sprintf(num,"%d",m_SensorNum);
        strcat(node,num);
        
        m_source = array[sourceIndex];
        
        ltpHelper.InstallAndLink (nodes.Get (m_source), m_sink, true);
        Ptr<LtpProtocol> protocol = nodes.Get (m_source)->GetObject<LtpProtocol> ();
        protocol->SetLocalEngineId(uint64_t (m_source));
        protocol->SetAttribute("Mode", UintegerValue (0));
        
        Ptr<Sensor> leafSource = 
            new Sensor (protocol, Address (InetSocketAddress(interfaces.GetAddress (
                        nodes.Get (m_sink)->GetId()), 8000)), true, 
                        uint64_t(nodes.Get (m_source)->GetId()), 
                        uint64_t(nodes.Get (m_sink)->GetId()), 
                        uint64_t(nodes.Get (m_sink)->GetId()));
        
        leafSource->SetAttribute ("PacketSize", UintegerValue (m_FxSize));
        leafSource->SetAttribute("FirstTime", TimeValue (Seconds (m_firstSendingTime-((shiftCounter+1)*0.1))));
        leafSource->SetAttribute("Interval", TimeValue (Seconds(m_pktInterval)));
        leafSource->SetAttribute("Mode", UintegerValue (0));
        leafSource->SetAttribute("MaxFragment", UintegerValue (536));
        //leafSource.SetAttribute ("Pseudonym", UintegerValue (m_consumerId) );
        //leafSource.SetAttribute ("OperationIdentifier", UintegerValue (opId));
        leafSource->SetNode(nodes.Get(m_source));
        
        std::cout << "The IPv4 address of leaf meter " << m_source << ": " << 
                     interfaces.GetAddress (m_source) << std::endl;
        
        if (m_randomAppStart){
            duration = rand_start->GetValue()+m_initstart;
        }
        else {
            duration = m_initstart+((shiftCounter+1)*0.1);
        }
        shiftCounter++;
        
        leafSource->SetStartTime(Seconds (duration));
        leafSource->SetStopTime(Seconds (m_totalTime+20));
        nodes.Get(m_source)->AddApplication(leafSource);
        
        Ptr<Node> meshNode = nodes.Get (m_source);
        Ptr<Ipv4StaticRouting> meshStaticRouting = ipv4RoutingHelper.GetStaticRouting (meshNode->GetObject<Ipv4> ());
        NS_ASSERT(meshStaticRouting != 0);
        meshStaticRouting->SetDefaultRoute (interfaces.GetAddress (m_sink), 1);
    }
    
    of.close ();
}

int EtoEAgg_PP_FHE_PRP_Protocol::Run (){
    ns3::Packet::EnablePrinting();
    std::ostringstream tmp;
    tmp << "sc1-EtoE-" << "-ps1-" << m_FxSize << "-conId-" << m_connectionType << "-" << m_UdpTcpMode << "-" << m_portId << "-";
    
    if (m_gridtopology)
        tmp << "grid-"<<m_initstart;
    else
        tmp << "r" << m_xSize;      
    
    m_filename = tmp.str ();
    
    CreateNodes ();
    
    if (!m_gridtopology) {
        std::ostringstream osp;
        osp << m_filename <<"-pos.txt";
        std::ofstream osf (osp.str().c_str(), std::ios::out | std::ios::app);
        
        for (NodeContainer::Iterator j = nodes.Begin(); j != nodes.End(); ++j) {
            Ptr<Node> object = *j;
            Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
            Vector pos = position->GetPosition();
            
            if (m_sink== (int)object->GetId()) {
                osf << m_xSize << "x" << m_ySize << " x=" << pos.x << ", y=" << pos.y << " " << m_shuffle << " " << m_sink  << "\n";
            }
        }
        osf.close();
    }
    
    InstallInternetStack ();
    InstallLTP();
    InstallHwmpTcpInterface ();

    // Create a mesh gateway pointer
    Ptr<Node> meshGateway = nodes.Get (m_sink);
   
    Ipv4InterfaceAddress iaddr = meshGateway->GetObject<Ipv4>()->GetAddress (1,0);
    Ipv4Address addri = iaddr.GetLocal (); 
    std::cout << "Mesh Gateway IP address : " << addri << " " << meshGateway->GetObject<Ipv4>()->GetInterfaceForAddress (addri) << "Node ID : " << meshGateway->GetId () << std::endl;   
  
    InstallApplication(m_packetInterval, m_opID);
 
    m_timeStart=clock();
    Simulator::Schedule (Seconds (m_totalTime), &EtoEAgg_PP_FHE_PRP_Protocol::Report, this);
    Simulator::Stop (Seconds (m_totalTime));

    Simulator::Run ();

    Simulator::Destroy ();
    m_timeEnd=clock();
    m_timeTotal=(m_timeEnd - m_timeStart)/(double) CLOCKS_PER_SEC;
    
    std::cout << "\n*** Simulation time: " << m_timeTotal << "s\n\n";

    return 0;
}

void EtoEAgg_PP_FHE_PRP_Protocol::Report (){
    std::ostringstream osf;
    osf << m_filename << "-stat.txt";
    std::ofstream osf1 (osf.str().c_str(), std::ios::out | std::ios::app);
  
    uint32_t m_totPerr = 0;
    uint32_t m_totFixRto = 0;
    uint32_t m_totPerrLink = 0;
    uint32_t m_totPerrForward = 0;
    uint32_t m_totPerrPath = 0;
    
    for (NetDeviceContainer::Iterator i = meshDevices.Begin (); i != meshDevices.End (); ++i){
        Ptr<MeshPointDevice> mp = (*i)->GetObject<MeshPointDevice> ();
        Ptr<ns3::dot11s::HwmpProtocol> hwmp = mp->GetObject<ns3::dot11s::HwmpProtocol> ();
        Ptr<Node> node = mp->GetNode () ;
        Ptr<HwmpTcpInterface> hti = node->GetObject <HwmpTcpInterface> ();
        NS_ASSERT(hti !=0);
        uint32_t m_totnorto = hti->GetReportRto();
        osf1 << m_xSize<<"x"<<m_ySize<< " " << Mac48Address::ConvertFrom (mp->GetAddress ()) << " " << m_shuffle << " " << m_totnorto <<" ";
        mp->Report (osf1);
        hwmp->Report (osf1);        
        m_totPerr += hwmp->ReportPathError();
        m_totPerrLink += hwmp->ReportPathErrorLink();
        m_totPerrForward += hwmp->ReportPathErrorForward();
        m_totPerrPath += hwmp->ReportPathErrorPath();
        m_totFixRto += m_totnorto;
    }
    
    osf1.close ();

    std::ostringstream osfile2;
    osfile2 << m_filename << "-lyr2.txt";
    std::ofstream osf3 (osfile2.str().c_str(), std::ios::out | std::ios::app);
  
    for (NetDeviceContainer::Iterator i = meshDevices.Begin (); i != meshDevices.End (); ++i){
        Ptr<MeshPointDevice> mp = (*i)->GetObject<MeshPointDevice> ();
        Ptr<Node> node = mp->GetNode () ;

        Ptr<WifiNetDevice> wifi;
        Ptr<MeshWifiInterfaceMac> mac;

        std::vector<Ptr<NetDevice> > ifaces = mp->GetInterfaces ();
        for (std::vector<Ptr<NetDevice> >::iterator j = ifaces.begin (); j != ifaces.end (); j++){
            wifi = DynamicCast<WifiNetDevice> (*j);
            mac = DynamicCast<MeshWifiInterfaceMac> (wifi->GetMac ());
            Ptr<RegularWifiMac> rmac = DynamicCast<RegularWifiMac> (mac);
            osf3 << m_xSize<<"x"<<m_ySize<< " " << Mac48Address::ConvertFrom (mp->GetAddress ()) << " " << node->GetId () <<" ";
            osf3 << std::endl;
        }
    }
    
    osf3.close ();
}

void
EtoEAgg_PP_FHE_PRP_Protocol::CreateCustId()
{
  m_consumerId++;
}

int main (int argc, char *argv[]){
//    LogComponentEnable ("Gateways", LOG_LEVEL_ALL);
//    LogComponentEnable ("Gateways", LOG_PREFIX_ALL);
//    LogComponentEnable ("LtePacketSink", LOG_LEVEL_INFO);
//    LogComponentEnable ("LtePacketSink", LOG_PREFIX_ALL);
//    LogComponentEnable ("LtePacketSource", LOG_LEVEL_INFO);
//    LogComponentEnable ("LtePacketSource", LOG_PREFIX_ALL);
//    LogComponentEnable ("MeshWifiInterfaceMac", LOG_LEVEL_WARN);
//    LogComponentEnable ("MeshWifiInterfaceMac", LOG_PREFIX_ALL);
//    LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);
//    LogComponentEnable ("Ipv4L3Protocol", LOG_PREFIX_ALL);
//    LogComponentEnable ("HwmpProtocolMac", LOG_LEVEL_ALL);
//    LogComponentEnable ("HwmpProtocolMac", LOG_PREFIX_ALL);
//    LogComponentEnable ("HwmpTcpInterface", LOG_LEVEL_INFO);
//    LogComponentEnable ("HwmpTcpInterface", LOG_PREFIX_ALL);
    LogComponentEnable ("LtpProtocol", LOG_LEVEL_ALL);
    
//    LogComponentEnable ("EtoEAgg_PP_FHE_PRP_ProtocolScript", LOG_LEVEL_ALL);
    LogComponentEnable ("Sensor", LOG_LEVEL_ALL);
    LogComponentEnable ("AggSensor", LOG_LEVEL_ALL);
//    LogComponentEnable ("HwmpProtocol", LOG_LEVEL_ALL);
//    LogComponentEnable ("LtpUdpConvergenceLayerAdapter", LOG_PREFIX_ALL);
    LogComponentEnable ("LtpUdpConvergenceLayerAdapter", LOG_LEVEL_ALL);
    LogComponentEnable ("EtoEAgg_PP_FHE_PRP_ProtocolScript", LOG_PREFIX_ALL); 
//    LogComponentEnable ("LtpHelper", LOG_LEVEL_ALL);
//    LogComponentEnable ("TcpL4Protocol", LOG_LEVEL_ALL); 
    LogComponentEnable ("TcpSocketBase", LOG_LEVEL_ALL);
//    LogComponentEnable ("TcpSocketBase", LOG_PREFIX_ALL);

    EtoEAgg_PP_FHE_PRP_Protocol t;
    t.Configure (argc, argv);
    t.ReadFHEAggTimes();
    t.Run ();
    return 0;
}