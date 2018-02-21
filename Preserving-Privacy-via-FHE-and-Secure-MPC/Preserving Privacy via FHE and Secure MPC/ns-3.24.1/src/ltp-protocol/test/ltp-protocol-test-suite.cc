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
#include "ns3/ltp-queue-set.h"
#include "ns3/ltp-session-state-record.h"
#include "ns3/ltp-protocol.h"
#include "ns3/random-variable-stream.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/sdnv.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/buffer.h"
#include "ns3/test.h"

using namespace ns3;
//using namespace ltp;

NS_LOG_COMPONENT_DEFINE ("LtpProtocolTests");


class LtpHeaderTestCase : public TestCase
{
public:
  LtpHeaderTestCase ();
  virtual ~LtpHeaderTestCase ();

private:
  virtual void DoRun (void);

  void SetSessionIds ();
  void SetExtensions ();
  void SetHeaderTests ();
  void SetTrailerTests ();
  void SetContentHeaderTests ();

  void SerializeTests ();



  template <class T>
  class TestVector
  {
public:
    uint8_t m_expectedEncodedSz;
    T m_data;
  };

  TestVectors<TestVector<SessionId> > m_sessionIds;
  TestVectors<TestVector<LtpExtension> > m_extensions;
  TestVectors<TestVector<LtpHeader> > m_mainHeaderTests;
  TestVectors<TestVector<LtpTrailer> > m_mainTrailerTests;
  TestVectors<TestVector<LtpContentHeader> > m_mainContentHeaderTests;


};

// Add some help text to this case to describe what it is intended to test
LtpHeaderTestCase::LtpHeaderTestCase ()
  : TestCase ("LtpHeaderTestCase test case (checks serialization and deserialization methods)")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
LtpHeaderTestCase::~LtpHeaderTestCase ()
{
}

/* Instantiate several SessionIds and save its expected encoded size */
void
LtpHeaderTestCase::SetSessionIds ()
{

  TestVector<SessionId> test;

  SessionId session (0,0);
  test.m_expectedEncodedSz = 2;
  test.m_data = session;
  m_sessionIds.Add (test);

  session.SetSessionOriginator (0x4234);
  session.SetSessionNumber (0x1234);
  test.m_expectedEncodedSz = 5;
  test.m_data = session;
  m_sessionIds.Add (test);

  session.SetSessionOriginator (0xFFFF);
  session.SetSessionNumber (0x7F);
  test.m_expectedEncodedSz = 4;
  test.m_data = session;
  m_sessionIds.Add (test);

}

/* Instantiate several LtpExtensions and save its expected serialized size */
void
LtpHeaderTestCase::SetExtensions ()
{
  TestVector<LtpExtension> test;
  uint8_t extensionSize = 10;

  LtpExtension extension;
  extension.SetExtensionType (LtpExtension::LTPEXT_AUTH);
  for (int i = 0; i < extensionSize; i++)
    {
      extension.AddExtensionData (0);    // Data Filled with zeros

    }
  // Type (1 byte) + SDNV Encoded Len (1 byte) + Data Size (extensionSize Bytes)
  test.m_expectedEncodedSz = 1 + 1 + extensionSize;
  test.m_data = extension;
  m_extensions.Add (test);

  extensionSize = 0x80;
  extension.SetExtensionType (LtpExtension::LTPEXT_COOKIE);
  extension.ClearExtensionData ();
  for (int i = 0; i < extensionSize; i++)
    {
      extension.AddExtensionData (0);    // Data Filled with zeros

    }
  // Type (1 byte) + SDNV Encoded Len (2 bytes) + Data Size (extensionSize Bytes)
  test.m_expectedEncodedSz = 1 + 2 + extensionSize;
  test.m_data = extension;
  m_extensions.Add (test);

}

