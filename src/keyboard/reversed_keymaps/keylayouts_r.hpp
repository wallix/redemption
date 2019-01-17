/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean, Dominique Lafages
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   header file. Global keyboard layouts include "iles

*/


#pragma once

#include "../src/keyboard/reversed_keymaps/keylayout_r.hpp"
#include "../src/keyboard/reversed_keymaps/keylayout_x8000201a.hpp" // Bosnian (Cyrillic) (Bosnia and Herzegovina))
#include "../src/keyboard/reversed_keymaps/keylayout_x80030402.hpp" // Bulgarian (Bulgaria)
#include "../src/keyboard/reversed_keymaps/keylayout_x80010402.hpp" // Bulgarian (Bulgaria) - latin
#include "../src/keyboard/reversed_keymaps/keylayout_x8000041a.hpp" // Croatian (Croatia) Croatian
#include "../src/keyboard/reversed_keymaps/keylayout_x80000405.hpp" // Czech (Czech Republic) Czech
#include "../src/keyboard/reversed_keymaps/keylayout_x80020405.hpp" // Czech (Czech Republic) Czech Programmers
#include "../src/keyboard/reversed_keymaps/keylayout_x80010405.hpp" // Czech (Czech Republic) Czech (QWERTY)
#include "../src/keyboard/reversed_keymaps/keylayout_x80000406.hpp" // Danish (Denmark) Danish
#include "../src/keyboard/reversed_keymaps/keylayout_x80000813.hpp" // Dutch (Belgium)
#include "../src/keyboard/reversed_keymaps/keylayout_x80000413.hpp" // Dutch (Netherlands)
#include "../src/keyboard/reversed_keymaps/keylayout_x80001009.hpp" // English (Canada) Canadian French
#include "../src/keyboard/reversed_keymaps/keylayout_x80011009.hpp" // English (Canada) Canadian Multilingual Standard
#include "../src/keyboard/reversed_keymaps/keylayout_x80011809.hpp" // English (Ireland) Gaelic
#include "../src/keyboard/reversed_keymaps/keylayout_x80001809.hpp" // English (Ireland) Irish
#include "../src/keyboard/reversed_keymaps/keylayout_x80000809.hpp" // English (United Kingdom)
#include "../src/keyboard/reversed_keymaps/keylayout_x80000409.hpp" // English (United States)
#include "../src/keyboard/reversed_keymaps/keylayout_x80010409.hpp" // English (United States) United States-Dvorak
#include "../src/keyboard/reversed_keymaps/keylayout_x80030409.hpp" // English (United States) United States-Dvorak for left hand
#include "../src/keyboard/reversed_keymaps/keylayout_x80040409.hpp" // English (United States) United States-Dvorak for right hand
#include "../src/keyboard/reversed_keymaps/keylayout_x80020409.hpp" // English (United States) United States-International
#include "../src/keyboard/reversed_keymaps/keylayout_x80000425.hpp" // Estonian (Estonia) Estonian
#include "../src/keyboard/reversed_keymaps/keylayout_x80000438.hpp" // Faroese (Faroe Islands) Faeroese
#include "../src/keyboard/reversed_keymaps/keylayout_x8000040b.hpp" // Finnish (Finland) Finnish
#include "../src/keyboard/reversed_keymaps/keylayout_x80000c0c.hpp" // French (Canada) Canadian French (Legacy)
#include "../src/keyboard/reversed_keymaps/keylayout_x8000080c.hpp" // French (Belgium)
#include "../src/keyboard/reversed_keymaps/keylayout_x8001080c.hpp" // French (Belgium) Belgian (Comma)
#include "../src/keyboard/reversed_keymaps/keylayout_x8000040c.hpp" // French (France)
#include "../src/keyboard/reversed_keymaps/keylayout_x8000100c.hpp" // French (Switzerland)
#include "../src/keyboard/reversed_keymaps/keylayout_x80000407.hpp" // German (Germany)
#include "../src/keyboard/reversed_keymaps/keylayout_x80010407.hpp" // German (Germany) IBM
#include "../src/keyboard/reversed_keymaps/keylayout_x80000807.hpp" // German (Switzerland)
#include "../src/keyboard/reversed_keymaps/keylayout_x80000408.hpp" // Greek (Greece) Greek
#include "../src/keyboard/reversed_keymaps/keylayout_x80010408.hpp" // Greek (Greece) Greek (220)
#include "../src/keyboard/reversed_keymaps/keylayout_x80020408.hpp" // Greek (Greece) Greek (319)
#include "../src/keyboard/reversed_keymaps/keylayout_x80030408.hpp" // Greek (Greece) Greek (220) Latin
#include "../src/keyboard/reversed_keymaps/keylayout_x80040408.hpp" // Greek (Greece) Greek (319) Latin
#include "../src/keyboard/reversed_keymaps/keylayout_x80050408.hpp" // Greek (Greece) Greek Latin
#include "../src/keyboard/reversed_keymaps/keylayout_x80060408.hpp" // Greek (Greece) Greek Polytonic
#include "../src/keyboard/reversed_keymaps/keylayout_x8001040e.hpp" // Hungarian (Hungary) Hungarian 101-key
#include "../src/keyboard/reversed_keymaps/keylayout_x8000040f.hpp" // Icelandic (Iceland) Icelandic
#include "../src/keyboard/reversed_keymaps/keylayout_x8000085d.hpp" // Inuktitut (Latin) (Canada) Inuktitut Latin
#include "../src/keyboard/reversed_keymaps/keylayout_x80000410.hpp" // Italian (Italy)
#include "../src/keyboard/reversed_keymaps/keylayout_x80010410.hpp" // Italian (Italy) 142
#include "../src/keyboard/reversed_keymaps/keylayout_x8000043f.hpp" // Kazakh (Kazakhstan) Kazakh
#include "../src/keyboard/reversed_keymaps/keylayout_x80000440.hpp" // Kyrgyz (Kyrgyzstan) Kyrgyz Cyrillic
#include "../src/keyboard/reversed_keymaps/keylayout_x80000426.hpp" // Latvian (Latvia) Latvian
#include "../src/keyboard/reversed_keymaps/keylayout_x80010426.hpp" // Latvian (Latvia) Latvian (QWERTY)
#include "../src/keyboard/reversed_keymaps/keylayout_x80010427.hpp" // Lithuanian (Lithuania) Lithuanian
#include "../src/keyboard/reversed_keymaps/keylayout_x80000427.hpp" // Lithuanian (Lithuania) Lithuanian IBM
#include "../src/keyboard/reversed_keymaps/keylayout_x8000046e.hpp" // Luxembourgish (Luxembourg)
#include "../src/keyboard/reversed_keymaps/keylayout_x8000042f.hpp" // Macedonian (Former Yugoslav Republic of Macedonia) FYRO Macedonian
#include "../src/keyboard/reversed_keymaps/keylayout_x8000043a.hpp" // Maltese (Malta) Maltese 47-key
#include "../src/keyboard/reversed_keymaps/keylayout_x8001043a.hpp" // Maltese (Malta) Maltese 48-key
#include "../src/keyboard/reversed_keymaps/keylayout_x80000481.hpp" // Maori (New Zealand) Maori
#include "../src/keyboard/reversed_keymaps/keylayout_x80000450.hpp" // Mongolian (Cyrillic, Mongolia) // Mongolian Cyrillic
#include "../src/keyboard/reversed_keymaps/keylayout_x80000414.hpp" // Norwegian, Bokmål (Norway) Norwegian
#include "../src/keyboard/reversed_keymaps/keylayout_x80010415.hpp" // Polish (Poland) Polish (214)
#include "../src/keyboard/reversed_keymaps/keylayout_x80000415.hpp" // Polish (Poland) Polish (Programmers)
#include "../src/keyboard/reversed_keymaps/keylayout_x80000816.hpp" // Portuguese (Portugal)
#include "../src/keyboard/reversed_keymaps/keylayout_x80000416.hpp" // Portuguese (Brazil) Portuguese (Brazilian ABNT)
#include "../src/keyboard/reversed_keymaps/keylayout_x80010416.hpp" // Portuguese (Brazil) Portuguese (Brazilian ABNT2)
#include "../src/keyboard/reversed_keymaps/keylayout_x80000418.hpp" // Romanian (Romania) Romanian
#include "../src/keyboard/reversed_keymaps/keylayout_x80000419.hpp" // Russian (Russia)
#include "../src/keyboard/reversed_keymaps/keylayout_x80010419.hpp" // Russian (Russia) typewriter
#include "../src/keyboard/reversed_keymaps/keylayout_x8000043b.hpp" // Sami (Northern) (Norway) Norwegian with Sami
#include "../src/keyboard/reversed_keymaps/keylayout_x8001043b.hpp" // Sami (Northern) (Norway) Sami Extended Norway
#include "../src/keyboard/reversed_keymaps/keylayout_x8000083b.hpp" // Sami (Northern) (Sweden)
#include "../src/keyboard/reversed_keymaps/keylayout_x8001083b.hpp" // Sami (Northern) (Sweden) Finnish with Sami
#include "../src/keyboard/reversed_keymaps/keylayout_x8002083b.hpp" // Sami (Northern) (Sweden) Sami Extended Finland-Sweden
#include "../src/keyboard/reversed_keymaps/keylayout_x80000c1a.hpp" // Serbian (Cyrillic, Serbia) Serbian (Cyrillic)
#include "../src/keyboard/reversed_keymaps/keylayout_x8000081a.hpp" // Serbian (Latin, Serbia) Serbian (Latin)
#include "../src/keyboard/reversed_keymaps/keylayout_x8000041b.hpp" // Slovak (Slovakia) Slovak
#include "../src/keyboard/reversed_keymaps/keylayout_x8001041b.hpp" // Slovak (Slovakia) Slovak (QWERTY)
#include "../src/keyboard/reversed_keymaps/keylayout_x80000424.hpp" // Slovenian (Slovenia) Slovenian
#include "../src/keyboard/reversed_keymaps/keylayout_x8000080a.hpp" // Spanish (Mexico) Latin American
#include "../src/keyboard/reversed_keymaps/keylayout_x8000040a.hpp" // Spanish (Spain)
#include "../src/keyboard/reversed_keymaps/keylayout_x8001040a.hpp" // Spanish (Spain) Spanish Variation
#include "../src/keyboard/reversed_keymaps/keylayout_x8000041d.hpp" // Swedish (Sweden)
#include "../src/keyboard/reversed_keymaps/keylayout_x80000444.hpp" // Tatar (Russia) Tatar
#include "../src/keyboard/reversed_keymaps/keylayout_x8001041f.hpp" // Turkish (Turkey) Turkish F
#include "../src/keyboard/reversed_keymaps/keylayout_x8000041f.hpp" // Turkish (Turkey) Turkish Q
#include "../src/keyboard/reversed_keymaps/keylayout_x80000422.hpp" // Ukrainian (Ukraine) Ukrainian
#include "../src/keyboard/reversed_keymaps/keylayout_x80000843.hpp" // Uzbek (Cyrillic, Uzbekistan) Uzbek Cyrillic
#include "../src/keyboard/reversed_keymaps/keylayout_x80000452.hpp" // Welsh (United Kingdom)




