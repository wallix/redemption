#include "scancodes_x00000405.hpp"
#include "scancodes_x00000406.hpp"
#include "scancodes_x00000407.hpp"
#include "scancodes_x00000408.hpp"
#include "scancodes_x00000409.hpp"
#include "scancodes_x0000040a.hpp"
#include "scancodes_x0000040b.hpp"
#include "scancodes_x0000040c.hpp"
#include "scancodes_x0000040f.hpp"
#include "scancodes_x00000410.hpp"
#include "scancodes_x00000413.hpp"
#include "scancodes_x00000414.hpp"
#include "scancodes_x00000415.hpp"
#include "scancodes_x00000416.hpp"
#include "scancodes_x00000418.hpp"
#include "scancodes_x00000419.hpp"
#include "scancodes_x0000041a.hpp"
#include "scancodes_x0000041b.hpp"
#include "scancodes_x0000041d.hpp"
#include "scancodes_x0000041f.hpp"
#include "scancodes_x00000422.hpp"
#include "scancodes_x00000424.hpp"
#include "scancodes_x00000425.hpp"
#include "scancodes_x00000426.hpp"
#include "scancodes_x00000427.hpp"
#include "scancodes_x0000042f.hpp"
#include "scancodes_x00000438.hpp"
#include "scancodes_x0000043a.hpp"
#include "scancodes_x0000043b.hpp"
#include "scancodes_x0000043f.hpp"
#include "scancodes_x00000440.hpp"
#include "scancodes_x00000444.hpp"
#include "scancodes_x00000450.hpp"
#include "scancodes_x00000452.hpp"
#include "scancodes_x0000046e.hpp"
#include "scancodes_x00000481.hpp"
#include "scancodes_x00000807.hpp"
#include "scancodes_x00000809.hpp"
#include "scancodes_x0000080a.hpp"
#include "scancodes_x0000080c.hpp"
#include "scancodes_x00000813.hpp"
#include "scancodes_x00000816.hpp"
#include "scancodes_x0000081a.hpp"
#include "scancodes_x0000083b.hpp"
#include "scancodes_x00000843.hpp"
#include "scancodes_x0000085d.hpp"
#include "scancodes_x00000c0c.hpp"
#include "scancodes_x00000c1a.hpp"
#include "scancodes_x00001009.hpp"
#include "scancodes_x0000100c.hpp"
#include "scancodes_x0000201a.hpp"
#include "scancodes_x00010402.hpp"
#include "scancodes_x00010405.hpp"
#include "scancodes_x00001809.hpp"
#include "scancodes_x00010407.hpp"
#include "scancodes_x00010408.hpp"
#include "scancodes_x0001040a.hpp"
#include "scancodes_x0001040e.hpp"
#include "scancodes_x00010409.hpp"
#include "scancodes_x00010410.hpp"
#include "scancodes_x00010415.hpp"
#include "scancodes_x00010416.hpp"
#include "scancodes_x00010419.hpp"
#include "scancodes_x0001041b.hpp"
#include "scancodes_x0001041f.hpp"
#include "scancodes_x00010426.hpp"
#include "scancodes_x00010427.hpp"
#include "scancodes_x0001043a.hpp"
#include "scancodes_x0001043b.hpp"
#include "scancodes_x0001080c.hpp"
#include "scancodes_x0001083b.hpp"
#include "scancodes_x00011009.hpp"
#include "scancodes_x00011809.hpp"
#include "scancodes_x00020405.hpp"
#include "scancodes_x00020408.hpp"
#include "scancodes_x00020409.hpp"
#include "scancodes_x0002083b.hpp"
#include "scancodes_x00030402.hpp"
#include "scancodes_x00030408.hpp"
#include "scancodes_x00030409.hpp"
#include "scancodes_x00040408.hpp"
#include "scancodes_x00040409.hpp"
#include "scancodes_x00050408.hpp"
#include "scancodes_x00060408.hpp"