void
LtpHeaderTestCase::SetHeaderTests ()
{
  LtpHeader header;
  TestVector<LtpHeader> test;
  uint8_t version = 0;
  SegmentType type = LTPTYPE_RD;
  uint8_t extensionCntHeader = 0b00000000;
  uint8_t extensionCntTrailer = 0b00000000;

  /* Simple Header No Extensions*/
  header.SetVersion (version);
  header.SetSegmentType (type);
  header.SetSessionId ( (SessionId)  m_sessionIds.Get (0).m_data);
  header.SetHeaderExtensionCount (extensionCntHeader);
  header.SetTrailerExtensionCount (extensionCntTrailer);

  test.m_data = header;
  test.m_expectedEncodedSz = 2 + m_sessionIds.Get (0).m_expectedEncodedSz;
  m_mainHeaderTests.Add (test);

  /* Header with 1 Extension */
  header.SetSessionId ( (SessionId)  m_sessionIds.Get (1).m_data);
  header.AddExtension (m_extensions.Get (0).m_data);

  test.m_data = header;
  test.m_expectedEncodedSz = 2 +
    m_sessionIds.Get (1).m_expectedEncodedSz +  // 5
    m_extensions.Get (0).m_expectedEncodedSz;   // 12

  m_mainHeaderTests.Add (test);

  /* Header with 2 Extensions */
  header.SetSessionId ( (SessionId)  m_sessionIds.Get (2).m_data);
  header.AddExtension (m_extensions.Get (1).m_data);

  test.m_data = header;
  test.m_expectedEncodedSz = 2 +
    m_sessionIds.Get (2).m_expectedEncodedSz +   // 4
    m_extensions.Get (0).m_expectedEncodedSz +   // 12
    m_extensions.Get (1).m_expectedEncodedSz;    // 131

  m_mainHeaderTests.Add (test);

}

/* Ltp trailer tests */
void
LtpHeaderTestCase::SetTrailerTests ()
{
  LtpTrailer trailer;

  trailer.AddExtension (m_extensions.Get (0).m_data);
  trailer.AddExtension (m_extensions.Get (0).m_data);
  trailer.AddExtension (m_extensions.Get (1).m_data);
  trailer.AddExtension (m_extensions.Get (0).m_data);

  TestVector<LtpTrailer> test;

  test.m_data = trailer;
  test.m_expectedEncodedSz =  3 * m_extensions.Get (0).m_expectedEncodedSz +       // 3*12
    m_extensions.Get (1).m_expectedEncodedSz;                                      // 131

  m_mainTrailerTests.Add (test);

}

void
LtpHeaderTestCase::SetContentHeaderTests ()
{
  LtpContentHeader header;
  TestVector<LtpContentHeader> test;
  SegmentType type = LTPTYPE_RD;
  uint64_t clientServiceId = 0;
  uint64_t offset = 0;
  uint64_t length = 0x7F;


  /* Test 1: Data Segment No CheckPoint Test */
  header.SetSegmentType (type);
  header.SetClientServiceId (clientServiceId);
  header.SetOffset (offset);
  header.SetLength (length);

  test.m_data = header;
  test.m_expectedEncodedSz = 1 + 1 + 1;
  m_mainContentHeaderTests.Add (test);

  /* Test 2: Report Segment */
  type = LTPTYPE_RS;
  uint64_t cpSerialNumber = 1;
  uint64_t rpSerialNumber = 1;
  uint64_t upperBound = 0x7F;
  uint64_t lowerBound = 0x0;

  header.SetSegmentType (type);
  header.SetCpSerialNumber (cpSerialNumber);
  header.SetRpSerialNumber (rpSerialNumber);
  header.SetUpperBound (upperBound);
  header.SetLowerBound (lowerBound);

  LtpContentHeader::ReceptionClaim claim;
  claim.offset = offset;
  claim.length = length;

  header.AddReceptionClaim (claim);

  test.m_data = header;
  test.m_expectedEncodedSz = 7;

  m_mainContentHeaderTests.Add (test);

  /* Test 3: Multiple Partial Claims Report Segment*/
  type = LTPTYPE_RS;
  upperBound = 6000;       // 2
  lowerBound = 1000;       // 2

  header.SetSegmentType (type);
  header.SetCpSerialNumber (cpSerialNumber);
  header.SetRpSerialNumber (rpSerialNumber);
  header.SetUpperBound (upperBound);
  header.SetLowerBound (lowerBound);
  header.ClearReceptionClaims ();

  claim.offset = 0;
  claim.length = 2000;       // 2
  header.AddReceptionClaim (claim);

  claim.offset = 3000;       // 2
  claim.length = 500;       // 2
  header.AddReceptionClaim (claim);

  test.m_data = header;
  test.m_expectedEncodedSz = 7 + 7;

  m_mainContentHeaderTests.Add (test);

  /* Test 4: Data Segment Checkpoint */
  type = LTPTYPE_RD_CP_EORP;
  header.SetSegmentType (type);

  test.m_data = header;
  test.m_expectedEncodedSz = 5;

  m_mainContentHeaderTests.Add (test);

  /* Test 5: Report ACK Segment */
  type =  LTPTYPE_RAS;
  header.SetSegmentType (type);

  test.m_data = header;
  test.m_expectedEncodedSz = 1;

  m_mainContentHeaderTests.Add (test);

  /* Test 6: Cancel Segment */
  type = LTPTYPE_CS;
  header.SetSegmentType (type);

  test.m_data = header;
  test.m_expectedEncodedSz = 1;

  m_mainContentHeaderTests.Add (test);

  /* Test 7: Cancel ACK Segment */
  type = LTPTYPE_CAS;
  header.SetSegmentType (type);

  test.m_data = header;
  test.m_expectedEncodedSz = 0;

  m_mainContentHeaderTests.Add (test);

}

