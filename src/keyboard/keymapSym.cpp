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
   Copyright (C) Wallix 2012-2013
   Author(s): Christophe Grosjean, Dominique Lafages

   header file. KeymapSym object for keymap translation from RDP to X (VNC)
*/

#include "keyboard/keymapSym.hpp"
#include "utils/log.hpp"
#include <cstring>


// KEYSYMS
// =======

//    space                               32     0x0020
//    exclam                              33     0x0021
//    quotedbl                            34     0x0022
//    numbersign                          35     0x0023
//    dollar                              36     0x0024
//    percent                             37     0x0025
//    ampersand                           38     0x0026
//    quoteright                          39     0x0027
//    parenleft                           40     0x0028
//    parenright                          41     0x0029
//    asterisk                            42     0x002a
//    plus                                43     0x002b
//    comma                               44     0x002c
//    minus                               45     0x002d
//    period                              46     0x002e
//    slash                               47     0x002f
//    0                                   48     0x0030
//    1                                   49     0x0031
//    2                                   50     0x0032
//    3                                   51     0x0033
//    4                                   52     0x0034
//    5                                   53     0x0035
//    6                                   54     0x0036
//    7                                   55     0x0037
//    8                                   56     0x0038
//    9                                   57     0x0039
//    colon                               58     0x003a
//    semicolon                           59     0x003b
//    less                                60     0x003c
//    equal                               61     0x003d
//    greater                             62     0x003e
//    question                            63     0x003f
//    at                                  64     0x0040
//    A                                   65     0x0041
//    B                                   66     0x0042
//    C                                   67     0x0043
//    D                                   68     0x0044
//    E                                   69     0x0045
//    F                                   70     0x0046
//    G                                   71     0x0047
//    H                                   72     0x0048
//    I                                   73     0x0049
//    J                                   74     0x004a
//    K                                   75     0x004b
//    L                                   76     0x004c
//    M                                   77     0x004d
//    N                                   78     0x004e
//    O                                   79     0x004f
//    P                                   80     0x0050
//    Q                                   81     0x0051
//    R                                   82     0x0052
//    S                                   83     0x0053
//    T                                   84     0x0054
//    U                                   85     0x0055
//    V                                   86     0x0056
//    W                                   87     0x0057
//    X                                   88     0x0058
//    Y                                   89     0x0059
//    Z                                   90     0x005a
//    bracketleft                         91     0x005b
//    backslash                           92     0x005c
//    bracketright                        93     0x005d
//    asciicircum                         94     0x005e
//    underscore                          95     0x005f
//    quoteleft                           96     0x0060
//    a                                   97     0x0061
//    b                                   98     0x0062
//    c                                   99     0x0063
//    d                                  100     0x0064
//    e                                  101     0x0065
//    f                                  102     0x0066
//    g                                  103     0x0067
//    h                                  104     0x0068
//    i                                  105     0x0069
//    j                                  106     0x006a
//    k                                  107     0x006b
//    l                                  108     0x006c
//    m                                  109     0x006d
//    n                                  110     0x006e
//    o                                  111     0x006f
//    p                                  112     0x0070
//    q                                  113     0x0071
//    r                                  114     0x0072
//    s                                  115     0x0073
//    t                                  116     0x0074
//    u                                  117     0x0075
//    v                                  118     0x0076
//    w                                  119     0x0077
//    x                                  120     0x0078
//    y                                  121     0x0079
//    z                                  122     0x007a
//    braceleft                          123     0x007b
//    bar                                124     0x007c
//    braceright                         125     0x007d
//    asciitilde                         126     0x007e
//    nobreakspace                       160     0x00a0
//    exclamdown                         161     0x00a1
//    cent                               162     0x00a2
//    sterling                           163     0x00a3
//    currency                           164     0x00a4
//    yen                                165     0x00a5
//    brokenbar                          166     0x00a6
//    section                            167     0x00a7
//    diaeresis                          168     0x00a8
//    copyright                          169     0x00a9
//    ordfeminine                        170     0x00aa
//    guillemotleft                      171     0x00ab
//    notsign                            172     0x00ac
//    hyphen                             173     0x00ad
//    registered                         174     0x00ae
//    macron                             175     0x00af
//    degree                             176     0x00b0
//    plusminus                          177     0x00b1
//    twosuperior                        178     0x00b2
//    threesuperior                      179     0x00b3
//    acute                              180     0x00b4
//    mu                                 181     0x00b5
//    paragraph                          182     0x00b6
//    periodcentered                     183     0x00b7
//    cedilla                            184     0x00b8
//    onesuperior                        185     0x00b9
//    masculine                          186     0x00ba
//    guillemotright                     187     0x00bb
//    onequarter                         188     0x00bc
//    onehalf                            189     0x00bd
//    threequarters                      190     0x00be
//    questiondown                       191     0x00bf
//    Agrave                             192     0x00c0
//    Aacute                             193     0x00c1
//    Acircumflex                        194     0x00c2
//    Atilde                             195     0x00c3
//    Adiaeresis                         196     0x00c4
//    Aring                              197     0x00c5
//    AE                                 198     0x00c6
//    Ccedilla                           199     0x00c7
//    Egrave                             200     0x00c8
//    Eacute                             201     0x00c9
//    Ecircumflex                        202     0x00ca
//    Ediaeresis                         203     0x00cb
//    Igrave                             204     0x00cc
//    Iacute                             205     0x00cd
//    Icircumflex                        206     0x00ce
//    Idiaeresis                         207     0x00cf
//    Eth                                208     0x00d0
//    Ntilde                             209     0x00d1
//    Ograve                             210     0x00d2
//    Oacute                             211     0x00d3
//    Ocircumflex                        212     0x00d4
//    Otilde                             213     0x00d5
//    Odiaeresis                         214     0x00d6
//    multiply                           215     0x00d7
//    Ooblique                           216     0x00d8
//    Ugrave                             217     0x00d9
//    Uacute                             218     0x00da
//    Ucircumflex                        219     0x00db
//    Udiaeresis                         220     0x00dc
//    Yacute                             221     0x00dd
//    Thorn                              222     0x00de
//    ssharp                             223     0x00df
//    agrave                             224     0x00e0
//    aacute                             225     0x00e1
//    acircumflex                        226     0x00e2
//    atilde                             227     0x00e3
//    adiaeresis                         228     0x00e4
//    aring                              229     0x00e5
//    ae                                 230     0x00e6
//    ccedilla                           231     0x00e7
//    egrave                             232     0x00e8
//    eacute                             233     0x00e9
//    ecircumflex                        234     0x00ea
//    ediaeresis                         235     0x00eb
//    igrave                             236     0x00ec
//    iacute                             237     0x00ed
//    icircumflex                        238     0x00ee
//    idiaeresis                         239     0x00ef
//    eth                                240     0x00f0
//    ntilde                             241     0x00f1
//    ograve                             242     0x00f2
//    oacute                             243     0x00f3
//    ocircumflex                        244     0x00f4
//    otilde                             245     0x00f5
//    odiaeresis                         246     0x00f6
//    division                           247     0x00f7
//    oslash                             248     0x00f8
//    ugrave                             249     0x00f9
//    uacute                             250     0x00fa
//    ucircumflex                        251     0x00fb
//    udiaeresis                         252     0x00fc
//    yacute                             253     0x00fd
//    thorn                              254     0x00fe
//    ydiaeresis                         255     0x00ff
//    Aogonek                            417     0x01a1
//    breve                              418     0x01a2
//    Lstroke                            419     0x01a3
//    Lcaron                             421     0x01a5
//    Sacute                             422     0x01a6
//    Scaron                             425     0x01a9
//    Scedilla                           426     0x01aa
//    Tcaron                             427     0x01ab
//    Zacute                             428     0x01ac

//    Zcaron                             430     0x01ae
//    Zabovedot                          431     0x01af
//    aogonek                            433     0x01b1
//    ogonek                             434     0x01b2
//    lstroke                            435     0x01b3
//    lcaron                             437     0x01b5
//    sacute                             438     0x01b6
//    caron                              439     0x01b7
//    scaron                             441     0x01b9
//    scedilla                           442     0x01ba
//    tcaron                             443     0x01bb
//    zacute                             444     0x01bc
//    doubleacute                        445     0x01bd
//    zcaron                             446     0x01be
//    zabovedot                          447     0x01bf
//    Racute                             448     0x01c0
//    Abreve                             451     0x01c3
//    Cacute                             454     0x01c6
//    Ccaron                             456     0x01c8
//    Eogonek                            458     0x01ca
//    Ecaron                             460     0x01cc
//    Dcaron                             463     0x01cf
//    Nacute                             465     0x01d1
//    Ncaron                             466     0x01d2
//    Odoubleacute                       469     0x01d5
//    Rcaron                             472     0x01d8
//    Uring                              473     0x01d9
//    Udoubleacute                       475     0x01db
//    Tcedilla                           478     0x01de
//    racute                             480     0x01e0
//    abreve                             483     0x01e3
//    cacute                             486     0x01e6
//    ccaron                             488     0x01e8
//    eogonek                            490     0x01ea
//    ecaron                             492     0x01ec
//    dcaron                             495     0x01ef
//    nacute                             497     0x01f1
//    ncaron                             498     0x01f2
//    odoubleacute                       501     0x01f5
//    rcaron                             504     0x01f8
//    uring                              505     0x01f9
//    udoubleacute                       507     0x01fb
//    tcedilla                           510     0x01fe
//    abovedot                           511     0x01ff
//    Hstroke                            673     0x02a1
//    Hcircumflex                        678     0x02a6
//    Iabovedot                          681     0x02a9
//    Gbreve                             683     0x02ab
//    Jcircumflex                        684     0x02ac
//    hstroke                            689     0x02b1
//    hcircumflex                        694     0x02b6
//    idotless                           697     0x02b9
//    gbreve                             699     0x02bb
//    jcircumflex                        700     0x02bc
//    Cabovedot                          709     0x02c5
//    Ccircumflex                        710     0x02c6
//    Gabovedot                          725     0x02d5
//    Gcircumflex                        728     0x02d8
//    Ubreve                             733     0x02dd
//    Scircumflex                        734     0x02de
//    cabovedot                          741     0x02e5
//    ccircumflex                        742     0x02e6
//    gabovedot                          757     0x02f5
//    gcircumflex                        760     0x02f8
//    ubreve                             765     0x02fd
//    scircumflex                        766     0x02fe
//    kappa                              930     0x03a2
//    Rcedilla                           931     0x03a3
//    Itilde                             933     0x03a5
//    Lcedilla                           934     0x03a6
//    Emacron                            938     0x03aa
//    Gcedilla                           939     0x03ab
//    Tslash                             940     0x03ac
//    rcedilla                           947     0x03b3
//    itilde                             949     0x03b5
//    lcedilla                           950     0x03b6
//    emacron                            954     0x03ba
//    gacute                             955     0x03bb
//    tslash                             956     0x03bc
//    ENG                                957     0x03bd
//    eng                                959     0x03bf
//    Amacron                            960     0x03c0
//    Iogonek                            967     0x03c7
//    Eabovedot                          972     0x03cc
//    Imacron                            975     0x03cf
//    Ncedilla                           977     0x03d1
//    Omacron                            978     0x03d2
//    Kcedilla                           979     0x03d3
//    Uogonek                            985     0x03d9
//    Utilde                             989     0x03dd
//    Umacron                            990     0x03de
//    amacron                            992     0x03e0
//    iogonek                            999     0x03e7
//    eabovedot                         1004     0x03ec
//    imacron                           1007     0x03ef
//    ncedilla                          1009     0x03f1
//    omacron                           1010     0x03f2
//    kcedilla                          1011     0x03f3
//    uogonek                           1017     0x03f9
//    utilde                            1021     0x03fd
//    umacron                           1022     0x03fe
//    overline                          1150     0x047e
//    kana_fullstop                     1185     0x04a1
//    kana_openingbracket               1186     0x04a2
//    kana_closingbracket               1187     0x04a3
//    kana_comma                        1188     0x04a4
//    kana_middledot                    1189     0x04a5
//    kana_WO                           1190     0x04a6
//    kana_a                            1191     0x04a7
//    kana_i                            1192     0x04a8
//    kana_u                            1193     0x04a9
//    kana_e                            1194     0x04aa
//    kana_o                            1195     0x04ab
//    kana_ya                           1196     0x04ac
//    kana_yu                           1197     0x04ad
//    kana_yo                           1198     0x04ae
//    kana_tu                           1199     0x04af
//    prolongedsound                    1200     0x04b0
//    kana_A                            1201     0x04b1
//    kana_I                            1202     0x04b2
//    kana_U                            1203     0x04b3
//    kana_E                            1204     0x04b4
//    kana_O                            1205     0x04b5
//    kana_KA                           1206     0x04b6
//    kana_KI                           1207     0x04b7
//    kana_KU                           1208     0x04b8
//    kana_KE                           1209     0x04b9
//    kana_KO                           1210     0x04ba
//    kana_SA                           1211     0x04bb
//    kana_SHI                          1212     0x04bc
//    kana_SU                           1213     0x04bd
//    kana_SE                           1214     0x04be
//    kana_SO                           1215     0x04bf
//    kana_TA                           1216     0x04c0
//    kana_TI                           1217     0x04c1
//    kana_TU                           1218     0x04c2
//    kana_TE                           1219     0x04c3
//    kana_TO                           1220     0x04c4
//    kana_NA                           1221     0x04c5
//    kana_NI                           1222     0x04c6
//    kana_NU                           1223     0x04c7
//    kana_NE                           1224     0x04c8
//    kana_NO                           1225     0x04c9
//    kana_HA                           1226     0x04ca
//    kana_HI                           1227     0x04cb
//    kana_HU                           1228     0x04cc
//    kana_HE                           1229     0x04cd
//    kana_HO                           1230     0x04ce
//    kana_MA                           1231     0x04cf
//    kana_MI                           1232     0x04d0
//    kana_MU                           1233     0x04d1
//    kana_ME                           1234     0x04d2
//    kana_MO                           1235     0x04d3
//    kana_YA                           1236     0x04d4
//    kana_YU                           1237     0x04d5
//    kana_YO                           1238     0x04d6
//    kana_RA                           1239     0x04d7
//    kana_RI                           1240     0x04d8
//    kana_RU                           1241     0x04d9
//    kana_RE                           1242     0x04da
//    kana_RO                           1243     0x04db
//    kana_WA                           1244     0x04dc
//    kana_N                            1245     0x04dd
//    voicedsound                       1246     0x04de
//    semivoicedsound                   1247     0x04df
//    Arabic_comma                      1452     0x05ac
//    Arabic_semicolon                  1467     0x05bb
//    Arabic_question_mark              1471     0x05bf
//    Arabic_hamza                      1473     0x05c1
//    Arabic_maddaonalef                1474     0x05c2
//    Arabic_hamzaonalef                1475     0x05c3
//    Arabic_hamzaonwaw                 1476     0x05c4
//    Arabic_hamzaunderalef             1477     0x05c5
//    Arabic_hamzaonyeh                 1478     0x05c6
//    Arabic_alef                       1479     0x05c7
//    Arabic_beh                        1480     0x05c8
//    Arabic_tehmarbuta                 1481     0x05c9
//    Arabic_teh                        1482     0x05ca
//    Arabic_theh                       1483     0x05cb
//    Arabic_jeem                       1484     0x05cc
//    Arabic_hah                        1485     0x05cd
//    Arabic_khah                       1486     0x05ce
//    Arabic_dal                        1487     0x05cf
//    Arabic_thal                       1488     0x05d0
//    Arabic_ra                         1489     0x05d1
//    Arabic_zain                       1490     0x05d2
//    Arabic_seen                       1491     0x05d3
//    Arabic_sheen                      1492     0x05d4
//    Arabic_sad                        1493     0x05d5
//    Arabic_dad                        1494     0x05d6
//    Arabic_tah                        1495     0x05d7
//    Arabic_zah                        1496     0x05d8
//    Arabic_ain                        1497     0x05d9
//    Arabic_ghain                      1498     0x05da
//    Arabic_tatweel                    1504     0x05e0
//    Arabic_feh                        1505     0x05e1
//    Arabic_qaf                        1506     0x05e2
//    Arabic_kaf                        1507     0x05e3
//    Arabic_lam                        1508     0x05e4
//    Arabic_meem                       1509     0x05e5