#include "all_LCIDs_scancodes.hpp"

namespace scancode
{
    const std::unordered_map<std::uint16_t, std::pair<std::uint8_t, std::uint8_t>> *get_scancodes_table_ptr(int LCID) noexcept
    {
        switch (LCID)
        {
           case 0x00000405 :
               return &x00000405::SCANCODES_TABLE;
           case 0x00000406 :
               return &x00000406::SCANCODES_TABLE;
           case 0x00000407 :
               return &x00000407::SCANCODES_TABLE;
           case 0x00000408 :
               return &x00000408::SCANCODES_TABLE;
           case 0x00000409 :
               return &x00000409::SCANCODES_TABLE;
           case 0x0000040a :
               return &x0000040a::SCANCODES_TABLE;
           case 0x0000040b :
               return &x0000040b::SCANCODES_TABLE;
           case 0x0000040c :
               return &x0000040c::SCANCODES_TABLE;
           case 0x0000040f :
               return &x0000040f::SCANCODES_TABLE;
           case 0x00000410 :
               return &x00000410::SCANCODES_TABLE;
           case 0x00000413 :
               return &x00000413::SCANCODES_TABLE;
           case 0x00000414 :
               return &x00000414::SCANCODES_TABLE;
           case 0x00000415 :
               return &x00000415::SCANCODES_TABLE;
           case 0x00000416 :
               return &x00000416::SCANCODES_TABLE;
           case 0x00000418 :
               return &x00000418::SCANCODES_TABLE;
           case 0x00000419 :
               return &x00000419::SCANCODES_TABLE;
           case 0x0000041a :
               return &x0000041a::SCANCODES_TABLE;
           case 0x0000041b :
               return &x0000041b::SCANCODES_TABLE;
           case 0x0000041d :
               return &x0000041d::SCANCODES_TABLE;
           case 0x0000041f :
               return &x0000041f::SCANCODES_TABLE;
           case 0x00000422 :
               return &x00000422::SCANCODES_TABLE;
           case 0x00000424 :
               return &x00000424::SCANCODES_TABLE;
           case 0x00000425 :
               return &x00000425::SCANCODES_TABLE;
           case 0x00000426 :
               return &x00000426::SCANCODES_TABLE;
           case 0x00000427 :
               return &x00000427::SCANCODES_TABLE;
           case 0x0000042f :
               return &x0000042f::SCANCODES_TABLE;
           case 0x00000438 :
               return &x00000438::SCANCODES_TABLE;
           case 0x0000043a :
               return &x0000043a::SCANCODES_TABLE;
           case 0x0000043b :
               return &x0000043b::SCANCODES_TABLE;
           case 0x0000043f :
               return &x0000043f::SCANCODES_TABLE;
           case 0x00000440 :
               return &x00000440::SCANCODES_TABLE;
           case 0x00000444 :
               return &x00000444::SCANCODES_TABLE;
           case 0x00000450 :
               return &x00000450::SCANCODES_TABLE;
           case 0x00000452 :
               return &x00000452::SCANCODES_TABLE;
           case 0x0000046e :
               return &x0000046e::SCANCODES_TABLE;
           case 0x00000481 :
               return &x00000481::SCANCODES_TABLE;
           case 0x00000807 :
               return &x00000807::SCANCODES_TABLE;
           case 0x00000809 :
               return &x00000809::SCANCODES_TABLE;
           case 0x0000080a :
               return &x0000080a::SCANCODES_TABLE;
           case 0x0000080c :
               return &x0000080c::SCANCODES_TABLE;
           case 0x00000813 :
               return &x00000813::SCANCODES_TABLE;
           case 0x00000816 :
               return &x00000816::SCANCODES_TABLE;
           case 0x0000081a :
               return &x0000081a::SCANCODES_TABLE;
           case 0x0000083b :
               return &x0000083b::SCANCODES_TABLE;
           case 0x00000843 :
               return &x00000843::SCANCODES_TABLE;
           case 0x0000085d :
               return &x0000085d::SCANCODES_TABLE;
           case 0x00000c0c :
               return &x00000c0c::SCANCODES_TABLE;
           case 0x00000c1a :
               return &x00000c1a::SCANCODES_TABLE;
           case 0x00001009 :
               return &x00001009::SCANCODES_TABLE;
           case 0x0000100c :
               return &x0000100c::SCANCODES_TABLE;
           case 0x0000201a :
               return &x0000201a::SCANCODES_TABLE;
           case 0x00010402 :
               return &x00010402::SCANCODES_TABLE;
           case 0x00010405 :
               return &x00010405::SCANCODES_TABLE;
           case 0x00001809 :
               return &x00001809::SCANCODES_TABLE;
           case 0x00010407 :
               return &x00010407::SCANCODES_TABLE;
           case 0x00010408 :
               return &x00010408::SCANCODES_TABLE;
           case 0x0001040a :
               return &x0001040a::SCANCODES_TABLE;
           case 0x0001040e :
               return &x0001040e::SCANCODES_TABLE;
           case 0x00010409 :
               return &x00010409::SCANCODES_TABLE;
           case 0x00010410 :
               return &x00010410::SCANCODES_TABLE;
           case 0x00010415 :
               return &x00010415::SCANCODES_TABLE;
           case 0x00010416 :
               return &x00010416::SCANCODES_TABLE;
           case 0x00010419 :
               return &x00010419::SCANCODES_TABLE;
           case 0x0001041b :
               return &x0001041b::SCANCODES_TABLE;
           case 0x0001041f :
               return &x0001041f::SCANCODES_TABLE;
           case 0x00010426 :
               return &x00010426::SCANCODES_TABLE;
           case 0x00010427 :
               return &x00010427::SCANCODES_TABLE;
           case 0x0001043a :
               return &x0001043a::SCANCODES_TABLE;
           case 0x0001043b :
               return &x0001043b::SCANCODES_TABLE;
           case 0x0001080c :
               return &x0001080c::SCANCODES_TABLE;
           case 0x0001083b :
               return &x0001083b::SCANCODES_TABLE;
           case 0x00011009 :
               return &x00011009::SCANCODES_TABLE;
           case 0x00011809 :
               return &x00011809::SCANCODES_TABLE;
           case 0x00020405 :
               return &x00020405::SCANCODES_TABLE;
           case 0x00020408 :
               return &x00020408::SCANCODES_TABLE;
           case 0x00020409 :
               return &x00020409::SCANCODES_TABLE;
           case 0x0002083b :
               return &x0002083b::SCANCODES_TABLE;
           case 0x00030402 :
               return &x00030402::SCANCODES_TABLE;
           case 0x00030408 :
               return &x00030408::SCANCODES_TABLE;
           case 0x00030409 :
               return &x00030409::SCANCODES_TABLE;
           case 0x00040408 :
               return &x00040408::SCANCODES_TABLE;
           case 0x00040409 :
               return &x00040409::SCANCODES_TABLE;
           case 0x00050408 :
               return &x00050408::SCANCODES_TABLE;
           case 0x00060408 :
               return &x00060408::SCANCODES_TABLE;
           default :
               return nullptr;
        }
    }