//
// This method is the pure virtual method from class TestCase that every
// TestCase must implement
//
void
LtpHeaderTestCase::DoRun (void)
{
  SetSessionIds ();
  SetExtensions ();
  SetHeaderTests ();
  SetTrailerTests ();
  SetContentHeaderTests ();

  for (uint32_t i = 0; i < m_mainHeaderTests.GetN (); i++)
    {
      TestVector<LtpHeader> test  = m_mainHeaderTests.Get (i);

      uint32_t actual = test.m_data.GetSerializedSize ();
      uint32_t limit = test.m_expectedEncodedSz;
      NS_TEST_ASSERT_MSG_EQ (actual, limit, "wrong header serialization size");

      Buffer buf;
      buf.AddAtStart (test.m_data.GetSerializedSize ());
      test.m_data.Serialize (buf.Begin ());

      LtpHeader header;
      header.Deserialize (buf.Begin ());
      NS_TEST_ASSERT_MSG_EQ ((header == test.m_data), true, "Header serialization methods failed");

    }

  for (uint32_t i = 0; i < m_mainTrailerTests.GetN (); i++)
    {
      TestVector<LtpTrailer> test  = m_mainTrailerTests.Get (i);

      uint32_t actual = test.m_data.GetSerializedSize ();
      uint32_t limit = test.m_expectedEncodedSz;
      NS_TEST_ASSERT_MSG_EQ (actual, limit, "wrong trailer serialization size");

      Buffer buf;
      buf.AddAtStart (test.m_data.GetSerializedSize ());
      test.m_data.Serialize (buf.Begin ());

      LtpTrailer trailer;
      trailer.Deserialize (buf.Begin ());
      NS_TEST_ASSERT_MSG_EQ ((trailer == test.m_data), true, "Trailer serialization methods failed");


    }

  for (uint32_t i = 0; i < m_mainContentHeaderTests.GetN (); i++)
    {
      TestVector<LtpContentHeader> test  = m_mainContentHeaderTests.Get (i);

      uint32_t actual = test.m_data.GetSerializedSize ();
      uint32_t limit = test.m_expectedEncodedSz;
      NS_TEST_ASSERT_MSG_EQ (actual, limit, "wrong trailer serialization size");

      Buffer buf;
      buf.AddAtStart (test.m_data.GetSerializedSize ());
      test.m_data.Serialize (buf.Begin ());

      LtpContentHeader content;
      content.SetSegmentType (test.m_data.GetSegmentType ());
      content.Deserialize (buf.Begin ());
      NS_TEST_ASSERT_MSG_EQ ((content == test.m_data), true, "Content Header serialization methods failed");
    }
}

class LtpQueueSetTestCase : public TestCase
{
public:
  LtpQueueSetTestCase ();
  virtual ~LtpQueueSetTestCase ();

private:
  virtual void DoRun (void);

  void SetTests ();

  template <class T>
  class TestVector
  {
public:
    uint8_t m_position;
    T m_data;
  };

  TestVectors<TestVector<Ptr<ns3::Packet> > > m_tests;
};

