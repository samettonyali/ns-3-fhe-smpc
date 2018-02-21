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
#ifndef LTP_PROTOCOL_HELPER_H
#define LTP_PROTOCOL_HELPER_H

#include "ns3/ltp-protocol.h"
#include "ns3/object.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/ptr.h"
#include "ns3/object-factory.h"
#include "ns3/address.h"
#include "ns3/ltp-convergence-layer-adapter.h"
#include "ns3/ltp-udp-convergence-layer-adapter.h"
#include "ns3/ltp-ip-resolution-table.h"

namespace ns3 {
//namespace ltp {

/**
 * \ingroup dtn
 *
 * \brief helps to manage and create Ltp Protocol objects
 *
 * This class can help to create Ltp Protocol Objects
 * and to configure their attributes during creation. It can
 * also create and initialize the Convergence Layer Adapter
 * for nodes linked together by an underlying net-device.
 *
 */
class LtpProtocolHelper
{
public:
  /**
   * \brief Create a LtpProtocolHelper helper in an empty state.
   */
  LtpProtocolHelper ();

  /**
   * \brief Installs LTP protocol instances in each node of the provided container.
   * \param c a set of nodes
   */
  void Install (NodeContainer c);

  /* \brief Install LTP protocol instances in each nodes, and for each individual node
   * it establishes a UDP transport association with the next node the container, the last one
   * is linked to the first.
   * Notes:An LtpIpResolutionTable must be assigned using SetLtpIpResolutionTable() before calling this method,
   * otherwise this method will terminate.
   * \param c a set of nodes
   */
  void InstallAndLink (NodeContainer c);

  /*
   * \brief Set base LTP engine id will be incremented sequentially with each installed node.
   * \param id
   */
  void SetBaseLtpEngineId (uint64_t id);

  /*
   * \brief Set specific ltp engine id to a given node.
   * \param node pointer to node.
   * \param id ltp engine id.
   */
  void SetLtpEngineId (Ptr<Node> node, uint64_t id);

  /*
   * \brief Set base LtpIpResolutionTable that will be used in installed ltp protocol instances.
   * \param rt pointer to resolution talbe.
   */
  void SetLtpIpResolutionTable (Ptr<LtpIpResolutionTable> rt);

  /*
   * \brief Set type and attributes of Convergence Layer.
   */
  void SetConvergenceLayerAdapter (std::string type,
                                   std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                                   std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                                   std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                                   std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                                   std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                                   std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                                   std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue (),
                                   std::string n8 = "", const AttributeValue &v8 = EmptyAttributeValue (),
                                   std::string n9 = "", const AttributeValue &v9 = EmptyAttributeValue ());
  /*
   * \brief Set attributes of LTP protocol instances.
   */
  void  SetAttributes (  std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
                         std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
                         std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
                         std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
                         std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
                         std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
                         std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue (),
                         std::string n8 = "", const AttributeValue &v8 = EmptyAttributeValue (),
                         std::string n9 = "", const AttributeValue &v9 = EmptyAttributeValue ());


  /*
   * \brief Add bindings between ltp engine id and ip address.
   */
  bool AddBinding (uint64_t dstLtpEngineId, Ipv4Address dstAddr);

  /*
   * \brief Add bindings between ltp engine id and ip address.
   */
  bool AddBinding (uint64_t dstLtpEngineId, Ipv4Address dstAddr, uint16_t port);

  /*
   * \brief Set the activation time of installed ltp protocols.
   * */
  void SetStartTransmissionTime (Time t);

  void Install (Ptr<Node> node);

  void InstallAndLink (Ptr<Node> node);

  void InstallAndLink (Ptr<Node> node, uint64_t remotePeer, bool isSensor);


  void Install (std::string nodeName);

private:

  Time m_startTime;

  ObjectFactory m_claFactory; //!< Cla Object factory.
  ObjectFactory m_ltpFactory; //!< LTP Object factory.

  uint64_t m_ltpid;	//!< Base ltp id.

  Ptr<LtpIpResolutionTable> m_resolutionTable;	//!< Ltp to Ip resolution table.

};

//}
}

#endif /* LTP_PROTOCOL_HELPER_H */

