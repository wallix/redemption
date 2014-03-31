#!/usr/bin/env python
# -*- coding: utf-8 -*-

import getopt
import sys
import distroinfo
import subprocess
import shutil
import os
import datetime
import re

def usage():
  print("Usage: %s [-h|--help] [--prefix path] [--etc-prefix path] [--cert-prefix path] [--package-distribution name] [--not-entry-changelog] [--force-distro name] [--force--distro-codename name] [--debug] --tag version" % sys.argv[0])

try:
  opts, args = getopt.getopt(sys.argv[1:], "h",
                             ["help", "package-distribution=", "tag=",
                              "prefix=", "etc-prefix=", "cert-prefix=",
                              "not-entry-changelog", "force-distro=",
                              "force--distro-codename=", "debug"])
except getopt.GetoptError as err:
  print(str(err))
  usage()
  sys.exit(2)

tag = None
prefix = '/usr/local'
etc_prefix = '/etc/rdpproxy'
cert_prefix = '/etc/rdpproxy/cert'
package_distribution = 'unstable'
debug = False
entry_changelog = True
force_distro = None
force_distro_codename = None

for o,a in opts:
  if o in ("-h", "--help"):
    usage()
    sys.exit()
  elif o == "--tag":
    tag = a
  elif o == "--package-distribution":
    package_distribution = a
  elif o == "--prefix":
    prefix = a
  elif o == "--etc-prefix":
    etc_prefix = a
  elif o == "--cert-prefix":
    cert_prefix = a
  elif o == "--not-entry-changelog":
    entry_changelog = False
  elif o == "--force-distro":
    force_distro = a
  elif o == "--force--distro-codename":
    force_distro_codename = a
  elif o == "--debug":
    debug = True

if tag == None:
  usage()
  sys.exit(1)


try:
  shutil.rmtree("debian")
except:
  pass

def readall(filename):
  with open(filename) as f:
    return f.read()

def writeall(filename, s):
  with open(filename, "w+") as f:
    f.write(s)

try:
  res = subprocess.Popen(["git", "diff", "--shortstat"],
                         stdout = subprocess.PIPE,
                         stderr = subprocess.STDOUT
                        ).communicate()[0]
  if res:
    raise Exception('Your repository has uncommited changes:\n%sPlease commit before packaging.' % (res))

  out = readall("main/version.hpp")
  out = re.sub('#\s*define\sVERSION\s".*"', '#define VERSION "%s"' % tag, out, 1)
  writeall("main/version.hpp", out)

  status = os.system("git tag %s" % tag)
  if status:
    exit(status)

  os.mkdir("debian", 0766)

  if force_distro == None or (force_distro == 'ubuntu' and force_distro_codename == None):
    distro, distro_release, distro_codename = distroinfo.get_distro()
  if force_distro != None:
    distro = force_distro
  if force_distro_codename != None:
    distro_codename = force_distro_codename

  packagetemp = "packaging/template/debian"

  writeall("debian/redemption.install",
           "%s/*\n%s/bin/*\n%s/share/rdpproxy/*" % (etc_prefix, prefix, prefix))

  env = 'PREFIX="%s"\nETC_PREFIX="%s"\nCERT_PREFIX="%s"' % (prefix, etc_prefix, cert_prefix)

  for fname in ["redemption.postinst", "rules"]:
    with open("%s/%s" % (packagetemp, fname), "r") as f:
      if debug and fname == "rules":
        env += "\nBJAM_EXTRA_INSTALL=debug"
      writeall("debian/%s" % fname, f.read().replace('%setenv', env))

  changelog = ""
  if entry_changelog:
    if not 'EDITOR' in os.environ:
      os.environ['EDITOR'] = 'nano'
    os.system("%s /tmp/redemption.changelog.tmp" % os.environ['EDITOR'])
    changelog = "redemption (%s%%target_name) %%pkg_distribution; urgency=low\n\n" % tag
    with open("/tmp/redemption.changelog.tmp", "r") as f:
      for line in f:
        if len(line) and line != "\n":
          changelog += "  * "
          changelog += line
    changelog += "\n\n -- cgrosjean <cgrosjean at wallix.com>  "
    changelog += datetime.datetime.today().strftime("%a, %d %b %Y %H:%M:%S +0200")
    changelog += "\n\n"
  changelog += readall("%s/changelog" % packagetemp)
  if entry_changelog:
    writeall("%s/changelog" % packagetemp, changelog)

  target = ''
  if distro == 'ubuntu':
    target += '+'
    target += distro_codename
  writeall("debian/changelog",
           changelog.replace('%target_name', target).replace('%pkg_distribution', package_distribution))

  shutil.copy("%s/compat" % packagetemp, "debian/compat")
  shutil.copy("%s/control" % packagetemp, "debian/control")
  shutil.copy("docs/copyright", "debian/copyright")

  exit(os.system("dpkg-buildpackage -b -tc -us -uc -r"))
except Exception, e:
  print "Build failed: %s" % e
  exit(-1)
