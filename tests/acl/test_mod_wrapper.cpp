#include <string_view>

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"
#include "test_only/front/fake_front.hpp"
#include "test_only/core/font.hpp"

#include "acl/mod_wrapper.hpp"
#include "core/client_info.hpp"
#include "configs/config.hpp"
#include "gdi/screen_info.hpp"
#include "keyboard/keymap.hpp"
#include "mod/null/null.hpp"
#include "RAIL/client_execute.hpp"
#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"


#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/acl/"

namespace
{
    struct GdRedraw : null_mod
    {
        GdRedraw(gdi::GraphicApi& gd)
        : gd(gd)
        {}

        void rdp_input_invalidate(Rect r) override
        {
            gd.draw(RDPOpaqueRect(r, RDPColor()), r, gdi::ColorCtx::depth24());
        }

    private:
        gdi::GraphicApi& gd;
    };

    class TestOSDMessageDisplayFixture
    {
    public:
        TestOSDMessageDisplayFixture() :
            client_info_(get_client_info()),
            font_(global_font_deja_vu_14()),
            front_(client_info_.screen_info),
            rail_client_execute_(time_base_,
                                 front_.gd(),
                                 front_,
                                 client_info_.window_list_caps,
                                 false),
            mod_wrapper_(mod_,
                         time_base_,
                         BGRPalette::classic_332(),
                         front_.gd(),
                         client_info_,
                         font_,
                         rail_client_execute_,
                         ini_)
        {
            mod_wrapper_.set_mod(mod_, nullptr, true);
            ini_.set<cfg::globals::target_device>("127.0.0.1");
        }

    protected:
        ImageView draw_osd(std::string_view msg, gdi::OsdMsgUrgency omu)
        {
            mod_wrapper_.display_osd_message(msg, omu);
            return front_;
        }

        ImageView scancode_insert()
        {
            auto flags = Keymap::KbdFlags(0x100);
            auto scancode = Keymap::Scancode(0x52);
            keymap_.event(flags, scancode);
            return this->scancode(flags, scancode);
        }

        ImageView scancode(kbdtypes::KbdFlags flags, kbdtypes::Scancode scancode)
        {
            mod_wrapper_.get_callback().rdp_input_scancode(flags, scancode, 0, keymap_);
            return front_;
        }

        ImageView click_on_osd()
        {
            int flags = MOUSE_FLAG_BUTTON1 | MOUSE_FLAG_DOWN;
            int x = client_info_.screen_info.width / 2;
            int y = 5;
            mod_wrapper_.get_callback().rdp_input_mouse(flags, x, y);
            return front_;
        }

    private:
        TimeBase time_base_;
        ClientInfo client_info_;
        const Font& font_;
        Keymap keymap_{KeyLayout::null_layout()};
        Inifile ini_;
        FakeFront front_;
        GdRedraw mod_{front_.gd()};
        ClientExecute rail_client_execute_;
        ModWrapper mod_wrapper_;

        static ClientInfo get_client_info()
        {
            ClientInfo client_info;

            client_info.screen_info.bpp = BitsPerPixel{24};
            client_info.screen_info.width = 800;
            client_info.screen_info.height = 600;

            return client_info;
        }
    };

    using namespace std::string_view_literals;
} // anonymous namespace

RED_FIXTURE_TEST_CASE(TestOSDMessageDisplay_NormalMultiLine,
                      TestOSDMessageDisplayFixture)
{
    RED_CHECK_IMG(
        draw_osd("Hello\nWorld !"sv, gdi::OsdMsgUrgency::NORMAL),
        IMG_TEST_PATH "osd_message_multi_line.png"
    );

    RED_CHECK_IMG(scancode_insert(), IMG_TEST_PATH "osd_message_empty.png");
}

RED_FIXTURE_TEST_CASE(TestOSDMessageDisplay_NormalUrgency,
                      TestOSDMessageDisplayFixture)
{
    RED_CHECK_IMG(
        draw_osd("Hello World !"sv, gdi::OsdMsgUrgency::NORMAL),
        IMG_TEST_PATH "osd_message_normal.png"
    );

    RED_CHECK_IMG(click_on_osd(), IMG_TEST_PATH "osd_message_empty.png");
}

RED_FIXTURE_TEST_CASE(TestOSDMessageDisplay_InfoUrgency,
                      TestOSDMessageDisplayFixture)
{
    RED_CHECK_IMG(
        draw_osd("Hello World !"sv, gdi::OsdMsgUrgency::INFO),
        IMG_TEST_PATH "osd_message_info.png"
    );
}

RED_FIXTURE_TEST_CASE(TestOSDMessageDisplay_WarningUrgency,
                      TestOSDMessageDisplayFixture)
{
    RED_CHECK_IMG(
        draw_osd("Hello World !"sv, gdi::OsdMsgUrgency::WARNING),
        IMG_TEST_PATH "osd_message_warning.png"
    );
}

RED_FIXTURE_TEST_CASE(TestOSDMessageDisplay_AlertUrgency,
                      TestOSDMessageDisplayFixture)
{
    RED_CHECK_IMG(
        draw_osd("Hello World !"sv, gdi::OsdMsgUrgency::ALERT),
        IMG_TEST_PATH "osd_message_alert.png"
    );
}

RED_FIXTURE_TEST_CASE(TestOSDMessageDisplay_EmptyMessage,
                      TestOSDMessageDisplayFixture)
{
    RED_CHECK_IMG(
        draw_osd(""sv, gdi::OsdMsgUrgency::NORMAL),
        IMG_TEST_PATH "osd_message_empty.png"
    );

    RED_CHECK_IMG(
        draw_osd(""sv, gdi::OsdMsgUrgency::INFO),
        IMG_TEST_PATH "osd_message_empty.png"
    );

    RED_CHECK_IMG(
        draw_osd(""sv, gdi::OsdMsgUrgency::WARNING),
        IMG_TEST_PATH "osd_message_empty.png"
    );

    RED_CHECK_IMG(
        draw_osd(""sv, gdi::OsdMsgUrgency::ALERT),
        IMG_TEST_PATH "osd_message_empty.png"
    );
}

RED_FIXTURE_TEST_CASE(TestOSDMessageDisplay_F12,
                      TestOSDMessageDisplayFixture)
{
    RED_CHECK_IMG(
        scancode(kbdtypes::KbdFlags(), kbdtypes::Scancode::F12),
        IMG_TEST_PATH "osd_message_f12.png"
    );
    RED_CHECK_IMG(
        scancode(kbdtypes::KbdFlags::Release, kbdtypes::Scancode::F12),
        IMG_TEST_PATH "osd_message_empty.png"
    );
    RED_CHECK_IMG(
        scancode(kbdtypes::KbdFlags(), kbdtypes::Scancode::F12),
        IMG_TEST_PATH "osd_message_f12.png"
    );
}
