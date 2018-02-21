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

#include "ns3/log.h"
#include "ns3/sdnv.h"
#include "ltp-header.h"
#include "ns3/tag-buffer.h"


NS_LOG_COMPONENT_DEFINE ("LtpHeader");

namespace ns3 {
//namespace ltp {

LtpHeader::LtpHeader (SegmentType t, SessionId id)
  : m_version (0x0),
    m_typeFlags (t),
    m_sessionId (id),
    m_hdrExtensionCnt (0),
    m_trailerExtensionCnt (0),
    m_extensions ()
{
  NS_LOG_FUNCTION (this);
}
LtpHeader::LtpHeader ()
  : m_version (0x0),
    m_typeFlags (0x0),
    m_sessionId (),
    m_hdrExtensionCnt (0),
    m_trailerExtensionCnt (0),
    m_extensions ()
{
  NS_LOG_FUNCTION (this);
}

LtpHeader::~LtpHeader ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
LtpHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LtpHeader")
    .SetParent<Header> ()
    .AddConstructor<LtpHeader> ()
  ;
  return tid;
}

TypeId
LtpHeader::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

void
LtpHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this);
  os << "LtpHeader ( Version: " << (uint32_t) m_version << ", Type : " << (uint32_t) m_typeFlags <<
  ", (SessionID ( Originator : " << m_sessionId.GetSessionOriginator () << ", Number: " <<
  m_sessionId.GetSessionNumber () << "), HeaderExtensions: " << (uint32_t) m_hdrExtensionCnt <<
  ", TrailerExtensions: " << (uint32_t) m_trailerExtensionCnt <<  "))";
}

uint32_t
LtpHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  //uint32_t size = 2; // Control Byte + Extensions Byte.
  
  uint32_t size = 0;
  
  size += sizeof(m_smartMeterID) + sizeof(m_fragmentType) + sizeof(m_fragmentID) + 
          sizeof(m_fragmentSize);
  
  //Sdnv codec;
  //size += codec.Encode (m_sessionId.GetSessionOriginator ()).size (); // SDNV encoded Session Originator
  //size += codec.Encode (m_sessionId.GetSessionNumber ()).size (); // SDNV encoded Session Number

//  NS_LOG_INFO("GetSerializedSize:Number of extensions: " << m_extensions.size ());
//  
//  for (uint32_t i = 0; i < m_extensions.size (); i++)
//    {
//      size += m_extensions.at (i).GetSerializedSize ();
//    }

  return size;
}
void
LtpHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this);
  Buffer::Iterator i = start;
//  Sdnv codec;

//  uint8_t control_byte = 0;
//
//  control_byte = m_version << 4;
//  control_byte += m_typeFlags;
//
//  i.WriteU8 (control_byte);      // Write Control Byte
  
  i.WriteU16(m_smartMeterID);
  i.WriteU8(m_fragmentType);
  i.WriteU8(m_fragmentID);
  i.WriteU16(m_fragmentSize);

//  std::vector<uint8_t> sessionOriginator = codec.Encode (m_sessionId.GetSessionOriginator ());
//  std::vector<uint8_t> sessionNumber = codec.Encode (m_sessionId.GetSessionNumber ());

//  for (uint32_t j = 0; j < sessionOriginator.size (); j++)
//    {
//      i.WriteU8 (sessionOriginator[j]);                 //Write SDNV encoded Session Originator
//    }
//
//  for (uint32_t j = 0; j < sessionNumber.size (); j++)
//    {
//      i.WriteU8 (sessionNumber[j]);          //Write SDNV encoded Session Number
//    }

//  uint8_t extensions_byte = 0;
//
//  extensions_byte = m_hdrExtensionCnt << 4;
//  extensions_byte += m_trailerExtensionCnt;
//
//  i.WriteU8 (extensions_byte);      // Write Extensions Byte
//
//  NS_ASSERT ( m_hdrExtensionCnt  == m_extensions.size ());
//
//  for (uint32_t j = 0; j < m_hdrExtensionCnt; j++)
//    {
//      m_extensions[j].Serialize (i);
//    }
}
uint32_t
LtpHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this);
  Buffer::Iterator i = start;
  
  m_smartMeterID = i.ReadU16();
  m_fragmentType = i.ReadU8();
  m_fragmentID = i.ReadU8();
  m_fragmentSize = i.ReadU16();

//  uint8_t control_byte = i.ReadU8 (); // Read Control Byte
//  m_version = control_byte >> 4;
//  m_typeFlags = control_byte & 0x0F;
//
//  Sdnv codec;
//  m_sessionId.SetSessionOriginator (codec.Decode (i)); // Read Session Originator
//  m_sessionId.SetSessionNumber (codec.Decode (i)); // Read Session Number
//
//  uint8_t extensions_byte = i.ReadU8 ();  // Read Extensions Byte
//  m_hdrExtensionCnt = extensions_byte >> 4;
//  m_trailerExtensionCnt = extensions_byte & 0x0F;
//
//  for (uint32_t j = 0; j < m_hdrExtensionCnt; j++)
//    {
//      LtpExtension extension;
//      static_cast<void> (extension.Deserialize (i));
//      m_extensions.push_back (extension);
//    }

  return GetSerializedSize ();
}

