#!/usr/bin/python

import sys
import os

class BuildError(Exception): pass

#ETC_PREFIX=/etc/opt/wab/rdp PREFIX=/opt/wab bjam
def build(builddir):
    os.chdir(builddir)
    os.environ["ETC_PREFIX"] = "/etc/opt/wab/rdp"
    os.environ["PREFIX"] = "/opt/wab"
    os.system("bjam %s" % os.environ.get('REDEMPTION_VARIANT', ''))

#ETC_PREFIX=/etc/opt/wab/rdp PREFIX=/opt/wab bjam install
def install(debprefix, builddir):
    os.chdir(builddir)
    os.environ["ETC_PREFIX"] = "%s/etc/opt/wab/rdp" % debprefix
    os.environ["PREFIX"] = "%s/opt/wab" % debprefix
    os.system("bjam %s install" % os.environ.get('REDEMPTION_VARIANT', ''))

if __name__ == '__main__':

    DEBPREFIX=sys.argv[1]
    REDEMPTIONINSTALLPREFIX="%s/%s" % (DEBPREFIX, "<!redemptioninstallprefix!>")

    try:
        build("<!builddir!>")
        install(DEBPREFIX, "<!builddir!>")
    except Exception, e:
        print "%s" % e
        sys.exit(1)
