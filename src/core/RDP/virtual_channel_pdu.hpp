/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  Product name: redemption, a FLOSS RDP proxy
  Copyright (C) Wallix 2010
  Author(s): Christophe Grosjean, Javier Caverni
  Based on xrdp Copyright (C) Jay Sorg 2004-2010

  Channels descriptors
*/


#pragma once

#include "utils/stream.hpp"
#include "utils/hexdump.hpp"
#include "core/RDP/sec.hpp"
#include "core/RDP/x224.hpp"
#include "core/RDP/mcs.hpp"
#include "mod/rdp/server_transport_context.hpp"

namespace CHANNELS
{
    // Instructions detailing how to compress a data stream are listed in section
    //  3.1.8.2, while decompression of a data stream is described in section
    //  3.1.8.3.

    static const uint32_t PROXY_CHUNKED_VIRTUAL_CHANNEL_DATA_LENGTH_LIMIT = 1024 * 1024 * 10;   // 10 Mb


    // TODO: The VirtualChannelPDU object should be splitted in two objects: VirtualChannelServerPDU and VirtualChannelClientPDU
    // as MCS lmayer is differen anyway, the PDU can't be used in both directions at the same time
    // Also it seems it's a mere empty shell used only to store verbose level, splitting it in two separate object
    // would avoid using the same object in both directions (merely forcing to use the same verbose level in both directions)
    struct VirtualChannelPDU {
        bool verbose;

        explicit VirtualChannelPDU(bool verbose = false) : verbose(verbose) {}

        void send_to_server( ServerTransportContext & stc,
                             uint16_t channelId, uint32_t length, uint32_t flags
                           , bytes_view chunk) {
            this->send_<false, MCS::SendDataRequest_Send>(
                stc.trans, stc.encrypt, stc.negociation_result.encryptionLevel,
                stc.negociation_result.userid, channelId, length, flags, chunk
            );
        }

        void send_to_client( OutTransport trans, CryptContext & crypt_context, int encryptionLevel
                           , uint16_t userId, uint16_t channelId, uint32_t length, uint32_t flags
                           , bytes_view chunk) {
            this->send_<true, MCS::SendDataIndication_Send>(
                trans, crypt_context, encryptionLevel,
                userId, channelId, length, flags, chunk
            );
        }

    private:
        template<bool enable_verbose, class MCS_SendData>
        void send_(
          OutTransport trans, CryptContext & crypt_context, int encryptionLevel
        , uint16_t userId, uint16_t channelId, uint32_t length, uint32_t flags
        , bytes_view chunk) {
            write_packets(
                trans,
                [&](StreamSize<65536-1024> /*maxlen*/, OutStream & stream) {
                    stream.out_uint32_le(length);
                    stream.out_uint32_le(flags);
                    stream.out_copy_bytes(chunk);

                    if (enable_verbose && this->verbose) {
                        LOG(LOG_INFO, "Sec clear payload to send (channelId=%d):", channelId);
                        hexdump_d(stream.get_produced_bytes());
                    }
                },
                SEC::write_sec_send_fn{0, crypt_context, encryptionLevel},
                [&](StreamSize<256> /*maxlen*/, OutStream & mcs_header, std::size_t packet_size) {
                    MCS_SendData(mcs_header, userId, channelId, 1, 3, packet_size, MCS::PER_ENCODING);
                },
                X224::write_x224_dt_tpdu_fn{}
            );
        }
    };  // struct VirtualChannelPDU
} // namespace CHANNELS