void
LtpHeader::SetVersion (uint8_t version)
{
  NS_LOG_FUNCTION (this);
}

uint8_t
LtpHeader::GetVersion (void) const
{
  NS_LOG_FUNCTION (this);
  return m_version;
}
void
LtpHeader::SetSegmentType (SegmentType segmentType)
{
  NS_LOG_FUNCTION (this << " " << segmentType);
  m_typeFlags = (uint8_t) segmentType;
}
SegmentType
LtpHeader::GetSegmentType () const
{
  NS_LOG_FUNCTION (this);
  return (SegmentType) m_typeFlags;
}

void
LtpHeader::SetSessionId (SessionId id)
{
  NS_LOG_FUNCTION (this << " " << id.GetSessionOriginator () << " " << id.GetSessionNumber ());
  m_sessionId = id;
}

SessionId
LtpHeader::GetSessionId () const
{
  NS_LOG_FUNCTION (this);
  return m_sessionId;
}

////////////////////////////////////////////////////////////////////////////////

void LtpHeader::SetSmartMeterID(uint16_t SM_ID){
    NS_LOG_FUNCTION (this);
    m_smartMeterID = SM_ID;
}

uint16_t LtpHeader::GetSmartMeterID() const{
    NS_LOG_FUNCTION (this);
    return m_smartMeterID;
}

void LtpHeader::SetFragmentType(uint8_t fragmentType){
    NS_LOG_FUNCTION (this);
    m_fragmentType = fragmentType;
}

uint8_t LtpHeader::GetFragmentType() const{
    NS_LOG_FUNCTION (this);
    return m_fragmentType;
}

void LtpHeader::SetFragmentID(uint8_t fragmentID){
    NS_LOG_FUNCTION (this);
    m_fragmentID = fragmentID;
}

uint8_t LtpHeader::GetFragmentID() const{
    NS_LOG_FUNCTION (this);
    return m_fragmentID;
}

void LtpHeader::SetFragmentSize(uint16_t fragmentSize){
    NS_LOG_FUNCTION (this);
    m_fragmentSize = fragmentSize;
}

uint16_t LtpHeader::GetFragmentSize() const{
    NS_LOG_FUNCTION (this);
    return m_fragmentSize;
}

////////////////////////////////////////////////////////////////////////////////

void
LtpHeader::SetHeaderExtensionCount (uint8_t count)
{
  NS_LOG_FUNCTION (this << " " << count);
  m_hdrExtensionCnt = count;
}
uint8_t
LtpHeader::GetHeaderExtensionCount () const
{
  NS_LOG_FUNCTION (this);
  return m_hdrExtensionCnt;
}

void
LtpHeader::SetTrailerExtensionCount (uint8_t count)
{
  NS_LOG_FUNCTION (this << " " << count);
  m_trailerExtensionCnt = count;
}

uint8_t
LtpHeader::GetTrailerExtensionCount () const
{
  NS_LOG_FUNCTION (this);
  return m_trailerExtensionCnt;
}

void
LtpHeader::AddExtension (LtpExtension extension)
{
  NS_LOG_FUNCTION (this);
  if (m_extensions.size () < 16)
    {
      m_extensions.push_back (extension);
      m_hdrExtensionCnt = m_extensions.size ();
    }
  // Otherwise do nothing
}

LtpExtension
LtpHeader::GetExtension (uint32_t index) const
{
  NS_LOG_FUNCTION (this << " " << index);
  return m_extensions.at (index);

}

bool
LtpHeader::IsDataSegment (SegmentType type)
{
  NS_LOG_FUNCTION (type);
  if (IsRedDataSegment (type) || IsGreenDataSegment (type))
    {
      return true;
    }
  else
    {
      return false;
    }
}


bool
LtpHeader::IsRedDataSegment (SegmentType type)
{
  NS_LOG_FUNCTION (type);
//  if ((type == LTPTYPE_RD)
//      || (type == LTPTYPE_RD_CP)
//      || (type == LTPTYPE_RD_CP_EORP)
//      || (type == LTPTYPE_RD_CP_EORP_EOB))
//    {
//      return true;
//    }
//  else
//    {
//      return false;
//    }
  return false;
  
}