//    Arabic_noon                       1510     0x05e6
//    Arabic_heh                        1511     0x05e7
//    Arabic_waw                        1512     0x05e8
//    Arabic_alefmaksura                1513     0x05e9
//    Arabic_yeh                        1514     0x05ea
//    Arabic_fathatan                   1515     0x05eb
//    Arabic_dammatan                   1516     0x05ec
//    Arabic_kasratan                   1517     0x05ed
//    Arabic_fatha                      1518     0x05ee
//    Arabic_damma                      1519     0x05ef
//    Arabic_kasra                      1520     0x05f0
//    Arabic_shadda                     1521     0x05f1
//    Arabic_sukun                      1522     0x05f2
//    Serbian_dje                       1697     0x06a1
//    Macedonia_gje                     1698     0x06a2
//    Cyrillic_io                       1699     0x06a3
//    Ukranian_je                       1700     0x06a4
//    Macedonia_dse                     1701     0x06a5
//    Ukranian_i                        1702     0x06a6
//    Ukranian_yi                       1703     0x06a7
//    Serbian_je                        1704     0x06a8
//    Serbian_lje                       1705     0x06a9
//    Serbian_nje                       1706     0x06aa
//    Serbian_tshe                      1707     0x06ab
//    Macedonia_kje                     1708     0x06ac
//    Byelorussian_shortu               1710     0x06ae
//    Serbian_dze                       1711     0x06af
//    numerosign                        1712     0x06b0
//    Serbian_DJE                       1713     0x06b1
//    Macedonia_GJE                     1714     0x06b2
//    Cyrillic_IO                       1715     0x06b3
//    Ukranian_JE                       1716     0x06b4
//    Macedonia_DSE                     1717     0x06b5
//    Ukranian_I                        1718     0x06b6
//    Ukranian_YI                       1719     0x06b7
//    Serbian_JE                        1720     0x06b8
//    Serbian_LJE                       1721     0x06b9
//    Serbian_NJE                       1722     0x06ba
//    Serbian_TSHE                      1723     0x06bb
//    Macedonia_KJE                     1724     0x06bc
//    Byelorussian_SHORTU               1726     0x06be
//    Serbian_DZE                       1727     0x06bf
//    Cyrillic_yu                       1728     0x06c0
//    Cyrillic_a                        1729     0x06c1
//    Cyrillic_be                       1730     0x06c2
//    Cyrillic_tse                      1731     0x06c3
//    Cyrillic_de                       1732     0x06c4
//    Cyrillic_ie                       1733     0x06c5
//    Cyrillic_ef                       1734     0x06c6
//    Cyrillic_ghe                      1735     0x06c7
//    Cyrillic_ha                       1736     0x06c8
//    Cyrillic_i                        1737     0x06c9
//    Cyrillic_shorti                   1738     0x06ca
//    Cyrillic_ka                       1739     0x06cb
//    Cyrillic_el                       1740     0x06cc
//    Cyrillic_em                       1741     0x06cd
//    Cyrillic_en                       1742     0x06ce
//    Cyrillic_o                        1743     0x06cf
//    Cyrillic_pe                       1744     0x06d0
//    Cyrillic_ya                       1745     0x06d1
//    Cyrillic_er                       1746     0x06d2
//    Cyrillic_es                       1747     0x06d3
//    Cyrillic_te                       1748     0x06d4
//    Cyrillic_u                        1749     0x06d5
//    Cyrillic_zhe                      1750     0x06d6
//    Cyrillic_ve                       1751     0x06d7
//    Cyrillic_softsign                 1752     0x06d8
//    Cyrillic_yeru                     1753     0x06d9
//    Cyrillic_ze                       1754     0x06da
//    Cyrillic_sha                      1755     0x06db
//    Cyrillic_e                        1756     0x06dc
//    Cyrillic_shcha                    1757     0x06dd
//    Cyrillic_che                      1758     0x06de
//    Cyrillic_hardsign                 1759     0x06df
//    Cyrillic_YU                       1760     0x06e0
//    Cyrillic_A                        1761     0x06e1
//    Cyrillic_BE                       1762     0x06e2
//    Cyrillic_TSE                      1763     0x06e3
//    Cyrillic_DE                       1764     0x06e4
//    Cyrillic_IE                       1765     0x06e5
//    Cyrillic_EF                       1766     0x06e6
//    Cyrillic_GHE                      1767     0x06e7
//    Cyrillic_HA                       1768     0x06e8
//    Cyrillic_I                        1769     0x06e9
//    Cyrillic_SHORTI                   1770     0x06ea
//    Cyrillic_KA                       1771     0x06eb
//    Cyrillic_EL                       1772     0x06ec
//    Cyrillic_EM                       1773     0x06ed
//    Cyrillic_EN                       1774     0x06ee
//    Cyrillic_O                        1775     0x06ef
//    Cyrillic_PE                       1776     0x06f0
//    Cyrillic_YA                       1777     0x06f1
//    Cyrillic_ER                       1778     0x06f2
//    Cyrillic_ES                       1779     0x06f3
//    Cyrillic_TE                       1780     0x06f4
//    Cyrillic_U                        1781     0x06f5
//    Cyrillic_ZHE                      1782     0x06f6
//    Cyrillic_VE                       1783     0x06f7
//    Cyrillic_SOFTSIGN                 1784     0x06f8
//    Cyrillic_YERU                     1785     0x06f9
//    Cyrillic_ZE                       1786     0x06fa
//    Cyrillic_SHA                      1787     0x06fb
//    Cyrillic_E                        1788     0x06fc
//    Cyrillic_SHCHA                    1789     0x06fd
//    Cyrillic_CHE                      1790     0x06fe
//    Cyrillic_HARDSIGN                 1791     0x06ff
//    Greek_ALPHAaccent                 1953     0x07a1
//    Greek_EPSILONaccent               1954     0x07a2
//    Greek_ETAaccent                   1955     0x07a3
//    Greek_IOTAaccent                  1956     0x07a4
//    Greek_IOTAdiaeresis               1957     0x07a5
//    Greek_IOTAaccentdiaeresis         1958     0x07a6
//    Greek_OMICRONaccent               1959     0x07a7
//    Greek_UPSILONaccent               1960     0x07a8
//    Greek_UPSILONdieresis             1961     0x07a9
//    Greek_UPSILONaccentdieresis       1962     0x07aa
//    Greek_OMEGAaccent                 1963     0x07ab
//    Greek_alphaaccent                 1969     0x07b1
//    Greek_epsilonaccent               1970     0x07b2
//    Greek_etaaccent                   1971     0x07b3
//    Greek_iotaaccent                  1972     0x07b4
//    Greek_iotadieresis                1973     0x07b5
//    Greek_iotaaccentdieresis          1974     0x07b6
//    Greek_omicronaccent               1975     0x07b7
//    Greek_upsilonaccent               1976     0x07b8
//    Greek_upsilondieresis             1977     0x07b9
//    Greek_upsilonaccentdieresis       1978     0x07ba
//    Greek_omegaaccent                 1979     0x07bb
//    Greek_ALPHA                       1985     0x07c1
//    Greek_BETA                        1986     0x07c2
//    Greek_GAMMA                       1987     0x07c3
//    Greek_DELTA                       1988     0x07c4
//    Greek_EPSILON                     1989     0x07c5
//    Greek_ZETA                        1990     0x07c6
//    Greek_ETA                         1991     0x07c7
//    Greek_THETA                       1992     0x07c8
//    Greek_IOTA                        1993     0x07c9
//    Greek_KAPPA                       1994     0x07ca
//    Greek_LAMBDA                      1995     0x07cb
//    Greek_MU                          1996     0x07cc
//    Greek_NU                          1997     0x07cd
//    Greek_XI                          1998     0x07ce
//    Greek_OMICRON                     1999     0x07cf
//    Greek_PI                          2000     0x07d0
//    Greek_RHO                         2001     0x07d1
//    Greek_SIGMA                       2002     0x07d2
//    Greek_TAU                         2004     0x07d4
//    Greek_UPSILON                     2005     0x07d5
//    Greek_PHI                         2006     0x07d6
//    Greek_CHI                         2007     0x07d7
//    Greek_PSI                         2008     0x07d8
//    Greek_OMEGA                       2009     0x07d9
//    Greek_alpha                       2017     0x07e1
//    Greek_beta                        2018     0x07e2
//    Greek_gamma                       2019     0x07e3
//    Greek_delta                       2020     0x07e4
//    Greek_epsilon                     2021     0x07e5
//    Greek_zeta                        2022     0x07e6
//    Greek_eta                         2023     0x07e7
//    Greek_theta                       2024     0x07e8
//    Greek_iota                        2025     0x07e9
//    Greek_kappa                       2026     0x07ea
//    Greek_lambda                      2027     0x07eb
//    Greek_mu                          2028     0x07ec
//    Greek_nu                          2029     0x07ed
//    Greek_xi                          2030     0x07ee
//    Greek_omicron                     2031     0x07ef
//    Greek_pi                          2032     0x07f0
//    Greek_rho                         2033     0x07f1
//    Greek_sigma                       2034     0x07f2
//    Greek_finalsmallsigma             2035     0x07f3
//    Greek_tau                         2036     0x07f4
//    Greek_upsilon                     2037     0x07f5
//    Greek_phi                         2038     0x07f6
//    Greek_chi                         2039     0x07f7
//    Greek_psi                         2040     0x07f8
//    Greek_omega                       2041     0x07f9
//    leftradical                       2209     0x08a1
//    topleftradical                    2210     0x08a2
//    horizconnector                    2211     0x08a3
//    topintegral                       2212     0x08a4
//    botintegral                       2213     0x08a5
//    vertconnector                     2214     0x08a6
//    topleftsqbracket                  2215     0x08a7
//    botleftsqbracket                  2216     0x08a8
//    toprightsqbracket                 2217     0x08a9
//    botrightsqbracket                 2218     0x08aa
//    topleftparens                     2219     0x08ab
//    botleftparens                     2220     0x08ac
//    toprightparens                    2221     0x08ad
//    botrightparens                    2222     0x08ae
//    leftmiddlecurlybrace              2223     0x08af
//    rightmiddlecurlybrace             2224     0x08b0
//    topleftsummation                  2225     0x08b1
//    botleftsummation                  2226     0x08b2
//    topvertsummationconnector         2227     0x08b3
//    botvertsummationconnector         2228     0x08b4
//    toprightsummation                 2229     0x08b5
//    botrightsummation                 2230     0x08b6
//    rightmiddlesummation              2231     0x08b7

//    lessthanequal                     2236     0x08bc
//    notequal                          2237     0x08bd
//    greaterthanequal                  2238     0x08be
//    integral                          2239     0x08bf
//    therefore                         2240     0x08c0
//    variation                         2241     0x08c1
//    infinity                          2242     0x08c2
//    nabla                             2245     0x08c5
//    approximate                       2248     0x08c8
//    similarequal                      2249     0x08c9
//    ifonlyif                          2253     0x08cd
//    implies                           2254     0x08ce
//    identical                         2255     0x08cf
//    radical                           2262     0x08d6
//    include "in                        2266     0x08da
//    include "                          2267     0x08db
//    intersection                      2268     0x08dc
//    union                             2269     0x08dd
//    logicaland                        2270     0x08de
//    logicalor                         2271     0x08df
//    partialderivative                 2287     0x08ef
//    function                          2294     0x08f6
//    leftarrow                         2299     0x08fb
//    uparrow                           2300     0x08fc
//    rightarrow                        2301     0x08fd
//    downarrow                         2302     0x08fe
//    blank                             2527     0x09df
//    soliddiamond                      2528     0x09e0
//    checkerboard                      2529     0x09e1
//    ht                                2530     0x09e2
//    ff                                2531     0x09e3
//    cr                                2532     0x09e4
//    lf                                2533     0x09e5
//    nl                                2536     0x09e8
//    vt                                2537     0x09e9
//    lowrightcorner                    2538     0x09ea
//    uprightcorner                     2539     0x09eb
//    upleftcorner                      2540     0x09ec
//    lowleftcorner                     2541     0x09ed
//    crossinglines                     2542     0x09ee
//    horizlinescan1                    2543     0x09ef
//    horizlinescan3                    2544     0x09f0
//    horizlinescan5                    2545     0x09f1
//    horizlinescan7                    2546     0x09f2
//    horizlinescan9                    2547     0x09f3
//    leftt                             2548     0x09f4
//    rightt                            2549     0x09f5
//    bott                              2550     0x09f6
//    topt                              2551     0x09f7
//    vertbar                           2552     0x09f8
//    emspace                           2721     0x0aa1
//    enspace                           2722     0x0aa2
//    em3space                          2723     0x0aa3
//    em4space                          2724     0x0aa4
//    digitspace                        2725     0x0aa5
//    punctspace                        2726     0x0aa6
//    thinspace                         2727     0x0aa7
//    hairspace                         2728     0x0aa8
//    emdash                            2729     0x0aa9
//    endash                            2730     0x0aaa
//    signifblank                       2732     0x0aac
//    ellipsis                          2734     0x0aae
//    doubbaselinedot                   2735     0x0aaf
//    onethird                          2736     0x0ab0
//    twothirds                         2737     0x0ab1
//    onefifth                          2738     0x0ab2
//    twofifths                         2739     0x0ab3
//    threefifths                       2740     0x0ab4
//    fourfifths                        2741     0x0ab5
//    onesixth                          2742     0x0ab6
//    fivesixths                        2743     0x0ab7
//    careof                            2744     0x0ab8
//    figdash                           2747     0x0abb
//    leftanglebracket                  2748     0x0abc
//    decimalpoint                      2749     0x0abd
//    rightanglebracket                 2750     0x0abe
//    marker                            2751     0x0abf
//    oneeighth                         2755     0x0ac3
//    threeeighths                      2756     0x0ac4
//    fiveeighths                       2757     0x0ac5
//    seveneighths                      2758     0x0ac6
//    trademark                         2761     0x0ac9
//    signaturemark                     2762     0x0aca
//    trademarkincircle                 2763     0x0acb
//    leftopentriangle                  2764     0x0acc
//    rightopentriangle                 2765     0x0acd
//    emopencircle                      2766     0x0ace
//    emopenrectangle                   2767     0x0acf
//    leftsinglequotemark               2768     0x0ad0
//    rightsinglequotemark              2769     0x0ad1
//    leftdoublequotemark               2770     0x0ad2
//    rightdoublequotemark              2771     0x0ad3
//    prescription                      2772     0x0ad4
//    minutes                           2774     0x0ad6
//    seconds                           2775     0x0ad7
//    latincross                        2777     0x0ad9
//    hexagram                          2778     0x0ada
//    filledrectbullet                  2779     0x0adb
//    filledlefttribullet               2780     0x0adc
//    filledrighttribullet              2781     0x0add
//    emfilledcircle                    2782     0x0ade
//    emfilledrect                      2783     0x0adf
//    enopencircbullet                  2784     0x0ae0
//    enopensquarebullet                2785     0x0ae1
//    openrectbullet                    2786     0x0ae2
//    opentribulletup                   2787     0x0ae3
//    opentribulletdown                 2788     0x0ae4
//    openstar                          2789     0x0ae5
//    enfilledcircbullet                2790     0x0ae6
//    enfilledsqbullet                  2791     0x0ae7
//    filledtribulletup                 2792     0x0ae8
//    filledtribulletdown               2793     0x0ae9
//    leftpointer                       2794     0x0aea
//    rightpointer                      2795     0x0aeb
//    club                              2796     0x0aec
//    diamond                           2797     0x0aed
//    heart                             2798     0x0aee
//    maltesecross                      2800     0x0af0
//    dagger                            2801     0x0af1
//    doubledagger                      2802     0x0af2
//    checkmark                         2803     0x0af3
//    ballotcross                       2804     0x0af4
//    musicalsharp                      2805     0x0af5
//    musicalflat                       2806     0x0af6
//    malesymbol                        2807     0x0af7
//    femalesymbol                      2808     0x0af8
//    telephone                         2809     0x0af9
//    telephonerecorder                 2810     0x0afa
//    phonographcopyright               2811     0x0afb
//    caret                             2812     0x0afc
//    singlelowquotemark                2813     0x0afd
//    doublelowquotemark                2814     0x0afe
//    cursor                            2815     0x0aff
//    leftcaret                         2979     0x0ba3
//    rightcaret                        2982     0x0ba6
//    downcaret                         2984     0x0ba8
//    upcaret                           2985     0x0ba9
//    overbar                           3008     0x0bc0
//    downtack                          3010     0x0bc2
//    upshoe                            3011     0x0bc3
//    downstile                         3012     0x0bc4
//    underbar                          3014     0x0bc6
//    jot                               3018     0x0bca
//    quad                              3020     0x0bcc
//    uptack                            3022     0x0bce
//    circle                            3023     0x0bcf
//    upstile                           3027     0x0bd3
//    downshoe                          3030     0x0bd6
//    rightshoe                         3032     0x0bd8
//    leftshoe                          3034     0x0bda
//    lefttack                          3036     0x0bdc
//    righttack                         3068     0x0bfc
//    hebrew_aleph                      3296     0x0ce0
//    hebrew_beth                       3297     0x0ce1
//    hebrew_gimmel                     3298     0x0ce2
//    hebrew_daleth                     3299     0x0ce3
//    hebrew_he                         3300     0x0ce4
//    hebrew_waw                        3301     0x0ce5
//    hebrew_zayin                      3302     0x0ce6
//    hebrew_het                        3303     0x0ce7
//    hebrew_teth                       3304     0x0ce8
//    hebrew_yod                        3305     0x0ce9
//    hebrew_finalkaph                  3306     0x0cea
//    hebrew_kaph                       3307     0x0ceb
//    hebrew_lamed                      3308     0x0cec
//    hebrew_finalmem                   3309     0x0ced
//    hebrew_mem                        3310     0x0cee
//    hebrew_finalnun                   3311     0x0cef
//    hebrew_nun                        3312     0x0cf0
//    hebrew_samekh                     3313     0x0cf1
//    hebrew_ayin                       3314     0x0cf2
//    hebrew_finalpe                    3315     0x0cf3
//    hebrew_pe                         3316     0x0cf4
//    hebrew_finalzadi                  3317     0x0cf5
//    hebrew_zadi                       3318     0x0cf6
//    hebrew_kuf                        3319     0x0cf7
//    hebrew_resh                       3320     0x0cf8
//    hebrew_shin                       3321     0x0cf9
//    hebrew_taf                        3322     0x0cfa
//    BackSpace                        65288     0xff08
//    Tab                              65289     0xff09
//    Linefeed                         65290     0xff0a
//    Clear                            65291     0xff0b
//    Return                           65293     0xff0d
//    Pause                            65299     0xff13
//    Scroll_Lock                      65300     0xff14
//    Sys_Req                          65301     0xff15
//    Escape                           65307     0xff1b
//    Multi_key                        65312     0xff20
//    Kanji                            65313     0xff21
//    Home                             65360     0xff50
//    Left                             65361     0xff51
//    Up                               65362     0xff52
//    Right                            65363     0xff53
//    Down                             65364     0xff54
//    Prior                            65365     0xff55
//    Next                             65366     0xff56
//    End                              65367     0xff57
//    Begin                            65368     0xff58
//    Win_L                            65371     0xff5b
//    Win_R                            65372     0xff5c

