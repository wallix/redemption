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
Copyright (C) Wallix 2010-2019
Author(s): Jonathan Poelen
*/

#include "redjs/clipboard.hpp"

#include "red_emscripten/em_asm.hpp"

#include "utils/sugar/buf_maker.hpp"
#include "utils/log.hpp"

#include "core/RDP/clipboard.hpp"
#include "core/callback.hpp"


namespace
{
    constexpr int first_last_channel_flags
      = CHANNELS::CHANNEL_FLAG_LAST
      | CHANNELS::CHANNEL_FLAG_FIRST
      | CHANNELS::CHANNEL_FLAG_SHOW_PROTOCOL
    ;

    enum class ClipboardFormat : uint32_t
    {
        Text = 1,
        UnicodeText = 13,
    };

    struct FormatListEmptyName
    {
        std::size_t size() const noexcept
        {
            return
                4 /*FormatId*/
            // empty name
              + (this->use_long_format_names ? 2 /* unicode null character*/ : 32);
        }

        void emit(ClipboardFormat cf, OutStream& out_stream)
        {
            out_stream.out_uint32_le(underlying_cast(cf));

            if (this->use_long_format_names)
            {
                out_stream.out_uint8(0);
                out_stream.out_uint8(0);
            }
            else
            {
                char buffer[32]{};
                out_stream.out_copy_bytes(make_array_view(buffer));
            }
        }

        bool use_long_format_names = true;
    };

    enum class Charset : bool
    {
        Ascii,
        Utf16,
    };

    struct FormatListExtractorData
    {
        uint32_t format_id;
        Charset charset;
        cbytes_view av_name;
    };

    enum class IsLongFormat : bool;
    enum class IsAscii : bool;

    bool format_list_extractor(
        FormatListExtractorData& data, InStream& stream,
        IsLongFormat is_long_format, IsAscii is_ascii)
    {
        // formatId(4) + wszFormatName(variable, min = "\x00\x00" => 2)
        constexpr size_t min_long_format_name_data_length = 6;

        // formatId(4) + formatName(32)
        constexpr size_t short_format_name_data_length = 36;

        constexpr size_t format_name_data_length[] = {
            short_format_name_data_length,
            min_long_format_name_data_length
        };

        if (stream.in_remain() < format_name_data_length[int(is_long_format)])
        {
            return false;
        }

        data.format_id = stream.in_uint32_le();

        if (bool(is_long_format))
        {
            data.charset = Charset::Utf16;
            data.av_name = {stream.get_current(), ::UTF16StrLen(stream.get_current())};

            stream.in_skip_bytes((data.av_name.size() + 1) * sizeof(uint16_t));
        }
        else if (bool(is_ascii))
        {
            data.charset = Charset::Ascii;
            data.av_name = {stream.get_current(), strlen(char_ptr_cast(stream.get_current()))};

            stream.in_skip_bytes(short_format_name_data_length - 4 /* formatId(4) */);
        }
        else
        {
            data.charset = Charset::Utf16;
            data.av_name = {stream.get_current(), ::UTF16StrLen(stream.get_current())};

            stream.in_skip_bytes(short_format_name_data_length - 4 /* formatId(4) */);
        }

        return true;
    }

}


struct redjs::Clipboard::D
{
    D(RDPVerbose verbose)
    : verbose(bool(verbose & RDPVerbose::cliprdr))
    {}