bool
LtpHeader::IsGreenDataSegment (SegmentType type)
{
  NS_LOG_FUNCTION (type);
//  if ((type == LTPTYPE_GD)
//      || (type == LTPTYPE_GD_UF1)
//      || (type == LTPTYPE_GD_UF2)
//      || (type == LTPTYPE_GD_EOB))
//    {
//      return true;
//    }
//  else
//    {
//      return false;
//    }
  
  return false;
}


SessionId::SessionId ()
  : m_sessionOriginator (0),
    m_sessionNumber (0)
{
  NS_LOG_FUNCTION (this);
}

SessionId::~SessionId ()
{
  NS_LOG_FUNCTION (this);
}

SessionId::SessionId (uint64_t originator, uint64_t value)
{
  SetSessionOriginator (originator);
  SetSessionNumber (value);
}

uint64_t
SessionId::GetSessionOriginator (void) const
{
  NS_LOG_FUNCTION (this);
  return m_sessionOriginator;
}

void
SessionId::SetSessionOriginator (uint64_t originator)
{
  NS_LOG_FUNCTION (this << " " << originator);
  m_sessionOriginator = originator;
}

uint64_t
SessionId::GetSessionNumber () const
{
  NS_LOG_FUNCTION (this);
  return m_sessionNumber;
}

void
SessionId::SetSessionNumber (uint64_t value)
{
  NS_LOG_FUNCTION (this << " " << value);
  if ((value > MIN_SESSION_NUMBER) && (value < MAX_SESSION_NUMBER))
    {
      m_sessionNumber = value;
    }
  else
    {
      m_sessionNumber = MIN_SESSION_NUMBER;
    }
}

void
LtpExtension::SetExtensionType (ExtensionType type)
{
  NS_LOG_FUNCTION (this << " " << type);
  m_extType = (uint8_t) type;
}

LtpExtension::ExtensionType
LtpExtension::GetExtensionType () const
{
  NS_LOG_FUNCTION (this);
  return (LtpExtension::ExtensionType) m_extType;
}

void
LtpExtension::AddExtensionData (uint8_t data)
{
  NS_LOG_FUNCTION (this << data);
  m_value.push_back (data);
  m_len = m_value.size ();
}

void
LtpExtension::ClearExtensionData ()
{
  NS_LOG_FUNCTION (this);
  m_value.clear ();
  m_len = 0;
}

uint64_t
LtpExtension::GetExtensionLength () const
{
  NS_LOG_FUNCTION (this);
  return m_len;
}

uint32_t
LtpExtension::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  Sdnv codec;

  NS_LOG_INFO ("LtpExtension::GetSerializedSize");
  
  uint32_t size = 1;
  size += codec.Encode (m_len).size ();
  size += m_value.size ();
  return size;
}

uint32_t
LtpExtension::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION (this);
  Sdnv codec;

  m_extType = start.ReadU8 ();
  m_len = codec.Decode (start);

  for (uint32_t i = 0; i < m_len; i++)
    {
      m_value.insert (m_value.end (),start.ReadU8 ());
    }

  return GetSerializedSize ();
}

void
LtpExtension::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION (this);
  Sdnv codec;

  std::vector<uint8_t> fields;
  std::vector<uint8_t> sdnv_len = codec.Encode (m_len);

  fields.insert (fields.end (), m_extType);
  fields.insert (fields.end (), sdnv_len.begin (), sdnv_len.end () );
  fields.insert (fields.end (), m_value.begin (),  m_value.end ());

  for (uint32_t i = 0; i < fields.size (); i++)
    {
      start.WriteU8 (fields[i]);
    }

}

LtpTrailer::LtpTrailer ()
  : m_extensions ()
{
  NS_LOG_FUNCTION (this);
}
LtpTrailer::~LtpTrailer ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
LtpTrailer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LtpTrailer")
    .SetParent<Trailer> ()
    .AddConstructor<LtpTrailer> ()
  ;
  return tid;
}

TypeId
LtpTrailer::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}

void
LtpTrailer::Print (std::ostream &os) const
{
  // This method is invoked by the packet printing
  // routines to print the content of the header.
  NS_LOG_FUNCTION (this);
  os << "data=";
}

uint32_t
LtpTrailer::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  uint32_t size = 0;

  for (uint32_t i = 0; i < m_extensions.size (); i++)
    {
      size += m_extensions[i].GetSerializedSize ();
    }

  return size;
}

uint32_t
LtpTrailer::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this);
  Buffer::Iterator i = start;

  while (!i.IsEnd ())
    {
      LtpExtension extension;
      static_cast<void> (extension.Deserialize (i));
      m_extensions.push_back (extension);
    }

  return GetSerializedSize ();
}

void
LtpTrailer::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this);
  Buffer::Iterator i = start;

  for (uint32_t j = 0; j < m_extensions.size (); j++)
    {
      m_extensions[j].Serialize (i);
    }
}

