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
   Author(s): Christophe Grosjean

   RDP Capabilities :

*/

#if !defined(__RDP_CAPABILITIES_INPUT_HPP__)
#define __RDP_CAPABILITIES_INPUT_HPP__

//2.2.7.1.6 Input Capability Set (TS_INPUT_CAPABILITYSET)
//=======================================================

//The TS_INPUT_CAPABILITYSET structure is used to advertise support for input formats and devices. This capability is sent by both client and server.

//capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the capability set. This field MUST be set to CAPSTYPE_INPUT (13).

//lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes of the capability data, including the size of the capabilitySetType and lengthCapability fields.

//inputFlags (2 bytes): A 16-bit, unsigned integer. Input support flags.

//0x0001 INPUT_FLAG_SCANCODES Indicates support for using scancodes in the Keyboard Event notifications (see sections 2.2.8.1.1.3.1.1.1 and 2.2.8.1.2.2.1).

//0x0004 INPUT_FLAG_MOUSEX Indicates support for Extended Mouse Event notifications (see sections 2.2.8.1.1.3.1.1.4 and 2.2.8.1.2.2.4).

//0x0008 INPUT_FLAG_FASTPATH_INPUT Advertised by RDP 5.0 and 5.1 servers. RDP 5.2, 6.0, 6.1, and 7.0 servers advertise the INPUT_FLAG_FASTPATH_INPUT2 flag to indicate support for fast-path input.

//0x0010 INPUT_FLAG_UNICODE Indicates support for Unicode Keyboard Event notifications (see sections 2.2.8.1.1.3.1.1.2 and 2.2.8.1.2.2.2).

//0x0020 INPUT_FLAG_FASTPATH_INPUT2 Advertised by RDP 5.2, 6.0, 6.1, and 7.0 servers. Clients that do not support this flag will not be able to use fast-path input when connecting to RDP 5.2, 6.0, 6.1, and 7.0 servers.

//The INPUT_FLAG_SCANCODES flag MUST be set and is required for a connection to proceed as RDP keyboard input is restricted to keyboard scancodes (unlike the code-point or virtual codes supported in [T128]).

//pad2octetsA (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field MUST be ignored.

//keyboardLayout (4 bytes): A 32-bit, unsigned integer. Keyboard layout (active input locale identifier). For a list of input locale identifiers, see [MSFT-DIL]. This value is only specified in the client Input Capability Set and SHOULD correspond with that sent in the Client Core Data (section 2.2.1.3.2).

//keyboardType (4 bytes): A 32-bit, unsigned integer. Keyboard type.

//0x00000001 IBM PC/XT or compatible (83-key) keyboard
//0x00000002 Olivetti "ICO" (102-key) keyboard
//0x00000003 IBM PC/AT (84-key) or similar keyboard
//0x00000004 IBM enhanced (101- or 102-key) keyboard
//0x00000005 Nokia 1050 and similar keyboards
//0x00000006 Nokia 9140 and similar keyboards
//0x00000007 Japanese keyboard

//This value is only specified in the client Input Capability Set and SHOULD correspond with that sent in the Client Core Data.

//keyboardSubType (4 bytes): A 32-bit, unsigned integer. Keyboard subtype (an original equipment manufacturer-dependent value). This value is only specified in the client Input Capability Set and SHOULD correspond with that sent in the Client Core Data.

//keyboardFunctionKey (4 bytes): A 32-bit, unsigned integer. Number of function keys on the keyboard. This value is only specified in the client Input Capability Set and SHOULD correspond with that sent in the Client Core Data.

//imeFileName (64 bytes): A 64-byte field. Input Method Editor (IME) file name associated with the input locale. This field contains up to 31 Unicode characters plus a null terminator and is only specified in the client Input Capability Set and its contents SHOULD correspond with that sent in the Client Core Data.

static inline void out_input_caps(Stream & stream)
{
    const char caps_input[] = {
    0x01, 0x00, 0x00, 0x00, 0x09, 0x04, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
    };
    stream.out_uint16_le(0x0D);
    stream.out_uint16_le(0x58);
    stream.out_copy_bytes(caps_input, 0x54);
}

static inline void front_out_input_caps(Stream & stream)
{
    stream.out_uint16_le(RDP_CAPSET_INPUT); /* 13(0xd) */
    stream.out_uint16_le(RDP_CAPLEN_INPUT); /* 88(0x58) */
    stream.out_uint8(1);
    stream.out_clear_bytes(83);
}
#endif