    void receive(InStream chunk, int flags)
    {
        if (!(flags & CHANNELS::CHANNEL_FLAG_FIRST))
        {
            return;
        }

        RDPECLIP::CliprdrHeader header;
        header.recv(chunk);

        switch (header.msgType())
        {
        case RDPECLIP::CB_CLIP_CAPS:
            LOG_IF(this->verbose, LOG_INFO, "Clipboard: Capabilities PDU");
            this->process_capabilities(chunk);
            break;

        case RDPECLIP::CB_MONITOR_READY:
            LOG_IF(this->verbose, LOG_INFO, "Clipboard: Monitor Ready PDU");
            this->process_monitor_ready();
            break;

        case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
            LOG_IF(this->verbose, LOG_INFO, "Clipboard: Format List Response PDU%s",
                (header.msgFlags() == RDPECLIP::CB_RESPONSE_FAIL) ? " Failed" : "");
            break;

        case RDPECLIP::CB_FORMAT_LIST:
            LOG_IF(this->verbose, LOG_INFO, "Clipboard: Format List PDU");

            LOG_IF(header.dataLen() > chunk.in_remain(), LOG_WARNING,
                "Server Format List PDU data length(%u) longer than chunk(%zu)",
                header.dataLen(), chunk.in_remain());

            this->process_format_list(chunk, header.msgFlags());
            break;

            // case RDPECLIP::CB_LOCK_CLIPDATA:
            //     LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
            //         "SERVER >> CB Channel: Lock Clipboard Data PDU");
            // break;
            //
            // case RDPECLIP::CB_UNLOCK_CLIPDATA:
            //     LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
            //         "SERVER >> CB Channel: Unlock Clipboard Data PDU");
            // break;
            //
            // case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
            //     if (header.msgFlags() == RDPECLIP::CB_RESPONSE_FAIL) {
            //         LOG(LOG_WARNING, "SERVER >> CB Channel: Format Data Response PDU FAILED");
            //     } else {
            //         LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
            //             "SERVER >> CB Channel: Format Data Response PDU");
            //
            //         if(this->_requestedFormatName == RDPECLIP::FILEGROUPDESCRIPTORW.data()) {
            //             this->_requestedFormatId = ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW;
            //         }
            //
            //         this->_cb_buffers.sizeTotal = header.dataLen();
            //
            //         this->process_server_clipboard_indata(flags, chunk, this->_cb_buffers, this->_cb_filesList);
            //     }
            // break;
            //
            // case RDPECLIP::CB_FORMAT_DATA_REQUEST:
            //     LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
            //         "SERVER >> CB Channel: Format Data Request PDU");
            //
            //     this->process_format_data_request(chunk);
            // break;
            //
            // case RDPECLIP::CB_FILECONTENTS_REQUEST:
            //     LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
            //         "SERVER >> CB Channel: File Contents Resquest PDU");
            //
            //     this->process_filecontents_request(chunk);
            // break;
            //
            // case RDPECLIP::CB_FILECONTENTS_RESPONSE:
            //     if (header.msgFlags() == RDPECLIP::CB_RESPONSE_FAIL) {
            //         LOG(LOG_WARNING, "SERVER >> CB Channel: File Contents Response PDU FAILED");
            //     } else {
            //         LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
            //             "SERVER >> CB Channel: File Contents Response PDU");
            //
            //         if (this->_requestedFormatId == ClientCLIPRDRConfig::CF_QT_CLIENT_FILEGROUPDESCRIPTORW) {
            //             this->_requestedFormatId = ClientCLIPRDRConfig::CF_QT_CLIENT_FILECONTENTS;
            //         }
            //
            //         this->_cb_buffers.sizeTotal = header.dataLen();
            //
            //         this->process_server_clipboard_indata(flags, chunk, this->_cb_buffers, this->_cb_filesList);
            //     }
            // break;
            //
            // default:
            //     LOG_IF(bool(this->verbose & RDPVerbose::cliprdr), LOG_INFO,
            //         "SERVER >> Default Process server PDU data");
            //     this->process_server_clipboard_indata(flags, chunk, this->_cb_buffers, this->_cb_filesList);
            //
            // break;

        }
    }