void
LtpTrailer::AddExtension (LtpExtension extension)
{
  NS_LOG_FUNCTION (this);
  if (m_extensions.size () < 16)
    {
      m_extensions.push_back (extension);
    }
  // Otherwise do nothing
}

LtpExtension
LtpTrailer::GetExtension (uint32_t index) const
{
  NS_LOG_FUNCTION (this << " " << index);
  return m_extensions.at (index);
}

LtpContentHeader::LtpContentHeader ()
  : /*m_type (LTPTYPE_RD),*/
    m_clientServiceId (0),
    m_offset (0),
    m_length (0),
    m_cpSerialNumber (0),
    m_rpSerialNumber (0),
    m_upperBound (0),
    m_lowerBound (0),
    m_rxClaimCnt (0),
    m_rxClaims (),
    m_cxReason (0)
{
  NS_LOG_FUNCTION (this);
}

LtpContentHeader::LtpContentHeader (SegmentType t)
  : m_type (t),
    m_clientServiceId (0),
    m_offset (0),
    m_length (0),
    m_cpSerialNumber (0),
    m_rpSerialNumber (0),
    m_upperBound (0),
    m_lowerBound (0),
    m_rxClaimCnt (0),
    m_rxClaims (),
    m_cxReason (0)
{
  NS_LOG_FUNCTION (this);
}

LtpContentHeader::~LtpContentHeader ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
LtpContentHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LtpContentHeader")
    .SetParent<Header> ()
    .AddConstructor<LtpContentHeader> ()
  ;
  return tid;
}

TypeId
LtpContentHeader::GetInstanceTypeId (void) const
{
  NS_LOG_FUNCTION (this);
  return GetTypeId ();
}


void
LtpContentHeader::SetSegmentType (SegmentType segmentType)
{
  NS_LOG_FUNCTION (this);
  m_type = segmentType;
}

SegmentType
LtpContentHeader::GetSegmentType () const
{
  NS_LOG_FUNCTION (this);
  return m_type;
}


uint32_t
LtpContentHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  uint32_t size = 0;
  Sdnv codec;

//  /*  Data Segment */
//  if ((m_type == LTPTYPE_RD)
//      || (m_type == LTPTYPE_RD_CP)
//      || (m_type == LTPTYPE_RD_CP_EORP)
//      || (m_type == LTPTYPE_RD_CP_EORP_EOB)
//      || (m_type == LTPTYPE_GD)
//      || (m_type == LTPTYPE_GD_UF1)
//      || (m_type == LTPTYPE_GD_UF2)
//      || (m_type == LTPTYPE_GD_EOB))
//    {
//      size += codec.Encode (m_clientServiceId).size ();
//      size += codec.Encode (m_offset).size ();
//      size += codec.Encode (m_length).size ();
//
//      if ((m_type == LTPTYPE_RD_CP)
//          || (m_type == LTPTYPE_RD_CP_EORP)
//          || (m_type == LTPTYPE_RD_CP_EORP_EOB))
//        {
//          size +=  codec.Encode (m_cpSerialNumber).size ();
//          size +=  codec.Encode (m_rpSerialNumber).size ();
//        }
//    }
//  else if  (m_type == LTPTYPE_RS)               /* Report Segments*/
//    {
//      size +=  codec.Encode (m_rpSerialNumber).size ();
//      size +=  codec.Encode (m_cpSerialNumber).size ();
//      size +=  codec.Encode (m_upperBound).size ();
//      size +=  codec.Encode (m_lowerBound).size ();
//      size +=  codec.Encode (m_rxClaimCnt).size ();
//
//      for (uint32_t j = 0; j < m_rxClaims.size (); j++)
//        {
//          size +=  codec.Encode (m_rxClaims[j].offset).size ();
//          size +=  codec.Encode (m_rxClaims[j].length).size ();
//        }
//    }
//  else if  (m_type == LTPTYPE_RAS)              /* Report ACK Segment*/
//    {
//      size +=  codec.Encode (m_rpSerialNumber).size ();
//    }
//  else if ( (m_type == LTPTYPE_CS) ||     (m_type == LTPTYPE_CR) )       /* Cancel Segments*/
//    {
//      size = 1;
//    }
//  else if ( (m_type == LTPTYPE_CAS) ||    (m_type == LTPTYPE_CAR) )       /* Cancel ACK Segments*/
//    {
//      size = 0;
//    }
//  else
//    {
//      NS_FATAL_ERROR ("Unexpected Segment Type");
//    }
  return size;
}

uint32_t
LtpContentHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this);
  //Buffer::Iterator i = start;
  Sdnv codec;

  /*The segment type member variable (which presumably was determined in a previous
    LtpHeader::Deserialize() call) must be set before calling this method. */