    const std::unordered_map<std::uint16_t, std::pair<std::uint8_t, std::uint16_t>> *get_extended_scancodes_table_ptr(int LCID) noexcept
    {
        switch (LCID)
        {
           case 0x00000405 :
               return &x00000405::EXTENDED_SCANCODES_TABLE;
           case 0x00000406 :
               return &x00000406::EXTENDED_SCANCODES_TABLE;
           case 0x00000407 :
               return &x00000407::EXTENDED_SCANCODES_TABLE;
           case 0x00000408 :
               return &x00000408::EXTENDED_SCANCODES_TABLE;
           case 0x00000409 :
               return &x00000409::EXTENDED_SCANCODES_TABLE;
           case 0x0000040a :
               return &x0000040a::EXTENDED_SCANCODES_TABLE;
           case 0x0000040b :
               return &x0000040b::EXTENDED_SCANCODES_TABLE;
           case 0x0000040c :
               return &x0000040c::EXTENDED_SCANCODES_TABLE;
           case 0x0000040f :
               return &x0000040f::EXTENDED_SCANCODES_TABLE;
           case 0x00000410 :
               return &x00000410::EXTENDED_SCANCODES_TABLE;
           case 0x00000413 :
               return &x00000413::EXTENDED_SCANCODES_TABLE;
           case 0x00000414 :
               return &x00000414::EXTENDED_SCANCODES_TABLE;
           case 0x00000415 :
               return &x00000415::EXTENDED_SCANCODES_TABLE;
           case 0x00000416 :
               return &x00000416::EXTENDED_SCANCODES_TABLE;
           case 0x00000418 :
               return &x00000418::EXTENDED_SCANCODES_TABLE;
           case 0x00000419 :
               return &x00000419::EXTENDED_SCANCODES_TABLE;
           case 0x0000041a :
               return &x0000041a::EXTENDED_SCANCODES_TABLE;
           case 0x0000041b :
               return &x0000041b::EXTENDED_SCANCODES_TABLE;
           case 0x0000041d :
               return &x0000041d::EXTENDED_SCANCODES_TABLE;
           case 0x0000041f :
               return &x0000041f::EXTENDED_SCANCODES_TABLE;
           case 0x00000422 :
               return &x00000422::EXTENDED_SCANCODES_TABLE;
           case 0x00000424 :
               return &x00000424::EXTENDED_SCANCODES_TABLE;
           case 0x00000425 :
               return &x00000425::EXTENDED_SCANCODES_TABLE;
           case 0x00000426 :
               return &x00000426::EXTENDED_SCANCODES_TABLE;
           case 0x00000427 :
               return &x00000427::EXTENDED_SCANCODES_TABLE;
           case 0x0000042f :
               return &x0000042f::EXTENDED_SCANCODES_TABLE;
           case 0x00000438 :
               return &x00000438::EXTENDED_SCANCODES_TABLE;
           case 0x0000043a :
               return &x0000043a::EXTENDED_SCANCODES_TABLE;
           case 0x0000043b :
               return &x0000043b::EXTENDED_SCANCODES_TABLE;
           case 0x0000043f :
               return &x0000043f::EXTENDED_SCANCODES_TABLE;
           case 0x00000440 :
               return &x00000440::EXTENDED_SCANCODES_TABLE;
           case 0x00000444 :
               return &x00000444::EXTENDED_SCANCODES_TABLE;
           case 0x00000450 :
               return &x00000450::EXTENDED_SCANCODES_TABLE;
           case 0x00000452 :
               return &x00000452::EXTENDED_SCANCODES_TABLE;
           case 0x0000046e :
               return &x0000046e::EXTENDED_SCANCODES_TABLE;
           case 0x00000481 :
               return &x00000481::EXTENDED_SCANCODES_TABLE;
           case 0x00000807 :
               return &x00000807::EXTENDED_SCANCODES_TABLE;
           case 0x00000809 :
               return &x00000809::EXTENDED_SCANCODES_TABLE;
           case 0x0000080a :
               return &x0000080a::EXTENDED_SCANCODES_TABLE;
           case 0x0000080c :
               return &x0000080c::EXTENDED_SCANCODES_TABLE;
           case 0x00000813 :
               return &x00000813::EXTENDED_SCANCODES_TABLE;
           case 0x00000816 :
               return &x00000816::EXTENDED_SCANCODES_TABLE;
           case 0x0000081a :
               return &x0000081a::EXTENDED_SCANCODES_TABLE;
           case 0x0000083b :
               return &x0000083b::EXTENDED_SCANCODES_TABLE;
           case 0x00000843 :
               return &x00000843::EXTENDED_SCANCODES_TABLE;
           case 0x0000085d :
               return &x0000085d::EXTENDED_SCANCODES_TABLE;
           case 0x00000c0c :
               return &x00000c0c::EXTENDED_SCANCODES_TABLE;
           case 0x00000c1a :
               return &x00000c1a::EXTENDED_SCANCODES_TABLE;
           case 0x00001009 :
               return &x00001009::EXTENDED_SCANCODES_TABLE;
           case 0x0000100c :
               return &x0000100c::EXTENDED_SCANCODES_TABLE;
           case 0x0000201a :
               return &x0000201a::EXTENDED_SCANCODES_TABLE;
           case 0x00010402 :
               return &x00010402::EXTENDED_SCANCODES_TABLE;
           case 0x00010405 :
               return &x00010405::EXTENDED_SCANCODES_TABLE;
           case 0x00001809 :
               return &x00001809::EXTENDED_SCANCODES_TABLE;
           case 0x00010407 :
               return &x00010407::EXTENDED_SCANCODES_TABLE;
           case 0x00010408 :
               return &x00010408::EXTENDED_SCANCODES_TABLE;
           case 0x0001040a :
               return &x0001040a::EXTENDED_SCANCODES_TABLE;
           case 0x0001040e :
               return &x0001040e::EXTENDED_SCANCODES_TABLE;
           case 0x00010409 :
               return &x00010409::EXTENDED_SCANCODES_TABLE;
           case 0x00010410 :
               return &x00010410::EXTENDED_SCANCODES_TABLE;
           case 0x00010415 :
               return &x00010415::EXTENDED_SCANCODES_TABLE;
           case 0x00010416 :
               return &x00010416::EXTENDED_SCANCODES_TABLE;
           case 0x00010419 :
               return &x00010419::EXTENDED_SCANCODES_TABLE;
           case 0x0001041b :
               return &x0001041b::EXTENDED_SCANCODES_TABLE;
           case 0x0001041f :
               return &x0001041f::EXTENDED_SCANCODES_TABLE;
           case 0x00010426 :
               return &x00010426::EXTENDED_SCANCODES_TABLE;
           case 0x00010427 :
               return &x00010427::EXTENDED_SCANCODES_TABLE;
           case 0x0001043a :
               return &x0001043a::EXTENDED_SCANCODES_TABLE;
           case 0x0001043b :
               return &x0001043b::EXTENDED_SCANCODES_TABLE;
           case 0x0001080c :
               return &x0001080c::EXTENDED_SCANCODES_TABLE;
           case 0x0001083b :
               return &x0001083b::EXTENDED_SCANCODES_TABLE;
           case 0x00011009 :
               return &x00011009::EXTENDED_SCANCODES_TABLE;
           case 0x00011809 :
               return &x00011809::EXTENDED_SCANCODES_TABLE;
           case 0x00020405 :
               return &x00020405::EXTENDED_SCANCODES_TABLE;
           case 0x00020408 :
               return &x00020408::EXTENDED_SCANCODES_TABLE;
           case 0x00020409 :
               return &x00020409::EXTENDED_SCANCODES_TABLE;
           case 0x0002083b :
               return &x0002083b::EXTENDED_SCANCODES_TABLE;
           case 0x00030402 :
               return &x00030402::EXTENDED_SCANCODES_TABLE;
           case 0x00030408 :
               return &x00030408::EXTENDED_SCANCODES_TABLE;
           case 0x00030409 :
               return &x00030409::EXTENDED_SCANCODES_TABLE;
           case 0x00040408 :
               return &x00040408::EXTENDED_SCANCODES_TABLE;
           case 0x00040409 :
               return &x00040409::EXTENDED_SCANCODES_TABLE;
           case 0x00050408 :
               return &x00050408::EXTENDED_SCANCODES_TABLE;
           case 0x00060408 :
               return &x00060408::EXTENDED_SCANCODES_TABLE;
           default :
               return nullptr;
        }
    }
}
