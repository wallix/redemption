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
import platform

def usage():
  print("Usage: %s [-h|--help] --update-version version [--no-entry-changelog] [--no-git-commit] [--git-tag] [--git-push-tag] | --build-package [--package-distribution name] [--force-target target] [--debug]" % sys.argv[0])

# [--prefix path] [--etc-prefix path] [--cert-prefix path]
# [--force-distro name] [--force-distro-codename name]
try:
  options, args = getopt.getopt(sys.argv[1:], "h",
                             ["help", "update-version=", "build-package",
                              "no-entry-changelog",
                              "no-git-commit", "git-tag", "git-push-tag",
                              # "prefix=", "etc-prefix=", "cert-prefix=",
                              "package-distribution=",
                              # "force-distro=", "force-distro-codename=",
                              "debug", "force-target="])
except getopt.GetoptError as err:
  print(str(err))
  usage()
  sys.exit(2)

class opts(object):
  tag = None
  target_param_path = "packaging/targets"
  force_target = None
  packagetemp = "packaging/template/debian"
  prefix = 'usr/local'
  etc_prefix = 'etc/rdpproxy'
  cert_prefix = 'etc/rdpproxy/cert'
  package_distribution = 'unstable'
  archi = 'any'
  debug = False
  entry_changelog = True
  force_distro = None
  force_distro_release = None
  force_distro_codename = None
  git_commit = True
  git_tag = False
  git_push_tag = False

  update_version = False
  build_package = False

for o,a in options:
  if o in ("-h", "--help"):
    usage()
    sys.exit()
  elif o in ("-u", "--update-version"):
    opts.update_version = True
    opts.tag = a
  elif o in ("-b", "--build-package"):
    opts.build_package = True
  elif o == "--no-entry-changelog":
    opts.entry_changelog = False
  elif o == "--no-git-commit":
    opts.git_commit = False
  elif o == "--git-tag":
    opts.git_tag = True
  elif o == "--git-push-tag":
    opts.git_push_tag = True
  elif o == "--package-distribution":
    opts.package_distribution = a
  # elif o == "--prefix":
  #   opts.prefix = a
  # elif o == "--etc-prefix":
  #   opts.etc_prefix = a
  # elif o == "--cert-prefix":
  #   opts.cert_prefix = a
  # elif o == "--force-distro":
  #   opts.force_distro = a
  # elif o == "--force-distro-codename":
  #   opts.force_distro_codename = a
  elif o == "--debug":
    opts.debug = True
  elif o == "--force-target":
    opts.force_target = a

# if ((not opts.build_package) and
#     ((not opts.update_version) or
#      (not opts.tag))):
if not (opts.build_package or (opts.update_version and opts.tag)):
  usage()
  sys.exit(1)

# remove existing deban directory BEGIN
try:
  shutil.rmtree("debian")
except:
  pass
# remove existing deban directory END

# IO Files functions BEGIN
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
rgx_prefix = re.compile("^\s*PREFIX=\s*(.*)$")
rgx_etc_prefix = re.compile("^\s*ETC_PREFIX=\s*(.*)$")
rgx_cert_prefix = re.compile("^\s*CERT_PREFIX=\s*(.*)$")
rgx_archi = re.compile("^\s*ARCHI=\s*(.*)$")
rgx_build_depends = re.compile("^\s*BUILD_DEPENDS=\s*(.*)$")
rgx_depends = re.compile("^\s*DEPENDS=\s*(.*)$")
rgx_distro = re.compile("^\s*DISTRO=\s*(.*)$")
rgx_distro_release = re.compile("^\s*DISTRO_RELEASE=\s*(.*)$")
rgx_distro_codename = re.compile("^\s*DISTRO_CODENAME=\s*(.*)$")

def _parse_target_param(fl):
  for l in fl:
    m = re.search(rgx_prefix, l)
    if m:
      opts.prefix = m.group(1)
      continue
    m = re.search(rgx_etc_prefix, l)
    if m:
      opts.etc_prefix = m.group(1)
      continue
    m = re.search(rgx_cert_prefix, l)
    if m:
      opts.cert_prefix = m.group(1)
      continue
    m = re.search(rgx_archi, l)
    if m:
      opts.archi = m.group(1)
      continue
    m = re.search(rgx_build_depends, l)
    if m:
      opts.build_depends = m.group(1)
      continue
    m = re.search(rgx_depends, l)
    if m:
      opts.depends = m.group(1)
      continue
    m = re.search(rgx_distro, l)
    if m:
      opts.force_distro = m.group(1)
      continue
    m = re.search(rgx_distro_release, l)
    if m:
      opts.force_distro_release = m.group(1)
      continue
    m = re.search(rgx_distro_codename, l)
    if m:
      opts.force_distro_codename = m.group(1)
      continue



def parse_target_param(filename):
  with open(filename) as f:
    _parse_target_param(f)
# IO Files functions END

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
  if opts.entry_changelog:
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
  changelog += readall("%s/changelog" % opts.packagetemp)
  writeall("%s/changelog" % opts.packagetemp, changelog)

