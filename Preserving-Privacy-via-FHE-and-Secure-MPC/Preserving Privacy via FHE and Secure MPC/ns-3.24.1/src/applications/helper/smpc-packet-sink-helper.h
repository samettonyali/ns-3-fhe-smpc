/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   smpc-packet-sink-helper.h
 * Author: samet
 *
 * Created on January 19, 2016, 1:02 PM
 */

#ifndef SMPC_PACKET_SINK_HELPER_H
#define SMPC_PACKET_SINK_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"

namespace ns3 {

/**
 * \brief A helper to make it easier to instantiate an ns3::PacketSinkApplication
 * on a set of nodes.
 */
class SmpcPacketSinkHelper
{
public:
  /**
   * Create a PacketSinkHelper to make it easier to work with PacketSinkApplications
   *
   * \param protocol the name of the protocol to use to receive traffic
   *        This string identifies the socket factory type used to create
   *        sockets for the applications.  A typical value would be 
   *        ns3::TcpSocketFactory.
   * \param address the address of the sink,
   *
   */
  SmpcPacketSinkHelper (std::string protocol, Address address);
  
  SmpcPacketSinkHelper (std::string protocol, Address address, uint32_t processingDelay);
  
  SmpcPacketSinkHelper (std::string protocol, Address address, Address target, uint32_t processingDelay);

  /**
   * Helper function used to set the underlying application attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * Install an ns3::PacketSinkApplication on each node of the input container
   * configured with all the attributes set with SetAttribute.
   *
   * \param c NodeContainer of the set of nodes on which a PacketSinkApplication 
   * will be installed.
   */
  ApplicationContainer Install (NodeContainer c) const;

  /**
   * Install an ns3::PacketSinkApplication on each node of the input container
   * configured with all the attributes set with SetAttribute.
   *
   * \param node The node on which a PacketSinkApplication will be installed.
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * Install an ns3::PacketSinkApplication on each node of the input container
   * configured with all the attributes set with SetAttribute.
   *
   * \param nodeName The name of the node on which a PacketSinkApplication will be installed.
   */
  ApplicationContainer Install (std::string nodeName) const;

private:
  /**
   * \internal
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory;
};

} // namespace ns3

#endif /* SMPC_PACKET_SINK_HELPER_H */

