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

#include "red_channels/clipboard.hpp"
#include "redjs/channel_receiver.hpp"

#include "red_emscripten/em_asm.hpp"
#include "red_emscripten/val.hpp"

#include "utils/sugar/buf_maker.hpp"
#include "utils/log.hpp"

#include "core/RDP/clipboard.hpp"
#include "core/callback.hpp"
#include "mod/rdp/rdp_verbose.hpp"


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


struct redjs::ClipboardChannel::D
{
    D(Callback& cb, emscripten::val&& callbacks, RDPVerbose verbose)
    : cb(cb)
    , callbacks(std::move(callbacks))
    , verbose(bool(verbose & RDPVerbose::cliprdr))
    {}

    void requestFormat(uint32_t format_id)
    {
        LOG_IF(this->verbose, LOG_INFO, "Clipboard: Send Request Format");

        RDPECLIP::CliprdrHeader formatListRequestPDUHeader(RDPECLIP::CB_FORMAT_DATA_REQUEST, RDPECLIP::CB_RESPONSE__NONE_, 4);
        RDPECLIP::FormatDataRequestPDU formatDataRequestPDU(format_id);
        StaticOutStream<256> out_stream;
        formatListRequestPDUHeader.emit(out_stream);
        formatDataRequestPDU.emit(out_stream);
        InStream chunkRequest(out_stream.get_bytes());

        this->send_to_mod_channel(out_stream);
    }

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

        emval_call(this->callbacks, "receiveFormat", false, true);

        while (format_list_extractor(
            data, chunk,
            IsLongFormat(this->format_list.use_long_format_names),
            IsAscii(msgFlags & RDPECLIP::CB_ASCII_NAMES)))
        {
            cbytes_view utf8_name = ""_av;

            if (data.av_name.size())
            {
                switch (data.charset)
                {
                case Charset::Ascii:
                    utf8_name = data.av_name;
                    break;
                case Charset::Utf16:
                    auto av = buf.dyn_array(data.av_name.size() * 2 + 1);
                    av = av.first(::UTF16toUTF8(
                        data.av_name.data(), data.av_name.size(),
                        av.data(), av.size()) + 1);
                    av.back() = '\0';
                    utf8_name = av;
                    break;
                }
            }
            else
            {
                switch (data.format_id)
                {
                case RDPECLIP::CF_TEXT:            utf8_name = "text"_av; break;
                case RDPECLIP::CF_BITMAP:          utf8_name = "bitmap"_av; break;
                case RDPECLIP::CF_METAFILEPICT:    utf8_name = "metafilepict"_av; break;
                case RDPECLIP::CF_SYLK:            utf8_name = "sylk"_av; break;
                case RDPECLIP::CF_DIF:             utf8_name = "dif"_av; break;
                case RDPECLIP::CF_TIFF:            utf8_name = "tiff"_av; break;
                case RDPECLIP::CF_OEMTEXT:         utf8_name = "oemtext"_av; break;
                case RDPECLIP::CF_DIB:             utf8_name = "dib"_av; break;
                case RDPECLIP::CF_PALETTE:         utf8_name = "palette"_av; break;
                case RDPECLIP::CF_PENDATA:         utf8_name = "pendata"_av; break;
                case RDPECLIP::CF_RIFF:            utf8_name = "riff"_av; break;
                case RDPECLIP::CF_WAVE:            utf8_name = "wave"_av; break;
                case RDPECLIP::CF_UNICODETEXT:     utf8_name = "unicodetext"_av; break;
                case RDPECLIP::CF_ENHMETAFILE:     utf8_name = "enhmetafile"_av; break;
                case RDPECLIP::CF_HDROP:           utf8_name = "hdrop"_av; break;
                case RDPECLIP::CF_LOCALE:          utf8_name = "locale"_av; break;
                case RDPECLIP::CF_DIBV5:           utf8_name = "dibv5"_av; break;
                case RDPECLIP::CF_OWNERDISPLAY:    utf8_name = "ownerdisplay"_av; break;
                case RDPECLIP::CF_DSPTEXT:         utf8_name = "dsptext"_av; break;
                case RDPECLIP::CF_DSPBITMAP:       utf8_name = "dspbitmap"_av; break;
                case RDPECLIP::CF_DSPMETAFILEPICT: utf8_name = "dspmetafilepict"_av; break;
                case RDPECLIP::CF_DSPENHMETAFILE:  utf8_name = "dspenhmetafile"_av; break;
                case RDPECLIP::CF_PRIVATEFIRST:    utf8_name = "privatefirst"_av; break;
                case RDPECLIP::CF_PRIVATELAST:     utf8_name = "privatelast"_av; break;
                case RDPECLIP::CF_GDIOBJFIRST:     utf8_name = "gdiobjfirst"_av; break;
                case RDPECLIP::CF_GDIOBJLAST:      utf8_name = "gdiobjlast"_av; break;
                }
            }

            emval_call(this->callbacks, "receiveFormat", data.format_id, utf8_name.data());
        }

        this->send_format_list_response_ok();

        emval_call(this->callbacks, "receiveFormat", false, false);
    }

    void send_format_list_response_ok()
    {
        StaticOutStream<256> out_stream;
        RDPECLIP::CliprdrHeader formatListResponsePDUHeader(
            RDPECLIP::CB_FORMAT_LIST_RESPONSE, RDPECLIP::CB_RESPONSE_OK, 0);
        formatListResponsePDUHeader.emit(out_stream);
        InStream chunk_format_list(out_stream.get_bytes());
        this->send_to_mod_channel(out_stream);
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

        this->cb.send_to_mod_channel(
            channel_names::cliprdr,
            chunk,
            out_stream.get_offset(),
            first_last_channel_flags
        );
    }

    Callback& cb;
    emscripten::val callbacks;
    FormatListEmptyName format_list;
    bool verbose;
};

namespace redjs
{

ClipboardChannel::ClipboardChannel(Callback& cb, emscripten::val callbacks, unsigned long verbose)
: d(std::make_unique<D>(cb, std::move(callbacks), RDPVerbose(verbose)))
{}

ClipboardChannel::~ClipboardChannel() = default;

void ClipboardChannel::receive(cbytes_view data, int flags)
{
    this->d->receive(InStream(data), flags);
}

} // namespace redjs


#include "red_emscripten/bind.hpp"

EMSCRIPTEN_BINDINGS(channel_clipboard)
{
    redjs::class_<redjs::ClipboardChannel>("ClipboardChannel")
        .constructor<uintptr_t, emscripten::val, unsigned long>([](uintptr_t&& icb, auto&&... args) {
            auto* pcb = reinterpret_cast<Callback*>(icb);
            return new redjs::ClipboardChannel(*pcb, std::move(args)...);
        })
        .function_ptr("getChannelReceiver", [](redjs::ClipboardChannel& clip) {
            auto receiver = [&clip](cbytes_view data, int channel_flags){
                clip.receive(data, channel_flags);
            };
            return redjs::ChannelReceiver(channel_names::cliprdr, receiver);
        })
        // .function("receive", &redjs::ClipboardChannel::receive)
    ;
}
