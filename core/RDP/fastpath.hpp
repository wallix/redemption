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
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   common fastpath layer at core module

*/

#ifndef _REDEMPTION_CORE_RDP_FASTPATH_HPP_
#define _REDEMPTION_CORE_RDP_FASTPATH_HPP_

// 2.2.9.1.2 Server Fast-Path Update PDU (TS_FP_UPDATE_PDU)
// ========================================================

// Fast-path revises server output packets from the first byte with the goal of
// improving bandwidth.

// The TPKT Header ([T123] section 8), X.224 Class 0 Data TPDU ([X224] section
//  13.7), and MCS Send Data Indication ([T125] section 11.33) are replaced; the
//  Security Header (section 2.2.8.1.1.2) is collapsed into the fast-path output
//  header; and the Share Data Header (section 2.2.8.1.1.1.2) is replaced by a
//  new fast-path format. The contents of the graphics and pointer updates (see
//  sections 2.2.9.1.1.3 and 2.2.9.1.1.4) are also changed to reduce their size,
//  particularly by removing or reducing headers. Support for fast-path output
//  is advertised in the General Capability Set (section 2.2.7.1.1).

// fpOutputHeader (1 byte): An 8-bit, unsigned integer. One-byte, bit-packed
//  header. This byte coincides with the first byte of the TPKT Header (see
// [T123] section 8). Two pieces of information are collapsed into this byte:
// - Encryption data
// - Action code

// actionCode (2 bits): Code indicating whether the PDU is in fast-path or
// slow-path format.

// +-------------------------------------+-----------------------------+
// | 0x0 FASTPATH_OUTPUT_ACTION_FASTPATH | Indicates that the PDU is a |
// |                                     | fast-path output PDU.       |
// +-------------------------------------+-----------------------------+
// | 0x3 FASTPATH_OUTPUT_ACTION_X224     | Indicates the presence of a |
// |                                     | TPKT Header (see [T123]     |
// |                                     | section 8) initial version  |
// |                                     | byte which indicates that   |
// |                                     | the PDU is a slow-path      |
// |                                     | output PDU (in this case the|
// |                                     | full value of the initial   |
// |                                     | byte MUST be 0x03).         |
// +-------------------------------------+-----------------------------+

// reserved (4 bits): Unused bits reserved for future use. This bitfield MUST
//  be set to 0.

// encryptionFlags (2 bits): Flags describing cryptographic parameters of the
//  PDU.

// +-------------------------------------+-------------------------------------+
// | 0x1 FASTPATH_OUTPUT_SECURE_CHECKSUM | Indicates that the MAC signature for|
// |                                     | the PDU was generated using the     |
// |                                     | "salted MAC generation" technique   |
// |                                     | (see section 5.3.6.1.1). If this bit|
// |                                     | is not set, then the standard       |
// |                                     | technique was used (see sections    |
// |                                     | 2.2.8.1.1.2.2 and 2.2.8.1.1.2.3).   |
// +-------------------------------------+-------------------------------------+
// | 0x2 FASTPATH_OUTPUT_ENCRYPTED       | Indicates that the PDU contains an  |
// |                                     | 8-byte MAC signature after the      |
// |                                     | optional length2 field (that is,    |
// |                                     | the dataSignature field is present),|
// |                                     | and the contents of the PDU are     |
// |                                     | encrypted using the negotiated      |
// |                                     | encryption package (see sections    |
// |                                     | 5.3.2 and 5.3.6).                   |
// +-------------------------------------+-------------------------------------+

// length1 (1 byte): An 8-bit, unsigned integer. If the most significant bit of
//  the length1 field is not set, then the size of the PDU is in the range 1 to
//  127 bytes and the length1 field contains the overall PDU length (the length2
//  field is not present in this case). However, if the most significant bit of
//  the length1 field is set, then the overall PDU length is given by the low
//  7 bits of the length1 field concatenated with the 8 bits of the length2
//  field, in big-endian order (the length2 field contains the low-order bits).

// length2 (1 byte): An 8-bit, unsigned integer. If the most significant bit of
// the length1 field is not set, then the length2 field is not present. If the
// most significant bit of the length1 field is set, then the overall PDU length
// is given by the low 7 bits of the length1 field concatenated with the 8 bits
// of the length2 field, in big-endian order (the length2 field contains the
// low-order bits).

// fipsInformation (4 bytes): Optional FIPS header information, present when the
//  Encryption Method selected by the server (sections 5.3.2 and 2.2.1.4.3) is
//  ENCRYPTION_METHOD_FIPS (0x00000010). The Fast-Path FIPS Information
//  structure is specified in section 2.2.8.1.2.1.

// dataSignature (8 bytes): MAC generated over the packet using one of the
// techniques specified in section 5.3.6 (the FASTPATH_OUTPUT_SECURE_CHECKSUM
// flag, which is set in the fpOutputHeader field, describes the method used to
// generate the signature). This field MUST be present if the
//  FASTPATH_OUTPUT_ENCRYPTED flag is set in the fpOutputHeader field.

// fpOutputUpdates (variable): An array of Fast-Path Update (section
// 2.2.9.1.2.1) structures to be processed by the client.

TODO("To implement fastpath, the idea is to replace the current layer stack X224->Mcs->Sec with only one FastPath object. The FastPath layer would also handle legacy packets still using several independant layers. That should lead to a much simpler code in both front.hpp and rdp.hpp but still keep a flat easy to test model.")


#endif
