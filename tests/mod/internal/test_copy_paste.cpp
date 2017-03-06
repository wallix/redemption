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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestCopyPaste
#include "system/redemption_unit_tests.hpp"

#include <string>

#define LOGNULL

#include "widget2/fake_draw.hpp"
#include "core/font.hpp"
#include "mod/internal/copy_paste.hpp"
#include "mod/internal/widget2/edit.hpp"
#include "mod/internal/widget2/screen.hpp"
#include "../../front/fake_front.hpp"
#include "check_sig.hpp"


struct CopyPasteFront : FakeFront
{
    CopyPasteFront(ClientInfo & info, CopyPaste & copy_paste)
    : FakeFront(info, 0)
    , copy_paste(copy_paste)
    {
        CHANNELS::ChannelDef def;
        memcpy(def.name, channel_names::cliprdr, strlen(channel_names::cliprdr) + 1);
        this->channel_def_array.push_back(def);
    }

    const CHANNELS::ChannelDefArray& get_channel_list() const override
    {
        return this->channel_def_array;
    }

    void send_to_channel(
        const CHANNELS::ChannelDef& channel, uint8_t const * data, size_t length, size_t, int
    ) override {
        BOOST_REQUIRE(!strcmp(channel.name, channel_names::cliprdr));

        InStream stream(data, length);
        RDPECLIP::RecvPredictor rp(stream);
        uint16_t msgType = rp.msgType;

        switch (msgType) {
            case RDPECLIP::CB_MONITOR_READY:
                this->send_to_server(RDPECLIP::FormatListPDU());
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
                size_t unicode_data_length = ::UTF8toUTF16(byte_ptr_cast(this->str.c_str()),
                    buf, buf_sz);
                buf[unicode_data_length    ] = 0;
                buf[unicode_data_length + 1] = 0;
                unicode_data_length += 2;
                this->send_to_server(RDPECLIP::FormatDataResponsePDU(true), buf, unicode_data_length);
            }
            break;
            default:
            break;
        }
    }

    void copy(const char * s) {
        this->str = s;
        this->send_to_server(RDPECLIP::FormatListPDU());
    }

private:
    template<class PDU, class... Args>
    void send_to_server(PDU && pdu, Args && ...args) {
        StaticOutStream<256> out_s;
        pdu.emit(out_s, std::move(args)...);
        InStream in_s(out_s.get_data(), out_s.get_offset());
        this->copy_paste.send_to_mod_channel(in_s, CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST);
    }

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

    void notify(Widget2 * sender, notify_event_t event) override {
        BOOST_REQUIRE(sender);
        copy_paste_process_event(this->copy_paste, *reinterpret_cast<WidgetEdit*>(sender), event);
    }
};

BOOST_AUTO_TEST_CASE(TestPaste)
{
    ClientInfo info;
    info.keylayout = 0x040C;
    info.console_session = 0;
    info.brush_cache_code = 0;
    info.bpp = 24;
    info.width = 120;
    info.height = 20;

    CopyPaste copy_paste;
    CopyPasteFront front(info, copy_paste);
    TestDraw mod(info.width, info.height);

    Keymap2 keymap;
    keymap.init_layout(info.keylayout);

    CopyPasteProcess notifier(copy_paste);

    Font font(FIXTURES_PATH "/dejavu-sans-10.fv1");

    WidgetScreen parent(mod.gd, font, nullptr, Theme{});
    parent.set_wh(info.width, info.height);

    WidgetEdit edit(mod.gd, parent, &notifier, "", 0, PINK, ORANGE, RED, font);
    Dimension dim = edit.get_optimal_dim();
    edit.set_wh(120, dim.h);
    edit.set_xy(0, 0);

    BOOST_REQUIRE(copy_paste.ready(front));

    #define edit_paste(s, sig) {                                   \
        keymap.push_kevent(Keymap2::KEVENT_PASTE);                 \
        copy_paste.paste(edit);                                    \
        BOOST_CHECK_EQUAL(s, edit.get_text());                     \
                                                                   \
        edit.rdp_input_invalidate(edit.get_rect());                \
                                                                   \
        /*char filename[1024];*/                                   \
        /*sprintf(filename, "test_copy_paste_%d.png", __LINE__);*/ \
        /*mod.save_to_png(filename);*/                             \
                                                                   \
        CHECK_SIG(mod.gd.impl(), sig);                             \
    }
    edit_paste("",
        "\x00\xc3\x7a\x5d\xfc\x63\x81\x79\x9b\x75\x8c\x58\x92\xc9\x2e\xec\x9d\xbe\x43\x5c");
    edit_paste("",
        "\x00\xc3\x7a\x5d\xfc\x63\x81\x79\x9b\x75\x8c\x58\x92\xc9\x2e\xec\x9d\xbe\x43\x5c");
    front.copy("plop");
    edit_paste("plop",
        "\x23\x7f\x9c\xa5\xbb\x4a\xdb\x79\x97\x8e\x53\xb7\x14\x56\xa7\x26\x6d\xaa\xec\x2d");
    edit.decrement_edit_pos();
    edit_paste("ploplopp",
        "\x76\x0d\xa0\x57\x73\xc1\x96\xc5\x4c\xb2\x67\x00\xe9\x51\x54\xa2\x27\x73\x45\xd2");
    front.copy("xxx");
    edit_paste("ploplopxxxp",
        "\x4f\xf1\xdf\x1c\x52\xe1\x44\x31\x0e\xd5\x7e\x0b\x5f\x5a\x0a\x43\x31\x0e\x6e\xf6");
    edit_paste("ploplopxxxxxxp",
        "\x97\x5d\xb1\x21\xb1\xce\x9d\x66\x27\xbb\x85\xf1\xc5\xb4\xef\x4d\x70\x71\xbb\xab");
    edit.set_text("");
    front.copy("abc\tde");
    edit_paste("abc de",
        "\x58\xa5\x4c\x53\xfe\xc6\xeb\x3b\xc8\x51\xd1\x68\x4d\x8c\xd2\x15\x78\x11\xec\xd5");
    front.copy("fg\nhi");
    edit_paste("abc defg",
        "\x6b\xba\x7b\x34\x7a\xf5\xb5\x99\xc4\x8a\x0a\x58\x72\xe8\x62\x25\x6e\xfa\x5e\x6a");
    front.copy("jk\tl\nmn");
    edit_paste("abc defgjk l",
        "\x96\x2b\xe7\xed\x23\xb6\x11\xcf\x6d\xf8\x23\xf3\x63\x29\xd7\x84\xae\xe9\x20\xda");
}
