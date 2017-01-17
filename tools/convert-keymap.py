#!/usr/bin/python


import sys
import re
#keymap = sys.argv[1]


for keymap in ['sys/etc/rdpproxy/km-0407.ini',
                'sys/etc/rdpproxy/km-0409.ini',
                'sys/etc/rdpproxy/km-040c.ini',
                'sys/etc/rdpproxy/km-0410.ini',
                'sys/etc/rdpproxy/km-0419.ini',
                'sys/etc/rdpproxy/km-041d.ini',
                'sys/etc/rdpproxy/km-046e.ini',
                'sys/etc/rdpproxy/km-0807.ini',
                'sys/etc/rdpproxy/km-0809.ini',
                'sys/etc/rdpproxy/km-080c.ini',
                'sys/etc/rdpproxy/km-0813.ini',
                'sys/etc/rdpproxy/km-100c.ini'
                ]:
    res =  re.match(".*km-(.*)[.]ini", keymap)
#    if not res:
#        print "%s is not a xrdp keymap file" % keymap
#        exit(0)

    keylayout = res.group(1)
    print "case 0x%s:\n{" % keylayout

    mapping = {}

    for name in ["shift", "noshift", "capslock", "altgr", "shiftcapslock"]:
        if mapping.get(name) is None:
            mapping[name] = {}
        for k in range(0, 127):
            mapping[name][k] = (0, 0)

    state = None
    for line in file(keymap):
        res =  re.match(r"\[(.*)\]", line)
        if res:
            state = res.group(1)
            continue

        res =  re.match("Key", line)
        if res:
            res =  re.match(r"Key(\d+)=(\d+):(\d+)", line)
            if res:
                keynum = int(res.group(1))
                keysym = int(res.group(2))
                keychar = int(res.group(3))
#                print state, keynum, keysym, keychar
                mapping[state][keynum] = (keysym, keychar)
                continue
            else:
                raise Exception("Regex Key... should match at %s" % line)

#        print "%s : %s" % (state, line)

    for code in mapping:
        print "const int keylayout_%s_%s[128] = {" % (keylayout, code)

        for r in [range(x, x+8) for x in range(0, 120, 8)]:
            print "/* {char: >3} */ ".format(char=r[0]),
            for num in r:
                print "{char: >6},".format(char=hex(mapping[code][num][1])),
            print
        print "};"

    print ("""
    for(size_t i = 0 ; i < 128 ; i++) {accopen}
        if (keylayout_{l}_noshift[i]){accopen}
            keylayout_WORK_noshift[i] = keylayout_{l}_noshift[i] ;
        {accclose}
        if (keylayout_{l}_shift[i]){accopen}
            keylayout_WORK_shift[i] = keylayout_{l}_shift[i] ;
        {accclose}
        if (keylayout_{l}_altgr[i]){accopen}
            keylayout_WORK_altgr[i] = keylayout_{l}_altgr[i] ;
        {accclose}
        if (keylayout_{l}_capslock[i]){accopen}
            keylayout_WORK_capslock[i] = keylayout_{l}_capslock[i] ;
        {accclose}
        if (keylayout_{l}_shiftcapslock[i]){accopen}
            keylayout_WORK_shiftcapslock[i] = keylayout_{l}_shiftcapslock[i] ;
        {accclose}
    {accclose}
    """.format(l=keylayout, accopen=r'{', accclose=r'}'))

    print "}\nbreak;"