//  /*  Data Segment */
//  if  ((m_type == LTPTYPE_RD)
//       || (m_type == LTPTYPE_RD_CP)
//       || (m_type == LTPTYPE_RD_CP_EORP)
//       || (m_type == LTPTYPE_RD_CP_EORP_EOB)
//       || (m_type == LTPTYPE_GD)
//       || (m_type == LTPTYPE_GD_UF1)
//       || (m_type == LTPTYPE_GD_UF2)
//       || (m_type == LTPTYPE_GD_EOB))
//    {
//      m_clientServiceId = codec.Decode (i);
//      m_offset = codec.Decode (i);
//      m_length = codec.Decode (i);
//
//      if ((m_type == LTPTYPE_RD_CP)
//          || (m_type == LTPTYPE_RD_CP_EORP)
//          || (m_type == LTPTYPE_RD_CP_EORP_EOB))
//        {
//          m_cpSerialNumber = codec.Decode (i);
//          m_rpSerialNumber = codec.Decode (i);
//        }
//    }
//  else if  (m_type == LTPTYPE_RS)               /* Report Segments*/
//    {
//      m_rpSerialNumber = codec.Decode (i);
//      m_cpSerialNumber = codec.Decode (i);
//      m_upperBound =  codec.Decode (i);
//      m_lowerBound =  codec.Decode (i);
//      m_rxClaimCnt =  codec.Decode (i);
//
//      for (uint32_t j = 0; j < m_rxClaimCnt; j++)
//        {
//          ReceptionClaim claim;
//          claim.offset =  codec.Decode (i);
//          claim.length =  codec.Decode (i);
//          m_rxClaims.push_back (claim);
//        }
//    }
//  else if  (m_type == LTPTYPE_RAS)              /* Report ACK Segment*/
//    {
//      m_rpSerialNumber = codec.Decode (i);
//    }
//  else if ( (m_type == LTPTYPE_CS) ||     (m_type == LTPTYPE_CR) )       /* Cancel Segments*/
//    {
//      m_cxReason = i.ReadU8 ();
//    }
//  else if ( (m_type == LTPTYPE_CAS) ||    (m_type == LTPTYPE_CAR) )       /* Cancel ACK Segments*/
//    {
//      return 0;
//    }
//  else
//    {
//      NS_FATAL_ERROR ("Unexpected Segment Type");
//    }

  return GetSerializedSize ();
}

void
LtpContentHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this);
  Buffer::Iterator i = start;
  Sdnv codec;

  std::vector<uint8_t> fields;

//  /*  Data Segment */
//  if  ((m_type == LTPTYPE_RD)
//       || (m_type == LTPTYPE_RD_CP)
//       || (m_type == LTPTYPE_RD_CP_EORP)
//       || (m_type == LTPTYPE_RD_CP_EORP_EOB)
//       || (m_type == LTPTYPE_GD)
//       || (m_type == LTPTYPE_GD_UF1)
//       || (m_type == LTPTYPE_GD_UF2)
//       || (m_type == LTPTYPE_GD_EOB))
//    {
//      std::vector<uint8_t> clientServiceId = codec.Encode (m_clientServiceId);
//      std::vector<uint8_t> offset = codec.Encode (m_offset);
//      std::vector<uint8_t> length = codec.Encode (m_length);
//
//      fields.insert (fields.end (), clientServiceId.begin (), clientServiceId.end () );
//      fields.insert (fields.end (), offset.begin (), offset.end () );
//      fields.insert (fields.end (), length.begin (), length.end () );
//
//      /* Data Segment with Check Point*/
//      if ((m_type == LTPTYPE_RD_CP)
//          || (m_type == LTPTYPE_RD_CP_EORP)
//          || (m_type == LTPTYPE_RD_CP_EORP_EOB))
//        {
//          std::vector<uint8_t> cpSerialNumber = codec.Encode (m_cpSerialNumber);
//          std::vector<uint8_t> rpSerialNumber = codec.Encode (m_rpSerialNumber);
//
//          fields.insert (fields.end (), cpSerialNumber.begin (), cpSerialNumber.end () );
//          fields.insert (fields.end (), rpSerialNumber.begin (), rpSerialNumber.end () );
//
//        }
//    }
//  else if  (m_type == LTPTYPE_RS)               /* Report Segments*/
//    {
//
//      std::vector<uint8_t> rpSerialNumber = codec.Encode (m_rpSerialNumber);
//      std::vector<uint8_t> cpSerialNumber = codec.Encode (m_cpSerialNumber);
//      std::vector<uint8_t> upperBound = codec.Encode (m_upperBound);
//      std::vector<uint8_t> lowerBound = codec.Encode (m_lowerBound);
//      std::vector<uint8_t> rxClaimCnt = codec.Encode (m_rxClaimCnt);
//
//      fields.insert (fields.end (), rpSerialNumber.begin (), rpSerialNumber.end () );
//      fields.insert (fields.end (), cpSerialNumber.begin (), cpSerialNumber.end () );
//      fields.insert (fields.end (), upperBound.begin (), upperBound.end () );
//      fields.insert (fields.end (), lowerBound.begin (), lowerBound.end () );
//      fields.insert (fields.end (), rxClaimCnt.begin (), rxClaimCnt.end () );
//
//      NS_ASSERT (m_rxClaims.size () == m_rxClaimCnt);
//
//      for (uint32_t j = 0; j < m_rxClaims.size (); j++)
//        {
//          std::vector<uint8_t> claimOffset = codec.Encode (m_rxClaims[j].offset);
//          std::vector<uint8_t> claimLen =  codec.Encode (m_rxClaims[j].length);
//
//          fields.insert (fields.end (), claimOffset.begin (), claimOffset.end () );
//          fields.insert (fields.end (), claimLen.begin (), claimLen.end () );
//        }
//
//    }
//  else if  (m_type == LTPTYPE_RAS)              /* Report ACK Segment*/
//    {
//      std::vector<uint8_t> rpSerialNumber = codec.Encode (m_rpSerialNumber);
//
//      fields.insert (fields.end (), rpSerialNumber.begin (), rpSerialNumber.end () );
//    }
//  else if ( (m_type == LTPTYPE_CS) ||     (m_type == LTPTYPE_CR) )       /* Cancel Segments*/
//    {
//      fields.insert (fields.end (),m_cxReason);
//    }
//  else if ( (m_type == LTPTYPE_CAS) ||    (m_type == LTPTYPE_CAR) )       /* Cancel ACK Segments*/
//    {
//      // CAX have no content
//      return;
//    }
//  else
//    {
//      NS_FATAL_ERROR ("Unexpected Segment Type");
//    }

  for (uint32_t j = 0; j < fields.size (); j++)
    {
      i.WriteU8 (fields[j]);
    }

}

