#! /usr/bin/env python
# -*- coding: utf-8 -*-

import cmd
import sys
from optparse import OptionParser # Old python, old optparse
import os.path
import shutil
import subprocess
import re
import platform
import os
import time

def get_distro_version():
    f = open("/etc/issue")
    issue = f.read(50)
    f.close()
    res = re.match(r"([A-Za-z]+)[^0-9]+([0-9]+([.][0-9]+)?)", issue)
    dist, version, name, arch = "Unknown", "0", "unknown", "comodore"
    if res:
        dist, version = res.group(1), res.group(2)
        print dist, version
        if dist == 'Debian':
            if version in ['5', '5.0']:
                version = '5'
                name = 'lenny'
            elif version in ['6', '6.0']:
                version = '6'
                name = 'squeeze'
        elif dist == 'Ubuntu':
            if version == '10.04':
                name = 'lucid'
            if version == '10.10':
                name = 'maverick'
            if version == '11.04':
                name = 'natty'
    else:
        words = re.split("\W", issue)
        if "squeeze" in words:
            dist, version, name = "Debian", "6", "squeeze"
        elif "wheezy" in words:
            dist, version, name = "Debian", "7", "wheezy"

    if (platform.uname()[4] == 'x86_64'):
        arch = "amd64"
    else:
        arch = "i386"

    return dist, version, name, arch


DIST, DISTVERSION, DISTCODENAME, ARCHITECTURE = get_distro_version()

BUILDPREFIX="/tmp/redemption-build"
LOGFILE="%s/redemption-build.log" % BUILDPREFIX
GITBASEURL="git@git:redemption-wab.9.x"
REDEMPTIONNAME="redemption"
TARGETBUILD="%s/target/%s/%s" % (os.path.abspath(os.path.dirname(__file__)), DIST, DISTVERSION)

class BuildError(Exception): pass

# arguments etc.
parser = OptionParser()

parser.add_option('-T', '--no-checkout',   action='store_true',
        help='Don\' tweak build prefix, just build it')
parser.add_option('-t', '--tag',      dest='git_tag',  action='store',
        help='Revision tag number to checkout, if missing, HEAD will be checked out')
parser.add_option('-C', '--no-clone', action='store_true',
        help="Don't clone the repo, use faith in human kind to copy files at the right place")
parser.add_option('-c', '--clone',    action='store_true',
        help="Do clone the repo, don't use faith in human kind to copy files at the right place")
parser.add_option('-p', '--build_prefix',    dest='build_prefix', action='store',
        help="Mandatory! You must give a build prefix to build from command line")


def execute(command):
    if os.system(command) != 0:
        raise BuildError("%s failed." % command)

def build_redemption_package(targetbuild, buildprefix, version, architecture=ARCHITECTURE):
    try:
        shutil.rmtree("%s/debian" % buildprefix)
    except Exception, e:
        print("Can't remove %s/debian" % buildprefix)
        pass

    # Copy template build files to buildprefix
    shutil.copytree("%s/debian" % targetbuild, "%s/debian" % buildprefix)

    # commented: temporary for testing purpose without having to change svn
    shutil.copy("%s/redemptionbuild.py" % targetbuild, "%s/redemptionbuild.py" % buildprefix)

    try:
        os.mkdir("%s/debian/tmp" % buildprefix)
    except Exception, e:
        print("Can't create dir %s/debian/tmp" % buildprefix)
        pass

    # then remplace templated constants with actual values
    for fname in ["debian/rules", "redemptionbuild.py", "debian/control"]:
        fullname = "%s/%s" % (buildprefix, fname)
        f = open(fullname, "r")
        fullfile = f.read(8192)
        f.close()
        base_name = 'redemption'
        for key, replacement in [('<!redemptionname!>', base_name),
                             ('<!redemptionver!>', version),
                             ('<!builddir!>', buildprefix),
                             ('<!redemptioninstallprefix!>', '/opt/wab'),
                             ('<!architecture!>', architecture)]:
            fullfile = re.sub(key, replacement, fullfile)
        f = open(fullname, "w")
        f.write(fullfile)
        f.close()

    # Now we are ready to build the debian package
    #TODO: we still have to figure out the correct version and package name from parameters
    path = os.getcwd()
    os.chdir(buildprefix)
    package_name = "%s/../%s_%s_%s.deb" % (buildprefix, base_name, version, architecture)
    try:
        execute("dch --create -v %s --package %s --empty" % (version, base_name))
        execute("dpkg-buildpackage -b -tc -us -uc -r")
    except Exception, e:
        if not os.path.exists(package_name):
            print ("Failed to create package %s (%s)" % (package_name, e))
            return
        else:
            print ("Package created anyway.")
    os.chdir(path)
    target_package = "%s/../packages/%s_%s_%s.deb" % (buildprefix, base_name, version, architecture)
    shutil.move(package_name, target_package)
    print ("Package %s seems fine, build went OK." % target_package)


