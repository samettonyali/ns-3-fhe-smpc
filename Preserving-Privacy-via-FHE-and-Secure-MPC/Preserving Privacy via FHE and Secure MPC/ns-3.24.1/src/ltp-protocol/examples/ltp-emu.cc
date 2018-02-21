/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University of Washington, 2012 INRIA
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
 */

// Adapted from ns-3 example program fd-emu-ping.cc
//
// Allow ns-3 LTP client to send data to an external LTP server
// (or external LTP client to send to ns-3 server)
// using emulation mode
//
//   +----------------------+
//   |          host        |
//   +----------------------+
//   |    ns-3 simulation   |
//   +----------------------+
//   |      ns-3 Node       |
//   |  +----------------+  |
//   |  |    ns-3 LTP    |  |
//   |  +----------------+  |
//   |  |    ns-3 UDP/IP |  |
//   |  +----------------+  |
//   |  |   FdNetDevice  |  |
//   |--+----------------+--+
//   |       | eth0 |       |          +----------------+
//   |       +------+       |          | LTP server     |
//   |          |           |          | implementation |
//   +----------|-----------+          +----------------+
//              | real device                  |
//              |                              |
//              .---------(network) -----------.
//
/// To use this example:
//  1) You need to decide on a physical device on your real system, and either
//     overwrite the hard-configured device name below (eth0) or pass this
//     device name in as a command-line argument
//  2) The host device must be set to promiscuous mode
//     (e.g. "sudo ifconfig eth0 promisc")
//  3) Be aware that ns-3 will generate a fake mac address, and that in
//     some enterprise networks, this may be considered bad form to be
//     sending packets out of your device with "unauthorized" mac addresses
//  4) You will need to assign an IP address to the ns-3 simulation node that
//     is consistent with the subnet that is active on the host device's link.
//     That is, you will have to assign an IP address to the ns-3 node as if
//     it were on your real subnet.  Search for "Ipv4Address localIp" and
//     replace the string "1.2.3.4" with a valid IP address.
//  5) If you are sending to a server off of the local subnet, you will
//     need to configure a default route in the ns-3 node to tell it
//     how to get off of your subnet.  This example doesn't use a gateway
//     as presently written, however (see fd-emu-ping.cc for an example)
/// 6) Give root suid to the raw socket creator binary, or run as root
//     If the --enable-sudo option was used to configure ns-3 with waf, then the following
//     step will not be necessary.
//
//     $ sudo chown root.root build/src/fd-net-device/ns3-dev-raw-sock-creator
//     $ sudo chmod 4755 build/src/fd-net-device/ns3-dev-raw-sock-creator
//
// Some details on the LTP server assumptions:  this example was tested
// against LTPlib found at http://down.dsg.cs.tcd.ie/ltplib/
// The LTP server was run as:
//   ltpd -m S -L 10.0.0.2:1113 -S 10.0.0.1:1113 -o ltpd.server.out
// On the client side, the ns-3 configuration below was designed to
// approximate a client ltpd command of:
//   ltpd -w20 -i a.txt -m C -D 10.0.0.2:1113 -S 10.0.0.1:1113
// where the file 'a.txt' contained 1024x100 characters identically set to 'A'
//
// The ns-3 value of 1000ms for OneWayLightTime was conservative; the LTPlib
// assumes a 200ms RTT

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/fd-net-device-module.h"
#include "ns3/applications-module.h"
#include "ns3/ltp-protocol.h"
#include "ns3/ltp-protocol-helper.h"

using namespace ns3;
using namespace ltp;

NS_LOG_COMPONENT_DEFINE ("LtpEmulationExample");

// Global variables for use in callbacks

Ptr<Node> node;

struct StartTransmissionParameters
{
  uint64_t clientServiceId;
  uint64_t ltpEngineId;
  std::vector<uint8_t> data;
  uint32_t redPartSize;
} params;

// This callback will launch the LTP data transfer once a ICMP response is
// received
void
RttTracer (Time receivedRtt)
{
  NS_LOG_DEBUG ("Received ping response at " << Simulator::Now ().GetSeconds ());
  node->GetObject<LtpProtocol> ()->StartTransmission (params.clientServiceId, params.clientServiceId, params.ltpEngineId, params.data, params.redPartSize);
}

