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

    cbytes_view quick_utf16_av(cbytes_view av)
    {
        auto first = av.begin();
        auto count = av.size() / 2;

        while (count > 0)
        {
            auto it = first;
            auto step = count / 2;
            it += step * 2;
            if (it[0] | it[1])
            {
                first = it + 2;
                count -= step + 1;
            }
            else
            {
                count = step;
            }
        }

        return av = av.first(first - av.begin());
    }

    cbytes_view utf16_av(uint8_t const* p)
    {
        return {p, UTF16StrLen(p) * 2};
    }

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
        constexpr size_t short_format_name_length = 32;
        constexpr size_t short_format_name_data_length = short_format_name_length + 4;

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
            data.av_name = utf16_av(stream.get_current());

            stream.in_skip_bytes(data.av_name.size() + 2);
        }
        else if (bool(is_ascii))
        {
            data.charset = Charset::Ascii;
            data.av_name = quick_utf16_av(stream.remaining_bytes());

            stream.in_skip_bytes(short_format_name_length);
        }
        else
        {
            data.charset = Charset::Utf16;
            data.av_name = quick_utf16_av(stream.remaining_bytes().first(short_format_name_length));

            stream.in_skip_bytes(short_format_name_length);
        }

        return true;
    }

}

namespace
{

enum class CustomFormat : uint32_t
{
    None = 0,

    FileGroupDescriptorW,
    FileContentsSize,
    FileContentsRange,
};

enum class CustomMessageType : int
{
    None = 0,
    // Is CustomFormat
    FileContentsSize,
    FileContentsRange,
};

}

struct redjs::ClipboardChannel::D
{
    D(Callback& cb, emscripten::val&& callbacks, RDPVerbose verbose)
    : cb(cb)
    , callbacks(std::move(callbacks))
    , verbose(bool(verbose & RDPVerbose::cliprdr))
    {}

    void send_file_contents_request(
        uint32_t request_type,
        uint32_t stream_id, uint32_t lindex,
        uint32_t pos_low, uint32_t pos_high)
    {
        StaticOutStream<64> out_stream;

        RDPECLIP::FileContentsRequestPDU request(
            stream_id,
            lindex,
            request_type,
            pos_low,
            pos_high,
            request_type == RDPECLIP::FILECONTENTS_SIZE
            ? RDPECLIP::FILECONTENTS_SIZE_CB_REQUESTED
            : 0x0000ffff // TODO maximum number of bytes to read from the remote file
        );

        RDPECLIP::CliprdrHeader header(
            RDPECLIP::CB_FILECONTENTS_REQUEST,
            RDPECLIP::CB_RESPONSE__NONE_,
            request.size());

        header.emit(out_stream);
        request.emit(out_stream);
        this->send_to_mod_channel(out_stream);

        this->custom_cf = (request_type == RDPECLIP::FILECONTENTS_SIZE)
            ? CustomFormat::FileContentsSize
            : CustomFormat::FileContentsRange ;
    }

    void send_request_format(uint32_t format_id, CustomFormat custom_cf)
    {
        LOG_IF(this->verbose, LOG_INFO, "Clipboard: Send Request Format");

        RDPECLIP::CliprdrHeader formatListRequestPDUHeader(RDPECLIP::CB_FORMAT_DATA_REQUEST, RDPECLIP::CB_RESPONSE__NONE_, 4);
        RDPECLIP::FormatDataRequestPDU formatDataRequestPDU(format_id);
        StaticOutStream<256> out_stream;
        formatListRequestPDUHeader.emit(out_stream);
        formatDataRequestPDU.emit(out_stream);
        InStream chunkRequest(out_stream.get_bytes());
        this->requested_format_id = format_id;
        this->custom_cf = custom_cf;

        this->send_to_mod_channel(out_stream);
    }

