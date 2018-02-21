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

#ifndef LTP_SESSION_STATE_RECORD_IMPL_H
#define LTP_SESSION_STATE_RECORD_IMPL_H

namespace ns3 {
//namespace ltp {


template <typename FN>
void SessionStateRecord::SetTimerFunction (FN fn,const Time delay,TimerCode type)
{
  switch (type)
    {
    case CHECKPOINT:
      m_CpTimer.SetFunction (fn);
      m_CpTimer.SetDelay (delay);
      break;
    case REPORT:
      m_RsTimer.SetFunction (fn);
      m_RsTimer.SetDelay (delay);
      break;
    case CANCEL:
      m_CxTimer.SetFunction (fn);
      m_CxTimer.SetDelay (delay);
      break;
    default:
      break;
    }
}

template <typename MEM_PTR, typename OBJ_PTR>
void SessionStateRecord::SetTimerFunction (MEM_PTR memPtr, OBJ_PTR objPtr, const Time delay,TimerCode type)
{
  switch (type)
    {
    case CHECKPOINT:
      m_CpTimer.SetFunction (memPtr,objPtr);
      m_CpTimer.SetDelay (delay);
      break;
    case REPORT:
      m_RsTimer.SetFunction (memPtr,objPtr);
      m_RsTimer.SetDelay (delay);
      break;
    case CANCEL:
      m_CxTimer.SetFunction (memPtr,objPtr);
      m_CxTimer.SetDelay (delay);
      break;
    default:
      break;
    }
}

template <typename MEM_PTR, typename OBJ_PTR, typename T1>
void SessionStateRecord::SetTimerFunction (MEM_PTR memPtr, OBJ_PTR objPtr, T1 param, const Time delay, TimerCode type)
{
  switch (type)
    {
    case CHECKPOINT:
      m_CpTimer.SetFunction (memPtr,objPtr);
      m_CpTimer.SetArguments (param);
      m_CpTimer.SetDelay (delay);
      break;
    case REPORT:
      m_RsTimer.SetFunction (memPtr,objPtr);
      m_RsTimer.SetArguments (param);
      m_RsTimer.SetDelay (delay);
      break;
    case CANCEL:
      m_CxTimer.SetFunction (memPtr,objPtr);
      m_CxTimer.SetArguments (param);
      m_CxTimer.SetDelay (delay);
      break;
    default:
      break;
    }
}

template <typename MEM_PTR, typename OBJ_PTR, typename T1, typename T2>
void SessionStateRecord::SetTimerFunction (MEM_PTR memPtr, OBJ_PTR objPtr, T1 param,T2 param2, const Time delay, TimerCode type)
{
  switch (type)
    {
    case CHECKPOINT:
      m_CpTimer.SetFunction (memPtr,objPtr);
      m_CpTimer.SetDelay (delay);
      m_CpTimer.SetArguments (param,param2);
      break;
    case REPORT:
      m_RsTimer.SetFunction (memPtr,objPtr);
      m_RsTimer.SetDelay (delay);
      m_RsTimer.SetArguments (param,param2);
      break;
    case CANCEL:
      m_CxTimer.SetFunction (memPtr,objPtr);
      m_CxTimer.SetDelay (delay);
      m_CxTimer.SetArguments (param,param2);
      break;
    default:
      break;
    }
}

//} // namespace ltp
} // namespace ns3

#endif /* LTP_SESSION_STATE_RECORD_IMPL_H */
