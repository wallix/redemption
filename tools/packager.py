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
  print("Usage: %s [-h|--help] --update-version version [--no-entry-changelog] [--no-git-commit] [--git-tag] [--git-push-tag] --build-package [--prefix path] [--etc-prefix path] [--cert-prefix path] [--package-distribution name] [--force-distro name] [--force-distro-codename name] [--debug]" % sys.argv[0])

try:
  opts, args = getopt.getopt(sys.argv[1:], "h",
                             ["help", "update-version=", "build-package",
                              "no-entry-changelog",
                              "no-git-commit", "git-tag", "git-push-tag",
                              "prefix=", "etc-prefix=", "cert-prefix=",
                              "package-distribution=",
                              "force-distro=", "force-distro-codename=",
                              "debug"])
except getopt.GetoptError as err:
  print(str(err))
  usage()
  sys.exit(2)

tag = None
packagetemp = "packaging/template/debian"
prefix = '/usr/local'
etc_prefix = '/etc/rdpproxy'
cert_prefix = '/etc/rdpproxy/cert'
package_distribution = 'unstable'
debug = False
entry_changelog = True
force_distro = None
force_distro_codename = None
git_commit = True
git_tag = False
git_push_tag = False

update_version = False
build_package = False

for o,a in opts:
  if o in ("-h", "--help"):
    usage()
    sys.exit()
  elif o in ("-u", "--update-version"):
    update_version = True
    tag = a
  elif o in ("-b", "--build-package"):
    build_package = True
  elif o == "--no-entry-changelog":
    entry_changelog = False
  elif o == "--no-git-commit":
    git_commit = False
  elif o == "--git-tag":
    git_tag = True
  elif o == "--git-push-tag":
    git_push_tag = True
  elif o == "--package-distribution":
    package_distribution = a
  elif o == "--prefix":
    prefix = a
  elif o == "--etc-prefix":
    etc_prefix = a
  elif o == "--cert-prefix":
    cert_prefix = a
  elif o == "--force-distro":
    force_distro = a
  elif o == "--force-distro-codename":
    force_distro_codename = a
  elif o == "--debug":
    debug = True

if ((not build_package) and
    ((not update_version) or
     (not tag))):
  usage()
  sys.exit(1)

# remove existing deban directory BEGIN
try:
  shutil.rmtree("debian")
except:
  pass
# remove existing deban directory END

def readall(filename):
  with open(filename) as f:
    return f.read()

def writeall(filename, s):
  with open(filename, "w+") as f:
    f.write(s)

def copy_and_replace(src, dst, old, new):
  out = readall(src)
  out = out.replace(old, new)
  writeall(dst, out)

rgx_split = re.compile("\s+")
rgx_if = re.compile("^\s*@if\s+")
rgx_else = re.compile("^\s*@else\s*")
rgx_endif = re.compile("^\s*@endif\s*$")
rgx_elif = re.compile("^\s*@elif\s+")

def _test_condition(filename, num, l, vars):
  a = re.split(rgx_split, l)
  if len(a) >= 2:
    idx = 0;
    if not a[0]:
      idx = 1
    if a[idx] == 'not':
      test = False
      idx += 1
    else:
      test = True
    if idx:
      a = a[idx:]

  if len(a) < 2 or not a[1]:
    raise Exception("conditional error in '%s' at line %s" % (filename, num))

  if a[0] in ['version-less', 'version-or-less']:
    if not (len(a) == 2 or (len(a) == 3 and a[2] == '')):
      raise Exception("%s too many argument in '%s' at line %s" % (a[0], filename, num))
    if a[0] == 'version-less':
      return (vars['version'] < float(a[1])) == test
    return (vars['version'] <= float(a[1])) == test

  if not a[0] in vars:
    raise Exception("test '%s' unknow in '%s' at line %s" % (a[0], filename, num))
  x = vars[a[0]]
  return (x and x in a[1:]) == test