static const Keylayout_r * keylayoutsList[] = {  &keylayout_x80030402 ,&keylayout_x80010402 ,&keylayout_x8000201a
                                                ,&keylayout_x80000405 ,&keylayout_x80020405 ,&keylayout_x80010405
                                                ,&keylayout_x80000452 ,&keylayout_x80000406 ,&keylayout_x80000807
                                                ,&keylayout_x80000407 ,&keylayout_x80010407 ,&keylayout_x80000408
                                                ,&keylayout_x80010408 ,&keylayout_x80030408 ,&keylayout_x80020408
                                                ,&keylayout_x80040408 ,&keylayout_x80050408 ,&keylayout_x80060408
                                                ,&keylayout_x80001009 ,&keylayout_x80011009 ,&keylayout_x80000809
                                                ,&keylayout_x80011809 ,&keylayout_x80001809 ,&keylayout_x80000409
                                                ,&keylayout_x80010409 ,&keylayout_x80030409 ,&keylayout_x80040409
                                                ,&keylayout_x80020409 ,&keylayout_x8000040a ,&keylayout_x8001040a
                                                ,&keylayout_x8000080a ,&keylayout_x80000425 ,&keylayout_x8000040b
                                                ,&keylayout_x80000438 ,&keylayout_x8001080c ,&keylayout_x8000080c
                                                ,&keylayout_x80000c0c ,&keylayout_x8000100c ,&keylayout_x8000040c
                                                ,&keylayout_x8000041a ,&keylayout_x8001040e ,&keylayout_x8000040f
                                                ,&keylayout_x80000410 ,&keylayout_x80010410 ,&keylayout_x8000085d
                                                ,&keylayout_x8000043f ,&keylayout_x80000440 ,&keylayout_x8000046e
                                                ,&keylayout_x80010427 ,&keylayout_x80000427 ,&keylayout_x80000426
                                                ,&keylayout_x80010426 ,&keylayout_x80000481 ,&keylayout_x8000042f
                                                ,&keylayout_x80000450 ,&keylayout_x8000043a ,&keylayout_x8001043a
                                                ,&keylayout_x80000414 ,&keylayout_x80000813 ,&keylayout_x80000413
                                                ,&keylayout_x80010415 ,&keylayout_x80000415 ,&keylayout_x80000416
                                                ,&keylayout_x80010416 ,&keylayout_x80000816 ,&keylayout_x80000418
                                                ,&keylayout_x80000419 ,&keylayout_x80010419 ,&keylayout_x8000043b
                                                ,&keylayout_x8001043b ,&keylayout_x8000083b ,&keylayout_x8001083b
                                                ,&keylayout_x8002083b ,&keylayout_x8000041b ,&keylayout_x8001041b
                                                ,&keylayout_x80000424 ,&keylayout_x80000c1a ,&keylayout_x8000081a
                                                ,&keylayout_x8000041d ,&keylayout_x8001041f ,&keylayout_x8000041f
                                                ,&keylayout_x80000444 ,&keylayout_x80000422 ,&keylayout_x80000843
                                              };