//    App                              65373     0xff5d
//    Select                           65376     0xff60
//    Print                            65377     0xff61
//    Execute                          65378     0xff62
//    Insert                           65379     0xff63
//    Undo                             65381     0xff65
//    Redo                             65382     0xff66
//    Menu                             65383     0xff67
//    Find                             65384     0xff68
//    Cancel                           65385     0xff69
//    Help                             65386     0xff6a
//    Break                            65387     0xff6b
//    Hebrew_switch                    65406     0xff7e
//    Num_Lock                         65407     0xff7f
//    KP_Space                         65408     0xff80
//    KP_Tab                           65417     0xff89
//    KP_Enter                         65421     0xff8d
//    KP_F1                            65425     0xff91
//    KP_F2                            65426     0xff92
//    KP_F3                            65427     0xff93
//    KP_F4                            65428     0xff94
//    KP_Multiply                      65450     0xffaa
//    KP_Add                           65451     0xffab
//    KP_Separator                     65452     0xffac
//    KP_Subtract                      65453     0xffad
//    KP_Decimal                       65454     0xffae
//    KP_Divide                        65455     0xffaf
//    KP_0                             65456     0xffb0
//    KP_1                             65457     0xffb1
//    KP_2                             65458     0xffb2
//    KP_3                             65459     0xffb3
//    KP_4                             65460     0xffb4
//    KP_5                             65461     0xffb5
//    KP_6                             65462     0xffb6
//    KP_7                             65463     0xffb7
//    KP_8                             65464     0xffb8
//    KP_9                             65465     0xffb9
//    KP_Equal                         65469     0xffbd
//    F1                               65470     0xffbe
//    F2                               65471     0xffbf
//    F3                               65472     0xffc0
//    F4                               65473     0xffc1
//    F5                               65474     0xffc2
//    F6                               65475     0xffc3
//    F7                               65476     0xffc4
//    F8                               65477     0xffc5
//    F9                               65478     0xffc6
//    F10                              65479     0xffc7
//    L1                               65480     0xffc8
//    L2                               65481     0xffc9
//    L3                               65482     0xffca
//    L4                               65483     0xffcb
//    L5                               65484     0xffcc
//    L6                               65485     0xffcd
//    L7                               65486     0xffce
//    L8                               65487     0xffcf
//    L9                               65488     0xffd0
//    L10                              65489     0xffd1
//    R1                               65490     0xffd2
//    R2                               65491     0xffd3
//    R3                               65492     0xffd4
//    R4                               65493     0xffd5
//    R5                               65494     0xffd6
//    R6                               65495     0xffd7
//    R7                               65496     0xffd8
//    R8                               65497     0xffd9
//    R9                               65498     0xffda
//    R10                              65499     0xffdb
//    R11                              65500     0xffdc
//    R12                              65501     0xffdd
//    F33                              65502     0xffde
//    R14                              65503     0xffdf
//    R15                              65504     0xffe0
//    Shift_L                          65505     0xffe1
//    Shift_R                          65506     0xffe2
//    Control_L                        65507     0xffe3
//    Control_R                        65508     0xffe4
//    Caps_Lock                        65509     0xffe5
//    Shift_Lock                       65510     0xffe6
//    Meta_L                           65511     0xffe7
//    Meta_R                           65512     0xffe8
//    Alt_L                            65513     0xffe9
//    Alt_R                            65514     0xffea
//    Super_L                          65515     0xffeb
//    Super_R                          65516     0xffec
//    Hyper_L                          65517     0xffed
//    Hyper_R                          65518     0xffee
//    Delete                           65535     0xffff

// using namespace std;

KeymapSym::KeymapSym(int verbose)
: ibuf_sym(0)
, nbuf_sym(0)
, dead_key(DEADKEY_NONE)
, verbose(verbose)
{
    memset(this->keys_down, 0, 256 * sizeof(int));

    memset(&this->keylayout_WORK_noshift_sym,       0, 128 * sizeof(int));
    memset(&this->keylayout_WORK_shift_sym,         0, 128 * sizeof(int));
    memset(&this->keylayout_WORK_altgr_sym,         0, 128 * sizeof(int));
    memset(&this->keylayout_WORK_capslock_sym,      0, 128 * sizeof(int));
    memset(&this->keylayout_WORK_shiftcapslock_sym, 0, 128 * sizeof(int));

    this->key_flags = 0;
    this->last_sym = 0;
}

void KeymapSym::synchronize(uint16_t param1)
{
    this->key_flags = param1 & 0x07;
    // non sticky keys are forced to be UP
    this->keys_down[LEFT_SHIFT] = 0;
    this->keys_down[RIGHT_SHIFT] = 0;
    this->keys_down[LEFT_CTRL] = 0;
    this->keys_down[RIGHT_CTRL] = 0;
    this->keys_down[LEFT_ALT] = 0;
    this->keys_down[RIGHT_ALT] = 0;
}

// The TS_KEYBOARD_EVENT structure is a standard T.128 Keyboard Event (see [T128] section
// 8.18.2). RDP keyboard input is restricted to keyboard scancodes, unlike the code-point or virtual
// codes supported in T.128 (a scancode is an 8-bit value specifying a key location on the keyboard).
// The server accepts a scancode value and translates it into the correct character depending on the
// language locale and keyboard layout used in the session.

// keyboardFlags (2 bytes): A 16-bit, unsigned integer. The flags describing the keyboard event.

// +--------------------------+------------------------------------------------+
// | 0x0100 KBDFLAGS_EXTENDED | The keystroke message contains an extended     |
// |                          | scancode. For enhanced 101-key and 102-key     |
// |                          | keyboards, extended keys include "he right     |
// |                          | ALT and right CTRL keys on the main section    |
// |                          | of the keyboard; the INS, DEL, HOME, END,      |
// |                          | PAGE UP, PAGE DOWN and ARROW keys in the       |
// |                          | clusters to the left of the numeric keypad;    |
// |                          | and the Divide ("/") and ENTER keys in the     |
// |                          | numeric keypad.                                |
// +--------------------------+------------------------------------------------+
// | 0x4000 KBDFLAGS_DOWN     | Indicates that the key was down prior to this  |
// |                          | event.                                         |
// +--------------------------+------------------------------------------------+
// | 0x8000 KBDFLAGS_RELEASE  | The absence of this flag indicates a key-down  |
// |                          | event, while its presence indicates a          |
// |                          | key-release event.                             |
// +--------------------------+------------------------------------------------+

// keyCode (2 bytes): A 16-bit, unsigned integer. The scancode of the key which
// triggered the event.

