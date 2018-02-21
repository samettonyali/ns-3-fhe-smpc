/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef ID_SEQ_TS_HEADER_H
#define ID_SEQ_TS_HEADER_H

#include "ns3/header.h"
#include "ns3/nstime.h"

namespace ns3 {
/**
 * \ingroup udpclientserver
 * \class SeqTsHeader
 * \brief Packet header for UDP client/server application.
 *
 * The header is made of a 32bits sequence number followed by
 * a 64bits time stamp.
 */
class IdSeqTsHeader : public Header
{
public:
  IdSeqTsHeader ();
  /**
   * \param id the consumer id
   */
  void SetCustId (uint32_t id);
  void SetOpId (uint32_t id);
  /**
   * \return the consumer id
   */
  uint32_t GetCustId (void) const;
  uint32_t GetOpId (void) const;
  /**
   * \param seq the sequence number
   */
  void SetSeq (uint32_t seq);
  /**
   * \return the sequence number
   */
  uint32_t GetSeq (void) const;
  /**
   * \return the time stamp
   */
  Time GetTs (void) const;

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint32_t m_custId; //!<consumer identity
  uint32_t m_opID; //!<operation identifier
  uint32_t m_seq; //!< Sequence number
  uint64_t m_ts; //!< Timestamp
};

} // namespace ns3

#endif /* ID_SEQ_TS_HEADER_H */
