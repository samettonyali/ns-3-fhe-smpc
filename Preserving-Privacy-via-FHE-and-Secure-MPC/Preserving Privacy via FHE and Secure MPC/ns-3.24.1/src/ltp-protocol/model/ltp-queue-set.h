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

#ifndef LTP_QUEUE_SET_H
#define LTP_QUEUE_SET_H

#include "ns3/packet.h"
#include "ns3/queue.h"
#include "ltp-header.h"

#include <queue>


namespace ns3 {
//namespace ltp {

/**
 * \ingroup dtn
 *
 * \brief Queue set class containing the two queues for outbound traffic.
 * Represents the dual queue structure and priority en/de-queueing policy described in
 * RFC 5325 - 3.1.2 Deferred Transmission.
 */
class LtpQueueSet : public Queue
{
public:
  /**
   * \brief Get Type Id.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief LtpQueueSet Constructor
   *
   * Create a ltp queue pair
   */
  LtpQueueSet ();

  /**
   * \brief Destructor
   * Destructor
  */
  virtual ~LtpQueueSet ();

private:
  /**
   * Push a packet in the queue set, this method checks the LTP Segment type and enqueues
   * it in the corresponding priority queue.
   * \param p the packet to enqueue
   * \return true if success, false on failure.
   */
  virtual bool DoEnqueue (Ptr<Packet> p);
  /**
   * Pull a packet from the queue based on priority (internal operation queue packets are extracted first)
   * \return the packet.
   */
  virtual Ptr<Packet> DoDequeue (void);
  /**
   * Peek the front packet based on priority
   * \return the packet.
   */
  virtual Ptr<const Packet> DoPeek (void) const;

  std::queue<Ptr<Packet> > m_internalOps;       //!< Internal Operation Queue.
  std::queue<Ptr<Packet> > m_appData;           //!< Application Data Queue.
};

//} // namespace ltp
} // namespace ns3


#endif /* LTP_QUEUE_SET_H_ */