# Check tag version BEGIN
def check_new_tag_version_with_local_and_remote_tags(newtag):
  locale_tags = subprocess.Popen(["git", "tag", "--list"],
                                 stdout = subprocess.PIPE,
                                 stderr = subprocess.STDOUT
                                 ).communicate()[0].split('\n')

  if newtag in locale_tags:
    raise Exception('tag %s already exists (locale).' % newtag)

  remote_tags = map(lambda x : x.split('/')[-1],
                    subprocess.Popen(["git", "ls-remote", "--tags", "origin"],
                                     stdout = subprocess.PIPE,
                                     stderr = subprocess.STDOUT
                                     ).communicate()[0].split('\n'))

  if newtag in remote_tags:
    raise Exception('tag %s already exists (remote).' % newtag)
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
  out = readall("%s/changelog" % opts.packagetemp)
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

def archi_to_control_archi(architecture):
  if architecture == 'x86_64':
    return 'amd64'
  if architecture == 'i386':
    return architecture
  return 'any'

def get_device_architecture():
  res = platform.machine()
  if res:
    return res
  # res = subprocess.Popen(["lscpu"], stdout=subprocess.PIPE, stderr = subprocess.STDOUT).communicate()[0]
  # for l in res.split('\n'):
  #   m = re.match('^Architecture.*:.*(x86_64|i386)', l)
  #   if m:
  #     return m.group(1)
  raise Exception('Device architecture not found')

status = 0
remove_diff = False
try:
  # TESTS
  # if opts.force_target:
  #   try:
  #     parse_target_param("%s/%s" % (opts.target_param_path, opts.force_target))
  #   except IOError:
  #     raise Exception('Target param file not found (%s/%s)' %
  #                     (opts.target_param_path, opts.force_target))
  #   print opts.prefix
  #   print opts.etc_prefix
  #   print opts.cert_prefix
  #   print opts.archi
  #   print opts.build_depends
  #   print opts.depends
  #   device_archi = get_device_architecture()
  #   if not ((opts.archi == 'any') or (device_archi == opts.archi)):
  #     print 'Target architecture (%s) does not match current device architecture (%s)' % (opts.archi, device_archi)
  #   exit(0)

  check_uncommited_changes()
  remove_diff = True
  if (opts.update_version and opts.tag):
    # check tag does not exist
    check_new_tag_version_with_local_and_remote_tags(opts.tag)
    # update changelog and version (write in main/version.hpp and changelog template)
    update_version_file(opts.tag)
    update_changelog_template(opts.tag)

    # tags and commits BEGIN
    if opts.git_commit:
      status = os.system("git commit -am 'Version %s'" % opts.tag)
      if status:
        raise ""
      if opts.git_tag:
        status = os.system("git tag %s" % opts.tag)
        if status:
          raise ""
        if opts.git_push_tag:
          status = os.system("git push --tags")
          if status:
            raise ""
    # tags and commits END

  if opts.build_package:
    opts.tag = check_matching_version_changelog()
    check_last_version_commited_match_current_version(opts.tag)
    # Set debian (packaging data) directory with distro specific packaging files BEGIN
    # Create temporary directory
    os.mkdir("debian", 0766)

    # existing target parameters
    if opts.force_target:
      try:
        parse_target_param("%s/%s" % (opts.target_param_path, opts.force_target))
      except IOError:
        raise Exception('Target param file not found (%s/%s)' %
                        (opts.target_param_path, opts.force_target))

    # check Distro
    distro, distro_release, distro_codename = distroinfo.get_distro()
    if opts.force_distro != None:
      distro = opts.force_distro
    if opts.force_distro_release != None:
      distro_release = opts.force_distro_release
    if opts.force_distro_codename != None:
      distro_codename = opts.force_distro_codename

    # Check Architecture
    device_archi = get_device_architecture()
    if not ((opts.archi == 'any') or (device_archi == opts.archi)):
      raise Exception('Target architecture (%s) does not match current device architecture (%s)' % (opts.archi, device_archi))
    opts.archi = device_archi

    # write redemption.install file
    install_file = readall("%s/redemption.install" % opts.packagetemp)
    install_file = install_file.replace('%prefix', opts.prefix)
    install_file = install_file.replace('%etc_prefix', opts.etc_prefix)
    writeall("debian/redemption.install", install_file)

    env = 'PREFIX="%s"\nETC_PREFIX="%s"\nCERT_PREFIX="%s"' % (opts.prefix,
                                                              opts.etc_prefix,
                                                              opts.cert_prefix)
    # write redemption.postinst
    copy_and_replace("%s/redemption.postinst" % opts.packagetemp,
                     "debian/redemption.postinst", '%setenv', env)

    # write rules
    if opts.debug:
      env += "\nBJAM_EXTRA_INSTALL=debug"
    copy_and_replace("%s/rules" % opts.packagetemp, "debian/rules", '%setenv', env)

    # write changelog
    target = ''
    if distro == 'ubuntu':
      target += '+'
      target += distro_codename
    else:
      # debian codename
      opts.package_distribution = distro_codename
    changelog = readall("%s/changelog" % opts.packagetemp)
    writeall("debian/changelog",
             changelog.replace('%target_name',
                               target).replace('%pkg_distribution',
                                               opts.package_distribution))

    # write control
    out = parse_template("%s/control" % opts.packagetemp,
                         distro, distro_codename, distro_release, distroinfo.get_arch())
    writeall("debian/control", out.replace('%REDEMPTION_VERSION', opts.tag).replace('%ARCHI', opts.archi))

    # write compat
    shutil.copy("%s/compat" % opts.packagetemp, "debian/compat")
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
