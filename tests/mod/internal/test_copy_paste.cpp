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
#include <boost/test/auto_unit_test.hpp>

#include <string>

#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

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
        const CHANNELS::ChannelDef& channel, uint8_t const * data, size_t length, size_t chunk_size, int flags
    ) override {
        BOOST_REQUIRE(!strcmp(channel.name, channel_names::cliprdr));

        InStream stream(data, length);
        RDPECLIP::RecvFactory recv_factory(stream);

        switch (recv_factory.msgType) {
            case RDPECLIP::CB_MONITOR_READY:
                this->send_to_server(RDPECLIP::FormatListPDU());
            break;
            //case RDPECLIP::CB_FORMAT_LIST:
            //    break;
            //case RDPECLIP::CB_FORMAT_LIST_RESPONSE:
            //    break;
            case RDPECLIP::CB_FORMAT_DATA_REQUEST:
            {
                RDPECLIP::FormatDataRequestPDU().recv(stream, recv_factory);
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

    virtual void notify(Widget2 * sender, notify_event_t event) {
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

    WidgetScreen parent(mod, info.width, info.height, font);
    WidgetEdit edit(mod, 0, 0, 120, parent, &notifier, "", 0, PINK, ORANGE, RED, font);

    BOOST_REQUIRE(copy_paste.ready(front));

    auto edit_paste = [&](const char * s, const char * sig){
        keymap.push_kevent(Keymap2::KEVENT_PASTE);
        copy_paste.paste(edit);
        edit.rdp_input_invalidate(edit.rect);
        //front.dump_png("/tmp/test_copy_paste_");
        BOOST_CHECK_EQUAL(s, edit.get_text());
        char message[1024];
        if (!check_sig(mod.gd.impl(), message, sig)){
            BOOST_CHECK_MESSAGE(false, message);
        }
    };
    edit_paste("", "\x00\xc3\x7a\x5d\xfc\x63\x81\x79\x9b\x75\x8c\x58\x92\xc9\x2e\xec\x9d\xbe\x43\x5c");
    edit_paste("", "\x00\xc3\x7a\x5d\xfc\x63\x81\x79\x9b\x75\x8c\x58\x92\xc9\x2e\xec\x9d\xbe\x43\x5c");
    front.copy("plop");
    edit_paste("plop", "\xd1\x79\xc9\xd1\x98\x18\x48\xb6\x11\x44\x21\x26\xc1\xd1\x80\x52\xa5\x04\x23\x90");
    edit.decrement_edit_pos();
    edit_paste("ploplopp", "\xca\xfa\xb0\xdd\xee\x87\x3c\x4d\xee\x3d\x88\x4e\x6e\x55\x18\x49\x6d\x3c\x6e\xe3");
    front.copy("xxx");
    edit_paste("ploplopxxxp", "\xf1\x16\x0e\xe8\xac\xa0\x1d\xd9\x54\x16\xba\xa8\x8d\x72\x0d\x44\xd7\x5e\xf4\x66");
    edit_paste("ploplopxxxxxxp", "\x70\x58\x4e\x18\x8a\xd6\x0c\x24\xf6\x82\x5b\xe7\x48\x2d\x01\x86\xe5\x9a\x62\xf3");
}
