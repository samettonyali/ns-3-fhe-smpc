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
#include "ns3/ltp-header.h"
#include "ns3/sdnv.h"
#include "ns3/log.h"
#include "ns3/node-container.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/error-model.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/ltp-protocol.h"
#include "ns3/ltp-protocol-helper.h"
#include "ns3/test.h"

using namespace ns3;
//using namespace ltp;

NS_LOG_COMPONENT_DEFINE ("LtpProtocolRetransTestCase");

/*
 * This test checks the reliability of the LTP protocol
 * under channels with varying degree of packet losses.
 */
class LtpProtocolRetransTestCase : public TestCase
{
public:
  LtpProtocolRetransTestCase ();
  LtpProtocolRetransTestCase (uint32_t sentData, uint32_t expectedData, uint32_t redPartSz, std::list<uint32_t> listReceiver,std::list<uint32_t> listSender );
  virtual ~LtpProtocolRetransTestCase ();

  void ClientServiceInstanceNotificationsSnd (SessionId id,
                                              StatusNotificationCode code,
                                              std::vector<uint8_t> data,
                                              uint32_t dataLength,
                                              bool endFlag,
                                              uint64_t srcLtpEngine,
                                              uint32_t offset );
  void ClientServiceInstanceNotificationsRcv (SessionId id,
                                              StatusNotificationCode code,
                                              std::vector<uint8_t> data,
                                              uint32_t dataLength,
                                              bool endFlag,
                                              uint64_t srcLtpEngine,
                                              uint32_t offset );

private:

  virtual void DoRun (void);

  uint32_t m_sentData;	    // Size in bytes of sent data.
  uint32_t m_rcvData;		// Size in bytes of sent data.
  uint32_t m_expectedData;  // Size in bytes of the data which is expected to be received
  uint32_t m_redPartSz;		// Block suffix that corresponds to red data.

  std::list<uint32_t> m_lossesReceiver;		// List of packets that will be lost upon reception at the receiver
  std::list<uint32_t> m_lossesSender;		// List of packets that will be lost upon reception at the sender
};


LtpProtocolRetransTestCase::LtpProtocolRetransTestCase ()
  : TestCase ("LtpHeaderTestCase test case (checks serialization and deserialization methods)"),
    m_sentData (0),
    m_rcvData (0),
    m_expectedData (0),
    m_redPartSz (0),
    m_lossesReceiver (),
    m_lossesSender ()
{
}

LtpProtocolRetransTestCase::LtpProtocolRetransTestCase (uint32_t sentData, uint32_t expectedData, uint32_t redPartSz, std::list<uint32_t> listReceiver, std::list<uint32_t> listSender )
  : TestCase ("LtpHeaderTestCase test case (checks serialization and deserialization methods)"),
    m_sentData (sentData),
    m_rcvData (0),
    m_expectedData (expectedData),
    m_redPartSz (redPartSz),
    m_lossesReceiver (listReceiver),
    m_lossesSender (listSender)
{
}


LtpProtocolRetransTestCase::~LtpProtocolRetransTestCase ()
{
}

void
LtpProtocolRetransTestCase::ClientServiceInstanceNotificationsSnd (SessionId id,
                                                                   StatusNotificationCode code,
                                                                   std::vector<uint8_t> data,
                                                                   uint32_t dataLength,
                                                                   bool endFlag,
                                                                   uint64_t srcLtpEngine,
                                                                   uint32_t offset )
{
}

/* This method receives LTP protocol instance notifications.
 * It keeps track of the received amount of data across each call.
 * Finally, upon reception of the SESSION END notice, it checks
 * whether the received data size is equal to the expected data size.
 * */
void
LtpProtocolRetransTestCase::ClientServiceInstanceNotificationsRcv (SessionId id,
                                                                   StatusNotificationCode code,
                                                                   std::vector<uint8_t> data,
                                                                   uint32_t dataLength,
                                                                   bool endFlag,
                                                                   uint64_t srcLtpEngine,
                                                                   uint32_t offset )
{
  m_rcvData += dataLength;
  std::cout << "Code : " << code << " " << dataLength << " " << m_rcvData << " " << endFlag << std::endl;

  if (code == ns3::SESSION_END)
    {
      std::cout << "Total: " <<  m_rcvData << std::endl;
      NS_TEST_ASSERT_MSG_EQ (m_rcvData, m_expectedData, "Wrong amount of data received");
    }
}