void
LtpContentHeader::Print (std::ostream &os) const
{
  // This method is invoked by the packet printing
  // routines to print the content of the header.
  NS_LOG_FUNCTION (this);

  os << "LtpContentHeader (";

//  if  ((m_type == LTPTYPE_RD)
//       || (m_type == LTPTYPE_RD_CP)
//       || (m_type == LTPTYPE_RD_CP_EORP)
//       || (m_type == LTPTYPE_RD_CP_EORP_EOB)
//       || (m_type == LTPTYPE_GD)
//       || (m_type == LTPTYPE_GD_UF1)
//       || (m_type == LTPTYPE_GD_UF2)
//       || (m_type == LTPTYPE_GD_EOB))
//    {
//
//      os << " Dst. Client Service ID: " << m_clientServiceId
//         << " Offset: " << m_offset << ", Length: " << m_length;
//
//      if ((m_type == LTPTYPE_RD_CP)
//          || (m_type == LTPTYPE_RD_CP_EORP)
//          || (m_type == LTPTYPE_RD_CP_EORP_EOB))
//        {
//          os << ", CP Serial Num: " << m_cpSerialNumber << ", RP Serial Num: " << m_rpSerialNumber;
//
//        }
//    }
//  else if  (m_type == LTPTYPE_RS)               /* Report Segments*/
//    {
//      os << "CP Serial Num: " << m_cpSerialNumber << ", RP Serial Num: " << m_rpSerialNumber
//         << ", Lower Bound: " << m_lowerBound << ", Upper Bound: " << m_upperBound << ", RxClaim Count : " <<
//      m_rxClaimCnt;
//
//    }
//  else if  (m_type == LTPTYPE_RAS)              /* Report ACK Segment*/
//    {
//      os << " RP Serial Num: " << m_rpSerialNumber;
//    }
//  else if ( (m_type == LTPTYPE_CS) ||     (m_type == LTPTYPE_CR) )       /* Cancel Segments*/
//    {
//      os << " CX Reason: " << m_cxReason;
//    }
//  else if ( (m_type == LTPTYPE_CAS) ||    (m_type == LTPTYPE_CAR) )       /* Cancel ACK Segments*/
//    {
//      // CAX have not contents
//    }
//  else
//    {
//      os << " Unkown Segment Type";
//    }

  os << ")";
}

void
LtpContentHeader::SetClientServiceId (uint64_t id)
{
  NS_LOG_FUNCTION (this << " " << id);

  if ((m_type >= 0) && (m_type <= 7))
    {
      m_clientServiceId = id;
    }
  else
    {
      m_clientServiceId = 0;
    }
}
uint64_t
LtpContentHeader::GetClientServiceId () const
{
  NS_LOG_FUNCTION (this);
  return m_clientServiceId;
}
void
LtpContentHeader::SetOffset (uint64_t val)
{
  NS_LOG_FUNCTION (this << " " << val);

  if ((m_type >= 0) && (m_type <= 7))
    {
      m_offset = val;
    }
  else
    {
      m_offset = 0;
    }
}
uint64_t
LtpContentHeader::GetOffset () const
{
  return m_offset;
}

