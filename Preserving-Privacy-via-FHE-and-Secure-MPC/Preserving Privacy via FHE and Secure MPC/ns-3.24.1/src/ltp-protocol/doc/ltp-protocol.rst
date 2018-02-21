LTP Protocol
----------------------------

This model implements the base specification of Licklider Transmission Protocol (LTP). The
implementation is based on [rfc5325]_[rfc5326]_.

Model Description
******************

The source code for the new module lives in the directory ``src/ltp-protocol``.

The Licklider Transmission Protocol (LTP) is a transport protocol designed to
provide retransmission-based reliability over links with extremely long message round-trip times
and frequent disconnections. This is achieved through Automatic  Repeat reQuest (ARQ) of data
transmissions by soliciting selective-acknowledgment reception reports. LTP is intended to serve as a reliable
"convergence layer" protocol, underlying the Bundle protocol [BP], and single-hop deep-space radio frequency (RF) links.

Terminology
============

* LTP Engine : Software implementation of LTP running on sender or receiver
* Client Service Instance:  An application or a higher-layer protocol (typically Bundle Protocol) that is using LTP to transfer data.
* Block: Array of contiguous octets of application data handed down by the Client Service Instance for transmission.
* Segment:  The minimum unit of LTP data transmission activity.
* Red-Part: Portion of the block that needs to be sent in a reliable manner (requires ack and is subject to retransmissions).
* Green-Part: The block suffix that is to be transmitted unreliably.

Design
======

The core structure of LTP in |ns3| is shown below:

.. _fig-ltp-protocol-structure:

.. figure:: figures/ltp-protocol-structure.*
   :align: center

  The core structure of LTP implementation.

The area inside the box represents the main classes of LTP in |ns3|.

* Class ``ns3::ltp::LtpProtocol`` implements several LTP APIs to Client Service Instance, such as
  RegisterClientService (), UnregisterClientService (), StartTransmission () and CancelTransmission().
  Client Service Instance can use these APIs to register within the LTP for reception 
  and transmission of Blocks between LTP engines.

* Class ``ns3::ltp::LtpSessionStateRecord`` is a class to keep track of the state of an LTP transmission session,
  this class is basically a wrapper that handles several flags, counters, timers and lists. This is a base
  class that contains member variables common in both a sender and receiver session, two more specific classes
  extend this: ``ns3::ltp::SenderSessionStateRecord`` and ``ns3::ltp::ReceiverSessionStateRecord``.

* Class ``ns3::ltp::LtpQueueSet`` is a class that contains the dual queue structure (internal operations and application data) used for LTP
  outbound traffic.

* Class ``ns3::ltp::LtpIpResolutionTable`` is a class responsible for resolution of Ltp engine Ids to Internet addresses, supporting both
Ipv4 and Ipv6 formats. It provides a public API for managing bindings and perform conversions.

* Class ``ns3::ltp::LtpConvergenceLayerAdapter`` abstract base class that defines the APIs for convergence layers between lower layer
datalinks and the LTP protocol.

* Class ``ns3::ltp::LtpUdpConvergenceLayerAdapter`` this is the default convergence layer used in the [ns3] LTP protocol implementation. This class defines a
convergence layer adapter between LTP and UDP. This class is responsible for the following operations: mapping LTP generated segments
into UDP socket send() operations, mapping received UDP segments to the LTP receiver, opening a port on the IANA-assigned listening
port 1113, providing the LTP a way to determine the MTU to avoid fragmentation.

In addition to the classes used in the protocol core structures, the [ns3] LTP also includes several packet header and trailer structures:

* Class ``ns3::ltp::LtpHeader`` implements the primary LTP segment header common for all types of segments.

* Class ``ns3::ltp::LtpTrailer`` implements the  LTP segment trailer, this is primarily used for extensions.

* Class ``ns3::ltp::LtpContentHeader`` implements the LTP segment content header with specific field for each type of segment.

Several other classes representing internal data structures (SessionIDs, LtpConfigParameters, etc. ) are also included
but they are not described here due to their low relevance.

Scope and Limitations
=====================

The current ltp protocol |ns3| implementation supports the following features:

