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

   RDP Capabilities : Window Activation Capability Set (section 2.2.7.2.3)

*/

#if !defined(__RDP_CAPABILITIES_ACTIVATE_HPP__)
#define __RDP_CAPABILITIES_ACTIVATE_HPP__

// 2.2.7.2.3 Window Activation Capability Set
//(TS_WINDOWACTIVATION_CAPABILITYSET)

// The TS_WINDOWACTIVATION_CAPABILITYSET structure is used by the client to advertise window
// activation characteristics capabilities and is fully specified in [T128] section 8.2.9. This capability is
// only sent from client to server and the server ignores its contents.

// capabilitySetType (2 bytes): A 16-bit, unsigned integer. The type of the capability set. This
//  field MUST be set to CAPSTYPE_ACTIVATION (7).

// lengthCapability (2 bytes): A 16-bit, unsigned integer. The length in bytes of the capability
//   data, including the size of the capabilitySetType and lengthCapability fields.

// helpKeyFlag (2 bytes): A 16-bit, unsigned integer. This field SHOULD be set to FALSE
//   (0x0000).

// helpKeyIndexFlag (2 bytes): A 16-bit, unsigned integer. This field SHOULD be set to FALSE
//   (0x0000).

// helpExtendedKeyFlag (2 bytes): A 16-bit, unsigned integer. This field SHOULD be set to
//   FALSE (0x0000).

// windowManagerKeyFlag (2 bytes): A 16-bit, unsigned integer. This field SHOULD be set to
//   FALSE (0x0000).


struct ActivationCaps : public Capability {
    uint16_t helpKeyFlag;
    uint16_t helpKeyIndexFlag;
    uint16_t helpExtendedKeyFlag;
    uint16_t windowManagerKeyFlag;
    ActivationCaps()
    : Capability(CAPSTYPE_ACTIVATION, RDP_CAPLEN_ACTIVATION)
    , helpKeyFlag(0)
    , helpKeyIndexFlag(0)
    , helpExtendedKeyFlag(0)
    , windowManagerKeyFlag(0)
    {
    }

    void emit(Stream & stream){
        stream.out_uint16_le(this->capabilityType);
        stream.out_uint16_le(this->len);
        stream.out_uint16_le(this->helpKeyFlag);
        stream.out_uint16_le(this->helpKeyIndexFlag);
        stream.out_uint16_le(this->helpExtendedKeyFlag);
        stream.out_uint16_le(this->windowManagerKeyFlag);
    }

    void log(const char * msg){
        LOG(LOG_INFO, "%s Activation caps (%u bytes)", msg, this->len);
        LOG(LOG_INFO, "Activation caps::helpKeyFlag %u", this->helpKeyFlag);
        LOG(LOG_INFO, "Activation caps::helpKeyIndexFlag %u", this->helpKeyIndexFlag);
        LOG(LOG_INFO, "Activation caps::helpExtendedKeyFlag %u", this->helpExtendedKeyFlag);
        LOG(LOG_INFO, "Activation caps::windowManagerKeyFlag %u", this->windowManagerKeyFlag);
    }
};


//static inline void out_activate_caps(Stream & stream)
//{
//    LOG(LOG_INFO, "Sending Activate caps to server");

//    stream.out_uint16_le(CAPSTYPE_ACTIVATION);
//    stream.out_uint16_le(RDP_CAPLEN_ACTIVATION);
//    stream.out_uint16_le(0); /* Help key */
//    stream.out_uint16_le(0); /* Help index key */
//    stream.out_uint16_le(0); /* Extended help key */
//    stream.out_uint16_le(0); /* Window activate */
//}

#endif