void
ClientServiceInstanceNotificationsSend (SessionId id,
                                        StatusNotificationCode code,
                                        std::vector<uint8_t> data,
                                        uint32_t dataLength,
                                        bool endFlag,
                                        uint64_t srcLtpEngine,
                                        uint32_t offset )
{
  NS_LOG_DEBUG ("ClientServiceNotification - Session ID: " << id.GetSessionNumber () << " Code : " << code);
}

int
main (int argc, char *argv[])
{
  NS_LOG_INFO ("Ltp Emulation Example");

  // If you are running with RTT greater than 2 seconds, increase the
  // below default value further
  Config::SetDefault ("ns3::ArpCache::WaitReplyTimeout", TimeValue (Seconds (3)));
  std::string deviceName ("eth0");  // set to your local emulation interface
  std::string remote ("10.0.0.2");  // set to destination LTP server IP address
  uint8_t testNum = 0;  // default
  bool verbose = true;
  bool server = false;

  //
  // Allow the user to override any of the defaults at run-time, via
  // command-line arguments
  //
  CommandLine cmd;
  cmd.AddValue ("deviceName", "Device name", deviceName);
  cmd.AddValue ("remote", "Remote IP address (dotted decimal only please)", remote);
  cmd.AddValue ("testNum", " Test to perform", testNum);
  cmd.AddValue ("verbose", "Verbose log if true", verbose);
  cmd.AddValue ("server", "Run as server mode", server);
  cmd.Parse (argc, argv);

  if (verbose)
    {
      LogComponentEnable ("LtpProtocol", LOG_LEVEL_ALL);
      LogComponentEnable ("LtpUdpConvergenceLayerAdapter", LOG_LEVEL_ALL);
      LogComponentEnable ("Ipv4L3Protocol", LOG_LEVEL_ALL);
      LogComponentEnable ("UdpSocketImpl", LOG_LEVEL_ALL);
      LogComponentEnable ("UdpL4Protocol", LOG_LEVEL_ALL);
    }

  uint32_t blockSize;
  uint32_t redPartSize;
  if (server == false)
    {  
      // these test numbers support emulation studies by varying the block
      // size and red part size
      switch (testNum)
        {
        case '1':
          blockSize = 500;
          redPartSize = blockSize;
          break;
        case '2':
          blockSize = 5000;
          redPartSize = blockSize;
          break;
        case '3':
          blockSize = 500;
          redPartSize = 200;
          break;
        case '4':
          blockSize = 5000;
          redPartSize = 2000;
          break;
        case '5':
          blockSize = 500;
          redPartSize = 0;
          break;
        case '6':
          blockSize = 5000;
          redPartSize = 0;
          break;
        default:
          blockSize = 1024 * 100;
          redPartSize = blockSize;
        }
    }
    
  Ipv4Address remoteIp (remote.c_str ());
  Ipv4Address localIp ("1.2.3.4");
  Ipv4Mask localMask ("255.255.255.0");

  NS_ABORT_MSG_IF (localIp == "1.2.3.4", "You must change the local IP address before running this example");

  //
  // Since we are using a real piece of hardware we need to use the realtime
  // simulator.
  //
  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  //
  // Since we are going to be talking to real-world machines, we need to enable
  // calculation of checksums in our protocols.
  //
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  //
  // In such a simple topology, the use of the helper API can be a hindrance
  // so we drop down into the low level API and do it manually.
  //
  // First we need a single node.
  //
  NS_LOG_INFO ("Create Node");
  node = CreateObject<Node> ();

  //
  // Create an emu device, allocate a MAC address and point the device to the
  // Linux device name.  The device needs a transmit queueing discipline so
  // create a droptail queue and give it to the device.  Finally, "install"
  // the device into the node.
  //
  // Understand that the ns-3 allocated MAC address will be sent out over
  // your network since the emu net device will spoof it.  By default, this
  // address will have an Organizationally Unique Identifier (OUI) of zero.
  // The Internet Assigned Number Authority IANA
  //
  //  http://www.iana.org/assignments/ethernet-numbers
  //
  // reports that this OUI is unassigned, and so should not conflict with
  // real hardware on your net.
  //
  NS_LOG_INFO ("Create Device");
  EmuFdNetDeviceHelper emu;
  emu.SetDeviceName (deviceName);
  NetDeviceContainer devices = emu.Install (node);
  Ptr<NetDevice> device = devices.Get (0);
  device->SetMtu (1500);
  device->SetAttribute ("Address", Mac48AddressValue (Mac48Address::Allocate ()));

  //
  // Add a default internet stack to the node.  This gets us the ns-3 versions
  // of ARP, IPv4, ICMP, UDP and TCP.
  //
  NS_LOG_INFO ("Add Internet Stack");
  InternetStackHelper internetStackHelper;
  internetStackHelper.Install (node);

  NS_LOG_INFO ("Create IPv4 Interface");
  Ptr<ns3::Ipv4> ipv4 = node->GetObject<ns3::Ipv4> ();
  uint32_t interface = ipv4->AddInterface (device);
  Ipv4InterfaceAddress address = Ipv4InterfaceAddress (localIp, localMask);
  ipv4->AddAddress (interface, address);
  ipv4->SetMetric (interface, 1);
  ipv4->SetUp (interface);

  // Begin LTP configuration.  Define the ClientService ID Code of the
  // Client Service Instance that will be using the Ltp protocol.
  uint64_t ClientServiceId;
  uint64_t remotePeer;
  if (server == false)
    {
      // ns-3 values
      ClientServiceId = 0;  // Bundle
      remotePeer = 1; // LTP engine id of remote host
    }
  else
    {
      // LTPlib values
      ClientServiceId = 1113;  // Bundle
      remotePeer = 167772162; // LTP engine id of remote host
    }
  
  // Create a LtpIpResolution table to perform mappings between Ipv4 adresses and LtpEngineIDs
  Ptr<LtpIpResolutionTable> routing =  CreateObjectWithAttributes<LtpIpResolutionTable> ("Addressing", StringValue ("Ipv4"));
  // Port 1113 reserved for LTP
  routing->AddBinding (remotePeer, remoteIp, 1113);

  // Use a helper to create and install Ltp Protocol instances in the nodes.
  // Make sure that the OneWayLightTime exceeds the real RTT; otherwise,
  // spurious transmissions may result
  LtpProtocolHelper ltpHelper;
  ltpHelper.SetAttributes ("CheckPointRtxLimit",  UintegerValue (20),
                           "ReportSegmentRtxLimit", UintegerValue (20),
                           "RetransCyclelimit",  UintegerValue (20),
                           "OneWayLightTime", TimeValue (MilliSeconds (100))
                           );
  ltpHelper.SetLtpIpResolutionTable (routing);
  ltpHelper.SetBaseLtpEngineId (0);
  ltpHelper.SetStartTransmissionTime (Seconds (1));
  ltpHelper.InstallAndLink (node, remotePeer);


  // Register ClientServiceInstances with the corresponding Ltp Engine of each node
  CallbackBase cb = MakeCallback (&ClientServiceInstanceNotificationsSend);
  node->GetObject<LtpProtocol> ()->RegisterClientService (ClientServiceId,cb);
  //
  // Enable a promiscuous pcap trace to see what is coming and going on our device.
  //
  emu.EnablePcap ("ltp-emu", device, true);

  if (server == false)
    {
      // Simulator will schedule transmission once a ping reply is received
      // Set the global variables that need to be accessed by the callback
      params.clientServiceId = ClientServiceId;
      params.ltpEngineId = 1;
      // '65' is ASCII character 'A'
      params.data = std::vector<uint8_t> ( blockSize, 65);
      params.redPartSize = redPartSize;
      NS_LOG_DEBUG ("Block size: " << params.data.size () << " Red prefix size: " << redPartSize);

      Ptr<V4Ping> v4ping = CreateObject<V4Ping> ();
      v4ping->SetAttribute ("Remote", Ipv4AddressValue (Ipv4Address (remote.c_str ())));
      // A value of 60 will cause only one ping to occur during a 30 second
      // simulation (i.e. this approximates the '-c' count option of the
      // real ping program, which is not a supported option in ns-3
      v4ping->SetAttribute ("Interval", TimeValue (Seconds (60)));
      node->AddApplication (v4ping);
      v4ping->SetStartTime (MilliSeconds (500));
      v4ping->TraceConnectWithoutContext ("Rtt", MakeCallback (&RttTracer));
     }

  //
  // Now, do the actual emulation.
  //
  NS_LOG_INFO ("Run Emulation.");
  Simulator::Stop (Seconds (40.0));
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
