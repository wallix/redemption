/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2013
 *   Author(s): Christophe Grosjean
 *
 */

#define RED_TEST_MODULE TestCopyPaste
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/clipboard.hpp"
#include "mod/internal/copy_paste.hpp"
#include "mod/internal/widget/edit.hpp"
#include "mod/internal/widget/screen.hpp"
#include "keyboard/keymap2.hpp"
#include "test_only/front/fake_front.hpp"
#include "test_only/gdi/test_graphic.hpp"
#include "test_only/check_sig.hpp"
#include "test_only/core/font.hpp"

#include <string>


struct CopyPasteFront : FakeFront
{
    CopyPasteFront(ScreenInfo & info, CopyPaste & copy_paste)
    : FakeFront(info)
    , copy_paste(copy_paste)
    {
        CHANNELS::ChannelDef def;
        def.name = channel_names::cliprdr;
        this->channel_def_array.push_back(def);
    }

    const CHANNELS::ChannelDefArray& get_channel_list() const override
    {
        return this->channel_def_array;
    }

    void send_to_channel(
        const CHANNELS::ChannelDef& channel, uint8_t const * data, size_t length, size_t, int
    ) override {
        RED_REQUIRE(channel.name == channel_names::cliprdr);

        InStream stream(data, length);
        RDPECLIP::RecvPredictor rp(stream);

        switch (rp.msgType()) {
            case RDPECLIP::CB_MONITOR_READY:
            {
                RDPECLIP::FormatListPDUEx format_list_pdu;
                format_list_pdu.add_format_name(RDPECLIP::CF_TEXT);

                const bool use_long_format_names = false;
                const bool in_ASCII_8 = format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names);

                RDPECLIP::CliprdrHeader clipboard_header(RDPECLIP::CB_FORMAT_LIST,
                    RDPECLIP::CB_RESPONSE__NONE_ | (in_ASCII_8 ? RDPECLIP::CB_ASCII_NAMES : 0),
                    format_list_pdu.size(use_long_format_names));

                StaticOutStream<256> out_s;
                clipboard_header.emit(out_s);
                format_list_pdu.emit(out_s, use_long_format_names);
                InStream in_s(out_s.get_bytes());
                this->copy_paste.send_to_mod_channel(in_s, CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST);
            }
            break;
            //case RDPECLIP::CB_FORMAT_LIST:
            //    break;
            //case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
            //    break;
            case RDPECLIP::CB_FORMAT_DATA_REQUEST:
            {
                RDPECLIP::FormatDataRequestPDU().recv(stream);
                constexpr std::size_t buf_sz = 65535;
                uint8_t buf[buf_sz];
                size_t unicode_data_length = ::UTF8toUTF16(this->str, buf, buf_sz);
                buf[unicode_data_length    ] = 0;
                buf[unicode_data_length + 1] = 0;
                unicode_data_length += 2;

                RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_DATA_RESPONSE, RDPECLIP::CB_RESPONSE_OK, unicode_data_length);
                RDPECLIP::FormatDataResponsePDU format_data_response_pdu;
                StaticOutStream<256> out_s;
                header.emit(out_s);
                format_data_response_pdu.emit(out_s, buf, unicode_data_length);
                InStream in_s(out_s.get_bytes());
                this->copy_paste.send_to_mod_channel(in_s, CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST);
            }
            break;
            default:
            break;
        }
    }

    void copy(const char * s) {
        this->str = s;

        RDPECLIP::FormatListPDUEx format_list_pdu;
        format_list_pdu.add_format_name(RDPECLIP::CF_TEXT);

        const bool use_long_format_names = false;
        const bool in_ASCII_8 = format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names);

        RDPECLIP::CliprdrHeader clipboard_header(RDPECLIP::CB_FORMAT_LIST,
            RDPECLIP::CB_RESPONSE__NONE_ | (in_ASCII_8 ? RDPECLIP::CB_ASCII_NAMES : 0),
            format_list_pdu.size(use_long_format_names));

        StaticOutStream<256> out_s;
        clipboard_header.emit(out_s);
        format_list_pdu.emit(out_s, use_long_format_names);
        InStream in_s(out_s.get_bytes());
        this->copy_paste.send_to_mod_channel(in_s, CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST);
    }