# Core object for redemption building, used both by arguments and CLI
class RedemptionCore():
    def __init__(self):
        self.tag_ver = str(time.time())[:7]
        self.build_suceeded = False

    def get_tag_ver(self, buildprefix):
        if self.tag_ver:
            return self.tag_ver
        else:
            pattern = re.compile(r"#define VERSION \"(.*)\"", re.S)
            with open("%s/main/version.hpp" % buildprefix) as f:
                ma = ""
                for line in f:
                    ma = pattern.search(line)
                    if ma:
                       break
                self.tag_ver = ma.group(1)
            #execute("""cat main/version.hpp | grep "#define VERSION" | sed -e 's/#define VERSION \"\(.*\)\"/\1/'""")
        return self.tag_ver

    def set_tag_ver(self, tag):
        self.tag_ver = tag

    def git_clone(self):
        execute("git clone -q %s %s" % (GITBASEURL, BUILDPREFIX))
        execute("cd %s; git submodule --quiet init" % BUILDPREFIX)
        execute("cd %s; git submodule --quiet update" % BUILDPREFIX)

    def git_checkout(self, git_tag="HEAD"):
        if git_tag != "HEAD":
            execute("cd %s; git checkout %s" % (BUILDPREFIX, git_tag))
        else:
            execute("cd %s; git checkout HEAD" % BUILDPREFIX)

        execute("cd %s; git submodule --quiet update" % BUILDPREFIX)

    def build(self):
        try:
            build_redemption_package(TARGETBUILD, BUILDPREFIX, self.tag_ver+"-"+DISTCODENAME+'.wallix1%s'%os.environ.get('REDEMPTION_VARIANT',''))
            self.build_suceeded = True
            return True
        except Exception, e:
            print "Build failed: %s" % e
            return False

    def clean_build_directory(self):
        if os.path.exists(BUILDPREFIX):
            shutil.rmtree(BUILDPREFIX)
        print "Build Target architecture is %s (%s)... processing" % (DIST, ARCHITECTURE)



# CLI interface for redemption builder script, 'ay 'umans t'is for you.
class RedemptionCLI(cmd.Cmd):
    """Command Line Redemption Build Tool."""
    def __init__(self):
        cmd.Cmd.__init__(self)
        self.redemptionCore = RedemptionCore()
        self.checkout_done  = False
        self.build_done     = False

    def do_info(self, line):
        print 'target           : %s %s "%s" (%s)' % get_distro_version()
        print "version to build : %s" % self.redemptionCore.tag_ver
        print "package to build : redemption_%s_%s.deb" % (self.redemptionCore.tag_ver+"-"+DISTCODENAME, ARCHITECTURE)
        print "build prefix     : %s" % (BUILDPREFIX)

    def help_checkout(self):
        print (self.do_checkout.__doc__ + "\n" + "Usage: checkout [version] (ex: checkout 0.3.1)")

    def do_checkout(self, line):
        """Get redemption version from git"""
        args = line.split()

        if len(args) < 1:
            print "You must provide tag revision number after checkout, like: 'checkout 0.3'"
            return

        self.redemptionCore.clean_build_directory()
        self.redemptionCore.set_tag_ver(args[0])

        # We can do a checkout with a tag_version and the distant repo.
        self.redemptionCore.git_clone()
        self.redemptionCore.git_checkout(args[0])

        self.checkout_done = True

    def do_build(self, line):
        print(self.redemptionCore.get_tag_ver(BUILDPREFIX))
        if not self.checkout_done:
            print "You are building hoping that someone did the checkout for you."
        if not os.path.exists(TARGETBUILD):
            print "Build target %s is missing" % TARGETBUILD
            return

        # Build does not need arguments
        self.redemptionCore.build()

        self.build_done = True

    def do_exit(self, line):
        exit(0)

    def do_EOF(self, line):
        return True

    def postcmd(self, stop, line):
        return stop

# This script is called, it must react!
if __name__ == '__main__':

    # If there are any arguments, we are in CLI mode
    if len(sys.argv) > 1:
        (options, args) = parser.parse_args()
        redemptionCore = RedemptionCore()

        # Altering path BUILDPREFIX
        if options.build_prefix:
            BUILDPREFIX = options.build_prefix
        else:
            print("You MUST give a prefix!")
            raise RuntimeError


        # This is silly, it will result in a 'no clone'
        if (options.clone and options.no_clone):
            print("Be serious, I will choose for you then")

        # The user don't want a clone
        if   options.no_clone:
            # If no path, it doesn't make sense
            if os.path.exists(BUILDPREFIX):
                if options.git_tag:
                    redemptionCore.set_tag_ver(options.git_tag)
                    print("You asked the tag to be %s" % redemptionCore.get_tag_ver(BUILDPREFIX))
                    redemptionCore.git_checkout(options.git_tag)
                elif not options.no_checkout:
                    redemptionCore.git_checkout()
                    print("I guessed the tag to be %s" % redemptionCore.get_tag_ver(BUILDPREFIX))
                redemptionCore.build()
            else:
                print("No source at the given path, not building anything")

        # The user do want a clone
        elif options.clone:
            redemptionCore.clean_build_directory()
            if options.git_tag:
                redemptionCore.set_tag_ver(options.git_tag)
                redemptionCore.git_clone()
                redemptionCore.git_checkout(options.git_tag)
                print("You asked the tag to be %s" % redemptionCore.get_tag_ver(BUILDPREFIX))
            else:
                redemptionCore.git_clone()
                print("I guessed the tag to be %s" % redemptionCore.get_tag_ver(BUILDPREFIX))

            # Build does not need arguments
            redemptionCore.build()

        else:
            if options.git_tag:
                print("You must specify either clone or no clone")

    # Otherwise we just fire up the command loop (you human, you)
    else:
        print('No option given, falling back in interactive mode')
        RedemptionCLI().cmdloop()
