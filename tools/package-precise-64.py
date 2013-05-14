#! /usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import shutil
import subprocess
import os

def execute(command):
    if os.system(command) != 0:
        raise BuildError("%s failed." % command)

if __name__ == '__main__':

    try:
        package_name = "redemption_0.9.247-1_amd64.deb"
        shutil.rmtree("debian")
        shutil.copytree("packaging/ubuntu/12.04/debian", "debian")
        #execute("DESTDIR=debian/tmp bjam -a install")
        path = os.getcwd() #pushd
        #execute("dch --create -v %s --package %s --empty" % (version, "redemption"))
        execute("dpkg-buildpackage -b -tc -us -uc -r")
        exit(0)
    except Exception, e:
        print "Build failed: %s" % e
        exit(-1)

