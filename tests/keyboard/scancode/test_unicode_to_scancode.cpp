#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "keyboard/scancode/unicode_to_scancode.hpp"

namespace
{
    using namespace scancode;
    using U16CArray = uint16_t[];

#define U16A(...)                           \
    make_array_view(U16CArray{__VA_ARGS__}) \
    
#define GET_SCANCODES_ARR_VIEW(lcid, unicode)      \
    unicode_to_scancode(lcid, unicode).scancodes() \


    class FrenchLocaleFixture
    {
      protected :
        static constexpr int LCID = 0x0000040c;
    };
}


RED_FIXTURE_TEST_CASE(unicode_to_scancode__simple_unicode_no_key_combination,
                      FrenchLocaleFixture)
{
    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x0061 /* 'a' */)
              == U16A(0x0010 /* 'A' */));

    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x006D /* 'm' */)
              == U16A(0x0027 /* 'M' */));

    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x0077 /* 'w' */)
              == U16A(0x002C /* 'W' */));
    
    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x0069 /* 'i' */)
              == U16A(0x0017 /* 'I' */));

    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x0026 /* '&' */)
              == U16A(0x0002 /* '1' */));

    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x005E /* '^' */)
              == U16A(0x001A /* OEM6 */));
}

RED_FIXTURE_TEST_CASE(unicode_to_scancode__simple_unicode_from_key_combination,
                      FrenchLocaleFixture)
{
    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x005A /* 'Z' */)
              == U16A(0x002A /* LSHIFT */, 0x0011 /* 'Z' */));

    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x0042 /* 'B' */)
              == U16A(0x002A /* LSHIFT */, 0x0030 /* 'B' */));

    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x0036 /* '6' */)
              == U16A(0x002A /* LSHIFT */, 0x0007 /* '6' */));
    
    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x0060 /* '`' */)
              == U16A(0xE038 /* ALTGR */, 0x0008 /* '7' */));
}

RED_FIXTURE_TEST_CASE(unicode_to_scancode__extended_unicode,
                      FrenchLocaleFixture)
{       
    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x00F4 /* 'ô' */)
              == U16A(0x001A /* OEM6 */, 0x0018 /* 'o' */));

    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x00C4 /* 'Ä' */)
              == U16A(0x001A /* OEM6 */, 0x002A /* LSHIFT */, 0x0010 /* 'A' */));
    
    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x00D1 /* 'Ñ' */)
              == U16A(0x0003 /* '2' */, 0x002A /* LSHIFT */, 0x0031 /* 'N' */));

    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x00CC /* 'Ì' */)
              == U16A(0x0008 /* '7' */, 0x002A /* LSHIFT */, 0x0017 /* 'I' */));
}

RED_FIXTURE_TEST_CASE(unicode_to_scancode__unicode_no_defined,
                      FrenchLocaleFixture)
{
    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x001A /* OEM6 */)
              == array_view<Scancode16bits>());

    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0xE052 /* INSERT */)
              == array_view<Scancode16bits>());

    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x0001 /* ESCAPE */)
              == array_view<Scancode16bits>());

    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x00A1 /* RSHIFT */)
              == array_view<Scancode16bits>());

    RED_CHECK(GET_SCANCODES_ARR_VIEW(LCID, 0x000F /* TAB */)
              == array_view<Scancode16bits>());
}
