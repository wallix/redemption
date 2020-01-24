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
#include "utils/sugar/array_view.hpp"

Keylayout_r const* find_keylayout_r(int LCID) noexcept;

array_view<Keylayout_r const*> get_keylayout_r_list() noexcept;

enum : int {
    KEYLAYOUTS_LIST_SIZE = 84
};

enum KEYBOARDS : int {
    CS_CZ               = 0x00405,  DA_DK                     = 0x00406,  DE_DE              = 0x00407,
    EL_GR               = 0x00408,  EN_US                     = 0x00409,  ES_ES              = 0x0040a,
    FI_FI_FINNISH       = 0x0040b,  FR_FR                     = 0x0040c,  IS_IS              = 0x0040f,
    IT_IT               = 0x00410,  NL_NL                     = 0x00413,  NB_NO              = 0x00414,
    PL_PL_PROGRAMMERS   = 0x00415,  PT_BR_ABNT                = 0x00416,  RO_RO              = 0x00418,
    RU_RU               = 0x00419,  HR_HR                     = 0x0041a,  SK_SK              = 0x0041b,
    SV_SE               = 0x0041d,  TR_TR_Q                   = 0x0041f,  UK_UA              = 0x00422,
    SL_SI               = 0x00424,  ET_EE                     = 0x00425,  LV_LV              = 0x00426,
    LT_LT_IBM           = 0x00427,  MK_MK                     = 0x0042f,  FO_FO              = 0x00438,
    MT_MT_47            = 0x0043a,  SE_NO                     = 0x0043b,  KK_KZ              = 0x0043f,
    KY_KG               = 0x00440,  TT_RU                     = 0x00444,  MN_MN              = 0x00450,
    CY_GB               = 0x00452,  LB_LU                     = 0x0046e,  MI_NZ              = 0x00481,
    DE_CH               = 0x00807,  EN_GB                     = 0x00809,  ES_MX              = 0x0080a,
    FR_BE_FR            = 0x0080c,  NL_BE                     = 0x00813,  PT_PT              = 0x00816,
    SR_LA               = 0x0081a,  SE_SE                     = 0x0083b,  UZ_CY              = 0x00843,
    IU_LA               = 0x0085d,  FR_CA                     = 0x00c0c,  SR_CY              = 0x00c1a,
    EN_CA_FR            = 0x01009,  FR_CH                     = 0x0100c,  BS_CY              = 0x0201a,
    BG_BG_LATIN         = 0x10402,  CS_CZ_QWERTY              = 0x10405,  EN_IE_IRISH        = 0x01809,
    DE_DE_IBM           = 0x10407,  EL_GR_220                 = 0x10408,  ES_ES_VARIATION    = 0x1040a,
    HU_HU               = 0x1040e,  EN_US_DVORAK              = 0x10409,  IT_IT_142          = 0x10410,
    PL_PL               = 0x10415,  PT_BR_ABNT2               = 0x10416,  RU_RU_TYPEWRITER   = 0x10419,
    SK_SK_QWERTY        = 0x1041b,  TR_TR_F                   = 0x1041f,  LV_LV_QWERTY       = 0x10426,
    LT_LT               = 0x10427,  MT_MT_48                  = 0x1043a,  SE_NO_EXT_NORWAY   = 0x1043b,
    FR_BE               = 0x1080c,  SE_SE_2                   = 0x1083b,  EN_CA_MULTILINGUAL = 0x11009,
    EN_IE               = 0x11809,  CS_CZ_PROGRAMMERS         = 0x20405,  EL_GR_319          = 0x20408,
    EN_US_INTERNATIONAL = 0x20409,  SE_SE_EXT_FINLAND_SWEDEN  = 0x2083b,  BG_BG              = 0x30402,
    EL_GR_220_LATIN     = 0x30408,  EN_US_DVORAK_LEFT         = 0x30409,  EL_GR_319_LATIN    = 0x40408,
    EN_US_DVORAK_RIGHT  = 0x40409,  EL_GR_LATIN               = 0x50408,  EL_GR_POLYTONIC    = 0x60408
};