void
LtpContentHeader::SetLength (uint64_t val)
{
  NS_LOG_FUNCTION (this << " " << val);

  if ((m_type >= 0) && (m_type <= 7))
    {
      m_length = val;
    }
  else
    {
      m_length = 0;
    }
}
uint64_t
LtpContentHeader::GetLength () const
{
  NS_LOG_FUNCTION (this);
  return m_length;
}

void
LtpContentHeader::SetCpSerialNumber (uint64_t num)
{
  NS_LOG_FUNCTION (this << " " << num);

//  if ((m_type == LTPTYPE_RD_CP)
//      || (m_type == LTPTYPE_RD_CP_EORP)
//      || (m_type == LTPTYPE_RD_CP_EORP_EOB)
//      || (m_type == LTPTYPE_RS)
//      || (m_type == LTPTYPE_RAS)
//      )
//    {
//      m_cpSerialNumber = num;
//    }
//  else
//    {
//      m_cpSerialNumber = 0;
//    }
}

uint64_t
LtpContentHeader::GetCpSerialNumber () const
{
  NS_LOG_FUNCTION (this);
  return m_cpSerialNumber;
}

void
LtpContentHeader::SetRpSerialNumber (uint64_t num)
{
  NS_LOG_FUNCTION (this << " " << num);

//  if ((m_type == LTPTYPE_RD_CP)
//      || (m_type == LTPTYPE_RD_CP_EORP)
//      || (m_type == LTPTYPE_RD_CP_EORP_EOB)
//      || (m_type == LTPTYPE_RS)
//      || (m_type == LTPTYPE_RAS)
//      )
//    {
//      m_rpSerialNumber = num;
//    }
//  else
//    {
//      m_rpSerialNumber = 0;
//    }
}

uint64_t
LtpContentHeader::GetRpSerialNumber () const
{
  NS_LOG_FUNCTION (this);
  return m_rpSerialNumber;
}

void
LtpContentHeader::SetUpperBound (uint64_t bound)
{
  NS_LOG_FUNCTION (this << " " << bound);
//  if  (m_type == LTPTYPE_RS)
//    {
//      m_upperBound = bound;
//    }
//  else
//    {
//      m_upperBound = 0;
//    }
}
uint64_t
LtpContentHeader::GetUpperBound () const
{
  NS_LOG_FUNCTION (this);
  return m_upperBound;
}

void
LtpContentHeader::SetLowerBound (uint64_t bound)
{
  NS_LOG_FUNCTION (this << " " << bound);
//  if  (m_type == LTPTYPE_RS)
//    {
//      m_lowerBound = bound;
//    }
//  else
//    {
//      m_lowerBound = 0;
//    }
}
uint64_t
LtpContentHeader::GetLowerBound () const
{
  NS_LOG_FUNCTION (this);
  return m_lowerBound;
}

uint64_t
LtpContentHeader::GetRxClaimCnt () const
{
  NS_LOG_FUNCTION (this);
  return m_rxClaimCnt;
}

void
LtpContentHeader::AddReceptionClaim (ReceptionClaim claim)
{
  NS_LOG_FUNCTION (this);
//  if  (m_type == LTPTYPE_RS)
//    {
//      uint64_t max_len = m_upperBound - m_lowerBound;
//      uint64_t min_offset = 0;
//      uint64_t claim_upperBound = claim.offset + claim.length + m_lowerBound;
//
//      if (!m_rxClaims.empty ())
//        {
//          min_offset = m_rxClaims.back ().offset + m_rxClaims.back ().length;
//        }
//
//      bool valid = false;
//
//      valid = ((claim.length > 1) && (claim.length <= max_len)              /* Reception Claim Length regulations */
//               && ((claim.offset >= min_offset) || (claim.offset == 0  && min_offset == 0))                  /* Reception Claim Offset regulations */
//               && ( claim_upperBound <= m_upperBound)               /* Claim upper bound must not exceed the report upper bound*/
//               );
//
//      if (valid)
//        {
//          m_rxClaims.push_back (claim);
//        }
//      else
//        {
//          NS_FATAL_ERROR ("Wrong Claim Boundaries - Offset:" << claim.offset << " Length:" << claim.length << " " << min_offset << " " << ((claim.length > 1) && (claim.length <= max_len)) << " Claim upper bound: " << claim_upperBound << " Report upper bound: " << m_upperBound  );
//        }
//
//      m_rxClaimCnt = m_rxClaims.size ();
//    }
}

LtpContentHeader::ReceptionClaim
LtpContentHeader::GetReceptionClaim (uint32_t index)
{
  NS_LOG_FUNCTION (this);
  return m_rxClaims.at (index);
}

