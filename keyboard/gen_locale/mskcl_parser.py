## -*- coding: utf-8 -*-

u"""
This script parses keyboard layout files created by the tool MSKCL.exe.
It exacts the data needed to create .hpp files containning keyboard layouts for Redemption
Refer to MSKCL for input format
Refer to any <redemption_source>/keyboard/keylayout_xxx.hpp for output format
D. Lafages - Nov 2012
u"""

import re
import copy

acco1 = u"{"
acco2 = u"}"

##############################################################
# GLOBAL CONTAINER variables
##############################################################

#------------------------------------------
# Keylayout base tables (only common codes)
#------------------------------------------
noshift =  [  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x2a
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x37
           , 0x38,   0x39,   0x2d,   0x34,   0x35,   0x36,   0x2b,   0x31
           , 0x32,   0x33,   0x30,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
           , 0x2f,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
shift    = [  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,   0x7e,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x2a
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,   0x2d,    0x0,    0x0,    0x0,   0x2b,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,   0x7f,   0x0d,    0x0,    0x0,    0x0
           , 0x2f,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
altgr    = [  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x2a
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,   0x2d,    0x0,    0x0,    0x0,   0x2b,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
           , 0x2f,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
capslock = [  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0xd,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x2a
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,   0x2d,    0x0,    0x0,    0x0,   0x2b,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
           , 0x2f,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
shiftcap = [  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x2a
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,   0x2d,    0x0,    0x0,    0x0,   0x2b,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
           , 0x2f,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
ctrl    =  [  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x2a
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,   0x2d,    0x0,    0x0,    0x0,   0x2b,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
           , 0x2f,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
shctalt =  [  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x2a
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,   0x2d,    0x0,    0x0,    0x0,   0x2b,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
           ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
           , 0x2f,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]

##############################################################
# FUNCTIONS
##############################################################

#===============================================================================
def deadkeyScancode(scan, value):
#===============================================================================
    """ inserts 'scan' variable (scancode) in the deadkey coresponding to 'value' variable (utf-8)"""
    for dk in dks:
        if hex(int(dk[u'code'], 16)) == hex(int(value, 16)):
            dk[u'scan'] = scan

# END FUNCTION - deadkeyScancode


#===============================================================================
def handleChr(code, value):
#===============================================================================
    # A '@' char in "value" indicates this is a deadkey
    # So, test if a '@' character is present in "value"
    # If found, the scancode is inserted in the deadkey corresponding to "value" (call deadkeyScancode())
    # and the '@' is stripped from "value"
    resu = value
    atPos = value.rfind(u'@')
    if atPos >= 0:
        tts = value[0:atPos]
        deadkeyScancode(code, tts)
        resu = tts

    # values in varaiable "resu" are both :
    #   - plain int
    #   - str containning int (length = 2+)
    #   - str containning char (length = 1)
    # the purpose is to have :
    #   - str containning int CONVERTED to a plain int
    #   - str containning a char not converted
    #   - plain int not converted
    try:
        if len(resu ) > 1:
            resu = int(resu, 16)
    except:
        pass

    return resu

# END FUNCTION - handleChr


#===============================================================================
def writeLicense(f):
#===============================================================================
    f.write(u"/*\n".encode(u'utf-8'))
    f.write(u"   This program is free software; you can redistribute it and/or modify\n".encode(u'utf-8'))
    f.write(u"   it under the terms of the GNU General Public License as published by\n".encode(u'utf-8'))
    f.write(u"   the Free Software Foundation; either version 2 of the License, or\n".encode(u'utf-8'))
    f.write(u"   (at your option) any later version.\n".encode(u'utf-8'))
    f.write(u"\n".encode(u'utf-8'))
    f.write(u"   This program is distributed in the hope that it will be useful,\n".encode(u'utf-8'))
    f.write(u"   but WITHOUT ANY WARRANTY; without even the implied warranty of\n".encode(u'utf-8'))
    f.write(u"   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n".encode(u'utf-8'))
    f.write(u"   GNU General Public License for more details.\n".encode(u'utf-8'))
    f.write(u"\n".encode(u'utf-8'))
    f.write(u"   You should have received a copy of the GNU General Public License\n".encode(u'utf-8'))
    f.write(u"   along with this program; if not, write to the Free Software\n".encode(u'utf-8'))
    f.write(u"   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n".encode(u'utf-8'))
    f.write(u"\n".encode(u'utf-8'))
    f.write(u"   Product name: redemption, a FLOSS RDP proxy\n".encode(u'utf-8'))
    f.write(u"   Copyright (C) Wallix 2011\n".encode(u'utf-8'))
    f.write(u"   Author(s): Christophe Grosjean, Dominique Lafages\n".encode(u'utf-8'))
    f.write(u"   Based on xrdp Copyright (C) Jay Sorg 2004-2010\n".encode(u'utf-8'))
    f.write(u"\n".encode(u'utf-8'))
    f.write(u"   header file. Keylayout object, used by keymap managers\n".encode(u'utf-8'))
    f.write(u"*/\n".encode(u'utf-8'))

# END FUNCTION - writeLicense


#===============================================================================
def writeHeader(f):
#===============================================================================
    line = u"\n#if !defined(__KEYLAYOUT_X{locid}_HPP__)\n".format(locid = localeId.upper())
    f.write(line.encode(u'utf-8'))
    line = u"#define __KEYLAYOUT_X{locid}_HPP__\n".format(locid = localeId.upper())
    f.write(line.encode(u'utf-8'))
    f.write(u'\n#include "keylayout.hpp"\n'.encode(u'utf-8'))
    line = u"\nnamespace x{locid} {acco}    // {locdesc}\n".format( locid = localeId
                                                                  , acco = acco1
                                                                  , locdesc = localeDesc
                                                                  )
    f.write(line.encode(u'utf-8'))
    line = u"\nconst static int LCID = 0x{locid};\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))

# END FUNCTION - writeHeader


#===============================================================================
def writeCloseNS(f):
#===============================================================================
    line = u"\n{acco} // END NAMESPACE - x{locid}\n".format( acco = acco2
                                                           , locid = localeId
                                                           )
    f.write(line.encode(u'utf-8'))

# END FUNCTION - writeCloseNS


#===============================================================================
def writeTrailer(f):
#===============================================================================
    f.write(u"\n#endif\n".encode(u'utf-8'))

# END FUNCTION - writeTrailer


#===============================================================================
def writeLayout(f, lo, tag):
#===============================================================================
    line = u"\nconst Keylayout::KeyLayout_t {tag} = {acco}\n".format(acco = acco1, tag = tag)
    f.write(line.encode(u'utf-8'))

    line = u""
    cptTag = 0
    cptCol = 0

    for code in lo:

        if cptCol == 0:
            line = u"    /* x{vald:0>2X} - x{valf:0>2X} */   ".format(vald = cptTag, valf = cptTag + 7)

        if isinstance(code, int):
            line = u"{deb} 0x{code:0>4X},".format(deb = line, code = code)
        else:
            line = u"{deb}    '{code}',".format(deb = line, code = code)

        u"%s 0x%s, " % (line, code)

        if cptCol < 7:
            cptCol = cptCol + 1
        else:
            line = u"%s\n" % (line)
            f.write(line.encode(u'utf-8'))
            cptCol = 0
            cptTag = cptTag + 8

    line = u"{acco};\n".format(acco = acco2)
    f.write(line.encode(u'utf-8'))

# END FUNCTION - writeLayout


#===============================================================================
def prepareNoshift(f):
#===============================================================================
    for kl in kls:
        if kl['noshift'] != u"-1":
            noshift[int(kl['scancode'], 16) + 8]  = handleChr(kl['scancode'], kl['noshift'])

# END FUNCTION - prepareNoshift


#===============================================================================
def prepareShift(f):
#===============================================================================
    for kl in kls:
        if kl['shift'] != u"-1":
            shift[int(kl['scancode'], 16) + 8] = handleChr(kl['scancode'], kl['shift'])

# END FUNCTION - prepareShift


#===============================================================================
def prepareCtrlAlt(f):
#===============================================================================
    for kl in kls:
        if kl['ctrlAlt'] != u"-1":
            altgr[int(kl['scancode'], 16) + 8] = handleChr(kl['scancode'], kl['ctrlAlt'])

# END FUNCTION - prepareCtrlAlt


#===============================================================================
def prepareCapslock(f):
#===============================================================================
    for kl in kls:
        if kl['isCapsLock'] == u"1":
            capslock[int(kl['scancode'], 16) + 8] = handleChr(kl['scancode'], kl['shift'])
        elif kl['isCapsLock'] == u"0":
            capslock[int(kl['scancode'], 16) + 8] = handleChr(kl['scancode'], kl['noshift'])
        elif kl['isCapsLock'] == u"SGCap":
            capslock[int(kl['scancode'], 16) + 8] = handleChr(kl['scancode'], kl['capslock'])

# END FUNCTION - prepareCapslock


#===============================================================================
def prepareShiftCapslock(f):
#===============================================================================
    for kl in kls:
        if kl['isCapsLock'] == u"1":
            shiftcap[int(kl['scancode'], 16) + 8] = handleChr(kl['scancode'], kl['noshift'])
        elif kl['isCapsLock'] == u"0":
            shiftcap[int(kl['scancode'], 16) + 8] = handleChr(kl['scancode'], kl['shift'])
        elif kl['isCapsLock'] == u"SGCap":
            shiftcap[int(kl['scancode'], 16) + 8] = handleChr(kl['scancode'], kl['shiftCapslock'])

# END FUNCTION - prepareShiftCapslock


#===============================================================================
def prepareCtrl(f):
#===============================================================================
    for kl in kls:
        if kl['ctrl'] != u"-1":
            ctrl[int(kl['scancode'], 16) + 8]  = handleChr(kl['scancode'], kl['ctrl'])

# END FUNCTION - prepareCtrl


#===============================================================================
def prepareShiftCtrlAlt(f):
#===============================================================================
    for kl in kls:
        if kl['shiftCtrlAlt'] != u"-1":
            shctalt[int(kl['scancode'], 16) + 8]  = handleChr(kl['scancode'], kl['shiftCtrlAlt'])

# END FUNCTION - prepareShiftCtrlAlt


#===============================================================================
def writeDeadkeys(f, deadkeys):
#===============================================================================
    line = u"\nconst Keylayout::dkey_t deadkeys[] = {acco}\n".format(acco = acco1)
    f.write(line.encode(u'utf-8'))
    nbDks = 0
    for dk in deadkeys:
        nbDks = nbDks + 1
        baseline = u"      {acco1} 0x{uchar}, 0x{scan}, {size:>2}, {acco1} {acco1}".format( acco1 = acco1
                                                                                       , acco2 = acco2
                                                                                       , uchar = dk["code"]
                                                                                       , scan = dk["scan"]
                                                                                       , size = len(dk[u'keylist'])
                                                                                       )
#        print baseline

        first = True
        for key in dk[u'keylist'].keys():
            if first:
                first = False
                line = u"{base}0x{car1}, 0x{car2}{acco2}\n".format( acco1 = acco1
                                                                  , acco2 = acco2
                                                                  , car1 = key
                                                                  , car2 = dk[u'keylist'][key]
#                                                                  , car1 = unichr(int(key, 16))
#                                                                  , car2 = unichr(int(dk[u'keylist'][key], 16))
                                                                  , base = baseline
                                                                  )
                f.write(line.encode(u'utf-8'))
            else:
                line = u"                          , {acco1}0x{car1}, 0x{car2}{acco2}\n".format( acco1 = acco1
                                                                                               , acco2 = acco2
                                                                                               , car1 = key
                                                                                               , car2 = dk[u'keylist'][key]
#                                                                                               , car1 = unichr(int(key, 16))
#                                                                                               , car2 = unichr(int(dk[u'keylist'][key], 16))
                                                                                               )
                f.write(line.encode(u'utf-8'))

        line = u"                          {acco}\n".format(acco = acco2)
        f.write(line.encode(u'utf-8'))

        line = u"      {acco},\n".format(acco = acco2)
        f.write(line.encode(u'utf-8'))

    # List must contains five elements. So, fill up with neutral entries
    for i in range(nbDks, 5):
        line = u"     {acco1} 0x00, 0x00,  0, {acco1}{acco2} {acco2},\n".format( acco1 = acco1
                                                                               , acco2 = acco2
                                                                               )
        f.write(line.encode(u'utf-8'))

    line = u"{acco};\n".format(acco = acco2)
    f.write(line.encode(u'utf-8'))

# END FUNCTION - writeDeadkeys


#===============================================================================
def writeInstance(f):
#===============================================================================
    line = u"\nstatic const Keylayout keylayout_x{locid}( x{locid}::LCID\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::noshift\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::shift\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::capslock\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::altgr\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::shiftcapslock\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::ctrl\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::shiftaltgr\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::deadkeys\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u");\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))

# END FUNCTION - writeInstance


##############################################################
##############################################################
##############################################################
##############################################################
##############################################################

##############################################################
# MAIN
##############################################################

#=============================================================
# Browse source keyboard layout files
#=============================================================
for keymap in [ u'./locale/belgian_french.klc'
              , u'./locale/canadian_french.klc'
              , u'./locale/canadian_multilingual.klc'
              , u'./locale/dutch.klc'
              , u'./locale/french.klc'
              , u'./locale/german.klc'
              , u'./locale/italian.klc'
              , u'./locale/luxembourgish.klc'
              , u'./locale/portuguese.klc'
              , u'./locale/russian.klc'
              , u'./locale/spanish.klc'
              , u'./locale/swiss_german.klc'
              , u'./locale/us.klc'
              ]:

    #==============================================================
    # LOCALE CONTAINER variables
    #=============================================================
    # locale tag
    locale = None
    # locale code
    localeId = None
    # locale label
    localeDesc = None
    # dictionay depicting the layouts for one scancode
    kls = []
    # list of deadkey dicts for a locale
    dks = []

    #=============================================================
    # TECHNICAL variables
    #=============================================================
    # list of shift states available for that locale (out of [1, 2, 6, 7])
    shifts = []
    # base of the regex to apply to a keylayout line (will be augmented dynamically depending on available shift states)
    reKL = u'([0-9]{1}[0-9a-f]{1})\t[^\t]*\t*([0-1]{1}|SGCap)\t*([^\t]*)\t*'
    # Dynamic flag indicating current operation
    state = u"LOCALE"         # first value is always "LOCALE"
    deadkeyCode = u"ffff"     # first value is set arbitrarily to "ffff" to allow detection of new deadkey code
    oldDeadkeyCode = u"ffff"  # previous value of deadkeyCode to allow detection of new deadkey code
    SGCap = False

    print u"\n>>>> LOCALE\n"

    #=============================================================
    # PHASE 1 : Parse the keylayout file and extract relevant data
    #=============================================================

     # Behave as a finite state automaton ; state are detected by regex against block titles in the parsed file ;
     # each state is handled according to its particular format and what data is to be extracted

    for line in file(keymap, u'r'):

        #------------------------------------------------------------
        # FLAG SHIFTSTATE
        #------------------------------------------------------------
        matchObj = re.match( u'^SHIFTSTATE', line)
        if matchObj:
            state = u"SHIFTSTATE"
            print u"\n>>>> FLAG SHIFTSTATE FOUND\n"
            continue
        #------------------------------------------------------------
        # FLAG LAYOUT
        #------------------------------------------------------------
        matchObj = re.match( u'^LAYOUT.*', line)
        if matchObj:
            state = u"LAYOUT"
            print u"\n>>>> FLAG LAYOUT FOUND\n"
            continue
        #------------------------------------------------------------
        # FLAG DEADKEY
        #------------------------------------------------------------
        matchObj = re.match( u'^DEADKEY\t([0-9a-f]{4})', line)
        if matchObj:
            state = u"DEADKEY"
            deadkeyCode = matchObj.group(1)
            print u"\n>>>> FLAG DEADKEY FOUND : ", deadkeyCode, "\n"
            continue
        #------------------------------------------------------------
        # FLAG KEYNAME
        #------------------------------------------------------------
        matchObj = re.match( u'^KEYNAME.*', line)
        if matchObj:
            state = u"KEYNAME"
            print u"\n>>>> FLAG KEYNAME FOUND\n"
            continue
        #------------------------------------------------------------
        # FLAG LANGUAGENAMES
        #------------------------------------------------------------
        matchObj = re.match( u'^LANGUAGENAMES.*', line)
        if matchObj:
            state = u"LANGUAGENAMES"
            print u"\n>>>> FLAG LANGUAGENAMES FOUND\n"
            continue
        #------------------------------------------------------------
        # FLAG ENDKBD
        #------------------------------------------------------------
        matchObj = re.match( u'^ENDKBD.*', line)
        if matchObj:
            state = u"KEYNAME"
            print u"\n>>>> FLAG ENDKBD FOUND\n"
            break

        if state == u"LOCALE":
            #------------------------------------------------------------
            # LOCALE NAME
            #------------------------------------------------------------
            matchObj = re.match( u'LOCALENAME\t"(.{5})', line)
            if matchObj:
                locale = matchObj.group(1)
                print u"    locale : >%s<" % (locale)
                continue
            #------------------------------------------------------------
            # LOCALE ID
            #------------------------------------------------------------
            matchObj = re.match( u'LOCALEID\t"(.{8})', line)
            if matchObj:
                localeId = matchObj.group(1)
                print u"    locale ID : >%s<" % (localeId)
                continue

        if state == u"SHIFTSTATE":
            #------------------------------------------------------------
            # SHIFSTATE 1 (001b)
            #------------------------------------------------------------
            matchObj = re.match( u'(1)\t//Column.*', line)
            if matchObj:
                shifts.append(u"1")
                reKL = u"%s([^\t]*)\t*0*" % (reKL)
                print u"    state 1"
                continue
            #------------------------------------------------------------
            # SHIFSTATE 2 (010b)
            #------------------------------------------------------------
            matchObj = re.match( u'(2)\t//Column.*', line)
            if matchObj:
                shifts.append(u"2")
                reKL = u"%s([^\t]*)\t*0*" % (reKL)
                print u"    state 2"
                continue
            #------------------------------------------------------------
            # SHIFSTATE 6 (110b)
            #------------------------------------------------------------
            matchObj = re.match( u'(6)\t//Column.*', line)
            if matchObj:
                shifts.append(u"6")
                reKL = "%s([^\t]*)\t*0*" % (reKL)
                print u"    state 6"
                continue
            #------------------------------------------------------------
            # SHIFSTATE 7 (111b)
            #------------------------------------------------------------
            matchObj = re.match( u'(7)\t//Column.*', line)
            if matchObj:
                shifts.append(u"7")
                reKL = u"%s([^\t]*)\t*0*" % (reKL)
                print u"    state 7"
                continue

        if state == "LAYOUT":
            #------------------------------------------------------------
            # KEYLAYOUT LINES
            #------------------------------------------------------------
            # Specific treatment for SGCap capslock behaviour :
            #  a second line (with a different format= have to be parsed to complete the previous key layout
            if SGCap == True:
                SGCap = False
                matchObj = re.match( u'[^\t]*\t*[^\t]*\t*[^\t]*\t*([^\t]*)\t*([^\t]*).*', line)
                if matchObj:
                    keylayout = matchObj.group()
                    klcar[u'capslock']   = matchObj.group(1)
                    klcar[u'shiftCapslock'] = matchObj.group(2)
                    kls.append(klcar)
                continue

            # make custom regex depending on the number of columns
            matchObj = re.match( reKL, line)
            if matchObj:
                keylayout = matchObj.group()
                klcar = {}
                klcar[u'scancode']   = matchObj.group(1)
                klcar[u'isCapsLock'] = matchObj.group(2)
                klcar[u'noshift']    = matchObj.group(3)

                # shift state counter (first value is always '4'; further on, increases each time a new state is found
                numState = 4

                for s in shifts:
                    if s == u"1":
                        klcar[u'shift']  = matchObj.group(numState)
                    if s == u"2":
                        klcar[u'ctrl']  = matchObj.group(numState)
                    if s == u"6":
                        klcar[u'ctrlAlt']  = matchObj.group(numState)
                    if s == u"7":
                        klcar[u'shiftCtrlAlt']  = matchObj.group(numState)
                    numState = numState + 1
                if klcar[u'isCapsLock'] == u"SGCap":
                    SGCap = True
                else:
                    kls.append(klcar)
#                print klcar
                continue

        if state == u"DEADKEY":
            #------------------------------------------------------------
            # DEADKEY LINES
            #------------------------------------------------------------
            if deadkeyCode != oldDeadkeyCode:
                # initialize a new deadkey definition
                oldDeadkeyCode = deadkeyCode
                dk = {}
                dk[u"scan"] = u"??"
                dk[u"code"] = deadkeyCode
                dk[u"keylist"] = {}
                # push the new deadkey definition in deakeys list
                dks.append(dk)
            else:
                # search for a key couple for the active deadkey
                matchObj = re.match(u'([0-9a-f]*)\t([0-9a-f]*)\t// (.*)', line)
                if matchObj:
                    ttdk = dks.pop()
                    ttdk[u"keylist"][matchObj.group(1)] = matchObj.group(2)
                    dks.append(ttdk)

        if state == u"LANGUAGENAMES":
            #------------------------------------------------------------
            # LANGUAGENAMES LINE
            #------------------------------------------------------------
            print line
            matchObj = re.match( u'([0-9a-f]*)\t(.*)', line)
            if matchObj:
                localeDesc = matchObj.group(2)

#    print kls
    #=============================================================
    # PHASE 2 : Use the extracted data to create the .hpp file
    #=============================================================
    filename = u"./keylayout_x{locid}.hpp".format(locid = localeId)
    f = open(filename, u'w')
    writeLicense(f)
    writeHeader(f)
    prepareNoshift(f)
    prepareCapslock(f)
    prepareShiftCapslock(f)

    for s in shifts:
        if s == u"1":
            prepareShift(f)
        if s == u"2":
            prepareCtrl(f)
        if s == u"6":
            prepareCtrlAlt(f)  # idem AltGr
        if s == u"7":
            prepareShiftCtrlAlt(f)

    writeLayout(f, noshift,  u'noshift')
    writeLayout(f, shift,    u'shift')
    writeLayout(f, altgr,    u'altgr')
    writeLayout(f, capslock, u'capslock')
    writeLayout(f, shiftcap, u'shiftcapslock')
    writeLayout(f, ctrl, u'ctrl')
    writeLayout(f, shctalt, u'shiftaltgr')

    writeDeadkeys(f, dks)
    writeCloseNS(f)
    writeInstance(f)
    writeTrailer(f)

    f.close()


# END SCRIPT