LtpQueueSetTestCase::LtpQueueSetTestCase ()
  : TestCase ("LtpQueueSetTestCase test case (check queues behaviour)")
{
}
LtpQueueSetTestCase::~LtpQueueSetTestCase ()
{
}

void
LtpQueueSetTestCase::SetTests ()
{
  Ptr<ns3::Packet> packet1 = Create<ns3::Packet> ();
  LtpHeader header;
  TestVector<Ptr<ns3::Packet> > test;

  /* Simple Header No Extensions*/
  header.SetVersion (0);
  header.SetSegmentType (LTPTYPE_RD);
  header.SetSessionId ( SessionId (0,0));
  header.SetHeaderExtensionCount (0b00000000);
  header.SetTrailerExtensionCount (0b00000000);

  /* Data Segment have low priority they should be returned in
   * following the same order of insertion */
  packet1->AddHeader (header);

  test.m_position = 2;
  test.m_data = packet1;
  m_tests.Add (test);

  header.SetSegmentType (LTPTYPE_GD);
  Ptr<ns3::Packet> packet2 = Create<ns3::Packet> ();
  packet2->AddHeader (header);

  test.m_position = 3;
  test.m_data = packet2;
  m_tests.Add (test);

  /* Report packets have higher priority they should be the first ones to be dequed*/
  header.SetSegmentType (LTPTYPE_RS);
  Ptr<ns3::Packet> packet3 = Create<ns3::Packet> ();
  packet3->AddHeader (header);

  test.m_position = 1;
  test.m_data = packet3;
  m_tests.Add (test);

}
void
LtpQueueSetTestCase::DoRun (void)
{
  LtpQueueSet queue;
  bool success = true;

  SetTests ();

  for (uint32_t i = 0; i < m_tests.GetN (); i++)
    {
      TestVector<Ptr<ns3::Packet> > test = m_tests.Get (i);
      success = success & queue.Enqueue (test.m_data);
    }
  /* Test 1: Check that all packets have been queued succesfully */
  NS_TEST_ASSERT_MSG_EQ (success,true, "Enqueuing failed");

  /* Test 2: Check size*/
  NS_TEST_ASSERT_MSG_EQ ( (queue.GetNPackets () == m_tests.GetN ()), true, "Wrong queue size");

  /* Test 3: Check that returned packets are ordered by priority */
  Ptr<ns3::Packet> packet = queue.Dequeue ();
  NS_TEST_ASSERT_MSG_EQ ((m_tests.Get (2).m_data == packet), true, "Wrong queue order");
  packet = queue.Dequeue ();
  NS_TEST_ASSERT_MSG_EQ ((m_tests.Get (0).m_data == packet), true, "Wrong queue order");
  packet = queue.Dequeue ();
  NS_TEST_ASSERT_MSG_EQ ((m_tests.Get (1).m_data == packet), true, "Wrong queue order");

}

class LtpSessionStateRecordTestCase : public TestCase
{
public:
  LtpSessionStateRecordTestCase ();
  virtual ~LtpSessionStateRecordTestCase ();

private:
  virtual void DoRun (void);

  void SetTimerTests ();
  void TimerTest (uint32_t);
  void ResumeTimers (uint32_t);


  struct TestTimer
  {
    uint64_t lapse;
    uint64_t total;
    uint8_t stops;
    uint8_t stop_lapses;
    TimerCode timeCode;
  };

  TestVectors<TestTimer> m_testTimers;

};

LtpSessionStateRecordTestCase::LtpSessionStateRecordTestCase ()
  : TestCase ("LtpSessionStateRecordTestCase test case (check queues behaviour)")
{
}
LtpSessionStateRecordTestCase::~LtpSessionStateRecordTestCase ()
{
}

void
LtpSessionStateRecordTestCase::TimerTest (uint32_t index)
{
  TestTimer test = m_testTimers.Get (index);



  uint64_t actual = Simulator::Now ().GetSeconds ();
  uint64_t limit = test.total;
  uint64_t tol = 0.005;

  NS_TEST_ASSERT_MSG_EQ_TOL (actual, limit, tol, "Test1 Failed");
}