void KeymapSym::event(const uint16_t keyboardFlags, const uint16_t keyCode)
{
    if (this->verbose){
        LOG(LOG_INFO, "KeymapSym::event(keyboardFlags=%04x, keyCode=%04x flags=%04x)", keyboardFlags, keyCode, unsigned(this->key_flags));
    }

    // The scancode and its extended nature are merged in a new variable (whose most significant bit indicates the extended nature)

    uint16_t keyboardFlags_pos = keyboardFlags;
    if (keyboardFlags_pos & KBDFLAGS_EXTENDED) {
        keyboardFlags_pos -=  KBDFLAGS_EXTENDED;
    }
    uint8_t extendedKeyCode = keyCode|((keyboardFlags_pos >> 1)&0x80);
    if ((keyboardFlags & KBDFLAGS_EXTENDED) && (keyCode == LEFT_ALT)) {
        extendedKeyCode = RIGHT_ALT;
    }
    // The state of that key is updated in the Keyboard status array (1=Make ; 0=Break)
    this->keys_down[extendedKeyCode] = !(keyboardFlags & KBDFLAGS_RELEASE);

    // if ctrl+alt+fin or ctrl+alt+suppr -> insert delete
    if (is_ctrl_pressed() && is_alt_pressed()
    && ((extendedKeyCode == 207)||(extendedKeyCode == 83))){
    //    Delete                           65535     0xffff
        extendedKeyCode = 211;
    }

    switch (extendedKeyCode){
    //----------------
    // Lock keys
    //----------------
        case 0x3A: // capslock
            if (this->keys_down[extendedKeyCode]){
                this->key_flags ^= CAPSLOCK;
            }
            break;
        case 0xC5: // numlock
            if (this->keys_down[extendedKeyCode]){
                this->key_flags ^= NUMLOCK;
            }
            break;
        case 0x46: // scrolllock
            if (this->keys_down[extendedKeyCode]){
                this->key_flags ^= SCROLLLOCK;
            }
            break;
    //----------------
    // Modifier keys
    //----------------
        // Not used (VNC server handles those keys state)

    //----------------
    // All other keys
    //----------------
        default: // all other codes
            // This table translates the RDP scancodes to X11 scancodes :
            //  - the fist block (0-127) simply applies the +8 Windows to X11 translation and forces some 0 values
            //  - the second block (128-255) give codes for the extended keys that have a meaningful one
            uint8_t map[256] =  {
                0x00, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, // 0x00 - 0x07
                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, // 0x08 - 0x0f
                0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, // 0x10 - 0x17
                0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, // 0x18 - 0x1f
                0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, // 0x20 - 0x27
                0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, // 0x28 - 0x2f
                0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, // 0x30 - 0x37
                0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, // 0x38 - 0x3f
                0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, // 0x40 - 0x47
                0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, // 0x48 - 0x4f
                0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, // 0x50 - 0x57
                0x60, 0x61, 0x62, 0x00, 0x00, 0x00, 0x66, 0x67, // 0x58 - 0x5f
                0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, // 0x60 - 0x67
                0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, // 0x68 - 0x6f
                0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, // 0x70 - 0x77
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x78 - 0x7f

                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80 - 0x87
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x88 - 0x8f
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x90 - 0x97
                0x00, 0x00, 0x00, 0x00, 0x6c, 0x6d, 0x00, 0x00, // 0x98 - 0x9f
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xa0 - 0xa7
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xa8 - 0xaf
                0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x6f, // 0xb0 - 0xb7
                0x71, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xb8 - 0xbf
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, // 0xc0 - 0xc7
                0x62, 0x63, 0x00, 0x64, 0x00, 0x66, 0x00, 0x67, // 0xc8 - 0xcf
                0x68, 0x69, 0x6a, 0x6b, 0x00, 0x00, 0x00, 0x00, // 0xd0 - 0xd7
                0x00, 0x00, 0x00, 0x73, 0x74, 0x75, 0x00, 0x00, // 0xd8 - 0xdf
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xe0 - 0xe7
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xe8 - 0xef
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xf0 - 0xf7
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // 0xf8 - 0xff
            } ;
            const KeyLayout_t * layout = &keylayout_WORK_noshift_sym;
            if (this->verbose) {
                LOG(LOG_INFO, "Use KEYLAYOUT WORK no shift");
            }

            // if ctrl+alt+fin or ctrl+alt+suppr -> insert delete
//                if (is_ctrl_pressed() && is_alt_pressed()
//                && ((extendedKeyCode == 207)||(extendedKeyCode == 83))){
                //    Delete                           65535     0xffff
//                    this->push_sym(0xFFFF);
//                    return;
//                }

            if ( ( (extendedKeyCode >= 0x47) && (extendedKeyCode <= 0x49) )
                || ( (extendedKeyCode >= 0x4b) && (extendedKeyCode <= 0x4d) )
                || ( (extendedKeyCode >= 0x4f) && (extendedKeyCode <= 0x53) )
                ){
                //------------------------------------------------------------------------
                // KEYPAD : Keypad keys whose meaning depends on Numlock are handled apart
                //------------------------------------------------------------------------
                if ((this->key_flags & NUMLOCK)) {
                    // if numlock is activated, keys are printable characters (logical SHIFT mode)
                    layout = &this->keylayout_WORK_shift_sym;
                    if (this->verbose) {
                        LOG(LOG_INFO, "Use KEYLAYOUT WORK shift (1)");
                    }
                    // Translate the scancode to an unicode char
                    uint8_t sym = map[extendedKeyCode];
                    uint32_t ksym = (*layout)[sym];
                    if (this->verbose) {
                        LOG(LOG_INFO, "extendedKeyCode=0x%X sym=0x%X ksym=0x%X", extendedKeyCode, sym, ksym);
                    }
                    this->push_sym(ksym);
                } // if numlock ON
                else {
                    // if numlock is not activated, keys are NOT printable characters (logical NO SHIFT mode)
                    switch (extendedKeyCode){
                        /* kEYPAD LEFT ARROW */
                        case 0x4b:
                                this->push_sym(0xFF51);
                            break;
                        /* kEYPAD UP ARROW */
                        case 0x48:
                            this->push_sym(0xFF52);
                            break;
                        /* kEYPAD RIGHT ARROW */
                        case 0x4d:
                            this->push_sym(0xFF53);
                            break;
                        /* kEYPAD DOWN ARROW */
                        case 0x50:
                            this->push_sym(0xFF54);
                            break;
                        /* kEYPAD HOME */
                        case 0x47:
                            this->push_sym(0xFF50);
                            break;
                        /* kEYPAD PGUP */
                        case 0x49:
                            this->push_sym(0xFF55);
                            break;
                        /* kEYPAD PGDOWN */
                        case 0x51:
                            this->push_sym(0xFF56);
                            break;
                        /* kEYPAD END */
                        case 0x4F:
                            this->push_sym(0xFF57);
                            break;
                        /* kEYPAD DELETE */
                        case 0x53:
                            this->push_sym(0xFFFF);
                            break;
                        default:
                            break;
                    }

                } // if numlock OFF
            }
            else {
                //--------------------
                // NOT KEYPAD Specific
                //--------------------
                if (this->verbose){
                    LOG(LOG_INFO, "Key not from keypad: %#x", extendedKeyCode);
                }
                // SET the LAYOUT to use (depending on current keyboard state)
                //----------------------------------------
                // if left ctrl and left alt are pressed, vnc server will convert key combination itself.
                //if ( (this->is_ctrl_pressed() && this->is_alt_pressed()) ||
                if (this->is_right_alt_pressed()) {
                    layout = &this->keylayout_WORK_altgr_sym;
                    if (this->verbose) {
                        LOG(LOG_INFO, "Use KEYLAYOUT WORK Altgr");
                    }
                }
                else if (this->is_shift_pressed() && this->is_caps_locked()){
                    layout = &this->keylayout_WORK_shiftcapslock_sym;
                    if (this->verbose) {
                        LOG(LOG_INFO, "Use KEYLAYOUT WORK shiftcapslock");
                    }
                }
                else if (this->is_shift_pressed()){
                    layout = &this->keylayout_WORK_shift_sym;
                    if (this->verbose) {
                        LOG(LOG_INFO, "Use KEYLAYOUT WORK shift (2)");
                    }
                }
                else if (this->is_caps_locked()) {
                    layout = &this->keylayout_WORK_capslock_sym;
                    if (this->verbose) {
                        LOG(LOG_INFO, "Use KEYLAYOUT WORK capslock");
                    }
                }
                // Translate the scancode to a KeySym
                //----------------------------------------
                uint8_t sym = map[extendedKeyCode];
                uint32_t ksym = (*layout)[sym];
                if (this->verbose){
                    LOG(LOG_INFO, "extendedKeyCode=0x%X sym=0x%X ksym=0x%X", extendedKeyCode, sym, ksym);
                }
                if ((ksym == 0xFE52 ) || (ksym == 0xFE57) || (ksym == 0x60) || (ksym == 0x7E)) {
                    //-------------------------------------------------
                    // ksym is NOT in Printable unicode character range
                    //-------------------------------------------------
                    // That is, A dead key (0xFE52 (^), 0xFE57 ("), 0x60 (`), 0x7E (~) )
                    // The flag is set accordingly
                    switch (extendedKeyCode){
                        case 0x1A:
                            this->is_shift_pressed() ? this->dead_key = DEADKEY_UML : this->dead_key = DEADKEY_CIRC;
                            break;
                        case 0x08:
                            this->dead_key = DEADKEY_GRAVE;
                            break;
                        case 0x03:
                            this->dead_key = DEADKEY_TILDE;
                            break;
                        default:
                            break;
                    } // Switch extendedKeyCode
                }
                else {
                    //-------------------------------------------------
                    // ksym is in Printable character range.
                    //-------------------------------------------------
                    if (this->dead_key != DEADKEY_NONE){
                        switch (dead_key){
                            case DEADKEY_CIRC:
                                switch (ksym){
                                    case 'a':
                                        this->push_sym(0xE2); // unicode for â (acirc)
                                        break;
                                    case 'A':
                                        this->push_sym(0xC2); // unicode for Â (Acirc)
                                        break;
                                    case 'e':
                                        this->push_sym(0xEA); // unicode for ê (ecirc)
                                        break;
                                    case 'E':
                                        this->push_sym(0xCA); // unicode for Ê (Ecirc)
                                        break;
                                    case 'i':
                                        this->push_sym(0xEE); // unicode for î (icirc)
                                        break;
                                    case 'I':
                                        this->push_sym(0xCE); // unicode for Î (Icirc)
                                        break;
                                    case 'o':
                                        this->push_sym(0xF4); // unicode for ô (ocirc)
                                        break;
                                    case 'O':
                                        this->push_sym(0xD4); // unicode for Ô (Ocirc)
                                        break;
                                    case 'u':
                                        this->push_sym(0xFB); // unicode for û (ucirc)
                                        break;
                                    case 'U':
                                        this->push_sym(0xDB); // unicode for Û (Ucirc)
                                        break;
                                    case ' ':
                                        this->push_sym(0x5E); // unicode for ^ (caret)
                                        break;
                                    default:
                                        this->push_sym(ksym); // unmodified unicode
                                        break;
                                }
                                break;

                            case DEADKEY_UML:
                                switch (ksym){
                                    case 'a':
                                        this->push_sym(0xE4); // unicode for ä (auml)
                                        break;
                                    case 'A':
                                        this->push_sym(0xC4); // unicode for Ä (Auml)
                                        break;
                                    case 'e':
                                        this->push_sym(0xEB); // unicode for ë (euml)
                                        break;
                                    case 'E':
                                        this->push_sym(0xCB); // unicode for Ë (Euml)
                                        break;
                                    case 'i':
                                        this->push_sym(0xEF); // unicode for ï (iuml)
                                        break;
                                    case 'I':
                                        this->push_sym(0xCF); // unicode for Ï (Iuml)
                                        break;
                                    case 'o':
                                        this->push_sym(0xF6); // unicode for ö (ouml)
                                        break;
                                    case 'O':
                                        this->push_sym(0xD6); // unicode for Ö (Ouml)
                                        break;
                                    case 'u':
                                        this->push_sym(0xFC); // unicode for ü (uuml)
                                        break;
                                    case 'U':
                                        this->push_sym(0xDC); // unicode for Ü (Uuml)
                                        break;
                                    case ' ':
                                        this->push_sym(0xA8); // unicode for " (umlaut)
                                        break;
                                    default:
                                        this->push_sym(ksym); // unmodified unicode
                                        break;
                                }
                                break;
                            case DEADKEY_GRAVE:
                                switch (ksym){
                                    case 'a':
                                        this->push_sym(0xE0); // unicode for à (agrave)
                                        break;
                                    case 'A':
                                        this->push_sym(0xC0); // unicode for À (Agrave)
                                        break;
                                    case 'e':
                                        this->push_sym(0xE8); // unicode for è (egrave)
                                        break;
                                    case 'E':
                                        this->push_sym(0xC8); // unicode for È (Egrave)
                                        break;
                                    case 'i':
                                        this->push_sym(0xEC); // unicode for ì (igrave)
                                        break;
                                    case 'I':
                                        this->push_sym(0xCC); // unicode for Ì (Igrave)
                                        break;
                                    case 'o':
                                        this->push_sym(0xF2); // unicode for ò (ograve)
                                        break;
                                    case 'O':
                                        this->push_sym(0xD2); // unicode for Ò (Ograve)
                                        break;
                                    case 'u':
                                        this->push_sym(0xF9); // unicode for ù (ugrave)
                                        break;
                                    case 'U':
                                        this->push_sym(0xD9); // unicode for Ù (Ugrave)
                                        break;
                                    case ' ':
                                        this->push_sym(0x60); // unicode for ` (backcote)
                                        break;
                                    default:
                                        this->push_sym(ksym); // unmodified unicode
                                        break;
                                }
                                break;
                            case DEADKEY_TILDE:
                                switch (ksym){
                                    case 'n':
                                        this->push_sym(0xF1); // unicode for ~n (ntilde)
                                        break;
                                    case 'N':
                                        this->push_sym(0xD1); // unicode for ~N (Ntilde)
                                        break;
                                    case ' ':
                                        this->push_sym(0x7E); // unicode for ~ (tilde)
                                        break;
                                    default:
                                        this->push_sym(ksym); // unmodified unicode
                                        break;
                                }
                                break;
                            default:
                                this->push_sym(ksym); // unmodified unicode
                                break;
                        } // Switch DEAD_KEY
                        // if event is a Make (mandatory because a BREAK on a modifier key would also reset this flag)
                        if (this->keys_down[extendedKeyCode])
                        {
                            this->dead_key = DEADKEY_NONE;
                        }
                    } // Is a dead Key
                    else {
                        // If previous key wasn't a dead key, simply push

                        // if key if ALT_GR
                        if (ksym == 0xFE03 && (keyboardFlags & KBDFLAGS_EXTENDED)) {
                            ksym = 0xffea;
                        }

                        this->push_sym(ksym);
                    }
                } // END if PRINTABLE / else
            } // END if KEYPAD specific / else
        break;
    } // END SWITCH : ExtendedKeyCode

} // END FUNCT : event


// Push only sym
void KeymapSym::push_sym(uint32_t sym)
{
    if (this->verbose & 2){
        LOG(LOG_INFO, "KeymapSym::push_sym(sym=%08x) nbuf_sym=%u", sym, this->nbuf_sym);
    }
    if (this->nbuf_sym < SIZE_KEYBUF_SYM){
        this->buffer_sym[this->ibuf_sym] = sym;
        this->ibuf_sym++;
        if (this->ibuf_sym >= SIZE_KEYBUF_SYM){
            this->ibuf_sym = 0;
        }
        this->nbuf_sym++;
    }
}

uint32_t KeymapSym::get_sym()
{
    if (this->verbose & 2){
        LOG(LOG_INFO, "KeymapSym::get_sym() nbuf_sym=%u", this->nbuf_sym);
    }
    if (this->nbuf_sym > 0){
        uint32_t res = this->buffer_sym[(SIZE_KEYBUF_SYM + this->ibuf_sym - this->nbuf_sym) % SIZE_KEYBUF_SYM];

        if (this->nbuf_sym > 0){
            this->nbuf_sym--;
        }
        if (this->verbose & 2){
            LOG(LOG_INFO, "KeymapSym::get_sym() nbuf_sym=%u -> %08x", this->nbuf_sym, res);
        }
        return res;
    }
    return 0;
}

uint32_t KeymapSym::nb_sym_available() const
{
    return this->nbuf_sym;
}

bool KeymapSym::is_caps_locked() const
{
    return this->key_flags & CAPSLOCK;
}

bool KeymapSym::is_scroll_locked() const
{
    return this->key_flags & SCROLLLOCK;
}

bool KeymapSym::is_num_locked() const
{
    return this->key_flags & NUMLOCK;
}

void KeymapSym::toggle_num_lock(bool on)
{
    if (((this->key_flags & NUMLOCK) == NUMLOCK) != on) {
        this->key_flags ^= NUMLOCK;
    }
}

bool KeymapSym::is_left_shift_pressed() const
{
    return this->keys_down[LEFT_SHIFT];
}

bool KeymapSym::is_right_shift_pressed() const
{
    return this->keys_down[RIGHT_SHIFT];
}

bool KeymapSym::is_shift_pressed() const
{
    return this->is_left_shift_pressed() || this->is_right_shift_pressed();
}

bool KeymapSym::is_left_ctrl_pressed() const
{
    return this->keys_down[LEFT_CTRL];
}

bool KeymapSym::is_right_ctrl_pressed() const
{
    return this->keys_down[RIGHT_CTRL];
}

bool KeymapSym::is_ctrl_pressed() const
{
    return is_right_ctrl_pressed() || is_left_ctrl_pressed();
}

bool KeymapSym::is_left_alt_pressed() const
{
    return this->keys_down[LEFT_ALT];
}

bool KeymapSym::is_right_alt_pressed() const // altgr
{
    return this->keys_down[RIGHT_ALT];
}

bool KeymapSym::is_alt_pressed() const
{
    return is_right_alt_pressed() || is_left_alt_pressed();
}


