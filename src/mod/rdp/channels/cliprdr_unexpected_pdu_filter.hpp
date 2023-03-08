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
  Author(s): Christophe Grosjean, Javier Caverni, Dominique Lafages,
             Raphael Zhou, Meng Tan, Clément Moroldo
  Based on xrdp Copyright (C) Jay Sorg 2004-2010

  rdp module main header file
*/

#pragma once

#include "mod/rdp/channels/virtual_channel_filter.hpp"
#include "utils/log.hpp"

class CliprdrUnexpectedPDUFilter :
    public RemovableVirtualChannelFilter<CliprdrVirtualChannelProcessor>
{
    bool verbose = false;

public:
    CliprdrUnexpectedPDUFilter(bool verbose) : verbose(verbose) {}

private:
    enum class InitializationState : uint8_t
    {
        WaitingServerMonitorReadyPDU                               = 1,
        WaitingClientClipboardCapabilitiesPDU                      = 2,
        WaitingClientTemporaryDirectoryPDUOrFormatListPDUOrLockPDU = 4,
        WaitingClientFormatListPDUOrLockPDU                        = 8,
        WaitingServerFormatListResponsePDUOrLockPDU                = 16,
        Ready                                                      = 32,
    } initialization_state = InitializationState::WaitingServerMonitorReadyPDU;

    static const char * get_initialization_state_name(InitializationState state) {
        switch (state) {
            case InitializationState::WaitingServerMonitorReadyPDU:
                return "WaitingServerMonitorReadyPDU";

            case InitializationState::WaitingClientClipboardCapabilitiesPDU:
                return "WaitingClientClipboardCapabilitiesPDU";

            case InitializationState::WaitingClientTemporaryDirectoryPDUOrFormatListPDUOrLockPDU:
                return "WaitingClientTemporaryDirectoryPDUOrFormatListPDUOrLockPDU";

            case InitializationState::WaitingClientFormatListPDUOrLockPDU:
                return "WaitingClientFormatListPDUOrLockPDU";

            case InitializationState::WaitingServerFormatListResponsePDUOrLockPDU:
                return "WaitingServerFormatListResponsePDUOrLockPDU";

            case InitializationState::Ready:
                return "Ready";
        }

        return "<unknown>";
    }

public:
    void process_client_message(uint32_t total_length, uint32_t flags,
        bytes_view chunk_data, RDPECLIP::CliprdrHeader const* decoded_header) override
    {
        InStream s(chunk_data);

        // break process when states is incorrect
        #define IGNORE_OUT_OF_SEQUENCE(accepted_states) do {                   \
            if (REDEMPTION_UNLIKELY(!(                                         \
                checked_cast<unsigned>(accepted_states)                        \
              & unsigned(this->initialization_state)                           \
            ))) {                                                              \
                LOG(LOG_WARNING,                                               \
                    "CliprdrUnexpectedPDUFilter::process_client_message: "     \
                        "Unexpected Client %s PDU! CurrentState=%s(%d)",       \
                    RDPECLIP::get_msgType_name(msgType),                       \
                    get_initialization_state_name(this->initialization_state), \
                    int(this->initialization_state));                          \
                return;                                                        \
            }                                                                  \
        } while (0)

        if ((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) &&
            (s.in_remain() >= 8 /* msgType(2) + msgFlags(2) + dataLen(4) */)) {

            const uint16_t msgType = s.in_uint16_le();
            switch (msgType) {
                case RDPECLIP::CB_CLIP_CAPS: {
                    IGNORE_OUT_OF_SEQUENCE(InitializationState::WaitingClientClipboardCapabilitiesPDU);

                    this->initialization_state
                        = InitializationState::WaitingClientTemporaryDirectoryPDUOrFormatListPDUOrLockPDU;
                }
                break;

                case RDPECLIP::CB_FORMAT_LIST: {
                    IGNORE_OUT_OF_SEQUENCE(
                        unsigned(InitializationState::WaitingClientTemporaryDirectoryPDUOrFormatListPDUOrLockPDU)
                      | unsigned(InitializationState::WaitingClientFormatListPDUOrLockPDU)
                      | unsigned(InitializationState::Ready));

                    if (REDEMPTION_UNLIKELY(InitializationState::Ready != this->initialization_state))
                    {
                        this->initialization_state = InitializationState::WaitingServerFormatListResponsePDUOrLockPDU;
                    }
                }
                break;

                case RDPECLIP::CB_FILECONTENTS_REQUEST:
                case RDPECLIP::CB_FILECONTENTS_RESPONSE:
                case RDPECLIP::CB_FORMAT_DATA_REQUEST:
                case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
                case RDPECLIP::CB_FORMAT_LIST_RESPONSE: {
                    IGNORE_OUT_OF_SEQUENCE(InitializationState::Ready);
                }
                break;

                case RDPECLIP::CB_LOCK_CLIPDATA:
                case RDPECLIP::CB_UNLOCK_CLIPDATA: {
                    IGNORE_OUT_OF_SEQUENCE(
                        unsigned(InitializationState::WaitingClientTemporaryDirectoryPDUOrFormatListPDUOrLockPDU)
                      | unsigned(InitializationState::WaitingServerFormatListResponsePDUOrLockPDU)
                      | unsigned(InitializationState::WaitingClientFormatListPDUOrLockPDU)
                      | unsigned(InitializationState::Ready));
                }
                break;

                case RDPECLIP::CB_TEMP_DIRECTORY: {
                    IGNORE_OUT_OF_SEQUENCE(InitializationState::WaitingClientTemporaryDirectoryPDUOrFormatListPDUOrLockPDU);

                    this->initialization_state = InitializationState::WaitingClientFormatListPDUOrLockPDU;
                }
                break;
            }
        }

        this->get_next_filter_ptr()->process_client_message(total_length,
            flags, chunk_data, decoded_header);
    }

    void process_server_message(uint32_t total_length, uint32_t flags,
        bytes_view chunk_data, RDPECLIP::CliprdrHeader const* decoded_header) override
    {
        InStream s(chunk_data);

        if ((flags & (CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST)) &&
            (s.in_remain() >= 8 /* msgType(2) + msgFlags(2) + dataLen(4) */)) {

            const uint16_t msgType = s.in_uint16_le();
            switch (msgType) {
                case RDPECLIP::CB_FORMAT_LIST_RESPONSE: {
                    if (REDEMPTION_UNLIKELY(
                        InitializationState::WaitingServerFormatListResponsePDUOrLockPDU
                        == this->initialization_state
                    )) {
                        this->initialization_state = InitializationState::Ready;
                    }
                }
                break;

                case RDPECLIP::CB_MONITOR_READY: {
                    this->initialization_state = InitializationState::WaitingClientClipboardCapabilitiesPDU;
                }
                break;
            }
        }

        this->get_previous_filter_ptr()->process_server_message(total_length,
            flags, chunk_data, decoded_header);
    }
};
