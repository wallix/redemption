#! /usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import shutil
import subprocess
import os
import re
import distroinfo

if __name__ == '__main__':
    try:

        uninstall_option = False
        if 'uninstall' in sys.argv:
            uninstall_option = True

        distro, dummy, codename = distroinfo.get_distro()

        sections = []
        section = {}
        current = None
        count = 0
        for line in open("packaging/%s/%s/debian/control" % (distro, codename)):
            res = re.match(r'^(?:([A-Za-z0-9-]+[:])\s*([^\r\n]+)\r*|(\s*)\r*$|([^\r\n]+)\r*)$', line)
            if res.group(1):
                current = res.group(1)
                section[current] = res.group(2)
            if current and res.group(4):
                section[current] += res.group(4)
            if res.group(3):
                sections.append(section)
                section = {}
                current = None
        sections.append(section)

        for d in sections:
            if d.get('Source:') == 'redemption':
                
                print  d.get('Build-Depends:')
                
                depends = re.split(r'\s*(?:[(].*?[)])*\s*[,]\s*', d.get('Build-Depends:'))
                
                if uninstall_option:
                    os.system("apt-get remove %s" % ' '.join(depends))
                else:
                    os.system("apt-get install %s" % ' '.join(depends))
                exit(0)

    except Exception, e:
        print "Dependency install failed %s" % e
        exit(-1)
