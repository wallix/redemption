#! /usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import shutil
import subprocess
import os
import re

def execute(command):
    if os.system(command) != 0:
        raise BuildError("%s failed." % command)

if __name__ == '__main__':

    try:
        try:
            shutil.rmtree("debian")
        except:
            pass
        
        # Set debian (packaging data) directory with distro specific packaging files
        shutil.copytree("packaging/ubuntu/12.04/debian", "debian")
        shutil.copy("docs/copyright", "debian/copyright")
        
        found = False
        for line in open("main/version.hpp"):
            res = re.match('^[#]define\sVERSION\s["](((\d+)[.](\d+)[.](\d+))(-[a-z]*)*)["]\s*$', line)
            if res:
                red_source_version = res.group(1)
                red_num_ver = res.group(2)
                bas_ver = res.group(3)
                rel_ver = res.group(4)
                fix_ver = res.group(5)
                found = True
                break

        if not found:
            raise Exception('Source Version not found in file main/version.hpp')
        package_name = "redemption_%s_amd64.deb" % red_source_version

        found = False
        for line in open("debian/changelog"):
            res = re.match('^redemption\s*[(](((\d+)[.](\d+)[.](\d+))(-[a-z]*)*)(.*)[)].*$', line)
            if res:
                changelog_red_source_version = res.group(1)
                changelog_red_num_ver = res.group(2)
                changelog_bas_ver = res.group(3)
                changelog_rel_ver = res.group(4)
                changelog_fix_ver = res.group(5)
                changelog_suffix_ver = res.group(6)
                found = True
                break

        if not found:
            raise Exception('Source Version not found in debian/changelog')

        if changelog_red_source_version != red_source_version:
            raise Exception('Version mismatch between changelog and main/version ("%s" != "%s")' % (
                changelog_red_source_version, red_source_version))
            
        package_name = "redemption_%s%s.deb" % (red_source_version, changelog_suffix_ver)

        # Create changelog from git history
        
        
        #execute("DESTDIR=debian/tmp bjam -a install")
        path = os.getcwd() #pushd
        #execute("dch --create -v %s --package %s --empty" % (version, "redemption"))
        execute("dpkg-buildpackage -b -tc -us -uc -r")
        exit(0)
    except Exception, e:
        print "Build failed: %s" % e
        exit(-1)