void
LtpContentHeader::ClearReceptionClaims ()
{
  NS_LOG_FUNCTION (this);
  m_rxClaims.clear ();
  m_rxClaimCnt = m_rxClaims.size ();
}


void
LtpContentHeader::SetCxReason (CxReasonCode code)
{
  NS_LOG_FUNCTION (this << " " << code);
//  if  ((m_type == LTPTYPE_CS) ||  (m_type == LTPTYPE_CR))
//    {
//      m_cxReason = (uint8_t) code;
//    }
//  else
//    {
//      m_cxReason = 0;
//    }
}
CxReasonCode
LtpContentHeader::GetCxReason () const
{
  NS_LOG_FUNCTION (this);
  return (CxReasonCode) m_cxReason;
}


bool
SessionId::operator == (SessionId const &o) const
{
  return ((m_sessionNumber == o.m_sessionNumber)
          &&      (m_sessionOriginator == o.m_sessionOriginator));
}

bool
SessionId::operator < (SessionId const &o) const
{
  return ((m_sessionNumber < o.m_sessionNumber)
          &&      (m_sessionOriginator <= o.m_sessionOriginator));
}


bool
LtpExtension::operator == (LtpExtension const &o) const
{
  return ((m_extType == o.m_extType)
          && (m_len == o.m_len)
          && (m_value == o.m_value));
}

bool
LtpHeader::operator == (LtpHeader const &o) const
{
  return ((m_version == o.m_version)
          && (m_typeFlags == o.m_typeFlags)
          && (m_sessionId == o.m_sessionId)
          && (m_hdrExtensionCnt == o.m_hdrExtensionCnt)
          && (m_trailerExtensionCnt == o.m_trailerExtensionCnt)
          && (m_extensions == o.m_extensions));
}

bool
LtpTrailer::operator == (LtpTrailer const &o) const
{
  return (m_extensions == o.m_extensions);
}

bool
LtpContentHeader::operator== (LtpContentHeader const &o) const
{
//  bool result = false;
//
//  if (m_type == o.m_type)
//    {
//      if  ((m_type == LTPTYPE_RD)
//           || (m_type  == LTPTYPE_RD_CP)
//           || (m_type  == LTPTYPE_RD_CP_EORP)
//           || (m_type  == LTPTYPE_RD_CP_EORP_EOB)
//           || (m_type  == LTPTYPE_GD)
//           || (m_type  == LTPTYPE_GD_UF1)
//           || (m_type  == LTPTYPE_GD_UF2)
//           || (m_type  == LTPTYPE_GD_EOB))
//        {
//
//          result = ((m_clientServiceId == o.m_clientServiceId)
//                    && (m_offset == o.m_offset)
//                    && (m_length == o.m_length));
//
//          if ((m_type == LTPTYPE_RD_CP)
//              || (m_type  == LTPTYPE_RD_CP_EORP)
//              || (m_type  == LTPTYPE_RD_CP_EORP_EOB))
//            {
//              result = (result
//                        && (m_cpSerialNumber == o.m_cpSerialNumber)
//                        && (m_rpSerialNumber == o.m_rpSerialNumber));
//
//            }
//        }
//      else if  (m_type == LTPTYPE_RS)                           /* Report Segments*/
//        {
//          result = (( m_rpSerialNumber == o.m_rpSerialNumber)
//                    && (m_cpSerialNumber == o.m_cpSerialNumber)
//                    && (m_upperBound == o.m_upperBound)
//                    && (m_lowerBound == o.m_lowerBound)
//                    && (m_rxClaimCnt == o.m_rxClaimCnt)
//                    && (m_rxClaims == o.m_rxClaims));
//
//        }
//      else if  (m_type  == LTPTYPE_RAS)                          /* Report ACK Segment*/
//        {
//          result = (m_rpSerialNumber == o.m_rpSerialNumber);
//        }
//      else if ( (m_type  == LTPTYPE_CS) ||     (m_type  == LTPTYPE_CR) )                   /* Cancel Segments*/
//        {
//          result = (m_cxReason == o.m_cxReason);
//        }
//      else if ( (m_type  == LTPTYPE_CAS) ||    (m_type  == LTPTYPE_CAR) )                   /* Cancel ACK Segments*/
//        {
//          result = true;
//        }
//      return result;
//    }
  return false;
}
bool operator == (LtpContentHeader::ReceptionClaim const &a, LtpContentHeader::ReceptionClaim const &b)
{
  return ((a.length == b.length) && (a.offset == b.offset));
}

bool
LtpContentHeader::ReceptionClaim::operator < (ReceptionClaim const &o) const
{
  return offset < o.offset;
}

std::ostream&
operator << (std::ostream& os, const SessionId& id)
{
  os << id.GetSessionNumber () << " " << id.GetSessionOriginator ();
  return os;
}

//} // namespace ltp
} // namespace ns3