enum : int {
    KEYLAYOUTS_LIST_SIZE = 84
};

enum KEYBOARDS : int {
    CS_CZ                     = 0x00405,     DA_DK                     = 0x00406,     DE_DE                     = 0x00407,
    EL_GR                     = 0x00408,     EN_US                     = 0x00409,     ES_ES                     = 0x0040a,
    FI_FI_FINNISH             = 0x0040b,     FR_FR                     = 0x0040c,     IS_IS                     = 0x0040f,
    IT_IT                     = 0x00410,     NL_NL                     = 0x00413,     NB_NO                     = 0x00414,
    PL_PL_PROGRAMMERS         = 0x00415,     PT_BR_ABNT                = 0x00416,     RO_RO                     = 0x00418,
    RU_RU                     = 0x00419,     HR_HR                     = 0x0041a,     SK_SK                     = 0x0041b,
    SV_SE                     = 0x0041d,     TR_TR_Q                   = 0x0041f,     UK_UA                     = 0x00422,
    SL_SI                     = 0x00424,     ET_EE                     = 0x00425,     LV_LV                     = 0x00426,
    LT_LT_IBM                 = 0x00427,     MK_MK                     = 0x0042f,     FO_FO                     = 0x00438,
    MT_MT_47                  = 0x0043a,     SE_NO                     = 0x0043b,     KK_KZ                     = 0x0043f,
    KY_KG                     = 0x00440,     TT_RU                     = 0x00444,     MN_MN                     = 0x00450,
    CY_GB                     = 0x00452,     LB_LU                     = 0x0046e,     MI_NZ                     = 0x00481,
    DE_CH                     = 0x00807,     EN_GB                     = 0x00809,     ES_MX                     = 0x0080a,
    FR_BE_FR                  = 0x0080c,     NL_BE                     = 0x00813,     PT_PT                     = 0x00816,
    SR_LA                     = 0x0081a,     SE_SE                     = 0x0083b,     UZ_CY                     = 0x00843,
    IU_LA                     = 0x0085d,     FR_CA                     = 0x00c0c,     SR_CY                     = 0x00c1a,
    EN_CA_FR                  = 0x01009,     FR_CH                     = 0x0100c,     BS_CY                     = 0x0201a,
    BG_BG_LATIN               = 0x10402,     CS_CZ_QWERTY              = 0x10405,     EN_IE_IRISH               = 0x01809,
    DE_DE_IBM                 = 0x10407,     EL_GR_220                 = 0x10408,     ES_ES_VARIATION           = 0x1040a,
    HU_HU                     = 0x1040e,     EN_US_DVORAK              = 0x10409,     IT_IT_142                 = 0x10410,
    PL_PL                     = 0x10415,     PT_BR_ABNT2               = 0x10416,     RU_RU_TYPEWRITER          = 0x10419,
    SK_SK_QWERTY              = 0x1041b,     TR_TR_F                   = 0x1041f,     LV_LV_QWERTY              = 0x10426,
    LT_LT                     = 0x10427,     MT_MT_48                  = 0x1043a,     SE_NO_EXT_NORWAY          = 0x1043b,
    FR_BE                     = 0x1080c,     SE_SE_2                   = 0x1083b,     EN_CA_MULTILINGUAL        = 0x11009,
    EN_IE                     = 0x11809,     CS_CZ_PROGRAMMERS         = 0x20405,     EL_GR_319                 = 0x20408,
    EN_US_INTERNATIONAL       = 0x20409,     SE_SE_EXT_FINLAND_SWEDEN  = 0x2083b,     BG_BG                     = 0x30402,
    EL_GR_220_LATIN           = 0x30408,     EN_US_DVORAK_LEFT         = 0x30409,     EL_GR_319_LATIN           = 0x40408,
    EN_US_DVORAK_RIGHT        = 0x40409,     EL_GR_LATIN               = 0x50408,     EL_GR_POLYTONIC           = 0x60408
};