void
LtpSessionStateRecordTestCase::DoRun (void)
{

  Ptr<UniformRandomVariable> sessionNumber = CreateObject<UniformRandomVariable> ();
  sessionNumber->SetAttribute ("Min", DoubleValue(SessionId::MIN_SESSION_NUMBER));
  sessionNumber->SetAttribute ("Max", DoubleValue(SessionId::MAX_SESSION_NUMBER));

  Ptr<UniformRandomVariable> serialNumber = CreateObject<UniformRandomVariable> ();
  serialNumber->SetAttribute ("Min", DoubleValue(SessionStateRecord::MIN_INITIAL_SERIAL_NUMBER));
  serialNumber->SetAttribute ("Max", DoubleValue(SessionStateRecord::MAX_INITIAL_SERIAL_NUMBER));

  uint64_t destEngine = 5000;
  uint64_t srcEngine = 5000;
  uint64_t destClient = 5000;
  uint64_t srcClient = 5000;
  Ptr<SenderSessionStateRecord> ssend = CreateObject<SenderSessionStateRecord> (srcEngine, srcClient, destEngine,destClient, sessionNumber,serialNumber);

  /* Test 1 : Check Session Id generation */
  SessionId id = ssend->GetSessionId ();

  bool test = ((id.GetSessionNumber () >= SessionId::MIN_SESSION_NUMBER)
               && (id.GetSessionNumber () <= SessionId::MAX_SESSION_NUMBER)
               && id.GetSessionOriginator () == srcEngine);

  NS_TEST_ASSERT_MSG_EQ (test,true,"Wrong result for sessionId generation");

  Ptr<ReceiverSessionStateRecord> srecv = CreateObject<ReceiverSessionStateRecord> (srcEngine,srcClient,id, serialNumber);

  SetTimerTests ();

  /* Test 2: Timers - Suspend and Resume */
  for (uint32_t i = 0; i < m_testTimers.GetN (); i++)
    {
      TestTimer test = m_testTimers.Get (i);

      srecv->SetTimerFunction (&LtpSessionStateRecordTestCase::TimerTest, this, i, Seconds (test.lapse),test.timeCode);
      srecv->StartTimer (test.timeCode);

      for (uint32_t i = 0; i < test.stops; i++)
        {

          Simulator::Schedule (Seconds (test.stop_lapses * i),&ReceiverSessionStateRecord::SuspendTimer, srecv, test.timeCode);
          Simulator::Schedule (Seconds (test.stop_lapses * (i + 1)),&ReceiverSessionStateRecord::ResumeTimer, srecv, test.timeCode);
        }

    }
  /* Test 3: Check Claims */
  LtpContentHeader::ReceptionClaim claim;

  claim.offset = 0;
  claim.length = 100;

  uint32_t lowerBound = 0;
  uint32_t upperBound = 1000;

  test = srecv->InsertClaim (srecv->GetRpCurrentSerialNumber (), lowerBound, upperBound, claim);
  NS_TEST_ASSERT_MSG_EQ (test, true, "First Claim not inserted");
  test = srecv->InsertClaim (srecv->GetRpCurrentSerialNumber (), lowerBound, upperBound, claim);
  NS_TEST_ASSERT_MSG_EQ (test, false, "Claim with repeated offset inserted");

  claim.offset = 100;
  claim.length = 100;

  test = srecv->InsertClaim (srecv->GetRpCurrentSerialNumber (), lowerBound, upperBound, claim);
  NS_TEST_ASSERT_MSG_EQ (test, true, "New Claim not inserted");
  test = srecv->InsertClaim (srecv->GetRpCurrentSerialNumber (), lowerBound, upperBound, claim);
  NS_TEST_ASSERT_MSG_EQ (test, false,"Claim with repeated offset inserted");

  srecv->IncrementRpCurrentSerialNumber ();
  test = srecv->InsertClaim (srecv->GetRpCurrentSerialNumber (), lowerBound, upperBound, claim);
  NS_TEST_ASSERT_MSG_EQ (test, false, "Claim was already included in the previous Report serial number");

  claim.offset = 200;
  claim.length = 100;
  test = srecv->InsertClaim (srecv->GetRpCurrentSerialNumber (), lowerBound, upperBound, claim);
  NS_TEST_ASSERT_MSG_EQ (test, true, "New claim with new serial number not inserted");

  // Test 4: Check Serial Numbers
  test = ((srecv->GetRpCurrentSerialNumber () < ReceiverSessionStateRecord::MAX_SERIAL_NUMBER + 1)
          &&
          (srecv->GetRpCurrentSerialNumber () > 1));

  NS_TEST_ASSERT_MSG_EQ (test,true,"Wrong RP Serial Number");

  test = ((ssend->GetCpCurrentSerialNumber () < ReceiverSessionStateRecord::MAX_SERIAL_NUMBER)
          &&
          (ssend->GetCpCurrentSerialNumber () > 1));

  NS_TEST_ASSERT_MSG_EQ (test,true,"Wrong CP Serial Number");

  Simulator::Run ();
  Simulator::Destroy ();
}