* Basic data transmission between two nodes.
* Retransmission based reliability.
* Notices to client service instance.
* Udp Convergence layer adapter with link state cue support.
* Static Ltp engine id to IP address conversion.
* All types of Ltp Headers and Content Headers.
* Self-Delimiting Numeric Values (SDNV) support in headers [rfc6250];

The LTP model DOES NOT support the following features:

* Asynchronous checkpoints.
* Cancellation Requests.
* LTP security extensions .. [rfc5327]
* LTP authentication extensions .. [rfc5327]
* Handle System Error conditions.
* Management Information Base, containing times of operation of remote ltp engines.

The current implementation has the following limitations:

* Only supports one concurrent transmission session per destination.
* Only a single timer of each type can be run concurrently in a single session (only a checkpoint or report can be handled at the same time).
* Segment reception offsets are assumed to be ordered.
* Checkpoint and Red Data Segment retransmission are handled together.
* When run over IP, and when the relevant IP interface has several addresses, only the primary IP address of an underlying IP interface is looked up in the address resolution process of the LTP helper.

Future developments (ordered by priority):

* Allow specific segment retransmission.
* Handle CP and Data retransmission independently.
* Allow reception of unordered data.
* Add Cancellation request support.
* Add support for multiple timers running simultaneously and asynchronous checkpoint support.

APIs
*****

The most relevant API provided in this model are:

Ltp Protocol
============

The ``ns3::ltp::LtpProtocol``

1. RegisterClientService(): method ``ns3::ltp::LtpProtocol::RegisterClientService ()`` registers a Client Service Instance into
 the ``ns3::LtpProtocol``. This method should always be the first method to call before using the other methods (StartTransmission).
 In the case of receivers, clients must always register so they can be valid destinations for the LTP engine to deliver received data,
 This function takes a callback as one of its parameters, this callback is used to connect to a TraceSource that notifies session status
 changes to the registered client service.

2. UnregisterClientService(): method ``ns3::ltp::LtpProtocol::UnregisterClientService ()`` removes the registered Client Service
 so it can no longer use the LTP services.

3. StartTransmission(): method ``ns3::ltp::LtpProtocol::StartTransmission ()`` requests the transmission of a block of client service data.
This method creates a new transmission session, uniquely identified by a session ID, and session state record to keep track
of the session status. If the block is bigger than the MTU of the lower datalink layer, the block is split into several segments and
queued for transmission into the LtpQueueSet of the corresponding session state record.

4. CancelTransmission(): method ``ns3::LtpProtocol::CancelTransmission ()`` cancels the transmission session and notifies the remote
LTP peer, starting the LTP cancellation procedure and freeing resources afterwards.

Ltp to Ip resolution table
==========================