def _parse_template(filename, num, f, vars, rec = 0, stateif = False, addtext = True):
  out = ''
  stateelse = False
  for l in f:
    num += 1
    m = re.search(rgx_if, l)
    if m:
      cond = _test_condition(filename, num, l[m.end()-1:], vars)
      if stateif or stateelse:
        num,ret = _parse_template(filename, num, f, vars, rec+1, True, cond)
        out += ret
      else:
        stateif = True
        addtext = cond
    else:
      m = re.search(rgx_else, l)
      if m:
        if not stateif or stateelse:
          raise Exception("else without if in '%s' at line %s" % (filename, num))
        addtext = not addtext
        stateelse = True
        stateif = False
      else:
        m = re.search(rgx_elif, l)
        if m:
          if not stateif or stateelse:
            raise Exception("elif without if in '%s' at line %s" % (filename, num))
          addtext = _test_condition(filename, num, l[m.end()-1:], vars)
        else:
          m = re.search(rgx_endif, l)
          if m:
            if not stateif and not stateelse:
              raise Exception("endif without if in '%s' at line %s" % (filename, num))
            if rec:
              return num,out
            addtext = True
            stateelse = False
            stateif = False
          elif addtext:
            out += l
  if stateif or stateelse:
    raise Exception("if without endif in '%s' at line %s" % (filename, num))
  return num,out


def parse_template(filename, distro, codename, version, arch):
  vars = {'dist': distro, 'codename':codename, 'version': float(version), 'arch': arch}
  with open(filename) as f:
    num,out = _parse_template(filename, 0, f, vars)
    return out
  return ''

# Check uncommited changes BEGIN
def check_uncommited_changes():
  res = subprocess.Popen(["git", "diff", "--shortstat"],
                         stdout = subprocess.PIPE,
                         stderr = subprocess.STDOUT
                        ).communicate()[0]
  if res:
    raise Exception('your repository has uncommited changes:\n%sPlease commit before packaging.' % (res))
# Check uncommited changes END

# UPDATE VERSION FUNCTIONS
def update_version_file(newtag):
  # Set tag version in main/version.hpp
  out = readall("main/version.hpp")
  out = re.sub('#\s*define\sVERSION\s".*"', '#define VERSION "%s"' % newtag, out, 1)
  writeall("main/version.hpp", out)
def update_changelog_template(newtag):
  # write changelog
  changelog = "redemption (%s%%target_name) %%pkg_distribution; urgency=low\n\n" % newtag
  if entry_changelog:
    if not 'EDITOR' in os.environ:
      os.environ['EDITOR'] = 'nano'
    os.system("%s /tmp/redemption.changelog.tmp" % os.environ['EDITOR'])
    with open("/tmp/redemption.changelog.tmp", "r") as f:
      for line in f:
        if len(line) and line != "\n":
          changelog += "  * "
          changelog += line
  changelog += "\n\n -- cgrosjean <cgrosjean at wallix.com>  "
  changelog += datetime.datetime.today().strftime("%a, %d %b %Y %H:%M:%S +0200")
  changelog += "\n\n"
  changelog += readall("%s/changelog" % packagetemp)
  writeall("%s/changelog" % packagetemp, changelog)

# Check tag version BEGIN
def check_new_tag_version_with_local_and_remote_tags(newtag):
  locale_tags = subprocess.Popen(["git", "tag", "--list"],
                               stdout = subprocess.PIPE,
                               stderr = subprocess.STDOUT
                               ).communicate()[0].split('\n')

  if newtag in locale_tags:
    raise Exception('tag %s already exists (locale).' % tag)

  remote_tags = map(lambda x : x.split('/')[-1], subprocess.Popen(["git", "ls-remote", "--tags", "origin"],
                               stdout = subprocess.PIPE,
                               stderr = subprocess.STDOUT
                               ).communicate()[0].split('\n'))

  if newtag in remote_tags:
    raise Exception('tag %s already exists (remote).' % tag)
# Check tag version END

