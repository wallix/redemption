/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   RDP Capabilities : Frame Acknowledge Capability Set ([MS-RDPRFX] section 2.2.1.3)

*/

#if !defined(__RDP_CAPABILITIES_WINDOW_HPP__)
#define __RDP_CAPABILITIES_WINDOW_HPP__

// The TS_FRAME_ACKNOWLEDGE_PDU structure is a client-to-server PDU sent to the server
// whenever the client receives a Frame Marker Command ([MS-RDPBCGR] section 2.2.9.2.3) with the
// frameAction field set to SURFACECMD_FRAMEACTION_END (0x0001) and it has finished
// processing this particular frame (that is, the surface bits have been rendered on the screen). The
// server uses this acknowledgment to throttle the rate at which it generates frames of data to avoid
// overloading the client with too many updates. The server SHOULD NOT wait for an acknowledgment
// for each frame before sending the next frame. Instead, the server SHOULD have a "window"
// mechanism where it allows multiple frames to be unacknowledged before it slows down its frame
// rate (or stops completely).


// tpktHeader (4 bytes): A TPKT Header as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224] section 13.7.

// mcsSDrq (variable): Variable-length PER-encoded MCS Domain PDU (DomainMCSPDU) that
//   encapsulates an MCS Send Data Request structure (SDrq, choice 25 from DomainMCSPDU).
//   This is as specified in [T125] section 11.32, and the ASN.1 structure definitions are specified
//   in [T125] section 7, parts 7 and 10. The userData field of the MCS Send Data Request PDU
//   contains a Security Header, Share Data Header, and a frame ID.

// securityHeader (variable): Optional security header. The presence and format of the security
//   header depend on the Encryption Level and Encryption Method selected by the server ([MS-
//   RDPBCGR] sections 5.3.2 and 2.2.1.4.3). If the Encryption Level selected by the server is
//   greater than ENCRYPTION_LEVEL_NONE (0) and the Encryption Method selected by the server
//   is greater than ENCRYPTION_METHOD_NONE (0), this field MUST contain one of the following
//   headers:

//   * Non-FIPS Security Header ([MS-RDPBCGR] section 2.2.8.1.1.2.2) if the Encryption Method
//     selected by the server is ENCRYPTION_METHOD_40BIT (0x00000001),
//     ENCRYPTION_METHOD_56BIT (0x00000008), or ENCRYPTION_METHOD_128BIT
//     (0x00000002).

//   * FIPS Security Header ([MS-RDPBCGR] section 2.2.8.1.1.2.3) if the Encryption Level
//     selected by the server is ENCRYPTION_METHOD_FIPS (0x00000010).

//   If the Encryption Level selected by the server is ENCRYPTION_LEVEL_NONE (0) and the
//   Encryption Method selected by the server is ENCRYPTION_METHOD_NONE (0), this header
//   MUST NOT be included in the PDU.

// shareDataHeader (18 bytes): Share Data Header ([MS-RDPBCGR] section 2.2.8.1.1.1.2)
//   containing information about the packet. The type subfield of the pduType field of the Share
//   Control Header ([MS-RDPBCGR] section 2.2.8.1.1.1.1) within the Share Data Header MUST be
//   set to PDUTYPE_DATAPDU (0x0007). The pduType2 field of the Share Data Header MUST be
//   set to PDUTYPE2_FRAME_ACKNOWLEDGE (0x38).

// frameID (4 bytes): A 32-bit unsigned integer. This field specifies the 32-bit identifier of the
//   frame that was sent to the client using a Frame Marker Command and is being acknowledged
//   as delivered. If frameID has the value 0xFFFFFFFF, the server SHOULD assume that all in-
//   flight frames have been acknowledged.


#endif
