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

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "red_emscripten/em_asm.hpp"
#include "red_emscripten/bind.hpp"
#include "red_channels/clipboard.hpp"

#include "core/callback.hpp"
#include "core/RDP/clipboard.hpp"

#include <vector>
#include <variant>

#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/variadic/to_list.hpp>
#include <boost/preprocessor/list/for_each.hpp>
#include <boost/preprocessor/seq/for_each.hpp>


#define MAKE_JS_TO_CPP_I_MEMBER(r, data, elem) elem;
#define JS_c(classname) \
    (classname, struct classname {}, +[]() { clip_datas.push_back(classname{}); })
#define JS_x(classname, ...) (                                         \
    classname, struct classname {                                      \
        BOOST_PP_LIST_FOR_EACH(                                        \
            MAKE_JS_TO_CPP_I_MEMBER, _,                                \
            BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))                    \
    }, static_cast<void(*)(__VA_ARGS__)>(                              \
        [](auto... args) { clip_datas.push_back(classname{args...}); } \
    )                                                                  \
)
#define JS_d(classname, data_type, ...) (                          \
    classname, struct classname {                                  \
        std::vector<data_type> data;                               \
        BOOST_PP_LIST_FOR_EACH(                                    \
            MAKE_JS_TO_CPP_I_MEMBER, _,                            \
            BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))                \
    }, static_cast<void(*)(uintptr_t, uint32_t, __VA_ARGS__)>(     \
        [](uintptr_t i_, uint32_t n_, auto... args) {              \
            auto p_ = reinterpret_cast<data_type*>(i_);            \
            clip_datas.push_back(classname{{p_, p_+n_}, args...}); \
    })                                                             \
)

#define MAKE_TYPE_NAME(classname, ...) classname

#define MAKE_BIND_TYPE_E(r, data, elem)                           \
    template<class... Ts> struct binding_type<__COUNTER__, Ts...> \
    : binding_type<__COUNTER__-3, Ts..., MAKE_TYPE_NAME elem>     \
    {};

#define MAKE_JS_TO_CPP_E(r, data, elem) MAKE_JS_TO_CPP_E_S elem
#define MAKE_JS_TO_CPP_E_S(classname, struct_def, func) struct_def;

#define MAKE_JS_CALL_E(r, data, elem) \
    MAKE_TYPE_NAME elem: (...args) => \
        BOOST_PP_CAT(Module.test_clip_, MAKE_TYPE_NAME elem),

#define MAKE_CPP_BINDING_E(r, data, elem) \
    redjs::function("test_clip_" MAKE_CPP_BINDING_F elem);

#define MAKE_CPP_BINDING_F(classname, struct_def, func) #classname, func

#define MAKE_BINDING_CALLBACKS(S)                       \
    BOOST_PP_SEQ_FOR_EACH(MAKE_JS_TO_CPP_E, _, S)       \
                                                        \
    template<class... Ts>                               \
    struct binding_type<__COUNTER__-1, Ts...>           \
    { using type = std::variant<DataChan, Ts...>; };    \
                                                        \
    BOOST_PP_SEQ_FOR_EACH(MAKE_BIND_TYPE_E, _, S)       \
                                                        \
    std::vector<binding_type<__COUNTER__-2>::type>      \
        clip_datas;                                     \
                                                        \
    EMSCRIPTEN_BINDINGS(test_clipboard_channel)         \
    {                                                   \
        BOOST_PP_SEQ_FOR_EACH(MAKE_CPP_BINDING_E, _, S) \
    }                                                   \
                                                        \
    void init_clip()                                    \
    {                                                   \
        clip_datas.clear();                             \
        RED_EM_ASM({Module.test_init_clip({             \
            BOOST_PP_SEQ_FOR_EACH(MAKE_JS_CALL_E, _, S) \
        });});                                          \
    }

namespace
{

template<int, class...> class binding_type;

struct DataChan
{
    CHANNELS::ChannelNameId front_channel_name;
    std::vector<uint8_t> chunk;
    std::size_t length;
    uint32_t flags;
};

MAKE_BINDING_CALLBACKS(
    (JS_c(receiveFormatStart))
    (JS_c(receiveFormatStop))
    (JS_d(receiveFormat, uint8_t, uint32_t formatId, bool isUTF8))
    (JS_d(receiveData, uint8_t, uint32_t formatId, uint32_t channelFlags))
    (JS_x(receiveNbFileName, uint32_t nb))
    (JS_d(receiveFileName, uint8_t, uint32_t attr, uint32_t flags, uint32_t sizeLow, uint32_t sizeHigh, uint32_t lastWriteTimeLow, uint32_t lastWriteTimeHigh))
    (JS_d(receiveFileContents, uint8_t, uint32_t streamId, uint32_t channelFlags))
    (JS_x(receiveFileSize, uint32_t sizeHigh, uint32_t sizeLow, uint32_t streamId))
    (JS_x(receiveFormatId, uint32_t id))
)

struct Mod : Callback
{
    void rdp_input_invalidate(Rect) override {}
    void rdp_input_mouse(int, int, int, Keymap2*) override {}
    void rdp_input_scancode(long, long, long, long, Keymap2*) override {}
    void rdp_input_synchronize(uint32_t, uint16_t, int16_t, int16_t) override {}
    void refresh(Rect) override {}

    void send_to_mod_channel(CHANNELS::ChannelNameId front_channel_name, InStream & chunk, std::size_t length, uint32_t flags) override
    {
        clip_datas.push_back(DataChan{
            front_channel_name,
            {chunk.get_data(), chunk.get_data_end()},
            length,
            flags
        });
    }
};

Mod mod;
std::unique_ptr<redjs::ClipboardChannel> clip;

EMSCRIPTEN_BINDINGS(test_clipboard)
{
    redjs::function("test_init_clip", [](emscripten::val&& v){
        clip = std::make_unique<redjs::ClipboardChannel>(mod, std::move(v), RDPVerbose{});
    });
}

constexpr int first_last_channel_flags
    = CHANNELS::CHANNEL_FLAG_LAST | CHANNELS::CHANNEL_FLAG_FIRST
;

void send(
    uint16_t msgType, uint16_t msgFlags, uint32_t dataLen,
    int channel_flags = first_last_channel_flags)
{
    StaticOutStream<65536> out_stream;
    RDPECLIP::CliprdrHeader header(msgType, msgFlags, dataLen);
    header.emit(out_stream);
    clip->receive(out_stream.get_bytes(), channel_flags);
}

}

RED_AUTO_TEST_CASE(TestClipboardChannel)
{
    init_clip();
    send(RDPECLIP::CB_CLIP_CAPS, RDPECLIP::CB_RESPONSE_OK, 0);

    RED_CHECK(clip_datas.size() == 1);
    RED_CHECK(0);
}
