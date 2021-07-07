#include <string_view>

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/check_img.hpp"
#include "test_only/front/fake_front.hpp"
#include "test_only/core/font.hpp"

#include "gdi/screen_info.hpp"
#include "utils/timebase.hpp"
#include "utils/colors.hpp"
#include "keyboard/keymap2.hpp"
#include "core/client_info.hpp"
#include "RAIL/client_execute.hpp"

#include "acl/mod_wrapper.hpp"

#define IMG_TEST_PATH FIXTURES_PATH "/img_ref/acl/"

namespace
{
    class TestOSDMessageDisplayFixture
    {
    public :
        TestOSDMessageDisplayFixture() :
            client_info_(get_client_info()),
            palette_(BGRPalette::classic_332()),
            font_(global_font_deja_vu_14()),
            front_(client_info_.screen_info),
            gd_(front_.gd()),
            rail_client_execute_(time_base_,
                                 gd_,
                                 front_,
                                 client_info_.window_list_caps,
                                 false),
            mod_wrapper_(time_base_,
                         palette_,
                         gd_,
                         keymap_,
                         client_info_,
                         font_,
                         rail_client_execute_,
                         ini_)
        {}

    private :
        TimeBase time_base_;
        ClientInfo client_info_;
        const BGRPalette& palette_;
        const Font& font_;
        Keymap2 keymap_;
        Inifile ini_;

    protected :
        FakeFront front_;

    private :
        gdi::GraphicApi& gd_;
        ClientExecute rail_client_execute_;

    protected :
        ModWrapper mod_wrapper_;

    private :
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
}

RED_FIXTURE_TEST_CASE(TestOSDMessageDisplay_NormalUrgency,
                      TestOSDMessageDisplayFixture)
{
    auto osd_message = "Hello World !"sv;
    auto omu = gdi::OsdMsgUrgency::NORMAL;

    mod_wrapper_.display_osd_message(osd_message, omu);

    RED_CHECK_IMG(front_, IMG_TEST_PATH "osd_message1.png");
}

RED_FIXTURE_TEST_CASE(TestOSDMessageDisplay_InfoUrgency,
                      TestOSDMessageDisplayFixture)
{
    auto osd_message = "Hello World !"sv;
    auto omu = gdi::OsdMsgUrgency::INFO;

    mod_wrapper_.display_osd_message(osd_message, omu);

    RED_CHECK_IMG(front_, IMG_TEST_PATH "osd_message2.png");
}

RED_FIXTURE_TEST_CASE(TestOSDMessageDisplay_WarningUrgency,
                      TestOSDMessageDisplayFixture)
{
    auto osd_message = "Hello World !"sv;
    auto omu = gdi::OsdMsgUrgency::WARNING;

    mod_wrapper_.display_osd_message(osd_message, omu);

    RED_CHECK_IMG(front_, IMG_TEST_PATH "osd_message3.png");
}

RED_FIXTURE_TEST_CASE(TestOSDMessageDisplay_AlertUrgency,
                      TestOSDMessageDisplayFixture)
{
    auto osd_message = "Hello World !"sv;
    auto omu = gdi::OsdMsgUrgency::ALERT;

    mod_wrapper_.display_osd_message(osd_message, omu);

    RED_CHECK_IMG(front_, IMG_TEST_PATH "osd_message4.png");
}

RED_FIXTURE_TEST_CASE(TestOSDMessageDisplay_EmptyMessage,
                      TestOSDMessageDisplayFixture)
{
    constexpr const char *filedata_path_c = IMG_TEST_PATH "osd_message5.png";

    auto osd_message = ""sv;
    gdi::OsdMsgUrgency omu;

    {
        omu = gdi::OsdMsgUrgency::NORMAL;
        mod_wrapper_.display_osd_message(osd_message);
        RED_CHECK_IMG(front_, filedata_path_c);
    }

    {
        omu = gdi::OsdMsgUrgency::INFO;
        mod_wrapper_.display_osd_message(osd_message, omu);
        RED_CHECK_IMG(front_, filedata_path_c);
    }

    {
        omu = gdi::OsdMsgUrgency::WARNING;
        mod_wrapper_.display_osd_message(osd_message, omu);
        RED_CHECK_IMG(front_, filedata_path_c);
    }

    {
        omu = gdi::OsdMsgUrgency::ALERT;
        mod_wrapper_.display_osd_message(osd_message, omu);
        RED_CHECK_IMG(front_, filedata_path_c);
    }
}