private:
//    template<class PDU, class... Args>
//    void send_to_server(PDU && pdu, Args && ...args) {
//        StaticOutStream<256> out_s;
//        pdu.emit(out_s, std::move(args)...);
//        InStream in_s(out_s.get_bytes());
//        this->copy_paste.send_to_mod_channel(in_s, CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST);
//    }

    CHANNELS::ChannelDefArray channel_def_array;
    CopyPaste & copy_paste;
    std::string str;
};

class CopyPasteProcess : public NotifyApi
{
    CopyPaste & copy_paste;

public:
    CopyPasteProcess(CopyPaste & copy_paste)
    : copy_paste(copy_paste)
    {}

    void notify(Widget * sender, notify_event_t event) override {
        RED_REQUIRE(sender);
        copy_paste_process_event(this->copy_paste, *reinterpret_cast<WidgetEdit*>(sender), event);
    }
};

RED_AUTO_TEST_CASE(TestPaste)
{
    ScreenInfo screen_info{BitsPerPixel{24}, 120, 20};

    CopyPaste copy_paste;
    CopyPasteFront front(screen_info, copy_paste);
    TestGraphic gd(screen_info.width, screen_info.height);

    Keymap2 keymap;
    keymap.init_layout(0x040C);

    CopyPasteProcess notifier(copy_paste);

    WidgetScreen parent(gd, global_font_lato_light_16(), nullptr, Theme{});
    parent.set_wh(screen_info.width, screen_info.height);

    WidgetEdit edit(gd, parent, &notifier, "", 0, PINK, ORANGE, RED, global_font_lato_light_16());
    Dimension dim = edit.get_optimal_dim();
    edit.set_wh(120, dim.h);
    edit.set_xy(0, 0);

    RED_REQUIRE(copy_paste.ready(front));

    #define edit_paste(s, sig) {                                   \
        keymap.push_kevent(Keymap2::KEVENT_PASTE);                 \
        copy_paste.paste(edit);                                    \
        RED_CHECK_EQUAL(s, edit.get_text());                       \
                                                                   \
        edit.rdp_input_invalidate(edit.get_rect());                \
                                                                   \
        /*char filename[256];*/                                    \
        /*sprintf(filename, "test_copy_paste_%d.png", __LINE__);*/ \
        /*mod.save_to_png(filename);*/                             \
                                                                   \
        RED_CHECK_SIG(gd, sig);                                    \
    }
    edit_paste("",
        "\x55\x78\x56\xd2\x65\x6c\x78\x4a\x23\x26\x2b\xf5\xfb\x67\xdd\x0f\xa9\x96\xaf\xa6");
    edit_paste("",
        "\x55\x78\x56\xd2\x65\x6c\x78\x4a\x23\x26\x2b\xf5\xfb\x67\xdd\x0f\xa9\x96\xaf\xa6");
    front.copy("plop");
    edit_paste("plop",
        "\xc0\x91\xd3\x21\x52\x48\xdb\x15\xb1\x56\x90\x18\x1d\x8c\x4a\x97\x73\x0d\x3a\x80");
    edit.decrement_edit_pos();
    edit_paste("ploplopp",
        "\x0d\xf1\x59\xb6\x59\xb1\x21\x17\x9d\x47\xa9\xff\x98\xa8\xd2\x44\x5f\x39\xbe\xb4");
    front.copy("xxx");
    edit_paste("ploplopxxxp",
        "\x54\x3f\x47\xe5\xc7\x84\xa5\x9c\x3c\xcd\x23\xc0\xa8\x41\xb8\x5a\xa8\xf9\xb5\x45");
    edit_paste("ploplopxxxxxxp",
        "\x97\xb1\xb1\x03\xa0\xac\x8a\x0b\xad\xc0\xd1\xe8\x48\x0a\x67\xd4\xdc\x5e\xf7\xcb");
    edit.set_text("");
    front.copy("abc\tde");
    edit_paste("abc de",
        "\x57\x6d\xf5\xff\xfd\x5a\x19\xeb\xb1\x54\xc7\x61\xfc\xd4\x44\xd3\x8c\x76\x14\x34");
    front.copy("fg\nhi");
    edit_paste("abc defg",
        "\xe9\x1a\x01\x5d\xe4\x0f\xb5\xfd\x8f\xce\xf7\x8c\x12\x97\xb9\x3f\xc7\xd0\xde\xc6");
    front.copy("jk\tl\nmn");
    edit_paste("abc defgjk l",
        "\x78\xe6\x4a\x5e\x03\x1c\xf8\x4c\x09\xb8\xb3\xba\x2a\x12\x8e\x64\xd9\xf4\x6b\xc5");
}
