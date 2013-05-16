#! /usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import shutil
import subprocess
import os
import re

if __name__ == '__main__':

    try:
        try:
            shutil.rmtree("debian")
        except:
            pass
        
        # Set debian (packaging data) directory with distro specific packaging files
        shutil.copytree("packaging/debian/squeeze/debian", "debian")
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
        package_name = "redemption_%s_i386.deb" % red_source_version

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

        res = subprocess.Popen(["git", "describe", "--tags"], stdout=subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]
        tag_describe = res.split("\n")[0]
        if red_source_version != tag_describe:
            raise Exception('Repository head mismatch current version ("%s" != "%s"), please tag current version before building packet' % (
                red_source_version, tag_describe))

        res = subprocess.Popen(["git", "diff", "--shortstat"], stdout = subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]
        if res:
            raise Exception('Your repository has uncommited changes ("%s"), please commit before packaging' % (res))
            
        os.system("dpkg-buildpackage -b -tc -us -uc -r")
        exit(0)
    except Exception, e:
        print "Build failed: %s" % e
        exit(-1)