//
// This method sets up two nodes with the following network topology
//
//       n0              n1
//       |               |
//       =================
//          PointToPoint
//
// The nodes use the LTP protocol to transmit a block of data from n0 to n1
// it defines a ReceiveListErrorModel that causes several transmission losses.
// The size of the block and the number of lost segments are provided in the
// parametized constructor of the class.

void
LtpProtocolRetransTestCase::DoRun (void)
{
  // Create the nodes required by the topology (shown above).
  NodeContainer nodes;
  nodes.Create (2);

  TimeValue channelDelay = TimeValue(MilliSeconds(5));

  // Create point to point links and instell them on the nodes
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("500Kbps"));
  pointToPoint.SetChannelAttribute ("Delay", channelDelay);

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  Ptr<ReceiveListErrorModel> errors_sender = CreateObject<ReceiveListErrorModel> ();
  Ptr<ReceiveListErrorModel> errors_receiver = CreateObject<ReceiveListErrorModel> ();

  errors_sender->SetList (m_lossesSender);
  devices.Get (0)->SetAttribute ("ReceiveErrorModel", PointerValue (errors_sender));

  errors_receiver->SetList (m_lossesReceiver);
  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (errors_receiver));

  // Install the internet stack on the nodes
  InternetStackHelper internet;
  internet.Install (nodes);

  // Assign IPv4 addresses.
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  // Define the ClientService ID Code of the Client Service Instance that will be using the Ltp protocol.
  uint64_t ClientServiceId = 0;          // Bundle

  // Creta a LtpIpResolution table to perform mappings between Ipv4 adresses and LtpEngineIDs
  Ptr<LtpIpResolutionTable> routing =  CreateObjectWithAttributes<LtpIpResolutionTable> ("Addressing", StringValue ("Ipv4"));

  // Use a helper to create and install Ltp Protocol instances in the nodes.
  LtpProtocolHelper ltpHelper;
  ltpHelper.SetAttributes ("CheckPointRtxLimit",  UintegerValue (20),
                           "ReportSegmentRtxLimit", UintegerValue (20),
                           "RetransCyclelimit",  UintegerValue (20),
                           "OneWayLightTime", channelDelay,
                           "RandomSessionNum", StringValue("ns3::ConstantRandomVariable[Constant=100000]"),
                           "RandomSerialNum", StringValue("ns3::ConstantRandomVariable[Constant=10000]"));
  ltpHelper.SetLtpIpResolutionTable (routing);
  ltpHelper.SetBaseLtpEngineId (0);
  ltpHelper.SetStartTransmissionTime (Seconds (1));
  ltpHelper.InstallAndLink (nodes);


  // Register ClientServiceInstances with the corresponding Ltp Engine of each node
  CallbackBase cb = MakeCallback (&LtpProtocolRetransTestCase::ClientServiceInstanceNotificationsSnd,this);
  CallbackBase cb2 = MakeCallback (&LtpProtocolRetransTestCase::ClientServiceInstanceNotificationsRcv,this);
  nodes.Get (0)->GetObject<LtpProtocol> ()->RegisterClientService (ClientServiceId,cb);
  nodes.Get (1)->GetObject<LtpProtocol> ()->RegisterClientService (ClientServiceId,cb2);

  // Create a block of data
  std::vector<uint8_t> data ( m_sentData, 65);

  // Transmit if from n0 to n1.
  uint64_t receiverLtpId = nodes.Get (1)->GetObject<LtpProtocol> ()->GetLocalEngineId ();
  nodes.Get (0)->GetObject<LtpProtocol> ()->StartTransmission (ClientServiceId,ClientServiceId,receiverLtpId,data,m_redPartSz);

  Simulator::Run ();
  Simulator::Destroy ();
}

class LtpProtocolChannelLossTestSuite : public TestSuite
{
public:
  LtpProtocolChannelLossTestSuite ();
};