    void process_format_list(InStream& chunk, uint32_t msgFlags)
    {
        FormatListExtractorData data;
        BufArrayMaker<256> buf;

        while (format_list_extractor(
            data, chunk,
            IsLongFormat(this->format_list.use_long_format_names),
            IsAscii(msgFlags & RDPECLIP::CB_ASCII_NAMES)))
        {
            cbytes_view utf8_name;
            switch (data.charset)
            {
            case Charset::Ascii:
                utf8_name = data.av_name;
                break;
            case Charset::Utf16:
                auto av = buf.dyn_array(data.av_name.size() * 2);
                utf8_name = av.first(::UTF16toUTF8(
                    data.av_name.data(), data.av_name.size(),
                    av.data(), av.size()));
                break;
            }
            LOG(LOG_INFO, "%d: %s: %.*s",
                data.format_id, RDPECLIP::get_FormatId_name(data.format_id),
                int(utf8_name.size()), utf8_name.as_charp());
        }
    }

    void process_capabilities(InStream& chunk)
    {
        RDPECLIP::ClipboardCapabilitiesPDU pdu;
        pdu.recv(chunk);

        RDPECLIP::GeneralCapabilitySet pdu2;
        pdu2.recv(chunk);

        this->format_list.use_long_format_names
            = bool(pdu2.generalFlags() & RDPECLIP::CB_USE_LONG_FORMAT_NAMES);
    }

    void process_monitor_ready()
    {
        {
            uint16_t cCapabilitiesSets = 1;
            uint32_t const generalFlags
                = (this->format_list.use_long_format_names ? RDPECLIP::CB_USE_LONG_FORMAT_NAMES : 0)
                | RDPECLIP::CB_STREAM_FILECLIP_ENABLED
                | RDPECLIP::CB_FILECLIP_NO_FILE_PATHS
                | RDPECLIP::CB_HUGE_FILE_SUPPORT_ENABLED
                // | RDPECLIP::CB_CAN_LOCK_CLIPDATA
            ;

            RDPECLIP::ClipboardCapabilitiesPDU clipboard_caps_pdu(cCapabilitiesSets);
            RDPECLIP::GeneralCapabilitySet general_cap_set(RDPECLIP::CB_CAPS_VERSION_2, generalFlags);
            RDPECLIP::CliprdrHeader header(RDPECLIP::CB_CLIP_CAPS, 0,
                clipboard_caps_pdu.size() + general_cap_set.size());

            StaticOutStream<1024> out_stream;
            header.emit(out_stream);
            clipboard_caps_pdu.emit(out_stream);
            general_cap_set.emit(out_stream);

            this->send_to_mod_channel(out_stream);

            LOG_IF(this->verbose, LOG_INFO, "Clipboard: Capabilities PDU");
        }

        {
            RDPECLIP::CliprdrHeader header(
                RDPECLIP::CB_FORMAT_LIST,
                RDPECLIP::CB_RESPONSE__NONE_ | RDPECLIP::CB_ASCII_NAMES,
                format_list.size());

            StaticOutStream<1600> out_stream;

            header.emit(out_stream);
            this->format_list.emit(ClipboardFormat::UnicodeText, out_stream);

            this->send_to_mod_channel(out_stream);

            LOG_IF(this->verbose, LOG_INFO, "Clipboard: Format List PDU");
        }
    }

    void send_to_mod_channel(OutStream const& out_stream)
    {
        InStream chunk(out_stream.get_bytes());

        this->cb->send_to_mod_channel(
            channel_names::cliprdr,
            chunk,
            out_stream.get_offset(),
            first_last_channel_flags
        );
    }


    Callback* cb = nullptr;
    FormatListEmptyName format_list;
    bool verbose;
};

namespace redjs
{

Clipboard::Clipboard(RDPVerbose verbose)
: d(std::make_unique<D>(verbose))
{}

Clipboard::~Clipboard() = default;

void Clipboard::receive(InStream chunk, int flags)
{
    this->d->receive(std::move(chunk), flags);
}

void Clipboard::set_cb(Callback* cb)
{
    this->d->cb = cb;
}



} // namespace redjs
