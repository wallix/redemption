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

#include "keyboard/keylayout.hpp"
#include "keylayout_x0000201a.hpp" // Bosnian (Cyrillic) (Bosnia and Herzegovina))
#include "keylayout_x00030402.hpp" // Bulgarian (Bulgaria)
#include "keylayout_x00010402.hpp" // Bulgarian (Bulgaria) - latin
#include "keylayout_x0000041a.hpp" // Croatian (Croatia) Croatian
#include "keylayout_x00000405.hpp" // Czech (Czech Republic) Czech
#include "keylayout_x00020405.hpp" // Czech (Czech Republic) Czech Programmers
#include "keylayout_x00010405.hpp" // Czech (Czech Republic) Czech (QWERTY)
#include "keylayout_x00000406.hpp" // Danish (Denmark) Danish
#include "keylayout_x00000813.hpp" // Dutch (Belgium)
#include "keylayout_x00000413.hpp" // Dutch (Netherlands)
#include "keylayout_x00001009.hpp" // English (Canada) Canadian French
#include "keylayout_x00011009.hpp" // English (Canada) Canadian Multilingual Standard
#include "keylayout_x00011809.hpp" // English (Ireland) Gaelic
#include "keylayout_x00001809.hpp" // English (Ireland) Irish
#include "keylayout_x00000809.hpp" // English (United Kingdom)
#include "keylayout_x00000409.hpp" // English (United States)
#include "keylayout_x00010409.hpp" // English (United States) United States-Dvorak
#include "keylayout_x00030409.hpp" // English (United States) United States-Dvorak for left hand
#include "keylayout_x00040409.hpp" // English (United States) United States-Dvorak for right hand
#include "keylayout_x00020409.hpp" // English (United States) United States-International
#include "keylayout_x00000425.hpp" // Estonian (Estonia) Estonian
#include "keylayout_x00000438.hpp" // Faroese (Faroe Islands) Faeroese
#include "keylayout_x0000040b.hpp" // Finnish (Finland) Finnish
#include "keylayout_x00000c0c.hpp" // French (Canada) Canadian French (Legacy)
#include "keylayout_x0000080c.hpp" // French (Belgium)
#include "keylayout_x0001080c.hpp" // French (Belgium) Belgian (Comma)
#include "keylayout_x0000040c.hpp" // French (France)
#include "keylayout_x0000100c.hpp" // French (Switzerland)
#include "keylayout_x00000407.hpp" // German (Germany)
#include "keylayout_x00010407.hpp" // German (Germany) IBM
#include "keylayout_x00000807.hpp" // German (Switzerland)
#include "keylayout_x00000408.hpp" // Greek (Greece) Greek
#include "keylayout_x00010408.hpp" // Greek (Greece) Greek (220)
#include "keylayout_x00020408.hpp" // Greek (Greece) Greek (319)
#include "keylayout_x00030408.hpp" // Greek (Greece) Greek (220) Latin
#include "keylayout_x00040408.hpp" // Greek (Greece) Greek (319) Latin
#include "keylayout_x00050408.hpp" // Greek (Greece) Greek Latin
#include "keylayout_x00060408.hpp" // Greek (Greece) Greek Polytonic
#include "keylayout_x0001040e.hpp" // Hungarian (Hungary) Hungarian 101-key
#include "keylayout_x0000040f.hpp" // Icelandic (Iceland) Icelandic
#include "keylayout_x0000085d.hpp" // Inuktitut (Latin) (Canada) Inuktitut Latin
#include "keylayout_x00000410.hpp" // Italian (Italy)
#include "keylayout_x00010410.hpp" // Italian (Italy) 142
#include "keylayout_x0000043f.hpp" // Kazakh (Kazakhstan) Kazakh
#include "keylayout_x00000440.hpp" // Kyrgyz (Kyrgyzstan) Kyrgyz Cyrillic
#include "keylayout_x00000426.hpp" // Latvian (Latvia) Latvian
#include "keylayout_x00010426.hpp" // Latvian (Latvia) Latvian (QWERTY)
#include "keylayout_x00010427.hpp" // Lithuanian (Lithuania) Lithuanian
#include "keylayout_x00000427.hpp" // Lithuanian (Lithuania) Lithuanian IBM
#include "keylayout_x0000046e.hpp" // Luxembourgish (Luxembourg)
#include "keylayout_x0000042f.hpp" // Macedonian (Former Yugoslav Republic of Macedonia) FYRO Macedonian
#include "keylayout_x0000043a.hpp" // Maltese (Malta) Maltese 47-key
#include "keylayout_x0001043a.hpp" // Maltese (Malta) Maltese 48-key
#include "keylayout_x00000481.hpp" // Maori (New Zealand) Maori
#include "keylayout_x00000450.hpp" // Mongolian (Cyrillic, Mongolia) // Mongolian Cyrillic
#include "keylayout_x00000414.hpp" // Norwegian, Bokmål (Norway) Norwegian
#include "keylayout_x00010415.hpp" // Polish (Poland) Polish (214)
#include "keylayout_x00000415.hpp" // Polish (Poland) Polish (Programmers)
#include "keylayout_x00000816.hpp" // Portuguese (Portugal)
#include "keylayout_x00000416.hpp" // Portuguese (Brazil) Portuguese (Brazilian ABNT)
#include "keylayout_x00010416.hpp" // Portuguese (Brazil) Portuguese (Brazilian ABNT2)
#include "keylayout_x00000418.hpp" // Romanian (Romania) Romanian
#include "keylayout_x00000419.hpp" // Russian (Russia)
#include "keylayout_x00010419.hpp" // Russian (Russia) typewriter
#include "keylayout_x00000424.hpp" // Slovenian (Slovenia) Slovenian
#include "keylayout_x0000043b.hpp" // Sami (Northern) (Norway) Norwegian with Sami
#include "keylayout_x0001043b.hpp" // Sami (Northern) (Norway) Sami Extended Norway
#include "keylayout_x0000083b.hpp" // Sami (Northern) (Sweden)
#include "keylayout_x0000083b.hpp" // Sami (Northern) (Sweden) Swedish with Sami
#include "keylayout_x0001083b.hpp" // Sami (Northern) (Sweden) Finnish with Sami
#include "keylayout_x0002083b.hpp" // Sami (Northern) (Sweden) Sami Extended Finland-Sweden
#include "keylayout_x00000c1a.hpp" // Serbian (Cyrillic, Serbia) Serbian (Cyrillic)
#include "keylayout_x0000081a.hpp" // Serbian (Latin, Serbia) Serbian (Latin)
#include "keylayout_x0000041b.hpp" // Slovak (Slovakia) Slovak
#include "keylayout_x0001041b.hpp" // Slovak (Slovakia) Slovak (QWERTY)
#include "keylayout_x00000424.hpp" // Slovenian (Slovenia) Slovenian
#include "keylayout_x0000080a.hpp" // Spanish (Mexico) Latin American
#include "keylayout_x0000040a.hpp" // Spanish (Spain)
#include "keylayout_x0001040a.hpp" // Spanish (Spain) Spanish Variation
#include "keylayout_x0000041d.hpp" // Swedish (Sweden)
#include "keylayout_x00000444.hpp" // Tatar (Russia) Tatar
#include "keylayout_x0001041f.hpp" // Turkish (Turkey) Turkish F
#include "keylayout_x0000041f.hpp" // Turkish (Turkey) Turkish Q
#include "keylayout_x00000422.hpp" // Ukrainian (Ukraine) Ukrainian
#include "keylayout_x00000843.hpp" // Uzbek (Cyrillic, Uzbekistan) Uzbek Cyrillic
#include "keylayout_x00000452.hpp" // Welsh (United Kingdom)