void
LtpSessionStateRecordTestCase::SetTimerTests ()
{

  TestTimer test;
  test.lapse = 1;
  test.stops = 0;
  test.stop_lapses = 0;
  test.total = test.lapse + ( test.stop_lapses * test.stops );
  test.timeCode = CHECKPOINT;

  m_testTimers.Add (test);

  test.lapse = 2;
  test.stops = 1;
  test.stop_lapses = 1;
  test.total = test.lapse + ( test.stop_lapses * test.stops );
  test.timeCode = REPORT;

  m_testTimers.Add (test);

  test.lapse = 4;
  test.stops = 3;
  test.stop_lapses = 2;
  test.total = test.lapse + ( test.stop_lapses * test.stops );
  test.timeCode = CANCEL;

  m_testTimers.Add (test);
}

class LtpProtocolAPITestCase : public TestCase
{
public:
  LtpProtocolAPITestCase ();
  virtual ~LtpProtocolAPITestCase ();

  void ClientServiceIntanceNotifications (SessionId id,
                                          StatusNotificationCode code,
                                          std::vector<uint8_t> data,
                                          uint32_t dataLength,
                                          bool endFlag,
                                          uint64_t srcLtpEngine,
                                          uint32_t offset );

private:
  virtual void DoRun (void);

};

LtpProtocolAPITestCase::LtpProtocolAPITestCase ()
  : TestCase ("LtpProtocolAPITestCase test case (check protocol core)")
{
}
LtpProtocolAPITestCase::~LtpProtocolAPITestCase ()
{
}

void
LtpProtocolAPITestCase::ClientServiceIntanceNotifications (SessionId id,
                                                           StatusNotificationCode code,
                                                           std::vector<uint8_t> data,
                                                           uint32_t dataLength,
                                                           bool endFlag,
                                                           uint64_t srcLtpEngine,
                                                           uint32_t offset )
{
  NS_LOG_UNCOND (id.GetSessionNumber () << " " << code);
  NS_TEST_ASSERT_MSG_EQ ((code == SESSION_START),true,"Session Start Notification Failed");
}


void
LtpProtocolAPITestCase::DoRun (void)
{
  Ptr<LtpProtocol> prot = CreateObject<LtpProtocol> ();

  /* Test 1: Register Client Service*/
  uint64_t id = 304;
  CallbackBase cb = MakeCallback (&LtpProtocolAPITestCase::ClientServiceIntanceNotifications, this);

  bool test = prot->RegisterClientService (id, cb);
  NS_TEST_ASSERT_MSG_EQ (test,true,"New Client Service registration failed");
  test = prot->RegisterClientService (id, cb);
  NS_TEST_ASSERT_MSG_EQ (test,false,"Client Service registered twice");
  id = 305;
  test = prot->RegisterClientService (id, cb);
  NS_TEST_ASSERT_MSG_EQ (test,true,"New Client Service registration failed");

  /* Test 2 : Unregister Client Service */
  prot->UnregisterClientService (id);
  id = 304;

}


// The TestSuite class names the TestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run.  Typically, only the constructor for
// this class must be defined
//
class LtpProtocolTestSuite : public TestSuite
{
public:
  LtpProtocolTestSuite ();
};

LtpProtocolTestSuite::LtpProtocolTestSuite ()
  : TestSuite ("ltp-protocol", UNIT)
{
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new LtpHeaderTestCase, TestCase::QUICK);
  AddTestCase (new LtpQueueSetTestCase, TestCase::QUICK);
  AddTestCase (new LtpSessionStateRecordTestCase, TestCase::QUICK);
  AddTestCase (new LtpProtocolAPITestCase, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static LtpProtocolTestSuite ltpProtocolTestSuite;