    void receive(InStream chunk, int channel_flags)
    {
        RDPECLIP::CliprdrHeader header;
        header.recv(chunk);

        if (header.msgFlags() == RDPECLIP::CB_RESPONSE_FAIL)
        {
            LOG(LOG_WARNING, "Clipboard: Format List Response PDU");
            this->response_buffer.release();
            return ;
        }

        if (this->response_buffer.waiting_for_data)
        {
            LOG_IF(this->verbose, LOG_INFO, "Clipboard: Format Data Response PDU continuation");
            this->process_format_data_response(chunk, channel_flags, header);
            return;
        }

        if (!(channel_flags & CHANNELS::CHANNEL_FLAG_FIRST))
        {
            return;
        }

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
            LOG_IF(this->verbose, LOG_INFO, "Clipboard: Format List Response PDU Failed");
            break;

        case RDPECLIP::CB_FORMAT_LIST:
            LOG_IF(this->verbose, LOG_INFO, "Clipboard: Format List PDU");

            LOG_IF(header.dataLen() > chunk.in_remain(), LOG_WARNING,
                "Server Format List PDU data length(%u) longer than chunk(%zu)",
                header.dataLen(), chunk.in_remain());

            this->process_format_list(chunk, channel_flags);
            break;

        case RDPECLIP::CB_FORMAT_DATA_RESPONSE:
            LOG_IF(this->verbose, LOG_INFO, "Clipboard: Format Data Response PDU");
            this->process_format_data_response(chunk, channel_flags, header);
        break;

        case RDPECLIP::CB_FILECONTENTS_RESPONSE: {
            LOG_IF(this->verbose, LOG_INFO, "Clipboard: File Contents Response PDU");
            // TODO
            this->process_format_data_response(chunk, channel_flags, header);
            }
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
            default:
                LOG_IF(this->verbose, LOG_ERR,
                    "Clipboard: Default Process server PDU data (%" PRIu16 ")", header.msgType());
                // this->process_server_clipboard_indata(flags, chunk, this->_cb_buffers, this->_cb_filesList);
                break;

        }
    }

    void process_format_data_response(InStream& chunk, uint32_t channel_flags, RDPECLIP::CliprdrHeader& header)
    {
        if (channel_flags & CHANNELS::CHANNEL_FLAG_FIRST) {
            this->response_buffer.reserve(header.dataLen());
        }

        hexdump_av(chunk.remaining_bytes());

        // TODO direct send, no buffer

        this->response_buffer.add(chunk.remaining_bytes());

        if (!(channel_flags & CHANNELS::CHANNEL_FLAG_LAST))
        {
            return ;
        }

        auto send_data = [&](char const* fname, cbytes_view data, auto const&... args){
            hexdump_av(data);
            emval_call(this->callbacks, fname, data.data(), data.size(), args...);
        };

        auto send_data2 = [&](char const* fname, auto const&... args){
            emval_call(this->callbacks, fname, args...);
        };

        auto remove_last_char = [](cbytes_view data, std::size_t strip_n) {
            return data.size() >= strip_n ? data.first(data.size() - strip_n) : cbytes_view{};
        };

        LOG(LOG_DEBUG, "id = %u  customcf = %u", this->requested_format_id, this->custom_cf);

        auto data = this->response_buffer.final();
        hexdump_av(data);

        switch (this->custom_cf)
        {
        case CustomFormat::None:
            switch (this->requested_format_id)
            {
                case RDPECLIP::CF_TEXT:
                    data = remove_last_char(data, 1);
                    break;
                case RDPECLIP::CF_UNICODETEXT:
                    data = remove_last_char(data, 2);
                    break;
                // case RDPECLIP::CF_BITMAP:          utf8_name = "bitmap"_av; break;
                // case RDPECLIP::CF_METAFILEPICT:    utf8_name = "metafilepict"_av; break;
                // case RDPECLIP::CF_SYLK:            utf8_name = "sylk"_av; break;
                // case RDPECLIP::CF_DIF:             utf8_name = "dif"_av; break;
                // case RDPECLIP::CF_TIFF:            utf8_name = "tiff"_av; break;
                // case RDPECLIP::CF_OEMTEXT:         utf8_name = "oemtext"_av; break;
                // case RDPECLIP::CF_DIB:             utf8_name = "dib"_av; break;
                // case RDPECLIP::CF_PALETTE:         utf8_name = "palette"_av; break;
                // case RDPECLIP::CF_PENDATA:         utf8_name = "pendata"_av; break;
                // case RDPECLIP::CF_RIFF:            utf8_name = "riff"_av; break;
                // case RDPECLIP::CF_WAVE:            utf8_name = "wave"_av; break;
                // case RDPECLIP::CF_ENHMETAFILE:     utf8_name = "enhmetafile"_av; break;
                // case RDPECLIP::CF_HDROP:           utf8_name = "hdrop"_av; break;
                // case RDPECLIP::CF_LOCALE:          utf8_name = "locale"_av; break;
                // case RDPECLIP::CF_DIBV5:           utf8_name = "dibv5"_av; break;
                // case RDPECLIP::CF_OWNERDISPLAY:    utf8_name = "ownerdisplay"_av; break;
                // case RDPECLIP::CF_DSPTEXT:         utf8_name = "dsptext"_av; break;
                // case RDPECLIP::CF_DSPBITMAP:       utf8_name = "dspbitmap"_av; break;
                // case RDPECLIP::CF_DSPMETAFILEPICT: utf8_name = "dspmetafilepict"_av; break;
                // case RDPECLIP::CF_DSPENHMETAFILE:  utf8_name = "dspenhmetafile"_av; break;
                // case RDPECLIP::CF_PRIVATEFIRST:    utf8_name = "privatefirst"_av; break;
                // case RDPECLIP::CF_PRIVATELAST:     utf8_name = "privatelast"_av; break;
                // case RDPECLIP::CF_GDIOBJFIRST:     utf8_name = "gdiobjfirst"_av; break;
                // case RDPECLIP::CF_GDIOBJLAST:      utf8_name = "gdiobjlast"_av; break;
            }
            send_data("receiveData", data);
            break;

        case CustomFormat::FileGroupDescriptorW: {
            LOG_IF(this->verbose, LOG_INFO, "Clipboard: File Group Descriptor Response PDU");
            InStream in_stream(data);
            // flags(4) + reserved1(32) + fileAttributes(4) + reserved2(16) + lastWriteTime(8)
            constexpr std::size_t useless_attributes_size = 64;
            // sizeHigh(4) + sizeLow(4)
            constexpr std::size_t filesize_attribute_size = 8;
            // filenameUtf16(520)
            constexpr std::size_t filename_attribute_size = 520;
            constexpr std::size_t file_packet_size
              = useless_attributes_size
              + filename_attribute_size
              + filesize_attribute_size;
            constexpr std::size_t size_chunk_attribute_size = 4;

            if (in_stream.in_remain() >= file_packet_size + size_chunk_attribute_size)
            {
                auto nb_item = in_stream.in_uint32_le();
                in_stream.in_skip_bytes(size_chunk_attribute_size);

                bool is_last;
                do
                {
                    in_stream.in_skip_bytes(useless_attributes_size);
                    auto size_high = in_stream.in_uint32_le();
                    auto size_low = in_stream.in_uint32_le();
                    auto remaining_buffer = in_stream.remaining_bytes();
                    auto name = quick_utf16_av(remaining_buffer.first(filename_attribute_size));
                    in_stream.in_skip_bytes(filename_attribute_size);

                    is_last = (in_stream.in_remain() < file_packet_size);

                    send_data("receiveFileName", name, size_low, size_high, is_last, nb_item);
                }
                while (/*--nb_item && */ !is_last);
            }
            break;
        }

        case CustomFormat::FileContentsSize: {
            LOG_IF(this->verbose, LOG_INFO, "Clipboard: File Contents Response PDU SIZE");

            InStream in_stream(data);
            auto stream_id = in_stream.in_uint32_le();
            auto size_low = in_stream.in_uint32_le();
            auto size_high = in_stream.in_uint32_le();

            send_data2("receiveFileSize", size_high, size_low, stream_id);
            break;
        }

        case CustomFormat::FileContentsRange: {
            LOG_IF(this->verbose, LOG_INFO, "Clipboard: File Contents Response PDU RANGE");

            InStream in_stream(data);
            uint32_t stream_id = in_stream.in_uint32_le();

            send_data("receiveFileContents", in_stream.remaining_bytes(), stream_id);
            break;
        }
        }

        this->response_buffer.release();
    }

    void process_format_list(InStream& chunk, uint32_t channel_flags)
    {
        FormatListExtractorData data;
        BufArrayMaker<256> buf;

        emval_call(this->callbacks, "receiveFormatStart");

        while (format_list_extractor(
            data, chunk,
            IsLongFormat(this->format_list.use_long_format_names),
            IsAscii(channel_flags & RDPECLIP::CB_ASCII_NAMES)))
        {
            cbytes_view name = data.av_name;
            bool is_utf8 = true;

            if (name.size())
            {
                switch (data.charset)
                {
                case Charset::Ascii: break;
                case Charset::Utf16: is_utf8 = false; break;
                }
            }
            else
            {
                switch (data.format_id)
                {
                case RDPECLIP::CF_TEXT:            name = "text"_av; break;
                case RDPECLIP::CF_BITMAP:          name = "bitmap"_av; break;
                case RDPECLIP::CF_METAFILEPICT:    name = "metafilepict"_av; break;
                case RDPECLIP::CF_SYLK:            name = "sylk"_av; break;
                case RDPECLIP::CF_DIF:             name = "dif"_av; break;
                case RDPECLIP::CF_TIFF:            name = "tiff"_av; break;
                case RDPECLIP::CF_OEMTEXT:         name = "oemtext"_av; break;
                case RDPECLIP::CF_DIB:             name = "dib"_av; break;
                case RDPECLIP::CF_PALETTE:         name = "palette"_av; break;
                case RDPECLIP::CF_PENDATA:         name = "pendata"_av; break;
                case RDPECLIP::CF_RIFF:            name = "riff"_av; break;
                case RDPECLIP::CF_WAVE:            name = "wave"_av; break;
                case RDPECLIP::CF_UNICODETEXT:     name = "unicodetext"_av; break;
                case RDPECLIP::CF_ENHMETAFILE:     name = "enhmetafile"_av; break;
                case RDPECLIP::CF_HDROP:           name = "hdrop"_av; break;
                case RDPECLIP::CF_LOCALE:          name = "locale"_av; break;
                case RDPECLIP::CF_DIBV5:           name = "dibv5"_av; break;
                case RDPECLIP::CF_OWNERDISPLAY:    name = "ownerdisplay"_av; break;
                case RDPECLIP::CF_DSPTEXT:         name = "dsptext"_av; break;
                case RDPECLIP::CF_DSPBITMAP:       name = "dspbitmap"_av; break;
                case RDPECLIP::CF_DSPMETAFILEPICT: name = "dspmetafilepict"_av; break;
                case RDPECLIP::CF_DSPENHMETAFILE:  name = "dspenhmetafile"_av; break;
                case RDPECLIP::CF_PRIVATEFIRST:    name = "privatefirst"_av; break;
                case RDPECLIP::CF_PRIVATELAST:     name = "privatelast"_av; break;
                case RDPECLIP::CF_GDIOBJFIRST:     name = "gdiobjfirst"_av; break;
                case RDPECLIP::CF_GDIOBJLAST:      name = "gdiobjlast"_av; break;
                }
            }

            emval_call(this->callbacks, "receiveFormat", data.format_id, name.data(), name.size(), is_utf8);
        }

        this->send_format_list_response_ok();

        emval_call(this->callbacks, "receiveFormatStop");
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
        this->send_to_mod_channel(out_stream.get_bytes());
    }

    void send_to_mod_channel(cbytes_view av)
    {
        InStream chunk(av);

        this->cb.send_to_mod_channel(
            channel_names::cliprdr,
            chunk,
            av.size(),
            first_last_channel_flags
        );
    }

    struct ResponseBuffer
    {
        std::unique_ptr<uint8_t[]> p;
        std::size_t len = 0;
        std::size_t ipos = 0;
        bool waiting_for_data = false;

        void reserve(std::size_t n)
        {
            this->p.reset(new uint8_t[n]);
            this->len = n;
            this->waiting_for_data = true;
            this->ipos = 0;
        }

        cbytes_view final() noexcept
        {
            this->len = 0;
            this->waiting_for_data = false;
            return {this->p.get(), this->ipos};
        }

        void release()
        {
            this->len = 0;
            this->p.reset();
        }

        void add(cbytes_view data) noexcept
        {
            std::size_t const remaining = this->len - this->ipos;
            std::size_t const len = std::min(remaining, data.size());
            memcpy(this->p.get() + this->ipos, data.as_u8p(), len);
            this->ipos += len;
        }

        // bytes_view remaining_buffer() noexcept
        // {
        //     return {p.get() + this->ipos, this->len - this->ipos};
        // }
        //
        // cbytes_view full_buffer() const noexcept
        // {
        //     return {p.get(), this->len};
        // }
    };

    Callback& cb;
    emscripten::val callbacks;
    FormatListEmptyName format_list;
    ResponseBuffer response_buffer;
    uint32_t requested_format_id = 0;
    CustomFormat custom_cf {};
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
        .function_ptr("sendRequestFormat", [](redjs::ClipboardChannel& clip, uint32_t id, int custom_cf) {
            clip.d->send_request_format(id, CustomFormat(custom_cf));
        })
        .function_ptr("sendMessage", [](redjs::ClipboardChannel& clip, emscripten::val val) {
            // TODO abort copy
            auto data = val.as<std::string>();
            clip.d->send_to_mod_channel(data);
        })
        .function_ptr("sendFileContentsRequest", [](redjs::ClipboardChannel& clip,
            uint32_t request_type, uint32_t stream_id, uint32_t lindex, uint32_t pos_low, uint32_t pos_high)
        {
            clip.d->send_file_contents_request(request_type, stream_id, lindex, pos_low, pos_high);
        })
        // .function("receive", &redjs::ClipboardChannel::receive)
    ;
}
