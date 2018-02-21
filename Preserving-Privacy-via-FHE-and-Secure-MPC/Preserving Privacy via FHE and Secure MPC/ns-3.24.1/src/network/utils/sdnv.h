/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 University of New Brunswick
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
 * Author: Dizhi Zhou        <dizhi.zhou@gmail.com>
 *         Tommaso Pecorella <tommaso.pecorella@unifi.it>
 */
#ifndef SDNV_H
#define SDNV_H

#include <vector>
#include <stdint.h>
#include "ns3/buffer.h"

namespace ns3 {

/**
 * \brief an implementation class of self-delimiting numeric values based on RFC 6256
 */
class Sdnv
{
public:
  /**
   * Constructor
   */
  Sdnv ();

  /**
   * Destroy
   */
  virtual ~Sdnv ();

  /**
   * \brief SDNV encoding algorithm
   *
   * The encoding algorithm is based on section 3.1, RFC 6256
   *
   * \param val value need to be encoded
   * \return a uint8_t vector
   */
  std::vector<uint8_t> Encode (uint64_t val);

  /**
   * \brief SDNV decoding algorithm for an integer
   *
   * The decoding algorithm is based on section 3.2, RFC 6256
   *
   * \param val value need to be decoded
   * \return uint64_t decoded integer; It is user's responsibility to 
   *         convert the return type to the type of variables in use
   */
  uint64_t Decode (std::vector<uint8_t> val);

  /**
   * \brief SDNV decoding algorithm for a Buffer
   *
   * This method read an integer from the Buffer and decodes it by 
   * Decode (std::vector<uint8_t val>)
   *
   * \param start buffer start iterator reference
   * \return uint64_t decoded integer; It is user's responsibility to 
   *         convert the return type to the type of variables in use
   */
  uint64_t Decode (Buffer::Iterator &start);

  /**
   * \brief is this the bolder of an encoded integer?
   *
   * \param val an entry of encoded vector
   * \return return true if it is the bolder
   */
  bool IsLast (uint8_t &val);
};

} // namespace ns3

#endif /* SDNV_H */
