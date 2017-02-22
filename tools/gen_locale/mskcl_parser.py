## -*- coding: utf-8 -*-

u"""
This script parses keyboard layout files created by the tool MSKCL.exe.
It exacts the data needed to create .hpp files containning keyboard layouts for Redemption
Refer to MSKCL for input format
Refer to any <redemption_source>/keyboard/keylayout_xxx.hpp for output format
D. Lafages - Nov 2012
u"""

# ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !
# ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !

# Keylayout Files are supposed to be UTF-8 encoded with UNIX/Linux terminated lines

# ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !
# ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! ! !


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
noModB =              [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '7'
                      ,  '8',    '9',    '-',    '4',    '5',    '6',    '+',    '1'
                      ,  '2',    '3',    '0',    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
shiftB =              [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    '~',    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '-',    0x0,    0x0,    0x0,    '+',    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,   0x7f,   0x0d,    0x0,    0x0,    0x0
                      ,  '/',    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
altGrB =              [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '-',    0x0,    0x0,    0x0,    '+',    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
shiftAltGrB =         [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '-',    0x0,    0x0,    0x0,    '+',    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
ctrlB =               [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '-',    0x0,    0x0,    0x0,    '+',    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
capslock_noModB =     [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0xd,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '-',    0x0,    0x0,    0x0,    '+',    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
capslock_shiftB =     [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '-',    0x0,    0x0,    0x0,    '+',    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
capslock_altGrB =     [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '-',    0x0,    0x0,    0x0,    '+',    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
capslock_shiftAltGrB= [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '-',    0x0,    0x0,    0x0,    '+',    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]

#------------------------------------------
# Keylayout WORK tables (only common codes)
#------------------------------------------
noMod =               [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '7'
                      ,  '8',    '9',    '-',    '4',    '5',    '6',    '+',    '1'
                      ,  '2',    '3',    '0',    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',   0x7f,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
shift =               [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    '~',    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '-',    0x0,    0x0,    0x0,    '+',    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
altGr =               [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '-',    0x0,    0x0,    0x0,    '+',    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
shiftAltGr =          [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '-',    0x0,    0x0,    0x0,    '+',    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
ctrl =                [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '-',    0x0,    0x0,    0x0,    '+',    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
capslock_noMod =      [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0xd,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '-',    0x0,    0x0,    0x0,    '+',    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
capslock_shift =      [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '-',    0x0,    0x0,    0x0,    '+',    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
capslock_altGr =      [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '-',    0x0,    0x0,    0x0,    '+',    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
]
capslock_shiftAltGr = [  0x0,   0x1b,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,   0x08,   0x09
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    '*'
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '-',    0x0,    0x0,    0x0,    '+',    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    '/',    0x0,   0x0d,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
                      ,  0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0,    0x0
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
    f.write(u"   Copyright (C) Wallix 2012\n".encode(u'utf-8'))
    f.write(u"   Author(s): Christophe Grosjean, Dominique Lafages\n".encode(u'utf-8'))
    f.write(u"   Based on xrdp Copyright (C) Jay Sorg 2004-2010\n".encode(u'utf-8'))
    f.write(u"\n".encode(u'utf-8'))
    f.write(u"   header file. Keylayout object, used by keymap managers\n".encode(u'utf-8'))
    f.write(u"*/\n".encode(u'utf-8'))

# END FUNCTION - writeLicense


#===============================================================================
def writeHeader(f, locale_prefix):
#===============================================================================
    line = u"\n#ifndef _REDEMPTION_KEYBOARD_KEYLAYOUT_X{locid}_HPP_\n".format(locid = localeId.upper())
    f.write(line.encode(u'utf-8'))
    line = u"#define _REDEMPTION_KEYBOARD_KEYLAYOUT_X{locid}_HPP_\n".format(locid = localeId.upper())
    f.write(line.encode(u'utf-8'))
    f.write(u'\n#include "keylayout.hpp"\n'.encode(u'utf-8'))
    line = u"\nnamespace x{locid} {acco}    // {loctag} // {locdesc}\n".format( locid = localeId
                                                                  , acco = acco1
                                                                  , loctag = localeTag.decode('utf-8')
                                                                  , locdesc = localeDesc.decode('utf-8')
                                                                  )
    f.write(line.encode(u'utf-8'))
    line = u"\nconst static int LCID = 0x{locid};\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"\nconst static char * const locale_name = \"{locname}{prx}\";\n".format(locname = locale, prx = locale_prefix)
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

#        print u"%s 0x%s, " % (line, code)

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
def razMaps(                ):
#===============================================================================
    for i in range (0, 119):
        noMod[i] = noModB[i]
        shift[i] = shiftB[i]
        capslock_noMod[i] = capslock_noModB[i]
        altGr[i] = altGrB[i]
        shiftAltGr[i] = shiftAltGrB[i]
        capslock_shift[i] = capslock_shiftB[i]
        capslock_altGr[i] = capslock_altGrB[i]
        capslock_shiftAltGr[i] = capslock_shiftAltGrB[i]
        ctrl[i] = ctrlB[i]

# END FUNCTION - razMaps


#===============================================================================
def prepareNoMod(f):
#===============================================================================
    for kl in kls:
        if kl['noMod'] != u"-1":
            noMod[int(kl['scancode'], 16)]  = handleChr(kl['scancode'], kl['noMod'])

# END FUNCTION - prepareNoMod


####################################
# The following "prepare" functions set the related map according to each line of keylayout.
# Each map is first initialised with international fixed values and zero in other values.
# Each line of the keylayout layout file (list of dicts) is then used to modify map entries that have a specific values for the locale
# Some rules for the maps :

# - NOSHIFT map entries are each modified if the "noMod" dict member is != -1
#        NB : always present ; simple rule

# - SHIFT map entries are each modified if a "shift" "noMod" dict member is != -1
#        NB : always present ; simple rule

#    NB : for the following, keep in mind that capslock dict entry is always present and may take one of the values :
#         0, 1, 4, 5, SGCap. It only make sense for shift state couples 0/1 and 6/7 that are minuscule/capitale couples,
#         with a very specific behaviour for SGCap in which the couples are
#      * 0 means that none of the glyphs of that key are 'shifted' by capslock
#      * 1 means that the glyph in shift block (shift state 0/1) is 'shifted' by capslock
#      * 4 means that the glyph in shift block  (shift state 0/1) is not 'shifted' by capslock, but the glyph in altGr block (shift state 6/7) is 'shifted'
#      * 5 means that both glyph in shift block  (shift state 0/1) and altGr block (shift state 6/7) are'shifted' by capslock
#      * SGCap means that the value in noshitf block (shift state 0) and the value in shift block (shift state 1) are both shifted by capslock,
#        with capitale letters that are given in a special line located below

# - CAPSLOCK map entries are each modified following the rule :
#      - if "caplock" dict member is 0, take the value of "noMod" dict member
#      - if "caplock" dict member is 1 or 5, take the value of "shift" dict member
#      - if "caplock" dict member is SGCap (spec for Swiss German), take the value of "capslock_SGCap" dict member

# - SHIFTCAPSLOCK map entries are each modified following the rule :
#        NB : a rule of thumb, consider that each result of CAPSLOCK map entries is simply reversed.
#      - if "caplock" dict member is 0, take the value of "shift" dict member
#      - if "caplock" dict member is 1 or 5, take the value of "noMod" dict member
#      - if "caplock" dict member is SGCap (spec for Swiss German), take the value of "shiftCapslock_SGCap" dict member

# - ALTGR map entries are each modified following the rule :
#      - if "caplock" dict member is 0, take the value of "altGr" dict member
#      - if "caplock" dict member contains 4 take the value of "shiftAltGr" dict member

# - SHIFTALTGR map entries are each modified following the rule :
#        NB : a rule of thumb, consider that each result of ALTGR map entries is simply reversed.
#      - if "caplock" dict member is 0, take the value of "shiftAltGr" dict member
#      - if "caplock" dict member contains 4 take the value of "altGr" dict member

# - CTRL map entries are each modified if the "ctrl" dict member is != -1
#        NB : "ctrl" dict member seems to be always present (but this an observation, not a rule)

#===============================================================================
def prepareShift(f):
#===============================================================================
    for kl in kls:
        if kl['shift'] != u"-1":
            shift[int(kl['scancode'], 16)] = handleChr(kl['scancode'], kl['shift'])

# END FUNCTION - prepareShift


#===============================================================================
def prepareCapslock_NoMod(f):
#===============================================================================
    for kl in kls:
        if kl['isCapsLock'] == u"SGCap":
            if 'capslock_SGCap' in kl:
                if kl['capslock_SGCap'] != u"-1":
                    capslock_noMod[int(kl['scancode'], 16)] = handleChr(kl['scancode'], kl['capslock_SGCap'])
        else:
            if int(kl['isCapsLock'], 16) & 1:
                capslock_noMod[int(kl['scancode'], 16)] = handleChr(kl['scancode'], kl['shift'])
            else:
                capslock_noMod[int(kl['scancode'], 16)] = handleChr(kl['scancode'], kl['noMod'])

# END FUNCTION - prepareCapslock_NoMod


#===============================================================================
def prepareCapslock_shift(f):
#===============================================================================
    for kl in kls:
        if kl['isCapsLock'] == u"SGCap":
            if 'shiftCapslock_SGCap' in kl:
                if kl['shiftCapslock_SGCap'] != u"-1":
                    capslock_shift[int(kl['scancode'], 16)] = handleChr(kl['scancode'], kl['shiftCapslock_SGCap'])
        else:
            if int(kl['isCapsLock'], 16) & 1:
                capslock_shift[int(kl['scancode'], 16)] = handleChr(kl['scancode'], kl['noMod'])
            else:
                capslock_shift[int(kl['scancode'], 16)] = handleChr(kl['scancode'], kl['shift'])

# END FUNCTION - prepareCapslock_shift


#===============================================================================
def prepareAltGr(f): # aka CtrlAlt
#===============================================================================
    for kl in kls:
        if 'altGr' in kl:
            if kl['altGr'] != u"-1":
                altGr[int(kl['scancode'], 16)]  = handleChr(kl['scancode'], kl['altGr'])

# END FUNCTION - prepareAltGr


#===============================================================================
def prepareShiftAltGr(f):
#===============================================================================
    for kl in kls:
        if 'shiftAltGr' in kl:
            if kl['shiftAltGr'] != u"-1":
                shiftAltGr[int(kl['scancode'], 16)]  = handleChr(kl['scancode'], kl['shiftAltGr'])

# END FUNCTION - prepareShiftAltGr


#===============================================================================
def prepareCapslock_altGr(f):
#===============================================================================
    for kl in kls:

        if kl['isCapsLock'] == u"SGCap":
            if 'altGr' in kl:
                if kl['altGr'] != u"-1":
                    capslock_altGr[int(kl['scancode'], 16)] = handleChr(kl['scancode'], kl['altGr'])
        else:
            if int(kl['isCapsLock'], 16) & 4:
              if 'shiftAltGr' in kl:
                  if kl['shiftAltGr'] != u"-1":
                      capslock_altGr[int(kl['scancode'], 16)] = handleChr(kl['scancode'], kl['shiftAltGr'])
            else:
              if 'altGr' in kl:
                  if kl['altGr'] != u"-1":
                      capslock_altGr[int(kl['scancode'], 16)] = handleChr(kl['scancode'], kl['altGr'])

# END FUNCTION - prepareCapslock_altGr


#===============================================================================
def prepareCapslock_shiftAltGr(f):
#===============================================================================
    for kl in kls:

        if kl['isCapsLock'] == u"SGCap":
            if 'shiftAltGr' in kl:
                if kl['shiftAltGr'] != u"-1":
                    capslock_shiftAltGr[int(kl['scancode'], 16)] = handleChr(kl['scancode'], kl['shiftAltGr'])
        else:
            if int(kl['isCapsLock'], 16) & 4:
                    if kl['altGr'] != u"-1":
                        capslock_shiftAltGr[int(kl['scancode'], 16)] = handleChr(kl['scancode'], kl['altGr'])
            else:
                if 'shiftAltGr' in kl:
                    if kl['shiftAltGr'] != u"-1":
                        capslock_shiftAltGr[int(kl['scancode'], 16)] = handleChr(kl['scancode'], kl['shiftAltGr'])

# END FUNCTION - prepareCapslock_shiftAltGr


#===============================================================================
def prepareCtrl(f):
#===============================================================================
    for kl in kls:
        if 'ctrl' in kl:
            if kl['ctrl'] != u"-1":
                ctrl[int(kl['scancode'], 16)]  = handleChr(kl['scancode'], kl['ctrl'])

# END FUNCTION - prepareCtrl


#===============================================================================
def writeDeadkeys(f, deadkeys):
#===============================================================================
    if len(deadkeys) == 0:
        line = u"\nconst Keylayout::dkey_t * const deadkeys = nullptr;\n"
        line += u"\nconst static uint8_t nbDeadkeys = 0;\n"
        f.write(line.encode(u'utf-8'))
        return

    line = u"\nconst Keylayout::dkey_t deadkeys[] = {acco}\n".format(acco = acco1)
    f.write(line.encode(u'utf-8'))

    nbDks = 0
    for dk in deadkeys:
        nbDks = nbDks + 1
        baseline = u"    {acco1} 0x{uchar}, 0x{scan}, {size:>2}, {acco1} {acco1}".format( acco1 = acco1
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
                line = u"{base}0x{car1}, 0x{car2}{acco2}  // '{car11}' = '{car12}'\n".format( acco1 = acco1
                                                                  , acco2 = acco2
                                                                  , car1 = key
                                                                  , car2 = dk[u'keylist'][key]
                                                                  , car11 = unichr(int(key, 16))
                                                                  , car12 = unichr(int(dk[u'keylist'][key], 16))
                                                                  , base = baseline
                                                                  )
                f.write(line.encode(u'utf-8'))
            else:
                line = u"                        , {acco1}0x{car1}, 0x{car2}{acco2}  // '{car11}' = '{car12}'\n".format( acco1 = acco1
                                                                                               , acco2 = acco2
                                                                                               , car1 = key
                                                                                               , car2 = dk[u'keylist'][key]
                                                                                               , car11 = unichr(int(key, 16))
                                                                                               , car12 = unichr(int(dk[u'keylist'][key], 16))
                                                                                               )
                f.write(line.encode(u'utf-8'))

        line = u"                        {acco}\n".format(acco = acco2)
        f.write(line.encode(u'utf-8'))

        line = u"    {acco},\n".format(acco = acco2)
        f.write(line.encode(u'utf-8'))

#    # List must contains a fixed number of elements. So, fill up with neutral entries
#    for i in range(nbDks, 35):
#        line = u"    {acco1} 0x00, 0x00, 0, {acco1}{acco2} {acco2},\n".format( acco1 = acco1
#                                                                             , acco2 = acco2
#                                                                             )
#        f.write(line.encode(u'utf-8'))

    line = u"{acco};\n".format(acco = acco2)
    f.write(line.encode(u'utf-8'))

    line = u"\nconst static uint8_t nbDeadkeys = {nbdk};\n".format(nbdk = nbDks)
    f.write(line.encode(u'utf-8'))


# END FUNCTION - writeDeadkeys


#===============================================================================
def writeInstance(f):
#===============================================================================
    line = u"\nstatic const Keylayout keylayout_x{locid}( x{locid}::LCID\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::locale_name\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::noMod\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::shift\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::altGr\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::shiftAltGr\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::ctrl\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::capslock_noMod\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::capslock_shift\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::capslock_altGr\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::capslock_shiftAltGr\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::deadkeys\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u"                                          , x{locid}::nbDeadkeys\n".format(locid = localeId)
    f.write(line.encode(u'utf-8'))
    line = u");\n"
    f.write(line.encode(u'utf-8'))

# END FUNCTION - writeInstance


########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################
########################################################################################################################


##############################################################
# MAIN
##############################################################

#=============================================================
# Browse source keyboard layout files
#=============================================================
for keymap,locale_prefix in [ (u'./locale/belgian_french.klc', ".fr")
                            , (u'./locale/belgian_comma.klc', "")
                            , (u'./locale/belgian_period.klc', "")
                            , (u'./locale/bosnian.klc', "")
                            , (u'./locale/bulgarian.klc', "")
                            , (u'./locale/bulgarian_latin.klc', ".latin")
                            , (u'./locale/canadian_french.klc', ".fr")
                            , (u'./locale/canadian_french_legacy.klc', "")
                            , (u'./locale/canadian_multilingual.klc', ".multilingual")
                            , (u'./locale/croatian.klc', "")
                            , (u'./locale/czech.klc', "")
                            , (u'./locale/czech_programmers.klc', ".programmers")
                            , (u'./locale/czech_qwerty.klc', ".qwerty")
                            , (u'./locale/danish.klc', "")
                            , (u'./locale/dutch.klc', "")
                            , (u'./locale/estonian.klc', "")
                            , (u'./locale/faeroese.klc', "")
                            , (u'./locale/finnish.klc', ".finnish")
                            , (u'./locale/finnish_sami.klc', "")
                            , (u'./locale/french.klc', "")
                            , (u'./locale/fyro_macedonian.klc', "")
                            , (u'./locale/gaelic.klc', "")
                            , (u'./locale/german_ibm.klc', ".ibm")
                            , (u'./locale/german.klc', "")
                            , (u'./locale/greek.klc', "")
                            , (u'./locale/greek_220.klc', ".220")
                            , (u'./locale/greek_220_latin.klc', ".220_latin")
                            , (u'./locale/greek_319.klc', ".319")
                            , (u'./locale/greek_319_latin.klc', ".319_latin")
                            , (u'./locale/greek_latin.klc', ".latin")
                            , (u'./locale/greek_polytonic.klc', ".polytonic")
                            , (u'./locale/hungarian_101.klc', "")
                            , (u'./locale/icelandic.klc', "")
                            , (u'./locale/inuktitut_latin.klc', "")
                            , (u'./locale/irish.klc', ".irish")
                            , (u'./locale/kazakh.klc', "")
                            , (u'./locale/kyrgyz_cyrillic.klc', "")
                            , (u'./locale/latin_american.klc', "")
                            , (u'./locale/latvian.klc', "")
                            , (u'./locale/latvian_qwerty.klc', ".qwerty")
                            , (u'./locale/lithuanian.klc', "")
                            , (u'./locale/lithuanian_ibm.klc', ".ibm")
                            , (u'./locale/italian_142.klc', ".142")
                            , (u'./locale/italian.klc', "")
                            , (u'./locale/luxembourgish.klc', "")
                            , (u'./locale/maltese_47_key.klc', ".47")
                            , (u'./locale/maltese_48_key.klc', ".48")
                            , (u'./locale/maori.klc', "")
                            , (u'./locale/mongolian_cyrillic.klc', "")
                            , (u'./locale/norwegian.klc', "")
                            , (u'./locale/norwegian_sami.klc', "")
                            , (u'./locale/polish_214.klc', "")
                            , (u'./locale/polish_programmers.klc', ".programmers")
                            , (u'./locale/portuguese.klc', "")
                            , (u'./locale/portuguese_brazilian_abnt.klc', ".abnt")
                            , (u'./locale/portuguese_brazilian_abnt2.klc', ".abnt2")
                            , (u'./locale/romanian.klc', "")
                            , (u'./locale/russian_typewriter.klc', ".typewriter")
                            , (u'./locale/russian.klc', "")
                            , (u'./locale/sami_ext_finland_sweden.klc', ".ext_finland_sweden")
                            , (u'./locale/sami_ext_norway.klc', ".ext_norway")
                            , (u'./locale/serbian_cyrillic.klc', "")
                            , (u'./locale/serbian_latin.klc', "")
                            , (u'./locale/slovak.klc', "")
                            , (u'./locale/slovak_qwerty.klc', ".qwerty")
                            , (u'./locale/slovenian.klc', "")
                            , (u'./locale/spanish.klc', "")
                            , (u'./locale/spanish_variation.klc', ".variation")
                            , (u'./locale/swedish_sami.klc', "")
                            , (u'./locale/swedish.klc', "")
                            , (u'./locale/swiss_french.klc', "")
                            , (u'./locale/swiss_german.klc', "")
                            , (u'./locale/tatar.klc', "")
                            , (u'./locale/turkish_f.klc', ".f")
                            , (u'./locale/turkish_q.klc', ".q")
                            , (u'./locale/uk_extended.klc', "")
                            , (u'./locale/uk.klc', "")
                            , (u'./locale/ukrainian.klc', "")
                            , (u'./locale/us_international.klc', ".international")
                            , (u'./locale/us.klc', "")
                            , (u'./locale/us_dvorak.klc', ".dvorak")
                            , (u'./locale/us_dvorak_left.klc', ".dvorak_left")
                            , (u'./locale/us_dvorak_right.klc', ".dvorak_right")
                            , (u'./locale/uzbek_cyrillic.klc', "")
                            ]:

    #==============================================================
    # LOCALE CONTAINER variables
    #=============================================================
    # locale tag
    locale = None
    # locale code
    localeId = None
    # locale labels
    localeTag  = None
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
    reKL = u'([0-9]{1}[0-9a-f]{1})\t[^\t]*\t*([0-5]{1}|SGCap)\t*([^\t]*)\t*'
    # Dynamic flag indicating current operation
    state = u"LOCALE"         # first value is always "LOCALE"
    deadkeyCode = u"ffff"     # first value is set arbitrarily to "ffff" to allow detection of new deadkey code
    oldDeadkeyCode = u"ffff"  # previous value of deadkeyCode to allow detection of new deadkey code
    SGCap = False

#    print u"\n>>>> LOCALE\n"

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
#            print u"\n>>>> FLAG SHIFTSTATE FOUND\n"
            continue
        #------------------------------------------------------------
        # FLAG LAYOUT
        #------------------------------------------------------------
        matchObj = re.match( u'^LAYOUT.*', line)
        if matchObj:
            state = u"LAYOUT"
#            print u"\n>>>> FLAG LAYOUT FOUND\n"
            continue
        #------------------------------------------------------------
        # FLAG DEADKEY
        #------------------------------------------------------------
        matchObj = re.match( u'^DEADKEY\t([0-9a-f]{4})', line)
        if matchObj:
            state = u"DEADKEY"
            deadkeyCode = matchObj.group(1)
#            print u"\n>>>> FLAG DEADKEY FOUND : ", deadkeyCode, "\n"
            continue
        #------------------------------------------------------------
        # FLAG KEYNAME
        #------------------------------------------------------------
        matchObj = re.match( u'^KEYNAME.*', line)
        if matchObj:
            state = u"KEYNAME"
#            print u"\n>>>> FLAG KEYNAME FOUND\n"
            continue
        #------------------------------------------------------------
        # FLAG DESCRIPTIONS
        #------------------------------------------------------------
        matchObj = re.match( u'^DESCRIPTIONS.*', line)
        if matchObj:
            state = u"DESCRIPTIONS"
#            print u"\n>>>> FLAG DESCRIPTIONS FOUND\n"
            continue
        #------------------------------------------------------------
        # FLAG LANGUAGENAMES
        #------------------------------------------------------------
        matchObj = re.match( u'^LANGUAGENAMES.*', line)
        if matchObj:
            state = u"LANGUAGENAMES"
#            print u"\n>>>> FLAG LANGUAGENAMES FOUND\n"
            continue
        #------------------------------------------------------------
        # FLAG ENDKBD
        #------------------------------------------------------------
        matchObj = re.match( u'^ENDKBD.*', line)
        if matchObj:
            state = u"KEYNAME"
#            print u"\n>>>> FLAG ENDKBD FOUND\n"
            break

        if state == u"LOCALE":
#            print line
            #------------------------------------------------------------
            # LOCALE NAME
            #------------------------------------------------------------
            matchObj = re.match( u'LOCALENAME\t"(.{5})', line)
            if matchObj:
                locale = matchObj.group(1)
                print u"--------                ------------------------------------"
                print u"locale : >%s<" % (locale),
                continue
            #------------------------------------------------------------
            # LOCALE ID
            #------------------------------------------------------------
            matchObj = re.match( u'LOCALEID\t"(.{8})', line)
            if matchObj:
                localeId = matchObj.group(1)
                print u" - locale ID : >%s<" % (localeId),
                continue

        if state == u"SHIFTSTATE":
            #------------------------------------------------------------
            # SHIFSTATE 1 (001b)
            #------------------------------------------------------------
            matchObj = re.match( u'(1)\t//Column.*', line)
            if matchObj:
                shifts.append(u"1")
                reKL = u"%s([^\t]*)\t*" % (reKL)
#                print u"    state 1"
                continue
            #------------------------------------------------------------
            # SHIFSTATE 2 (010b)
            #------------------------------------------------------------
            matchObj = re.match( u'(2)\t//Column.*', line)
            if matchObj:
                shifts.append(u"2")
                reKL = u"%s([^\t]*)\t*" % (reKL)
#                print u"    state 2"
                continue
            #------------------------------------------------------------
            # SHIFSTATE 6 (110b)
            #------------------------------------------------------------
            matchObj = re.match( u'(6)\t//Column.*', line)
            if matchObj:
                shifts.append(u"6")
                reKL = "%s([^\t]*)\t*" % (reKL)
#                print u"    state 6"
                continue
            #------------------------------------------------------------
            # SHIFSTATE 7 (111b)
            #------------------------------------------------------------
            matchObj = re.match( u'(7)\t//Column.*', line)
            if matchObj:
                shifts.append(u"7")
                reKL = u"%s([^\t]*)\t*" % (reKL)
#                print u"    state 7"
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
                    klcar[u'capslock_SGCap']   = matchObj.group(1)
                    testgrp2 = re.match( u'([0-9a-f]{4}.*)', matchObj.group(2))
                    if testgrp2:
                        klcar[u'shiftCapslock_SGCap'] = testgrp2.group(1)
#                    klcar[u'shiftCapslock_SGCap'] = matchObj.group(2)
                    kls.append(klcar)
                continue

            # make custom regex depending on the number of columns
            matchObj = re.match( reKL, line)
            if matchObj:
                keylayout = matchObj.group()
                klcar = {}
                klcar[u'scancode']   = matchObj.group(1)
                klcar[u'isCapsLock'] = matchObj.group(2)
                klcar[u'noMod']    = matchObj.group(3)

                # shift state counter (first value is always '4'; further on, increases each time a new state is found
                numState = 4

                for s in shifts:
                    if s == u"1":
                        klcar[u'shift'] = matchObj.group(numState)
                    if s == u"2":
                        klcar[u'ctrl'] = matchObj.group(numState)
                    if s == u"6":
                        klcar[u'altGr'] = matchObj.group(numState)
                    if s == u"7":
                        klcar[u'shiftAltGr'] = matchObj.group(numState)

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

        if state == u"DESCRIPTIONS":
            #------------------------------------------------------------
            # DESCRIPTIONS LINE
            #------------------------------------------------------------
            matchObj = re.match( u'([0-9a-f]*)\t(.*) - Custom', line)
            if matchObj:
                localeDesc = matchObj.group(2)
                print "-", localeDesc,

        if state == u"LANGUAGENAMES":
            #------------------------------------------------------------
            # LANGUAGENAMES LINE
            #------------------------------------------------------------
            matchObj = re.match( u'([0-9a-f]*)\t(.*)', line)
            if matchObj:
                localeTag = matchObj.group(2)
                print "-", localeTag

#    print kls

    #=============================================================
    # PHASE 2 : Use the extracted data to create the .hpp file
    #=============================================================
    filename = u"./keylayout_x{locid}.hpp".format(locid = localeId)
    f = open(filename, u'w')
    writeLicense(f)
    writeHeader(f, locale_prefix)

    razMaps()

    # Prepare those blocks that always exists :
    prepareNoMod(f)
    prepareCapslock_NoMod(f)

    # Prepare those blocks only if they exists (an to keep an order that is variable from one locale to another) :
    for s in shifts:
        if s == u"1":
            prepareShift(f)
            prepareCapslock_shift(f)
        if s == u"2":
            prepareCtrl(f)
        if s == u"6":
            prepareAltGr(f)      # idem ctrlAlt
            prepareCapslock_altGr(f)
        if s == u"7":
            prepareShiftAltGr(f) # idem shitfCtrlAlt
            prepareCapslock_shiftAltGr(f)

    # Write prepared blocks to file
    writeLayout(f, noMod, u'noMod')
    writeLayout(f, shift, u'shift')
    writeLayout(f, altGr, u'altGr')
    writeLayout(f, shiftAltGr, u'shiftAltGr')
    writeLayout(f, ctrl,  u'ctrl')
    writeLayout(f, capslock_noMod, u'capslock_noMod')
    writeLayout(f, capslock_shift, u'capslock_shift')
    writeLayout(f, capslock_altGr, u'capslock_altGr')
    writeLayout(f, capslock_shiftAltGr, u'capslock_shiftAltGr')

    writeDeadkeys(f, dks)
    writeCloseNS(f)
    writeInstance(f)
    writeTrailer(f)

    f.close()


# END SCRIPT