LtpProtocolChannelLossTestSuite::LtpProtocolChannelLossTestSuite ()
  : TestSuite ("ltp-channel-loss", UNIT)
{
  LogComponentEnable ("LtpProtocol", LOG_LEVEL_ALL);
 // LogComponentEnable ("SessionStateRecord", LOG_LEVEL_ALL);
  
  /* Block of 5000 bytes of data, mtu 1500
   * split in 5 data segments:
   *    - 1 Red Segment                 (Id : 0)    -- Receiver
   *    - 1 Red Segment + EORP + CP     (Id: 1)		-- Receiver
   *    - 2 Green Segments				(Id: 2,3)	-- Receiver
   *    - 1 Green Segment + EOB         (Id: 4)		-- Receiver
   *
   *    Additionally the transmission generates 2 additional control segments:
   *
   *    - 1 Report Segment.				(Id: 0)     -- Sender
   *    - 1 Report ACK Segment.			(Id: 5)		-- Receiver
   *
   *    */

  uint32_t block_len = 5000;
  uint32_t red_len = 1500;
  uint32_t expected = 5000;

  std::list<uint32_t> receiver_losses;
  std::list<uint32_t> sender_losses;


  // Test 1: No losses
 AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

  // Test 2: Red Segment lost
  receiver_losses.push_back (0);
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

  // Test 3: Red Segment CP lost
  receiver_losses.clear ();
  receiver_losses.push_back (1);
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

  // Test 4: Both red segments are lost.
  receiver_losses.clear ();
  receiver_losses.push_back (0);
  receiver_losses.push_back (1);
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

  // Test 5: Report segment is lost.
  receiver_losses.clear ();
  sender_losses.push_back (0);
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

  //Test 6: Report ACK segment is lost
  receiver_losses.clear ();
  receiver_losses.push_back (5);
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

  // Test 7: Both Report segment and ACK are lost.
  receiver_losses.clear ();
  receiver_losses.push_back (5);
  sender_losses.clear ();
  sender_losses.push_back (0);
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

  //Test 8: A Green Segment is lost
  sender_losses.clear ();
  receiver_losses.clear ();
  receiver_losses.push_back (2);
  expected = 3539;
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

  //Test 9: Two Green Segments are lost
  receiver_losses.clear ();
  receiver_losses.push_back (2);
  receiver_losses.push_back (3);
  expected = 2078;
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

  //Test 10: A Red and Green Segment are lost
  receiver_losses.clear ();
  receiver_losses.push_back (0);
  receiver_losses.push_back (3);
  expected = 3539;
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

  //Test 11: A Checkpoint Segment and a Green Segment are lost
  receiver_losses.clear ();
  receiver_losses.push_back (1);
  receiver_losses.push_back (3);
  expected = 3539;
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

  //Test 12: Two Red Segments and a Two green Segments are lost
  receiver_losses.clear ();
  receiver_losses.push_back (0);
  receiver_losses.push_back (1);
  receiver_losses.push_back (2);
  receiver_losses.push_back (3);
  expected = 2078;
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

  //Test 13: A Green Segment and a Report segment are lost
  receiver_losses.clear ();
  receiver_losses.push_back (2);
  sender_losses.clear ();
  sender_losses.push_back (0);
  expected = 3539;
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

  
  /* CCDS 743.1-R2: 3.4.2 Notes: Green-part data is not reliably transmitted under LTP. In particular, if there is green-
          part data in a block, the LTP segment containing the EOB marker may not be
          delivered to the destination. As a consequence, if the session contains green-part data
          there is no way for an LTP sender to know when an LTP receiver has closed a
          session.*/

  //Test 14: A Green EOB Segment is lost -- Session in receiver side remains in a waiting for EOB state, session is closed by inactivity time-out
  receiver_losses.clear ();
  receiver_losses.push_back (4);
  expected = 4422;
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

  // Test 15: Full Red Part No Loss
  red_len = 5000;
  expected = 5000;
  receiver_losses.clear ();
  sender_losses.clear ();
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

 
  // Test 16: Full Red Part - Red Segments are lost
  red_len = 5000;
  expected = 5000;
  receiver_losses.clear ();
  sender_losses.clear ();
  receiver_losses.push_back (0);
  receiver_losses.push_back (1);

  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);


  // Test 17: Full Green Part.
  red_len = 0;
  expected = 5000;
  receiver_losses.clear ();
  sender_losses.clear ();
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

  // Test 18: Full Green Part - Green Segments are lost
  red_len = 0;
  expected = 2078;
  receiver_losses.clear ();
  sender_losses.clear ();
  receiver_losses.push_back (1);
  receiver_losses.push_back (2);
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

  // Test 19: Two Red Segments + Two green Segments + Report are lost
  sender_losses.clear ();
  sender_losses.push_back (0);
  receiver_losses.clear ();
  receiver_losses.push_back (0);
  receiver_losses.push_back (1);
  receiver_losses.push_back (2);
  receiver_losses.push_back (3);
  expected = 2078;
  AddTestCase (new LtpProtocolRetransTestCase (block_len, expected, red_len, receiver_losses, sender_losses), TestCase::QUICK);

}

// Do not forget to allocate an instance of this TestSuite
static LtpProtocolChannelLossTestSuite ltpProtocolTestSuite;