# BUILD FUNCTIONS
# Check matching versions BEGIN
def check_matching_version_changelog():
  found = False
  out = readall("main/version.hpp")
  out = out.split('\n')
  for line in out:
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

  found = False
  out = readall("%s/changelog" % packagetemp)
  out = out.split('\n')
  for line in out:
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
    raise Exception('Version mismatch between changelog and main/version ("%s" != "%s")' %
                    (changelog_red_source_version, red_source_version))
  return red_source_version
# Check matching versions END


# Check last version tag commited match current version tag BEGIN
def check_last_version_commited_match_current_version(version):
  res = subprocess.Popen(["git", "describe", "--tags"], stdout=subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]
  tag_describe = res.split("\n")[0]
  if version != tag_describe:
    raise Exception('Repository head mismatch current version ("%s" != "%s"), please tag current version before building packet' % (version, tag_describe))
# Check last version tag commited match current version tag END

status = 0
remove_diff = False
try:
  check_uncommited_changes()
  remove_diff = True
  if (update_version and tag):
    # check tag does not exist
    check_new_tag_version_with_local_and_remote_tags(tag)
    # update changelog and version (write in main/version.hpp and changelog template)
    update_version_file(tag)
    update_changelog_template(tag)

    # tags and commits BEGIN
    if git_commit:
      status = os.system("git commit -am 'Version %s'" % tag)
      if status:
        raise ""
      if git_tag:
        status = os.system("git tag %s" % tag)
        if status:
          raise ""
        if git_push_tag:
          status = os.system("git push --tags")
          if status:
            raise ""
    # tags and commits END

  if build_package:
    tag = check_matching_version_changelog()
    check_last_version_commited_match_current_version(tag)
    # Set debian (packaging data) directory with distro specific packaging files BEGIN
    # Create temporary directory
    os.mkdir("debian", 0766)

    distro, distro_release, distro_codename = distroinfo.get_distro()
    if force_distro != None:
      distro = force_distro
    if force_distro_codename != None:
      distro_codename = force_distro_codename

    # write redemption.install file
    writeall("debian/redemption.install",
               "%s/*\n%s/bin/*\n%s/share/rdpproxy/*" % (etc_prefix, prefix, prefix))

    env = 'PREFIX="%s"\nETC_PREFIX="%s"\nCERT_PREFIX="%s"' % (prefix, etc_prefix, cert_prefix)
    # write redemption.postinst
    copy_and_replace("%s/redemption.postinst" % packagetemp, "debian/redemption.postinst", '%setenv', env)

    # write rules
    if debug:
      env += "\nBJAM_EXTRA_INSTALL=debug"
    copy_and_replace("%s/rules" % packagetemp, "debian/rules", '%setenv', env)

    # write changelog
    target = ''
    if distro == 'ubuntu':
      target += '+'
      target += distro_codename
    else:
      # debian codename
      package_distribution = distro_codename
    writeall("debian/changelog",
             changelog.replace('%target_name', target).replace('%pkg_distribution', package_distribution))

    # write control
    out = parse_template("%s/control" % packagetemp,
                         distro, distro_codename, distro_release, distroinfo.get_arch())
    writeall("debian/control", out.replace('%REDEMPTION_VERSION', tag))

    # write compat
    shutil.copy("%s/compat" % packagetemp, "debian/compat")
    # write copyright
    shutil.copy("docs/copyright", "debian/copyright")

    # Set debian (packaging data) directory with distro specific packaging files END

    status = os.system("dpkg-buildpackage -b -tc -us -uc -r")
    if status:
      raise ""
  exit(0)
except Exception, e:
  if remove_diff:
    res = subprocess.Popen(["git", "diff", "--shortstat"],
                           stdout = subprocess.PIPE,
                           stderr = subprocess.STDOUT
                           ).communicate()[0]
    if res:
      os.system("git stash")
      os.system("git stash drop")
  print "Build failed: %s" % e
  exit(status if status else -1)
