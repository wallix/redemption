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

#include "keyboard/reversed_keymaps/keylayouts_r.hpp"
#include "keylayout_x8000201a.hpp" // Bosnian (Cyrillic) (Bosnia and Herzegovina))
#include "keylayout_x80030402.hpp" // Bulgarian (Bulgaria)
#include "keylayout_x80010402.hpp" // Bulgarian (Bulgaria) - latin
#include "keylayout_x8000041a.hpp" // Croatian (Croatia) Croatian
#include "keylayout_x80000405.hpp" // Czech (Czech Republic) Czech
#include "keylayout_x80020405.hpp" // Czech (Czech Republic) Czech Programmers
#include "keylayout_x80010405.hpp" // Czech (Czech Republic) Czech (QWERTY)
#include "keylayout_x80000406.hpp" // Danish (Denmark) Danish
#include "keylayout_x80000813.hpp" // Dutch (Belgium)
#include "keylayout_x80000413.hpp" // Dutch (Netherlands)
#include "keylayout_x80001009.hpp" // English (Canada) Canadian French
#include "keylayout_x80011009.hpp" // English (Canada) Canadian Multilingual Standard
#include "keylayout_x80011809.hpp" // English (Ireland) Gaelic
#include "keylayout_x80001809.hpp" // English (Ireland) Irish
#include "keylayout_x80000809.hpp" // English (United Kingdom)
#include "keylayout_x80000409.hpp" // English (United States)
#include "keylayout_x80010409.hpp" // English (United States) United States-Dvorak
#include "keylayout_x80030409.hpp" // English (United States) United States-Dvorak for left hand
#include "keylayout_x80040409.hpp" // English (United States) United States-Dvorak for right hand
#include "keylayout_x80020409.hpp" // English (United States) United States-International
#include "keylayout_x80000425.hpp" // Estonian (Estonia) Estonian
#include "keylayout_x80000438.hpp" // Faroese (Faroe Islands) Faeroese
#include "keylayout_x8000040b.hpp" // Finnish (Finland) Finnish
#include "keylayout_x80000c0c.hpp" // French (Canada) Canadian French (Legacy)
#include "keylayout_x8000080c.hpp" // French (Belgium)
#include "keylayout_x8001080c.hpp" // French (Belgium) Belgian (Comma)
#include "keylayout_x8000040c.hpp" // French (France)
#include "keylayout_x8000100c.hpp" // French (Switzerland)
#include "keylayout_x80000407.hpp" // German (Germany)
#include "keylayout_x80010407.hpp" // German (Germany) IBM
#include "keylayout_x80000807.hpp" // German (Switzerland)
#include "keylayout_x80000408.hpp" // Greek (Greece) Greek
#include "keylayout_x80010408.hpp" // Greek (Greece) Greek (220)
#include "keylayout_x80020408.hpp" // Greek (Greece) Greek (319)
#include "keylayout_x80030408.hpp" // Greek (Greece) Greek (220) Latin
#include "keylayout_x80040408.hpp" // Greek (Greece) Greek (319) Latin
#include "keylayout_x80050408.hpp" // Greek (Greece) Greek Latin
#include "keylayout_x80060408.hpp" // Greek (Greece) Greek Polytonic
#include "keylayout_x8001040e.hpp" // Hungarian (Hungary) Hungarian 101-key
#include "keylayout_x8000040f.hpp" // Icelandic (Iceland) Icelandic
#include "keylayout_x8000085d.hpp" // Inuktitut (Latin) (Canada) Inuktitut Latin
#include "keylayout_x80000410.hpp" // Italian (Italy)
#include "keylayout_x80010410.hpp" // Italian (Italy) 142
#include "keylayout_x8000043f.hpp" // Kazakh (Kazakhstan) Kazakh
#include "keylayout_x80000440.hpp" // Kyrgyz (Kyrgyzstan) Kyrgyz Cyrillic
#include "keylayout_x80000426.hpp" // Latvian (Latvia) Latvian
#include "keylayout_x80010426.hpp" // Latvian (Latvia) Latvian (QWERTY)
#include "keylayout_x80010427.hpp" // Lithuanian (Lithuania) Lithuanian
#include "keylayout_x80000427.hpp" // Lithuanian (Lithuania) Lithuanian IBM
#include "keylayout_x8000046e.hpp" // Luxembourgish (Luxembourg)
#include "keylayout_x8000042f.hpp" // Macedonian (Former Yugoslav Republic of Macedonia) FYRO Macedonian
#include "keylayout_x8000043a.hpp" // Maltese (Malta) Maltese 47-key
#include "keylayout_x8001043a.hpp" // Maltese (Malta) Maltese 48-key
#include "keylayout_x80000481.hpp" // Maori (New Zealand) Maori
#include "keylayout_x80000450.hpp" // Mongolian (Cyrillic, Mongolia) // Mongolian Cyrillic
#include "keylayout_x80000414.hpp" // Norwegian, Bokmål (Norway) Norwegian
#include "keylayout_x80010415.hpp" // Polish (Poland) Polish (214)
#include "keylayout_x80000415.hpp" // Polish (Poland) Polish (Programmers)
#include "keylayout_x80000816.hpp" // Portuguese (Portugal)
#include "keylayout_x80000416.hpp" // Portuguese (Brazil) Portuguese (Brazilian ABNT)
#include "keylayout_x80010416.hpp" // Portuguese (Brazil) Portuguese (Brazilian ABNT2)
#include "keylayout_x80000418.hpp" // Romanian (Romania) Romanian
#include "keylayout_x80000419.hpp" // Russian (Russia)
#include "keylayout_x80010419.hpp" // Russian (Russia) typewriter
#include "keylayout_x8000043b.hpp" // Sami (Northern) (Norway) Norwegian with Sami
#include "keylayout_x8001043b.hpp" // Sami (Northern) (Norway) Sami Extended Norway
#include "keylayout_x8000083b.hpp" // Sami (Northern) (Sweden)
#include "keylayout_x8001083b.hpp" // Sami (Northern) (Sweden) Finnish with Sami
#include "keylayout_x8002083b.hpp" // Sami (Northern) (Sweden) Sami Extended Finland-Sweden
#include "keylayout_x80000c1a.hpp" // Serbian (Cyrillic, Serbia) Serbian (Cyrillic)
#include "keylayout_x8000081a.hpp" // Serbian (Latin, Serbia) Serbian (Latin)
#include "keylayout_x8000041b.hpp" // Slovak (Slovakia) Slovak
#include "keylayout_x8001041b.hpp" // Slovak (Slovakia) Slovak (QWERTY)
#include "keylayout_x80000424.hpp" // Slovenian (Slovenia) Slovenian
#include "keylayout_x8000080a.hpp" // Spanish (Mexico) Latin American
#include "keylayout_x8000040a.hpp" // Spanish (Spain)
#include "keylayout_x8001040a.hpp" // Spanish (Spain) Spanish Variation
#include "keylayout_x8000041d.hpp" // Swedish (Sweden)
#include "keylayout_x80000444.hpp" // Tatar (Russia) Tatar
#include "keylayout_x8001041f.hpp" // Turkish (Turkey) Turkish F
#include "keylayout_x8000041f.hpp" // Turkish (Turkey) Turkish Q
#include "keylayout_x80000422.hpp" // Ukrainian (Ukraine) Ukrainian
#include "keylayout_x80000843.hpp" // Uzbek (Cyrillic, Uzbekistan) Uzbek Cyrillic
#include "keylayout_x80000452.hpp" // Welsh (United Kingdom)