void KeymapSym::init_layout_sym(int keyb)
{
    // %s/^[^"]*"[^"]*"[^"]*"\([^:]*\):.*$/\1/

    // DEFAULT KEYMAP
    const KeyLayout_t DEFAULT_noshift_sym = {
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0xff1b,  0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
            0x37,    0x38,    0x39,    0x30,    0xdf,    0xfe51,  0xff08,  0xff09,
            0x71,    0x77,    0x65,    0x72,    0x74,    0x7a,    0x75,    0x69,
            0x6f,    0x70,    0xfc,    0x2b,    0xff0d,  0xffe3,  0x61,    0x73,
            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0xf6,
            0xe4,    0xfe52,  0xffe1,  0x23,    0x79,    0x78,    0x63,    0x76,
            0x62,    0x6e,    0x6d,    0x2c,    0x2e,    0x2d,    0xffe2,  0xffaa,
            0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
            0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x3c,    0xffc8,
            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,
    } ;

    const KeyLayout_t DEFAULT_shift_sym = {
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0xff1b,  0x21,    0x22,    0xa7,    0x24,    0x25,    0x26,
            0x2f,    0x28,    0x29,    0x3d,    0x3f,    0xfe50,  0xff08,  0xfe20,
            0x51,    0x57,    0x45,    0x52,    0x54,    0x5a,    0x55,    0x49,
            0x4f,    0x50,    0xdc,    0x2a,    0xff0d,  0xffe3,  0x41,    0x53,
            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0xd6,
            0xc4,    0xb0,    0xffe1,  0x27,    0x59,    0x58,    0x43,    0x56,
            0x42,    0x4e,    0x4d,    0x3b,    0x3a,    0x5f,    0xffe2,  0xffaa,
            0xffe7,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
            0xffb2,  0xffb3,  0xffb0,  0xffac,  0x0,     0xff7e,  0x3e,    0xffc8,
            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0xfe03,  0xffe9,  0xffbd,  0xffeb,
    } ;

    const KeyLayout_t DEFAULT_altgr_sym = {
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0xff1b,  0xb9,    0xb2,    0xb3,    0xbc,    0xbd,    0xac,
            0x7b,    0x5b,    0x5d,    0x7d,    0x5c,    0xfe5b,  0xff08,  0xff09,
            0x40,    0x1b3,   0x20ac,  0xb6,    0x3bc,   0x8fb,   0x8fe,   0x8fd,
            0xf8,    0xfe,    0xfe57,  0xfe53,  0xff0d,  0xffe3,  0xe6,    0xdf,
            0xf0,    0x1f0,   0x3bf,   0x2b1,   0x6a,    0x3a2,   0x1b3,   0xfe59,
            0xfe52,  0xac,    0xffe1,  0xfe50,  0xab,    0xbb,    0xa2,    0xad2,
            0xad3,   0x6e,    0xb5,    0x8a3,   0xb7,    0xfe60,  0xffe2,  0xffaa,
            0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
            0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x7c,    0xffc8,
            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0x0,
            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,
    } ;

    const KeyLayout_t DEFAULT_capslock_sym = {
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0xff1b,  0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
            0x37,    0x38,    0x39,    0x30,    0xdf,    0xfe51,  0xff08,  0xff09,
            0x51,    0x57,    0x45,    0x52,    0x54,    0x5a,    0x55,    0x49,
            0x4f,    0x50,    0xdc,    0x2b,    0xff0d,  0xffe3,  0x41,    0x53,
            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0xd6,
            0xc4,    0xfe52,  0xffe1,  0x23,    0x59,    0x58,    0x43,    0x56,
            0x42,    0x4e,    0x4d,    0x2c,    0x2e,    0x2d,    0xffe2,  0xffaa,
            0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
            0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x3c,    0xffc8,
            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,
    } ;

    const KeyLayout_t DEFAULT_shiftcapslock_sym = {
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
    } ;

    // Intialize the WORK tables
    for(int i=0 ; i < 128 ; i++) {
        keylayout_WORK_noshift_sym[i] = DEFAULT_noshift_sym[i] ;
        keylayout_WORK_shift_sym[i] = DEFAULT_shift_sym[i] ;
        keylayout_WORK_altgr_sym[i] = DEFAULT_altgr_sym[i] ;
        keylayout_WORK_capslock_sym[i] = DEFAULT_capslock_sym[i] ;
        keylayout_WORK_shiftcapslock_sym[i] = DEFAULT_shiftcapslock_sym[i] ;
    }

    switch (keyb){

        case 0x0407: // GERMAN
        {
                const KeyLayout_t x0407_noshift_sym = {
                        0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,    // 00 - 07
                        0x0,     0xff1b,  0x31,    0x32,    0x33,    0x34,    0x35,    0x36,   // 08 - 0F
                        0x37,    0x38,    0x39,    0x30,    0xdf,    0xfe51,  0xff08,  0xff09, // 10 - 17
                        0x71,    0x77,    0x65,    0x72,    0x74,    0x7a,    0x75,    0x69,   // 18 - 1F
                        0x6f,    0x70,    0xfc,    0x2b,    0xff0d,  0xffe3,  0x61,    0x73,   // 20 - 27
                        0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0xf6,   // 28 - 2F
                        0xe4,    0xfe52,  0xffe1,  0x23,    0x79,    0x78,    0x63,    0x76,   // 30 - 37
                        0x62,    0x6e,    0x6d,    0x2c,    0x2e,    0x2d,    0xffe2,  0xffaa, // 38 - 3F
                        0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2, // 40 - 47
                        0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95, // 48 - 4F
                        0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c, // 50 - 57
                        0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x3c,    0xffc8, // 58 - 5F
                        0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57, // 60 - 67
                        0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61, // 68 - 6F
                        0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,    // 70 - 77
                        0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,    // 78 - 7F
                };
                const KeyLayout_t x0407_shift_sym = {
                        0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,    // 00 - 07
                        0x0,     0xff1b,  0x21,    0x22,    0xa7,    0x24,    0x25,    0x26,   // 08 - 0F
                        0x2f,    0x28,    0x29,    0x3d,    0x3f,    0xfe50,  0xff08,  0xfe20, // 10 - 17
                        0x51,    0x57,    0x45,    0x52,    0x54,    0x5a,    0x55,    0x49,   // 18 - 1F
                        0x4f,    0x50,    0xdc,    0x2a,    0xff0d,  0xffe3,  0x41,    0x53,   // 20 - 27
                        0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0xd6,   // 28 - 2F
                        0xc4,    0xb0,    0xffe1,  0x27,    0x59,    0x58,    0x43,    0x56,   // 30 - 37
                        0x42,    0x4e,    0x4d,    0x3b,    0x3a,    0x5f,    0xffe2,  0xffaa, // 38 - 3F
                        0xffe7,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2, // 40 - 47
                        0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7, // 48 - 4F
                        0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1, // 50 - 57
                        0xffb2,  0xffb3,  0xffb0,  0xffac,  0x0,     0xff7e,  0x3e,    0xffc8, // 58 - 5F
                        0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57, // 60 - 67
                        0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61, // 68 - 6F
                        0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,    // 70 - 77
                        0x0,     0x0,     0x0,     0x0,     0xfe03,  0xffe9,  0xffbd,  0xffeb, // 78 - 7F
                };
                const KeyLayout_t x0407_altgr_sym = {
                        0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,    // 00 - 07
                        0x0,     0xff1b,  0xb9,    0xb2,    0xb3,    0xbc,    0xbd,    0xac,   // 08 - 0F
                        0x7b,    0x5b,    0x5d,    0x7d,    0x5c,    0xfe5b,  0xff08,  0xff09, // 10 - 17
                        0x40,    0x1b3,   0x20ac,  0xb6,    0x3bc,   0x8fb,   0x8fe,   0x8fd,  // 18 - 1F
                        0xf8,    0xfe,    0xfe57,  0xfe53,  0xff0d,  0xffe3,  0xe6,    0xdf,   // 20 - 27
                        0xf0,    0x1f0,   0x3bf,   0x2b1,   0x6a,    0x3a2,   0x1b3,   0xfe59, // 28 - 2F
                        0xfe52,  0xac,    0xffe1,  0xfe50,  0xab,    0xbb,    0xa2,    0xad2,  // 30 - 37
                        0xad3,   0x6e,    0xb5,    0x8a3,   0xb7,    0xfe60,  0xffe2,  0xffaa, // 38 - 3F
                        0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2, // 40 - 47
                        0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95, // 48 - 4F
                        0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c, // 50 - 57
                        0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x7c,    0xffc8, // 58 - 5F
                        0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57, // 60 - 67
                        0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0x0,    // 68 - 6F
                        0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,    // 70 - 77
                        0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,     0xffbd,  0x0,    // 78 - 7F
                };
                const KeyLayout_t x0407_capslock_sym = {
                        0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,    // 00 - 07
                        0x0,     0xff1b,  0x31,    0x32,    0x33,    0x34,    0x35,    0x36,   // 08 - 0F
                        0x37,    0x38,    0x39,    0x30,    0xdf,    0xfe51,  0xff08,  0xff09, // 10 - 17
                        0x51,    0x57,    0x45,    0x52,    0x54,    0x5a,    0x55,    0x49,   // 18 - 1F
                        0x4f,    0x50,    0xdc,    0x2b,    0xff0d,  0xffe3,  0x41,    0x53,   // 20 - 27
                        0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0xd6,   // 28 - 2F
                        0xc4,    0xfe52,  0xffe1,  0x23,    0x59,    0x58,    0x43,    0x56,   // 30 - 37
                        0x42,    0x4e,    0x4d,    0x2c,    0x2e,    0x2d,    0xffe2,  0xffaa, // 38 - 3F
                        0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2, // 40 - 47
                        0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95, // 48 - 4F
                        0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c, // 50 - 57
                        0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x3c,    0xffc8, // 58 - 5F
                        0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57, // 60 - 67
                        0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61, // 68 - 6F
                        0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,    // 70 - 77
                        0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,    // 78 - 7F
                };
                const KeyLayout_t x0407_shiftcapslock_sym = {
                        0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,    // 00 - 07
                        0x0,     0xff1b,  0x21,    0x22,    0xa7,    0x24,    0x25,    0x26,   // 08 - 0F
                        0x2f,    0x28,    0x29,    0x3d,    0x3f,    0xfe50,  0xff08,  0xfe20, // 10 - 17
                        0x71,    0x77,    0x65,    0x72,    0x74,    0x7a,    0x75,    0x69,   // 18 - 1F
                        0x6f,    0x70,    0xfc,    0x2a,    0xff0d,  0xffe3,  0x61,    0x73,   // 20 - 27
                        0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0xf6,   // 28 - 2F
                        0xe4,    0xb0,    0xffe1,  0x27,    0x79,    0x78,    0x63,    0x76,   // 30 - 37
                        0x62,    0x6e,    0x6d,    0x3b,    0x3a,    0x5f,    0xffe2,  0xffaa, // 38 - 3F
                        0xffe7,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2, // 40 - 47
                        0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7, // 48 - 4F
                        0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1, // 50 - 57
                        0xffb2,  0xffb3,  0xffb0,  0xffac,  0x0,     0xff7e,  0x3e,    0xffc8, // 58 - 5F
                        0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57, // 60 - 67
                        0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61, // 68 - 6F
                        0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,    // 70 - 77
                        0x0,     0x0,     0x0,     0x0,     0xfe03,  0xffe9,  0xffbd,  0xffeb, // 78 - 7F
                };

                for(size_t i = 0 ; i < 128 ; i++) {
                    if (x0407_noshift_sym[i]){
                        keylayout_WORK_noshift_sym[i] = x0407_noshift_sym[i] ;
                    }
                    if (x0407_shift_sym[i]){
                        keylayout_WORK_shift_sym[i] = x0407_shift_sym[i] ;
                    }
                    if (x0407_altgr_sym[i]){
                        keylayout_WORK_altgr_sym[i] = x0407_altgr_sym[i] ;
                    }
                    if (x0407_capslock_sym[i]){
                        keylayout_WORK_capslock_sym[i] = x0407_capslock_sym[i] ;
                    }
                    if (x0407_shiftcapslock_sym[i]){
                        keylayout_WORK_shiftcapslock_sym[i] = x0407_shiftcapslock_sym[i] ;
                    }
                }
            }
            break;

        case 0x0409: // United States
        {
                const KeyLayout_t x0409_noshift_sym = {
                        0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                        0x0,     0xff1b,  0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                        0x37,    0x38,    0x39,    0x30,    0x2d,    0x3d,    0xff08,  0xff09,
                        0x71,    0x77,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,
                        0x6f,    0x70,    0x5b,    0x5d,    0xff0d,  0xffe3,  0x61,    0x73,
                        0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0x3b,
                        0x27,    0x60,    0xffe1,  0x5c,    0x7a,    0x78,    0x63,    0x76,
                        0x62,    0x6e,    0x6d,    0x2c,    0x2e,    0x2f,    0xffe2,  0xffaa,
                        0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                        0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                        0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                        0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x3c,    0xffc8,
                        0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                        0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                        0xffaf,  0xffea,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                        0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,

                };
                const KeyLayout_t x0409_shift_sym = {
                        0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                        0x0,     0xff1b,  0x21,    0x40,    0x23,    0x24,    0x25,    0x5e,
                        0x26,    0x2a,    0x28,    0x29,    0x5f,    0x2b,    0xff08,  0xfe20,
                        0x51,    0x57,    0x45,    0x52,    0x54,    0x59,    0x55,    0x49,
                        0x4f,    0x50,    0x7b,    0x7d,    0xff0d,  0xffe3,  0x41,    0x53,
                        0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0x3a,
                        0x22,    0x7e,    0xffe1,  0x7c,    0x5a,    0x58,    0x43,    0x56,
                        0x42,    0x4e,    0x4d,    0x3c,    0x3e,    0x3f,    0xffe2,  0xffaa,
                        0xffe7,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                        0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                        0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                        0xffb2,  0xffb3,  0xffb0,  0xffae,  0x0,     0xff7e,  0x3e,    0xffc8,
                        0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                        0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                        0xffaf,  0xffe8,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                        0x0,     0x0,     0x0,     0x0,     0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };
                const KeyLayout_t x0409_altgr_sym = {
                        0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                        0x0,     0xff1b,  0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                        0x37,    0x38,    0x39,    0x30,    0x2d,    0x3d,  0xff08,  0xff09,
                        0x71,    0x77,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,
                        0x6f,    0x70,    0x5b,    0x5d,  0xff0d,  0xffe3,    0x61,    0x73,
                        0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0x3b,
                        0x27,    0x60,    0xffe1,  0x5c,    0x7a,    0x78,    0x63,    0x76,
                        0x62,    0x6e,    0x6d,    0x2c,    0x2e,    0x2f,  0xffe2,  0xffaa,
                        0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                        0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                        0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                        0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x7c,  0xffc8,
                        0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                        0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,     0x0,
                        0xffaf,  0xffea,  0x0,     0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                        0x0,     0x0,     0x0,     0x0,    0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x0409_capslock_sym = {
                        0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                        0x0,     0xff1b,  0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                        0x37,    0x38,    0x39,    0x30,    0x2d,    0x3d,    0xff08,  0xff09,
                        0x51,    0x57,    0x45,    0x52,    0x54,    0x59,    0x55,    0x49,
                        0x4f,    0x50,    0x5b,    0x5d,    0xff0d,  0xffe3,  0x41,    0x53,
                        0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0x3b,
                        0x27,    0x60,    0xffe1,  0x5c,    0x5a,    0x58,    0x43,    0x56,
                        0x42,    0x4e,    0x4d,    0x2c,    0x2e,    0x2f,    0xffe2,  0xffaa,
                        0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                        0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                        0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                        0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x3c,    0xffc8,
                        0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                        0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                        0xffaf,  0xffea,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                        0x0,     0x0,     0x0,     0x0,     0xfe03,   0x0,    0xffbd,  0x0,
                };
                const KeyLayout_t x0409_shiftcapslock_sym = {
                        0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                        0x0,     0xff1b,  0x21,    0x40,    0x23,    0x24,    0x25,    0x5e,
                        0x26,    0x2a,    0x28,    0x29,    0x5f,    0x2b,    0xff08,  0xfe20,
                        0x71,    0x77,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,
                        0x6f,    0x70,    0x7b,    0x7d,    0xff0d,  0xffe3,  0x61,    0x73,
                        0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0x3a,
                        0x22,    0x7e,    0xffe1,  0x7c,    0x7a,    0x78,    0x63,    0x76,
                        0x62,    0x6e,    0x6d,    0x3c,    0x3e,    0x3f,    0xffe2,  0xffaa,
                        0xffe7,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                        0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                        0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                        0xffb2,  0xffb3,  0xffb0,  0xffae,  0x0,     0xff7e,  0x3e,    0xffc8,
                        0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                        0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                        0xffaf,  0xffe8,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                        0x0,     0x0,     0x0,     0x0,     0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };

                for(size_t i = 0 ; i < 128 ; i++) {
                    if (x0409_noshift_sym[i]){
                        keylayout_WORK_noshift_sym[i] = x0409_noshift_sym[i] ;
                    }
                    if (x0409_shift_sym[i]){
                        keylayout_WORK_shift_sym[i] = x0409_shift_sym[i] ;
                    }
                    if (x0409_altgr_sym[i]){
                        keylayout_WORK_altgr_sym[i] = x0409_altgr_sym[i] ;
                    }
                    if (x0409_capslock_sym[i]){
                        keylayout_WORK_capslock_sym[i] = x0409_capslock_sym[i] ;
                    }
                    if (x0409_shiftcapslock_sym[i]){
                        keylayout_WORK_shiftcapslock_sym[i] = x0409_shiftcapslock_sym[i] ;
                    }
                }
            }
            break;

        case 0x040c: // French
        {
// +----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +----+----+-------+
// | 01 |  | 3B | 3C | 3D | 3E |  | 3F | 40 | 41 | 42 |  | 43 | 44 | 57 | 58 |  | 37 | 46 | 1D+45 |
// +----+  +----+----+----+----+  +----+----+----+----+  +----+----+----+----+  +----+----+-------+
//                                     ***  keycodes suffixed by 'x' are extended ***
// +----+----+----+----+----+----+----+----+----+----+----+----+----+--------+  +----+----+----+  +--------------------+
// | 29 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 0A | 0B | 0C | 0D |   0E   |  | 52x| 47x| 49x|  | 45 | 35x| 37 | 4A  |
// +-------------------------------------------------------------------------+  +----+----+----+  +----+----+----+-----+
// |  0F  | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 1A | 1B |      |  | 53x| 4Fx| 51x|  | 47 | 48 | 49 |     |
// +------------------------------------------------------------------+  1C  |  +----+----+----+  +----+----+----| 4E  |
// |  3A   | 1E | 1F | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 2B |     |                    | 4B | 4C | 4D |     |
// +-------------------------------------------------------------------------+       +----+       +----+----+----+-----+
// |  2A | 56 | 2C | 2D | 2E | 2F | 30 | 31 | 32 | 33 | 34 | 35 |     36     |       | 48x|       | 4F | 50 | 51 |     |
// +-------------------------------------------------------------------------+  +----+----+----+  +---------+----| 1Cx |
// |  1D  |  5Bx | 38 |           39           |  38x  |  5Cx |  5Dx |  1Dx  |  | 4Bx| 50x| 4Dx|  |    52   | 53 |     |
// +------+------+----+------------------------+-------+------+------+-------+  +----+----+----+  +---------+----+-----+

                const KeyLayout_t x040c_noshift_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,    // 00 - 07
                            0x0,     0xff1b,  0x26,    0xe9,    0x22,    0x27,    0x28,    0x2d,   // 08 - 0F
                            0xe8,    0x5f,    0xe7,    0xe0,    0x29,    0x3d,    0xff08,  0xff09, // 10 - 17
                            0x61,    0x7a,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,   // 18 - 1F
                            0x6f,    0x70,    0xfe52,  0x24,    0xff0d,  0xffe3,  0x71,    0x73,   // 20 - 27
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0x6d,   // 28 - 2F
                            0xf9,    0xb2,    0xffe1,  0x2a,    0x77,    0x78,    0x63,    0x76,   // 30 - 37
                            0x62,    0x6e,    0x2c,    0x3b,    0x3a,    0x21,    0xffe2,  0xffaa, // 38 - 3F
                            0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2, // 40 - 47
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95, // 48 - 4F
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c, // 50 - 57
                            0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x3c,    0xffc8, // 58 - 5F
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57, // 60 - 67
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61, // 68 - 6F
                            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,    // 70 - 77
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,    // 78 - 7F
                };
                const KeyLayout_t x040c_shift_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0, // 00 - 07
                            0x0,     0xff1b,  0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0xb0,    0x2b,    0xff08,  0xfe20,
                            0x41,    0x5a,    0x45,    0x52,    0x54,    0x59,    0x55,    0x49,
                            0x4f,    0x50,    0xfe57,  0xa3,    0xff0d,  0xffe3,  0x51,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0x4d,
                            0x25,    0x7e,    0xffe1,  0xb5,    0x57,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x3f,    0x2e,    0x2f,    0xa7,    0xffe2,  0xffaa,
                            0xffe7,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffae,  0x0,     0xff7e,  0x3e,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };
                const KeyLayout_t x040c_capslock_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                            0x0,     0xff1b,  0x26,    0xc9,    0x22,    0x27,    0x28,    0x2d,
                            0xc8,    0x5f,    0xc7,    0xc0,    0x29,    0x3d,    0xff08,  0xff09,
                            0x41,    0x5a,    0x45,    0x52,    0x54,    0x59,    0x55,    0x49,
                            0x4f,    0x50,    0xfe52,  0x24,    0xff0d,  0xffe3,  0x51,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0x4d,
                            0xd9,    0xb2,    0xffe1,  0x2a,    0x57,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x2c,    0x3b,    0x3a,    0x21,    0xffe2,  0xffaa,
                            0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x3c,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,
                };
                const KeyLayout_t x040c_altgr_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                            0x0,     0xff42,  0xb9,    0x7e,    0x23,    0x7b,    0x5b,    0x7c,
                            0x60,    0x5c,    0x5e,    0x40,    0x5d,    0x7d,    0xff08,  0xff09,
                            0xe6,    0xab,    0x20ac,  0xb6,    0x3bc,   0x8fb,   0x8fe,   0x8fd,
                            0xf8,    0xfe,    0xfe57,  0xa4,    0xff0d,  0xffe3,  0x40,    0xdf,
                            0xf0,    0x1f0,   0x3bf,   0x2b1,   0x6a,    0x3a2,   0x1b3,   0xb5,
                            0xfe52,  0xac,    0xffe1,  0xfe50,  0x1b3,   0xbb,    0xa2,    0xad2,
                            0xad3,   0x6e,    0xfe51,  0x8a3,   0xb7,    0xfe60,  0xffe2,  0xffaa,
                            0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x7c,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0x0,
                            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,
                };
                const KeyLayout_t x040c_shiftcapslock_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                            0x0,     0xff1b,  0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0xb0,    0x2b,    0xff08,  0xfe20,
                            0x61,    0x7a,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,
                            0x6f,    0x70,    0xfe57,  0xa3,    0xff0d,  0xffe3,  0x71,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0x6d,
                            0x25,    0x7e,    0xffe1,  0x39c,   0x77,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x3f,    0x2e,    0x2f,    0xa7,    0xffe2,  0xffaa,
                            0xffe7,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffae,  0x0,     0xff7e,  0x3e,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };

                for(size_t i = 0 ; i < 128 ; i++) {
                    if (x040c_noshift_sym[i]){
                        keylayout_WORK_noshift_sym[i] = x040c_noshift_sym[i] ;
                    }
                    if (x040c_shift_sym[i]){
                        keylayout_WORK_shift_sym[i] = x040c_shift_sym[i] ;
                    }
                    if (x040c_altgr_sym[i]){
                        keylayout_WORK_altgr_sym[i] = x040c_altgr_sym[i] ;
                    }
                    if (x040c_capslock_sym[i]){
                        keylayout_WORK_capslock_sym[i] = x040c_capslock_sym[i] ;
                    }
                    if (x040c_shiftcapslock_sym[i]){
                        keylayout_WORK_shiftcapslock_sym[i] = x040c_shiftcapslock_sym[i] ;
                    }
                }
            }
            break;

        case 0x0410: // Italian
        {
                const KeyLayout_t x0410_noshift_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                            0x0,     0xff1b,  0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0x27,    0xec,    0xff08,  0xff09,
                            0x71,    0x77,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,
                            0x6f,    0x70,    0xe8,    0x2b,    0xff0d,  0xffe3,  0x61,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0xf2,
                            0xe0,    0x5c,    0xffe1,  0xf9,    0x7a,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x6d,    0x2c,    0x2e,    0x2d,    0xffe2,  0xffaa,
                            0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x3c,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,
                };
                const KeyLayout_t x0410_shift_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                            0x0,     0xff1b,  0x21,    0x22,    0xa3,    0x24,    0x25,    0x26,
                            0x2f,    0x28,    0x29,    0x3d,    0x3f,    0x5e,    0xff08,  0xfe20,
                            0x51,    0x57,    0x45,    0x52,    0x54,    0x59,    0x55,    0x49,
                            0x4f,    0x50,    0xe9,    0x2a,    0xff0d,  0xffe3,  0x41,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0xe7,
                            0xb0,    0x7c,    0xffe1,  0xa7,    0x5a,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x4d,    0x3b,    0x3a,    0x5f,    0xffe2,  0xffaa,
                            0xffe7,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffae,  0x0,     0xff7e,  0x3e,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };
                const KeyLayout_t x0410_altgr_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                            0x0,     0xff1b,  0xb9,    0xb2,    0xb3,    0xbc,    0xbd,    0xac,
                            0x7b,    0x5b,    0x5d,    0x7d,    0x60,    0x7e,    0xff08,  0xff09,
                            0x40,    0x1b3,   0x20ac,  0xb6,    0x3bc,   0x8fb,   0x8fe,   0x8fd,
                            0xf8,    0xfe,    0x5b,    0x5d,    0xff0d,  0xffe3,  0xe6,    0xdf,
                            0xf0,    0x1f0,   0x3bf,   0x2b1,   0x6a,    0x3a2,   0x1b3,   0x40,
                            0x23,    0xac,    0xffe1,  0xfe50,  0xab,    0xbb,    0xa2,    0xad2,
                            0xad3,   0xf1,    0xb5,    0xfe51,  0xb7,    0xfe54,  0xffe2,  0xffaa,
                            0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,   0x0,    0xff7e,  0xab,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0x0,
                            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,
                };
                const KeyLayout_t x0410_capslock_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                            0x0,     0xff1b,  0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0x27,    0xcc,    0xff08,  0xff09,
                            0x51,    0x57,    0x45,    0x52,    0x54,    0x59,    0x55,    0x49,
                            0x4f,    0x50,    0xc8,    0x2b,    0xff0d,  0xffe3,  0x41,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0xd2,
                            0xc0,    0x5c,   0xffe1,   0xd9,    0x5a,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x4d,    0x2c,    0x2e,    0x2d,    0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x3c,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,
                };
                const KeyLayout_t x0410_shiftcapslock_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                            0x0,     0xff1b,  0x21,    0x22,    0xa3,    0x24,    0x25,    0x26,
                            0x2f,    0x28,    0x29,    0x3d,    0x3f,    0x5e,    0xff08,  0xfe20,
                            0x71,    0x77,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,
                            0x6f,    0x70,    0xc9,    0x2a,    0xff0d,  0xffe3,  0x61,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0xc7,
                            0xb0,    0x7c,    0xffe1,  0xa7,    0x7a,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x6d,    0x3b,    0x3a,    0x5f,    0xffe2,  0xffaa,
                            0xffe7,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffae,  0x0,     0xff7e,  0x3e,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };

                for(size_t i = 0 ; i < 128 ; i++) {
                    if (x0410_noshift_sym[i]){
                        keylayout_WORK_noshift_sym[i] = x0410_noshift_sym[i] ;
                    }
                    if (x0410_shift_sym[i]){
                        keylayout_WORK_shift_sym[i] = x0410_shift_sym[i] ;
                    }
                    if (x0410_altgr_sym[i]){
                        keylayout_WORK_altgr_sym[i] = x0410_altgr_sym[i] ;
                    }
                    if (x0410_capslock_sym[i]){
                        keylayout_WORK_capslock_sym[i] = x0410_capslock_sym[i] ;
                    }
                    if (x0410_shiftcapslock_sym[i]){
                        keylayout_WORK_shiftcapslock_sym[i] = x0410_shiftcapslock_sym[i] ;
                    }
                }
            }
            break;

        case 0x0419: // Russian
        {
                const KeyLayout_t x0419_noshift_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                            0x0,     0xff1b,  0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0x2d,    0x3d,    0xff08,  0xff09,
                            0x6ca,   0x6c3,   0x6d5,   0x6cb,   0x6c5,   0x6ce,   0x6c7,   0x6db,
                            0x6dd,   0x6da,   0x6c8,   0x6df,   0xff0d,  0xffe3,  0x6c6,   0x6d9,
                            0x6d7,   0x6c1,   0x6d0,   0x6d2,   0x6cf,   0x6cc,   0x6c4,   0x6d6,
                            0x6dc,   0x6a3,   0xffe1,  0x5c,    0x6d1,   0x6de,   0x6d3,   0x6cd,
                            0x6c9,   0x6d4,   0x6d8,   0x6c2,   0x6c0,   0x2f,    0xffe2,  0xffaa,
                            0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x2f,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xffea,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,
                };
                const KeyLayout_t x0419_shift_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                            0x0,     0xff1b,  0x21,    0x22,    0x23,    0x2a,    0x3a,    0x2c,
                            0x2e,    0x3b,    0x28,    0x29,    0x5f,    0x2b,    0xff08,  0xfe20,
                            0x6ea,   0x6e3,   0x6f5,   0x6eb,   0x6e5,   0x6ee,   0x6e7,   0x6fb,
                            0x6fd,   0x6fa,   0x6e8,   0x6ff,   0xff0d,  0xffe3,  0x6e6,   0x6f9,
                            0x6f7,   0x6e1,   0x6f0,   0x6f2,   0x6ef,   0x6ec,   0x6e4,   0x6f6,
                            0x6fc,   0x6b3,   0xffe1,  0x7c,    0x6f1,   0x6fe,   0x6f3,   0x6ed,
                            0x6e9,   0x6f4,   0x6f8,   0x6e2,   0x6e0,   0x3f,    0xffe2,  0xffaa,
                            0xffe7,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffac,  0x0,     0xff7e,  0x7c,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xffe8,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };
                const KeyLayout_t x0419_altgr_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0x2d,    0x3d,  0xff08,  0xff09,
                            0x6ca,   0x6c3,   0x6d5,   0x6cb,   0x6c5,   0x6ce,   0x6c7,   0x6db,
                            0x6dd,   0x6da,   0x6c8,   0x6df,  0xff0d,  0xffe3,   0x6c6,   0x6d9,
                            0x6d7,   0x6c1,   0x6d0,   0x6d2,   0x6cf,   0x6cc,   0x6c4,   0x6d6,
                            0x6dc,   0x6a3,  0xffe1,    0x5c,   0x6d1,   0x6de,   0x6d3,   0x6cd,
                            0x6c9,   0x6d4,   0x6d8,   0x6c2,   0x6c0,    0x2f,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x7c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,     0x0,
                            0xffaf,  0xffea,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x0419_capslock_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0x2d,    0x3d,  0xff08,  0xff09,
                            0x6ea,   0x6e3,   0x6f5,   0x6eb,   0x6e5,   0x6ee,   0x6e7,   0x6fb,
                            0x6fd,   0x6fa,   0x6e8,   0x6ff,  0xff0d,  0xffe3,   0x6e6,   0x6f9,
                            0x6f7,   0x6e1,   0x6f0,   0x6f2,   0x6ef,   0x6ec,   0x6e4,   0x6f6,
                            0x6fc,   0x6b3,  0xffe1,    0x5c,   0x6f1,   0x6fe,   0x6f3,   0x6ed,
                            0x6e9,   0x6f4,   0x6f8,   0x6e2,   0x6e0,    0x2f,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x2f,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xffea,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x0419_shiftcapslock_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x21,    0x22,    0x23,    0x2a,    0x3a,    0x2c,
                            0x2e,    0x3b,    0x28,    0x29,    0x5f,    0x2b,  0xff08,  0xfe20,
                            0x6ca,   0x6c3,   0x6d5,   0x6cb,   0x6c5,   0x6ce,   0x6c7,   0x6db,
                            0x6dd,   0x6da,   0x6c8,   0x6df,  0xff0d,  0xffe3,   0x6c6,   0x6d9,
                            0x6d7,   0x6c1,   0x6d0,   0x6d2,   0x6cf,   0x6cc,   0x6c4,   0x6d6,
                            0x6dc,   0x6a3,  0xffe1,    0x7c,   0x6d1,   0x6de,   0x6d3,   0x6cd,
                            0x6c9,   0x6d4,   0x6d8,   0x6c2,   0x6c0,    0x3f,  0xffe2,  0xffaa,
                            0xffe7,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffac,     0x0,  0xff7e,    0x7c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xffe8,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };

                for(size_t i = 0 ; i < 128 ; i++) {
                    if (x0419_noshift_sym[i]){
                        keylayout_WORK_noshift_sym[i] = x0419_noshift_sym[i] ;
                    }
                    if (x0419_shift_sym[i]){
                        keylayout_WORK_shift_sym[i] = x0419_shift_sym[i] ;
                    }
                    if (x0419_altgr_sym[i]){
                        keylayout_WORK_altgr_sym[i] = x0419_altgr_sym[i] ;
                    }
                    if (x0419_capslock_sym[i]){
                        keylayout_WORK_capslock_sym[i] = x0419_capslock_sym[i] ;
                    }
                    if (x0419_shiftcapslock_sym[i]){
                        keylayout_WORK_shiftcapslock_sym[i] = x0419_shiftcapslock_sym[i] ;
                    }
                }
            }
            break;

        case 0x041d: // Swedish
        {
                const KeyLayout_t x041d_noshift_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0x2b,  0xfe51,  0xff08,  0xff09,
                            0x71,    0x77,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,
                            0x6f,    0x70,    0xe5,  0xfe57,  0xff0d,  0xffe3,    0x61,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0xf6,
                            0xe4,    0xa7,  0xffe1,    0x27,    0x7a,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x6d,    0x2c,    0x2e,    0x2d,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x3c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x041d_shift_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x21,    0x22,    0x23,    0xa4,    0x25,    0x26,
                            0x2f,    0x28,    0x29,    0x3d,    0x3f,  0xfe50,  0xff08,  0xfe20,
                            0x51,    0x57,    0x45,    0x52,    0x54,    0x59,    0x55,    0x49,
                            0x4f,    0x50,    0xc5,  0xfe52,  0xff0d,  0xffe3,    0x41,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0xd6,
                            0xc4,    0xbd,  0xffe1,    0x2a,    0x5a,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x4d,    0x3b,    0x3a,    0x5f,  0xffe2,  0xffaa,
                            0xffe7,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffac,     0x0,  0xff7e,    0x3e,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };
                const KeyLayout_t x041d_altgr_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0xa1,    0x40,    0xa3,    0x24,  0x20ac,    0xa5,
                            0x7b,    0x5b,    0x5d,    0x7d,    0x5c,    0xb1,  0xff08,  0xff09,
                            0x40,   0x1b3,  0x20ac,    0xae,    0xfe,   0x8fb,   0x8fe,   0x8fd,
                            0x13bd,    0xfe,  0xfe57,  0xfe53,  0xff0d,  0xffe3,    0xaa,    0xdf,
                            0xf0,   0x1f0,   0x3bf,   0x2b1,    0x6a,   0x3a2,   0x1b3,    0xf8,
                            0xe6,    0xb6,  0xffe1,    0xb4,    0xab,    0xbb,    0xa9,   0xad2,
                            0xad3,    0x6e,    0xb5,  0xfe5b,    0xb7,  0xfe60,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x7c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,     0x0,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x041d_capslock_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0x2b,  0xfe51,  0xff08,  0xff09,
                            0x51,    0x57,    0x45,    0x52,    0x54,    0x59,    0x55,    0x49,
                            0x4f,    0x50,    0xc5,  0xfe57,  0xff0d,  0xffe3,    0x41,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0xd6,
                            0xc4,    0xa7,  0xffe1,    0x27,    0x5a,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x4d,    0x2c,    0x2e,    0x2d,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x3c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x041d_shiftcapslock_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x21,    0x22,    0x23,    0xa4,    0x25,    0x26,
                            0x2f,    0x28,    0x29,    0x3d,    0x3f,  0xfe50,  0xff08,  0xfe20,
                            0x71,    0x77,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,
                            0x6f,    0x70,    0xe5,  0xfe52,  0xff0d,  0xffe3,    0x61,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0xf6,
                            0xe4,    0xbd,  0xffe1,    0x2a,    0x7a,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x6d,    0x3b,    0x3a,    0x5f,  0xffe2,  0xffaa,
                            0xffe7,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffac,     0x0,  0xff7e,    0x3e,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };

                for(size_t i = 0 ; i < 128 ; i++) {
                    if (x041d_noshift_sym[i]){
                        keylayout_WORK_noshift_sym[i] = x041d_noshift_sym[i] ;
                    }
                    if (x041d_shift_sym[i]){
                        keylayout_WORK_shift_sym[i] = x041d_shift_sym[i] ;
                    }
                    if (x041d_altgr_sym[i]){
                        keylayout_WORK_altgr_sym[i] = x041d_altgr_sym[i] ;
                    }
                    if (x041d_capslock_sym[i]){
                        keylayout_WORK_capslock_sym[i] = x041d_capslock_sym[i] ;
                    }
                    if (x041d_shiftcapslock_sym[i]){
                        keylayout_WORK_shiftcapslock_sym[i] = x041d_shiftcapslock_sym[i] ;
                    }
                }
            }
            break;

        case 0x046e: // Luxemburgish
        {
                const KeyLayout_t x046e_noshift_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0x27,    0x5e,  0xff08,  0xff09,
                            0x71,    0x77,    0x65,    0x72,    0x74,    0x7a,    0x75,    0x69,
                            0x6f,    0x70,    0x5b,    0x5d,  0xff0d,  0xffe3,    0x61,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0xf6,
                            0xe4,    0x5f,  0xffe1,    0x24,    0x79,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x6d,    0x2c,    0x2e,    0x2d,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x3c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x046e_shift_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x2b,    0x22,    0x2a,     0x0,    0x25,    0x26,
                            0x2f,    0x28,    0x29,    0x3d,    0x3f,    0x60,  0xff08,  0xfe20,
                            0x51,    0x57,    0x45,    0x52,    0x54,    0x5a,    0x55,    0x49,
                            0x4f,    0x50,    0x7b,    0x21,  0xff0d,  0xffe3,    0x41,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0x3a,
                            0x22,     0x0,     0x0,    0x23,    0x5a,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x4d,    0x3b,    0x3a,    0x5f,  0xffe2,  0xffaa,
                            0xffe7,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffac,     0x0,  0xff7e,    0x3e,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };
                const KeyLayout_t x046e_altgr_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x7e,    0x23,    0x7b,    0x5b,    0x7c,    0x60,
                            0x5c,    0x5e,    0x40,    0x5d,    0x7d,    0x3d,  0xff08,  0xff09,
                            0x40,   0x1b3,  0x20ac,    0xb6,   0x3bc,   0x8fb,   0x8fe,   0x8fd,
                            0xf8,    0xfe,  0xfe57,  0xfe53,  0xff0d,  0xffe3,    0xe6,    0xdf,
                            0xf0,   0x1f0,   0x3bf,   0x2b1,    0x6a,   0x3a2,   0x1b3,  0xfe59,
                            0xfe52,    0xac,  0xffe1,    0x5c,    0xab,    0xbb,    0xa2,   0xad2,
                            0xad3,    0x6e,    0xb5,   0x8a3,    0xb7,  0xfe60,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x7c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,     0x0,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x046e_capslock_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0xdf,  0xfe51,  0xff08,  0xff09,
                            0x51,    0x57,    0x45,    0x52,    0x54,    0x5a,    0x55,    0x49,
                            0x4f,    0x50,    0xdc,    0x2b,  0xff0d,  0xffe3,    0x41,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0xd6,
                            0xc4,  0xfe52,  0xffe1,    0x23,    0x59,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x4d,    0x2c,    0x2e,    0x2d,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x3c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x046e_shiftcapslock_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x21,    0x22,    0xa7,    0x24,    0x25,    0x26,
                            0x2f,    0x28,    0x29,    0x3d,    0x3f,  0xfe50,  0xff08,  0xfe20,
                            0x71,    0x77,    0x65,    0x72,    0x74,    0x7a,    0x75,    0x69,
                            0x6f,    0x70,    0xfc,    0x2a,  0xff0d,  0xffe3,    0x61,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0xf6,
                            0xe4,    0xb0,  0xffe1,    0x27,    0x79,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x6d,    0x3b,    0x3a,    0x5f,  0xffe2,  0xffaa,
                            0xffe7,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffac,     0x0,  0xff7e,    0x3e,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };

                for(size_t i = 0 ; i < 128 ; i++) {
                    if (x046e_noshift_sym[i]){
                        keylayout_WORK_noshift_sym[i] = x046e_noshift_sym[i] ;
                    }
                    if (x046e_shift_sym[i]){
                        keylayout_WORK_shift_sym[i] = x046e_shift_sym[i] ;
                    }
                    if (x046e_altgr_sym[i]){
                        keylayout_WORK_altgr_sym[i] = x046e_altgr_sym[i] ;
                    }
                    if (x046e_capslock_sym[i]){
                        keylayout_WORK_capslock_sym[i] = x046e_capslock_sym[i] ;
                    }
                    if (x046e_shiftcapslock_sym[i]){
                        keylayout_WORK_shiftcapslock_sym[i] = x046e_shiftcapslock_sym[i] ;
                    }
                }
            }
            break;

        case 0x0807: // German Swizerland
        {
                const KeyLayout_t x0807_noshift_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0xdf,  0xfe51,  0xff08,  0xff09,
                            0x71,    0x77,    0x65,    0x72,    0x74,    0x7a,    0x75,    0x69,
                            0x6f,    0x70,    0xfc,    0x2b,  0xff0d,  0xffe3,    0x61,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0xf6,
                            0xe4,  0xfe52,  0xffe1,    0x23,    0x79,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x6d,    0x2c,    0x2e,    0x2d,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x3c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x0807_shift_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x21,    0x22,    0xa7,    0x24,    0x25,    0x26,
                            0x2f,    0x28,    0x29,    0x3d,    0x3f,  0xfe50,  0xff08,  0xfe20,
                            0x51,    0x57,    0x45,    0x52,    0x54,    0x5a,    0x55,    0x49,
                            0x4f,    0x50,    0xdc,    0x2a,  0xff0d,  0xffe3,    0x41,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0xd6,
                            0xc4,    0xb0,  0xffe1,    0x27,    0x59,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x4d,    0x3b,    0x3a,    0x5f,  0xffe2,  0xffaa,
                            0xffe7,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffac,     0x0,  0xff7e,    0x3e,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };
                const KeyLayout_t x0807_altgr_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x7c,    0x40,    0x23,     0x0,     0x0,     0x0,
                            0x7c,     0x0,     0x0,    0x5d,    0x60,    0x7e,  0xff08,  0xff09,
                            0x40,   0x1b3,  0x20ac,    0xb6,   0x3bc,   0x8fb,   0x8fe,   0x8fd,
                            0x5b,    0x5b,    0x5b,    0x5d,  0xff0d,  0xffe3,    0xe6,    0xdf,
                            0xf0,   0x1f0,   0x3bf,   0x2b1,    0x6a,   0x3a2,   0x1b3,  0xfe59,
                            0x7b,    0xac,  0xffe1,    0x7d,    0xab,    0xbb,    0xa2,   0xad2,
                            0xad3,    0x6e,    0xb5,   0x8a3,    0xb7,  0xfe60,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x7c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,     0x0,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x0807_capslock_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0xdf,  0xfe51,  0xff08,  0xff09,
                            0x51,    0x57,    0x45,    0x52,    0x54,    0x5a,    0x55,    0x49,
                            0x4f,    0x50,    0xdc,    0x2b,  0xff0d,  0xffe3,    0x41,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0xd6,
                            0xc4,  0xfe52,  0xffe1,    0x23,    0x59,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x4d,    0x2c,    0x2e,    0x2d,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x3c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x0807_shiftcapslock_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x21,    0x22,    0xa7,    0x24,    0x25,    0x26,
                            0x2f,    0x28,    0x29,    0x3d,    0x3f,  0xfe50,  0xff08,  0xfe20,
                            0x71,    0x77,    0x65,    0x72,    0x74,    0x7a,    0x75,    0x69,
                            0x6f,    0x70,    0xfc,    0x2a,  0xff0d,  0xffe3,    0x61,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0xf6,
                            0xe4,    0xb0,  0xffe1,    0x27,    0x79,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x6d,    0x3b,    0x3a,    0x5f,  0xffe2,  0xffaa,
                            0xffe7,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffac,     0x0,  0xff7e,    0x3e,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };

                for(size_t i = 0 ; i < 128 ; i++) {
                    if (x0807_noshift_sym[i]){
                        keylayout_WORK_noshift_sym[i] = x0807_noshift_sym[i] ;
                    }
                    if (x0807_shift_sym[i]){
                        keylayout_WORK_shift_sym[i] = x0807_shift_sym[i] ;
                    }
                    if (x0807_altgr_sym[i]){
                        keylayout_WORK_altgr_sym[i] = x0807_altgr_sym[i] ;
                    }
                    if (x0807_capslock_sym[i]){
                        keylayout_WORK_capslock_sym[i] = x0807_capslock_sym[i] ;
                    }
                    if (x0807_shiftcapslock_sym[i]){
                        keylayout_WORK_shiftcapslock_sym[i] = x0807_shiftcapslock_sym[i] ;
                    }
                }
            }
            break;

        case 0x0809: // English UK
        {
                const KeyLayout_t x0809_noshift_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0x2d,    0x3d,  0xff08,  0xff09,
                            0x71,    0x77,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,
                            0x6f,    0x70,    0x5b,    0x5d,  0xff0d,  0xffe3,    0x61,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0x3b,
                            0x27,    0x60,  0xffe1,    0x23,    0x7a,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x6d,    0x2c,    0x2e,    0x2f,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x5c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x0809_shift_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x21,    0x22,    0xa3,    0x24,    0x25,    0x5e,
                            0x26,    0x2a,    0x28,    0x29,    0x5f,    0x2b,  0xff08,  0xfe20,
                            0x51,    0x57,    0x45,    0x52,    0x54,    0x59,    0x55,    0x49,
                            0x4f,    0x50,    0x7b,    0x7d,  0xff0d,  0xffe3,    0x41,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0x3a,
                            0x40,    0x7e,  0xffe1,  0xfe53,    0x5a,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x4d,    0x3c,    0x3e,    0x3f,  0xffe2,  0xffaa,
                            0xffe7,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffae,     0x0,  0xff7e,    0x7c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xffe8,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };
                const KeyLayout_t x0809_altgr_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x31,    0x32,    0x33,  0x20ac,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0x2d,    0x3d,  0xff08,  0xff09,
                            0x71,    0x77,    0xe9,    0x72,    0x74,    0x79,    0xfa,    0xed,
                            0xf3,    0x70,    0x5b,    0x5d,  0xff0d,  0xffe3,    0xe1,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0x3b,
                            0x27,    0xa6,  0xffe1,    0x5c,    0x7a,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x6d,    0x2c,    0x2e,    0x2f,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x7c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,     0x0,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x0809_capslock_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0x2d,    0x3d,  0xff08,  0xff09,
                            0x51,    0x57,    0x45,    0x52,    0x54,    0x59,    0x55,    0x49,
                            0x4f,    0x50,    0x5b,    0x5d,  0xff0d,  0xffe3,    0x41,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0x3b,
                            0x27,    0x60,  0xffe1,    0x23,    0x5a,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x4d,    0x2c,    0x2e,    0x2f,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x5c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xffea,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x0809_shiftcapslock_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x21,    0x22,    0xa3,    0x24,    0x25,    0x5e,
                            0x26,    0x2a,    0x28,    0x29,    0x5f,    0x2b,  0xff08,  0xfe20,
                            0x71,    0x77,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,
                            0x6f,    0x70,    0x7b,    0x7d,  0xff0d,  0xffe3,    0x61,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0x3a,
                            0x40,    0x7e,  0xffe1,  0xfe53,    0x7a,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x6d,    0x3c,    0x3e,    0x3f,  0xffe2,  0xffaa,
                            0xffe7,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffae,     0x0,  0xff7e,    0x7c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xffe8,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };

                for(size_t i = 0 ; i < 128 ; i++) {
                    if (x0809_noshift_sym[i]){
                        keylayout_WORK_noshift_sym[i] = x0809_noshift_sym[i] ;
                    }
                    if (x0809_shift_sym[i]){
                        keylayout_WORK_shift_sym[i] = x0809_shift_sym[i] ;
                    }
                    if (x0809_altgr_sym[i]){
                        keylayout_WORK_altgr_sym[i] = x0809_altgr_sym[i] ;
                    }
                    if (x0809_capslock_sym[i]){
                        keylayout_WORK_capslock_sym[i] = x0809_capslock_sym[i] ;
                    }
                    if (x0809_shiftcapslock_sym[i]){
                        keylayout_WORK_shiftcapslock_sym[i] = x0809_shiftcapslock_sym[i] ;
                    }
                }
            }
            break;

        case 0x080c: // French Belgium
        {
                const KeyLayout_t x080c_noshift_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x26,    0xe9,    0x22,    0x27,    0x28,    0x2d,
                            0xe8,    0x5f,    0xe7,    0xe0,    0x29,    0x3d,  0xff08,  0xff09,
                            0x61,    0x7a,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,
                            0x6f,    0x70,  0xfe52,    0x24,  0xff0d,  0xffe3,    0x71,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0x6d,
                            0xf9,    0xb2,  0xffe1,    0x2a,    0x77,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x2c,    0x3b,    0x3a,    0x21,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x3c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x080c_shift_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0xb0,    0x2b,  0xff08,  0xfe20,
                            0x41,    0x5a,    0x45,    0x52,    0x54,    0x59,    0x55,    0x49,
                            0x4f,    0x50,  0xfe57,    0xa3,  0xff0d,  0xffe3,    0x51,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0x4d,
                            0x25,    0x7e,  0xffe1,    0xb5,    0x57,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x3f,    0x2e,    0x2f,    0xa7,  0xffe2,  0xffaa,
                            0xffe7,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffae,     0x0,  0xff7e,    0x3e,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };
                const KeyLayout_t x080c_altgr_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                            0x0,     0xff1b,  0xb9,    0x7e,    0x24,    0x7b,    0x5b,    0x7c,
                            0x60,    0x5c,    0x5e,    0x40,    0x5d,    0x7d,    0xff08,  0xff09,
                            0xe6,    0xab,    0x20ac,  0xb6,    0x3bc,   0x8fb,   0x8fe,   0x8fd,
                            0xf8,    0xfe,    0xfe57,  0xa4,    0xff0d,  0xffe3,  0x40,    0xdf,
                            0xf0,    0x1f0,   0x3bf,   0x2b1,   0x6a,    0x3a2,   0x1b3,   0xb5,
                            0xfe52,  0xac,    0xffe1,  0xfe50,  0x1b3,   0xbb,    0xa2,    0xad2,
                            0xad3,   0x6e,    0xfe51,  0x8a3,   0xb7,    0xfe60,  0xffe2,  0xffaa,
                            0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x7c,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0x0,
                            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,
                };
                const KeyLayout_t x080c_capslock_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x26,    0xc9,    0x22,    0x27,    0x28,    0x2d,
                            0xc8,    0x5f,    0xc7,    0xc0,    0x29,    0x3d,  0xff08,  0xff09,
                            0x41,    0x5a,    0x45,    0x52,    0x54,    0x59,    0x55,    0x49,
                            0x4f,    0x50,  0xfe52,    0x24,  0xff0d,  0xffe3,    0x51,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0x4d,
                            0xd9,    0xb2,  0xffe1,    0x2a,    0x57,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x2c,    0x3b,    0x3a,    0x21,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x3c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x080c_shiftcapslock_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0xb0,    0x2b,  0xff08,  0xfe20,
                            0x61,    0x7a,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,
                            0x6f,    0x70,  0xfe57,    0xa3,  0xff0d,  0xffe3,    0x71,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0x6d,
                            0x25,    0x7e,  0xffe1,   0x39c,    0x77,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x3f,    0x2e,    0x2f,    0xa7,  0xffe2,  0xffaa,
                            0xffe7,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffae,     0x0,  0xff7e,    0x3e,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };

                for(size_t i = 0 ; i < 128 ; i++) {
                    if (x080c_noshift_sym[i]){
                        keylayout_WORK_noshift_sym[i] = x080c_noshift_sym[i] ;
                    }
                    if (x080c_shift_sym[i]){
                        keylayout_WORK_shift_sym[i] = x080c_shift_sym[i] ;
                    }
                    if (x080c_altgr_sym[i]){
                        keylayout_WORK_altgr_sym[i] = x080c_altgr_sym[i] ;
                    }
                    if (x080c_capslock_sym[i]){
                        keylayout_WORK_capslock_sym[i] = x080c_capslock_sym[i] ;
                    }
                    if (x080c_shiftcapslock_sym[i]){
                        keylayout_WORK_shiftcapslock_sym[i] = x080c_shiftcapslock_sym[i] ;
                    }
                }
            }
            break;

        case 0x0813: // Dutch Belgium
        {
                const KeyLayout_t x0813_noshift_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                            0x0,     0xff1b,  0x26,    0xe9,    0x22,    0x27,    0x28,    0x2d,
                            0xe8,    0x21,    0xe7,    0xe0,    0x29,    0x2d,    0xff08,  0xff09,
                            0x61,    0x7a,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,
                            0x6f,    0x70,    0xfe52,  0x24,    0xff0d,  0xffe3,  0x71,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0x6d,
                            0xf9,    0xb2,    0xffe1,  0x2a,    0x77,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x2c,    0x3b,    0x3a,    0x3d,    0xffe2,  0xffaa,
                            0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x3c,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,
                };
                const KeyLayout_t x0813_shift_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                            0x0,     0xff1b,  0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0xb0,    0x5f,    0xff08,  0xfe20,
                            0x41,    0x5a,    0x45,    0x52,    0x54,    0x59,    0x55,    0x49,
                            0x4f,    0x50,    0xfe57,  0xa3,    0xff0d,  0xffe3,  0x51,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0x4d,
                            0x25,    0x7e,    0xffe1,  0xb5,    0x57,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x3f,    0x2e,    0x2f,    0x2b,    0xffe2,  0xffaa,
                            0xffe7,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffae,  0x0,     0xff7e,  0x3e,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };
                const KeyLayout_t x0813_altgr_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                            0x0,     0xff1b,  0xb9,    0x40,    0x23,    0x7b,    0x5b,    0x5e,
                            0x0,     0x0,     0x7b,    0x7d,    0x0,     0x0,     0xff08,  0xff09,
                            0xe6,    0xab,    0x20ac,  0xb6,    0x3bc,   0x8fb,   0x8fe,   0x8fd,
                            0xf8,    0xfe,    0x5b,    0x5d,    0xff0d,  0xffe3,  0x40,    0xdf,
                            0xf0,    0x1f0,   0x3bf,   0x2b1,   0x6a,    0x3a2,   0x1b3,   0xb5,
                            0x27,    0x60,    0xffe1,  0xfe50,  0x1b3,   0xbb,    0xa2,    0xad2,
                            0xad3,   0x6e,    0xfe51,  0x8a3,   0xb7,    0x7e,    0xffe2,  0xffaa,
                            0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x7c,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0x0,
                            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,
                };
                const KeyLayout_t x0813_capslock_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                            0x0,     0xff1b,  0x26,    0xc9,    0x22,    0x27,    0x28,    0x2d,
                            0xc8,    0x5f,    0xc7,    0xc0,    0x29,    0x3d,    0xff08,  0xff09,
                            0x41,    0x5a,    0x45,    0x52,    0x54,    0x59,    0x55,    0x49,
                            0x4f,    0x50,    0xfe52,  0x24,    0xff0d,  0xffe3,  0x51,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0x4d,
                            0xd9,    0xb2,    0xffe1,  0x2a,    0x57,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x2c,    0x3b,    0x3a,    0x21,    0xffe2,  0xffaa,
                            0xffe9,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,  0x0,     0xff7e,  0x3c,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0x0,     0xffbd,  0x0,
                };
                const KeyLayout_t x0813_shiftcapslock_sym = {
                            0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                            0x0,     0xff1b,  0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0xb0,    0x2b,    0xff08,  0xfe20,
                            0x61,    0x7a,    0x65,    0x72,    0x74,    0x79,    0x75,    0x69,
                            0x6f,    0x70,    0xfe57,  0xa3,    0xff0d,  0xffe3,  0x71,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0x6d,
                            0x25,    0x7e,    0xffe1,  0x39c,   0x77,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x3f,    0x2e,    0x2f,    0xa7,    0xffe2,  0xffaa,
                            0xffe7,  0x20,    0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffae,  0x0,     0xff7e,  0x3e,    0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,  0x0,     0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,  0x0,     0xffeb,  0xffec,  0xff67,  0x0,     0x0,
                            0x0,     0x0,     0x0,     0x0,     0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };

                for(size_t i = 0 ; i < 128 ; i++) {
                    if (x0813_noshift_sym[i]){
                        keylayout_WORK_noshift_sym[i] = x0813_noshift_sym[i] ;
                    }
                    if (x0813_shift_sym[i]){
                        keylayout_WORK_shift_sym[i] = x0813_shift_sym[i] ;
                    }
                    if (x0813_altgr_sym[i]){
                        keylayout_WORK_altgr_sym[i] = x0813_altgr_sym[i] ;
                    }
                    if (x0813_capslock_sym[i]){
                        keylayout_WORK_capslock_sym[i] = x0813_capslock_sym[i] ;
                    }
                    if (x0813_shiftcapslock_sym[i]){
                        keylayout_WORK_shiftcapslock_sym[i] = x0813_shiftcapslock_sym[i] ;
                    }
                }
            }
            break;

        case 0x100c: // French Swizerland
        {
                const KeyLayout_t x100c_noshift_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0x27,    0x3d,  0xff08,  0xff09,
                            0x71,    0x77,    0x65,    0x72,    0x74,    0x7a,    0x75,    0x69,
                            0x6f,    0x70,    0x5b,    0x5d,  0xff0d,  0xffe3,    0x61,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0xf6,
                            0xe4,    0x5f,  0xffe1,    0x24,    0x79,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x6d,    0x2c,    0x2e,    0x2d,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x3c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x100c_shift_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x2b,    0x22,    0x2a,     0x0,    0x25,    0x26,
                            0x2f,    0x28,    0x29,    0x3d,    0x3f,  0xfe50,  0xff08,  0xfe20,
                            0x51,    0x57,    0x45,    0x52,    0x54,    0x5a,    0x55,    0x49,
                            0x4f,    0x50,    0x7b,    0x21,  0xff0d,  0xffe3,    0x41,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0x3a,
                            0x22,     0x0,     0x0,    0x23,    0x5a,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x4d,    0x3b,    0x3a,    0x5f,  0xffe2,  0xffaa,
                            0xffe7,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffac,     0x0,  0xff7e,    0x3e,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };
                const KeyLayout_t x100c_altgr_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x7c,    0x40,    0x23,     0x0,     0x0,     0x0,
                                0x0,    0x7c,     0x0,    0x5d,    0x60,    0x7e,  0xff08,  0xff09,
                            0x40,   0x1b3,  0x20ac,    0xb6,   0x3bc,   0x8fb,   0x8fe,   0x8fd,
                                0x0,     0x0,    0x5b,    0x5d,  0xff0d,  0xffe3,    0xe6,    0xdf,
                            0xf0,   0x1f0,   0x3bf,   0x2b1,    0x6a,   0x3a2,   0x1b3,  0xfe59,
                            0x7b,    0xac,  0xffe1,    0x7d,    0xab,    0xbb,    0xa2,   0xad2,
                            0xad3,    0x6e,    0xb5,   0x8a3,    0xb7,  0xfe60,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x7c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,     0x0,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x100c_capslock_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x31,    0x32,    0x33,    0x34,    0x35,    0x36,
                            0x37,    0x38,    0x39,    0x30,    0xdf,  0xfe51,  0xff08,  0xff09,
                            0x51,    0x57,    0x45,    0x52,    0x54,    0x5a,    0x55,    0x49,
                            0x4f,    0x50,    0xdc,    0x2b,  0xff0d,  0xffe3,    0x41,    0x53,
                            0x44,    0x46,    0x47,    0x48,    0x4a,    0x4b,    0x4c,    0xd6,
                            0xc4,  0xfe52,  0xffe1,    0x23,    0x59,    0x58,    0x43,    0x56,
                            0x42,    0x4e,    0x4d,    0x2c,    0x2e,    0x2d,  0xffe2,  0xffaa,
                            0xffe9,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xff7f,  0xff14,  0xff95,
                            0xff97,  0xff9a,  0xffad,  0xff96,  0xff9d,  0xff98,  0xffab,  0xff9c,
                            0xff99,  0xff9b,  0xff9e,  0xff9f,     0x0,  0xff7e,    0x3c,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,     0x0,  0xffbd,     0x0,
                };
                const KeyLayout_t x100c_shiftcapslock_sym = {
                                0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,     0x0,
                                0x0,  0xff1b,    0x21,    0x22,    0xa7,    0x24,    0x25,    0x26,
                            0x2f,    0x28,    0x29,    0x3d,    0x3f,  0xfe50,  0xff08,  0xfe20,
                            0x71,    0x77,    0x65,    0x72,    0x74,    0x7a,    0x75,    0x69,
                            0x6f,    0x70,    0xfc,    0x2a,  0xff0d,  0xffe3,    0x61,    0x73,
                            0x64,    0x66,    0x67,    0x68,    0x6a,    0x6b,    0x6c,    0xf6,
                            0xe4,    0xb0,  0xffe1,    0x27,    0x79,    0x78,    0x63,    0x76,
                            0x62,    0x6e,    0x6d,    0x3b,    0x3a,    0x5f,  0xffe2,  0xffaa,
                            0xffe7,    0x20,  0xffe5,  0xffbe,  0xffbf,  0xffc0,  0xffc1,  0xffc2,
                            0xffc3,  0xffc4,  0xffc5,  0xffc6,  0xffc7,  0xfef9,  0xff14,  0xffb7,
                            0xffb8,  0xffb9,  0xffad,  0xffb4,  0xffb5,  0xffb6,  0xffab,  0xffb1,
                            0xffb2,  0xffb3,  0xffb0,  0xffac,     0x0,  0xff7e,    0x3e,  0xffc8,
                            0xffc9,  0xff50,  0xff52,  0xff55,  0xff51,     0x0,  0xff53,  0xff57,
                            0xff54,  0xff56,  0xff63,  0xffff,  0xff8d,  0xffe4,  0xff13,  0xff61,
                            0xffaf,  0xfe03,     0x0,  0xffeb,  0xffec,  0xff67,     0x0,     0x0,
                                0x0,     0x0,     0x0,     0x0,  0xfe03,  0xffe9,  0xffbd,  0xffeb,
                };

                for(size_t i = 0 ; i < 128 ; i++) {
                    if (x100c_noshift_sym[i]){
                        keylayout_WORK_noshift_sym[i] = x100c_noshift_sym[i] ;
                    }
                    if (x100c_shift_sym[i]){
                        keylayout_WORK_shift_sym[i] = x100c_shift_sym[i] ;
                    }
                    if (x100c_altgr_sym[i]){
                        keylayout_WORK_altgr_sym[i] = x100c_altgr_sym[i] ;
                    }
                    if (x100c_capslock_sym[i]){
                        keylayout_WORK_capslock_sym[i] = x100c_capslock_sym[i] ;
                    }
                    if (x100c_shiftcapslock_sym[i]){
                        keylayout_WORK_shiftcapslock_sym[i] = x100c_shiftcapslock_sym[i] ;
                    }
                }
            }
            break;

        default:
            break;
    }

} // KeymapSym::init_layout
