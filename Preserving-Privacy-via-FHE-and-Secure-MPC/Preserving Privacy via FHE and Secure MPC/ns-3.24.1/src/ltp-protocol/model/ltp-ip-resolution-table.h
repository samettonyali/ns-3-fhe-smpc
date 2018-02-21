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

#ifndef LTP_IP_RESOLUTION_TABLE_H_
#define LTP_IP_RESOLUTION_TABLE_H_

#include "ltp-protocol.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/output-stream-wrapper.h"
#include <map>

namespace ns3 {
//namespace ltp {

class LtpProtocol;

/**
 * \ingroup dtn
 *
 * \brief This class contains mappings between LTP Engine IDs and IP addresses
 * It provides a public API for managing bindings and perform conversions.
 */
class LtpIpResolutionTable : public Object
{

public:


	/* \enum AddressMode
	 * \brief Defines the type of address that will be used
	 * for the mapping of ltp engine ids.
	 * */
	enum AddressMode
	{
	  Ipv4 = 0,
	  Ipv6 = 1
	};



  static TypeId GetTypeId (void);

  /**
   * Default Constructor
   */
  LtpIpResolutionTable ();

  /**
   * Destructor
   */
  ~LtpIpResolutionTable ();

  /*
   * Methods to add/remove bindings to the database.
   */

  /*
   * \brief Add Binding between Ipv4 Address and LTP Engine.
   * \param LtpEngineId LTP engine Id.
   * \param Addr Ipv4 Address
   * \return true if binding added successfully, false if there is
   *  already an existing entry.
   */
  bool AddBinding (uint64_t LtpEngineId, Ipv4Address Addr);

  /*
   * \brief Add Binding between Ipv4 Address with port and LTP Engine.
   * \param LtpEngineId LTP engine Id.
   * \param Addr Ipv4 Address,
   * \param port Port number.
   * \return true if binding added successfully, false if there is
   *  already an existing entry.
   */
  bool AddBinding (uint64_t LtpEngineId, Ipv4Address Addr, uint16_t port);

  /*
   * \brief Add Binding between Ipv6 Address and LTP Engine.
   * \param LtpEngineId LTP engine Id.
   * \param Addr Ipv6 Address.
   * \return true if binding added successfully, false if there is
   *  already an existing entry.
   */
  bool AddBinding (uint64_t LtpEngineId, Ipv6Address Addr);

  /*
   * \brief Add Binding between Ipv6 Address with port and LTP Engine.
   * \param LtpEngineId LTP engine Id.
   * \param Addr Ipv6 Address.
   * \param port Port number.
   * \return true if binding added successfully, false if there is
   *  already an existing entry.
   */
  bool AddBinding (uint64_t LtpEngineId, Ipv6Address Addr, uint16_t port);

  /* \brief Remove bindings;
   * \param LtpEngineId LTP engine id.
   * \param Addr Ipv4 Addr.
   * \return true if the class instance found a match and was able to delete it */
  bool RemoveBinding (uint64_t LtpEngineId, Ipv4Address Addr);

  /* \brief Remove bindings;
   * \param LtpEngineId LTP engine id
   * \param Addr Ipv4 Addr
   * \param port Port number
   * \return true if the class instance found a match and was able to delete it */
  bool RemoveBinding (uint64_t LtpEngineId, Ipv4Address Addr, uint16_t port);

  /* \brief Remove bindings;
   * \param LtpEngineId LTP engine id
   * \param Addr Ipv6 Addr
   * \return true if the class instance found a match and was able to delete it */
  bool RemoveBinding (uint64_t LtpEngineId, Ipv6Address Addr);

  /* \brief Remove bindings;
   * \param LtpEngineId LTP engine id
   * \param Addr Ipv6 Addr
   * \param port Port number
   * \return true if the class instance found a match and was able to delete it */
  bool RemoveBinding (uint64_t LtpEngineId, Ipv6Address Addr, uint16_t port);


  /* If there are multiple bindings, flag m_adddressMode
   * controls whether the IPv4 or Ipv6 binding map
   *  is searched first, and the first entry that was added that matches
       * the requested EngineId will be used.
       * \param LtpEngineId Ltp Engine Id to search.
       * \return Corresponding address for Ltp Engine Id.
  */
  Address GetRoute (uint64_t LtpEngineId);

  /*
   * Gets Ipv4 Address for given LtpEngineId
   * \param LtpEngineId Ltp Engine Id to search.
   * \return Corresponding Ipv4 address for Ltp Engine Id, or loopback addr if not found.
   */
  InetSocketAddress GetIpv4Route (uint64_t LtpEngineId);

  /*
   * Gets Ipv6 Address for given LtpEngineId
   * \param LtpEngineId Ltp Engine Id to search.
   * \return Corresponding Ipv4 address for Ltp Engine Id,  or loopback addr if not found.
   */
  Inet6SocketAddress GetIpv6Route (uint64_t LtpEngineId);

  /**
   * \brief Print the Bindings for Ipv4 Addresses
   *
   * \param stream the ostream the Routing table is printed to
   */
  void PrintIpv4Bindings (Ptr<OutputStreamWrapper> stream) const;

  /**
   * \brief Print the Bindings for Ipv6 Addresses
   *
   * \param stream the ostream the Routing table is printed to
   */
  void PrintIpv6Bindings (Ptr<OutputStreamWrapper> stream) const;

  /*
   * Get type of address being used
   */
  AddressMode GetAddressMode () const;


private:

  std::map <uint64_t, InetSocketAddress> m_ltpToIpv4Addr;      //!<  LTP Engine - Ipv4 Addr Mappings
  std::map <uint64_t, Inet6SocketAddress> m_ltpToIpv6Addr;     //!<  LTP Engine - Ipv6 Addr Mappings

  AddressMode m_addressMode;                                   //!< Flag used to select address format.
};

//}  // namespace ltp
}  // namespace ns3


#endif /* LTP_IP_RESOLUTION_TABLE_H_ */