static const Keylayout_r * keylayoutsList[] = {
    &keylayout_x80030402 ,&keylayout_x80010402 ,&keylayout_x8000201a,
    &keylayout_x80000405 ,&keylayout_x80020405 ,&keylayout_x80010405,
    &keylayout_x80000452 ,&keylayout_x80000406 ,&keylayout_x80000807,
    &keylayout_x80000407 ,&keylayout_x80010407 ,&keylayout_x80000408,
    &keylayout_x80010408 ,&keylayout_x80030408 ,&keylayout_x80020408,
    &keylayout_x80040408 ,&keylayout_x80050408 ,&keylayout_x80060408,
    &keylayout_x80001009 ,&keylayout_x80011009 ,&keylayout_x80000809,
    &keylayout_x80011809 ,&keylayout_x80001809 ,&keylayout_x80000409,
    &keylayout_x80010409 ,&keylayout_x80030409 ,&keylayout_x80040409,
    &keylayout_x80020409 ,&keylayout_x8000040a ,&keylayout_x8001040a,
    &keylayout_x8000080a ,&keylayout_x80000425 ,&keylayout_x8000040b,
    &keylayout_x80000438 ,&keylayout_x8001080c ,&keylayout_x8000080c,
    &keylayout_x80000c0c ,&keylayout_x8000100c ,&keylayout_x8000040c,
    &keylayout_x8000041a ,&keylayout_x8001040e ,&keylayout_x8000040f,
    &keylayout_x80000410 ,&keylayout_x80010410 ,&keylayout_x8000085d,
    &keylayout_x8000043f ,&keylayout_x80000440 ,&keylayout_x8000046e,
    &keylayout_x80010427 ,&keylayout_x80000427 ,&keylayout_x80000426,
    &keylayout_x80010426 ,&keylayout_x80000481 ,&keylayout_x8000042f,
    &keylayout_x80000450 ,&keylayout_x8000043a ,&keylayout_x8001043a,
    &keylayout_x80000414 ,&keylayout_x80000813 ,&keylayout_x80000413,
    &keylayout_x80010415 ,&keylayout_x80000415 ,&keylayout_x80000416,
    &keylayout_x80010416 ,&keylayout_x80000816 ,&keylayout_x80000418,
    &keylayout_x80000419 ,&keylayout_x80010419 ,&keylayout_x8000043b,
    &keylayout_x8001043b ,&keylayout_x8000083b ,&keylayout_x8001083b,
    &keylayout_x8002083b ,&keylayout_x8000041b ,&keylayout_x8001041b,
    &keylayout_x80000424 ,&keylayout_x80000c1a ,&keylayout_x8000081a,
    &keylayout_x8000041d ,&keylayout_x8001041f ,&keylayout_x8000041f,
    &keylayout_x80000444 ,&keylayout_x80000422 ,&keylayout_x80000843
};

static_assert(KEYLAYOUTS_LIST_SIZE == (sizeof keylayoutsList / sizeof keylayoutsList[0]), "");

Keylayout_r const* find_keylayout_r(int LCID) noexcept
{
    for (Keylayout_r const* layout: keylayoutsList) {
        if (layout->LCID == LCID){
            return layout;
        }
    }
    return nullptr;
}

array_view<Keylayout_r const*> get_keylayout_r_list() noexcept
{
    return keylayoutsList;
}