The `ns3::ltp::LtpIpResolutionTable`` class offers several public APIs:

1. AddBinding(): ``ns3::ltp::LtpIpResolutionTable::AddBinding ()`` method adds a binding between Ip Address and a LTP Engine,
there are multiple versions allowing both ipv4 and ipv6.

2. RemoveBinding(): ``ns3::ltp::LtpIpResolutionTable::RemoveBinding ()``  method removes a binding between Ip Address and a LTP Engine,
there are multiple versions allowing both ipv4 and ipv6.

3. GetRoute():  ``ns3::ltp::LtpIpResolutionTable::GetRoute (uint64_t ltpEngineId)`` method acquires the corresponding IP address bound to a LTP
engine id. If there are multiple bindings, internal flag m_adddressMode controls whether the IPv4 or Ipv6 binding map is searched first,
the first entry that matches the requested EngineId will be used.

4. PrintRoute(): ``ns3::ltp::LtpIpResolutionTable::GetRoute (uint64_t ltpEngineId)`` prints bindings.

Convergence Layer Adapter
=========================

The ``ns3::ltp::LtpConvergenceLayerAdapter`` is an abstract base class that offers several
functions that must be implemented by specific convergence layer adapter classes, the main
methods defined are:

1. Send(): ``ns3::ltp::LtpConvergenceLayerAdapter::Send()`` method to send a packet using the underlying layer.

2. Receive(): ``ns3::ltp::LtpConvergenceLayerAdapter::Send()`` method to receive a packet from the underlying layer.

3. GetMtu(): ``ns3::ltp::LtpConvergenceLayerAdapter::GetMtu()`` Get the maximum transmission unit (MTU) associated with this destination Engine ID. This is implemented by resolving the
Engine ID to an IP address, then opening a connected UDP socket to the destination IP address and checking it for its MTU.

Link State Cues are implemented as callbacks, the API provides several functions to set them:

* SetLinkUpCallback ();
* SetLinkDownCallback ();
* SetCheckPointSentCallback ();
* SetReportSentCallback ();
* SetEndOfBlockSentCallback ();
* SetCancellationCallback ();


Attributes
***********

LtpProtocol most relevant attributes:

* "LocalEngineId" : Defines the engine id used by this object.
* "CheckPointRtxLimit": Defines the maximum number of checkpoint retransmissions allowed per session.
* "ReportSegmentRtxLimit":  Defines the maximum number of report retransmissions allowed per session.
* "LocalProcessingDelays": Defines the interval of time required for processing operations (queueing/dequeing).
* "OneWayLightTime": Defines the time required for transmitted data to reach the destination. (TODO: expand)

LtpConvergenceLayerAdapter most significant attribute:

* "RemotePeer" : Defines the ltp engine id of the remote peer to which this CLA links.

LtpIpResolutionTable most significant attribute:

* "Addressing" : flag that defines whether ipv4 or ipv6 is used.

Helpers
*******

A ``LtpProtocolHelper`` object is used to setup simulations and configure the
various components. This class helps create Ltp Protocol objects
and configures their attributes according to a preset configuration.

It provides two main methods:

1. Install (NodeContainer): This method creates ltp objects and installs them on the nodes
of the node container.

2. InstallAndLink (NodeContainer): This method creates ltp protocol and converegence 
layer adapter objects and installs them on nodes linking them together.

Examples
********

Example scripts can be found in ``src/ltp-protocol/examples``:

1. ``ltp-protocol-example.cc`` shows how to use the ltp protocol helper
in order to set up a network topology and transfer data between two nodes
using ltp protocol. It defines two functions implementing the LTP API
functions provided to receive client service status notifications.

2. ``ltp-protocol-long-transmission-example.cc` shows how two define an ``ns3::Application``
to act as a client service instance, and uses the LTP protocol to establish to send 
big ammounts of data over a channel with long transmission delays. 

Output
******

Trace Sources are provided returning Client Service notifications. The callback
is implicitly set up when a client service instance is registered  using the
``ns3::ltp::LtpProtocol::RegisterClientService ()``, this trace source
provides session status notification changes and provides the following parameters:

 * Session Id which originated the notification.
 * StatusNotificationCode reason for this notification.
 * Data in the case in which data reception is reported.
 * Data length.
 * Source Ltp Engine: the ltp engine id of the data source.
 * Offset of this data within a whole block of data.

Several debug level logging output can be enabled to observe the type and contents
of received segments during the transmission session.

NS_LOG_FUNCTION() is also available for debugging purposes.


Validation
**********

The model provides two main test-suites:

- ltp-protocol: This test suite contains unit tests for the auxiliary data structures of the protocol. 
Each test creates objects assigning different values and uses its functions with extensive combinations and use cases.
The following classes are tested in this suite:
 
* SessionId
* LtpHeader
* LtpTrailer
* LtpContentHeader
* LtpQueueSet
* SessionStateRecord,
* LtpProtocol public API.

- ltp-channel-loss: This test-suite builds a network topology and uses the protocol to transmit
data, while forcing data corruption using an ErrorModel. The different test-cases check the
retransmission capabilities of the protocol by providing different patterns for these
data-losses.

References
**********

.. [rfc5325] S. Burleigh, M. Ramadas, S. Farrell, "Licklider Transmission Protocol - Motivation" RFC 5326, Sept. 2008
.. [rfc5326] M. Ramadas, S. Burleigh, S. Farrell, "Licklider Transmission Protocol - Specification" RFC 5326, Sept. 2008
.. [rfc5327] S. Farrell, M. Ramadas, S. Burleigh, "Licklider Transmission Protocol - Security Extensions" RFC 5327, Sept. 2008

