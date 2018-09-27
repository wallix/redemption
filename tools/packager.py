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
  print("Usage: %s [-h|--help] [--force-build] [--update-version version] [--no-entry-changelog] [--no-git-commit] [--git-push] [--git-tag] [--git-push-tag] | --build-package [--package-distribution name] [--force-target target] [--debug]" % sys.argv[0])

try:
  options, args = getopt.getopt(sys.argv[1:], "h",
                                ["help", "update-version=", "build-package",
                                 "no-entry-changelog",
                                 "no-git-commit", "git-tag", "git-push-tag", "git-push",
                                 "package-distribution=", "force-build",
                                 "debug", "force-target="])
except getopt.GetoptError as err:
  print(str(err))
  usage()
  sys.exit(2)

class opts(object):
  tag = None
  target_param_path = "packaging/targets"
  packagetemp = "packaging/template/debian"
  force_target = None

  debug = False
  entry_changelog = True
  git_commit = True
  git_tag = False
  git_push_tag = False
  git_push = False

  update_version = False
  build_package = False
  force_build = False

  config = {}
  config["%PREFIX%"] = 'usr/local'
  config["%ETC_PREFIX%"] = 'etc/rdpproxy'
  config["%CERT_PREFIX%"] = 'etc/rdpproxy/cert'
  config["%METRICS_PATH%"] = '/var/rdpproxy/recorded/metrics'
  config["%RECORD_PATH%"] = '/var/rdpproxy/recorded/rdp'
  config["%RECORD_TMP_PATH%"] = '/var/rdpproxy/tmp'
  config["%PKG_DISTRIBUTION%"] = 'unstable'
  config["%ARCHI%"] = 'any'
  config["%REDEMPTION_USER%"] = ''

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
  elif o == "--git-push":
    opts.git_push = True
  elif o == "--git-tag":
    opts.git_tag = True
  elif o == "--git-push-tag":
    opts.git_push_tag = True
  elif o == "--package-distribution":
    opts.config["%PKG_DISTRIBUTION%"] = a
  elif o == "--debug":
    opts.debug = True
  elif o == "--force-target":
    opts.force_target = a
  elif o == "--force-build":
    opts.force_build = True

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

# rgx_split = re.compile("\s+")
# rgx_if = re.compile("^\s*@if\s+")
# rgx_else = re.compile("^\s*@else\s*")
# rgx_endif = re.compile("^\s*@endif\s*$")
# rgx_elif = re.compile("^\s*@elif\s+")

# def _test_condition(filename, num, l, vars):
#   a = re.split(rgx_split, l)
#   if len(a) >= 2:
#     idx = 0;
#     if not a[0]:
#       idx = 1
#     if a[idx] == 'not':
#       test = False
#       idx += 1
#     else:
#       test = True
#     if idx:
#       a = a[idx:]

#   if len(a) < 2 or not a[1]:
#     raise Exception("conditional error in '%s' at line %s" % (filename, num))

#   if a[0] in ['version-less', 'version-or-less']:
#     if not (len(a) == 2 or (len(a) == 3 and a[2] == '')):
#       raise Exception("%s too many argument in '%s' at line %s" % (a[0], filename, num))
#     if a[0] == 'version-less':
#       return (vars['version'] < float(a[1])) == test
#     return (vars['version'] <= float(a[1])) == test

#   if not a[0] in vars:
#     raise Exception("test '%s' unknow in '%s' at line %s" % (a[0], filename, num))
#   x = vars[a[0]]
#   return (x and x in a[1:]) == test


# def _parse_template(filename, num, f, vars, rec = 0, stateif = False, addtext = True):
#   out = ''
#   stateelse = False
#   for l in f:
#     num += 1
#     m = re.search(rgx_if, l)
#     if m:
#       cond = _test_condition(filename, num, l[m.end()-1:], vars)
#       if stateif or stateelse:
#         num,ret = _parse_template(filename, num, f, vars, rec+1, True, cond)
#         out += ret
#       else:
#         stateif = True
#         addtext = cond
#     else:
#       m = re.search(rgx_else, l)
#       if m:
#         if not stateif or stateelse:
#           raise Exception("else without if in '%s' at line %s" % (filename, num))
#         addtext = not addtext
#         stateelse = True
#         stateif = False
#       else:
#         m = re.search(rgx_elif, l)
#         if m:
#           if not stateif or stateelse:
#             raise Exception("elif without if in '%s' at line %s" % (filename, num))
#           addtext = _test_condition(filename, num, l[m.end()-1:], vars)
#         else:
#           m = re.search(rgx_endif, l)
#           if m:
#             if not stateif and not stateelse:
#               raise Exception("endif without if in '%s' at line %s" % (filename, num))
#             if rec:
#               return num,out
#             addtext = True
#             stateelse = False
#             stateif = False
#           elif addtext:
#             out += l
#   if stateif or stateelse:
#     raise Exception("if without endif in '%s' at line %s" % (filename, num))
#   return num,out


# def parse_template(filename, distro, codename, version, arch):
#   vars = {'dist': distro, 'codename':codename, 'version': float(version), 'arch': arch}
#   with open(filename) as f:
#     num,out = _parse_template(filename, 0, f, vars)
#     return out
#   return ''


# Parse target Config files
rgx_general = re.compile("^([^#][^=]*)\s*=\s*(.*)$")
def parse_config_line(line):
  m = re.search(rgx_general, line.strip())
  if m:
    opts.config['%%%s%%' % m.group(1).strip()] = m.group(2).strip()

def parse_target_param(filename):
  with open(filename) as f:
    for l in f:
      parse_config_line(l)
# IO Files functions END

# Replace occurences in templates with dictionnaire
rgx_var = re.compile("(%[A-Z][A-Z0-9_]*%)")

def replace_dict_line(line, dico):
  res = line
  miter = rgx_var.finditer(res)
  for m in reversed(list(miter)):
    value = dico.get(m.group(1))
    if not value:
      value = ''
    res = res[:m.start(1)] + value + res[m.end(1):]
  return res

def replace_dict_file(filename, dico):
  res = ''
  with open(filename) as f:
    for l in f:
      res += replace_dict_line(l, dico)
  return res

def copy_and_replace_dict_file(filename, dico, target):
  out = replace_dict_file(filename, dico)
  writeall(target, out)

# Unused
# def export_var(key):
#   res = ""
#   value = opts.config["%%%s%%" % key.upper()]
#   if value:
#     res = "export %s=%s" % (key.upper(), value)
#   return res


# UPDATE VERSION FUNCTIONS
def update_version_file(newtag):
  # Set tag version in include/main/version.hpp
  out = readall("include/main/version.hpp")
  out = re.sub('#\s*define\sVERSION\s".*"', '#define VERSION "%s"' % newtag, out, 1)
  writeall("include/main/version.hpp", out)
def update_changelog_template(newtag):
  # write changelog
  changelog = "redemption (%s%%TARGET_NAME%%) %%PKG_DISTRIBUTION%%; urgency=low\n\n" % newtag
  if opts.entry_changelog:
    if not 'EDITOR' in os.environ:
      os.environ['EDITOR'] = 'nano'
    os.system("%s /tmp/redemption.changelog.tmp" % os.environ['EDITOR'])
    with open("/tmp/redemption.changelog.tmp", "r") as f:
      for line in f:
        if len(line) and line != "\n":
          changelog += "  * "
          changelog += line
  changelog += "\n -- cgrosjean <cgrosjean at wallix.com>  "
  changelog += datetime.datetime.today().strftime("%a, %d %b %Y %H:%M:%S +0200")
  changelog += "\n\n"
  changelog += readall("%s/changelog" % opts.packagetemp)
  writeall("%s/changelog" % opts.packagetemp, changelog)

# CHECKERS
# Check uncommited changes BEGIN
def check_uncommited_changes():
  res = subprocess.Popen(["git", "diff", "--shortstat"],
                         stdout = subprocess.PIPE,
                         stderr = subprocess.STDOUT
                         ).communicate()[0]
  if res:
    raise Exception('your repository has uncommited changes:\n%sPlease commit before packaging.' % (res))
# Check uncommited changes END
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

# Check matching versions BEGIN
def check_matching_version_changelog():
  found = False
  out = readall("include/main/version.hpp")
  out = out.split('\n')
  for line in out:
    res = re.match('^[#]define\sVERSION\s"(([-a-zA-Z]*(\d+)[.](\d+)[.](\d+))(-?[a-z]*)*)"\s*$', line)
    if res:
      red_source_version = res.group(1)
      red_num_ver = res.group(2)
      bas_ver = res.group(3)
      rel_ver = res.group(4)
      fix_ver = res.group(5)
      found = True
      break
  if not found:
    raise Exception('Source Version not found in file include/main/version.hpp')

  if opts.force_build:
    return red_source_version

  found = False
  out = readall("%s/changelog" % opts.packagetemp)
  out = out.split('\n')
  for line in out:
    res = re.match('^redemption\s*[(](([-a-zA-Z]*(\d+)[.](\d+)[.](\d+))(-?[a-z]*)*)(.*)[)].*$', line)
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
    raise Exception('Version mismatch between changelog and include/main/version ("%s" != "%s")' %
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

# INFO GETTERS
def get_device_architecture():
  res = platform.machine()
  if res:
    return res
  raise Exception('Device architecture not found')

# control file for dpkg packager only allow 'amd64' for 64bits architectures and 'i386' for 32bits or any if there is no specification
def archi_to_control_archi(architecture):
  if architecture in ['x86_64', 'amd64']:
    return 'amd64'
  if architecture in ['i686', 'i386']:
    return 'i386'
  return 'any'


status = 0
remove_diff = False
try:

  update_tag = (opts.update_version and opts.tag)
  if update_tag or (opts.build_package and not opts.force_build):
    check_uncommited_changes()
    remove_diff = True

  if update_tag:
    # check tag does not exist
    check_new_tag_version_with_local_and_remote_tags(opts.tag)
    # update changelog and version (write in include/main/version.hpp and changelog template)
    update_version_file(opts.tag)
    update_changelog_template(opts.tag)

    # tags and commits BEGIN
    if opts.git_commit:
      status = os.system("git commit -am 'Version %s'" % opts.tag)
      if status:
        raise ""
      if opts.git_push:
        status = os.system("git push")
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
    if not opts.force_build:
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
    if opts.config.get("%DISTRO%"):
      distro = opts.config.get("%DISTRO%")
    if opts.config.get("%DISTRO_RELEASE%"):
      distro_release = opts.config["%DISTRO_RELEASE%"]
    if opts.config.get("%DISTRO_CODENAME%"):
      distro_codename = opts.config["%DISTRO_CODENAME%"]

    # Check Architecture
    device_archi = get_device_architecture()
    if not ((opts.config["%ARCHI%"] == 'any') or (device_archi == opts.config["%ARCHI%"])):
      raise Exception('Target architecture (%s) does not match current device architecture (%s)' % (opts.config["%ARCHI%"], device_archi))
    opts.config["%ARCHI%"] = archi_to_control_archi(device_archi)

    # write redemption.install file
    copy_and_replace_dict_file("%s/redemption.install" % opts.packagetemp,
                               opts.config,
                               "debian/redemption.install")

    # write redemption.postinst
    try:
      copy_and_replace_dict_file("%s/redemption.postinst" % opts.packagetemp,
                                 opts.config,
                                 "debian/redemption.postinst")
    except IOError, e:
      if e.errno != 2:
        raise e

    # write rules
    if opts.debug:
      opts.config["%DEBUG_OPT%"] = "debug"
    copy_and_replace_dict_file("%s/rules" % opts.packagetemp,
                               opts.config,
                               "debian/rules")
    # write changelog
    target = ''
    if distro == 'ubuntu':
      target += '+'
      target += distro_codename
    else:
      # debian codename
      opts.config["%PKG_DISTRIBUTION%"] = distro_codename
    if not "%TARGET_NAME%" in opts.config:
      opts.config["%TARGET_NAME%"] = target
    copy_and_replace_dict_file("%s/changelog" % opts.packagetemp,
                               opts.config,
                               "debian/changelog")

    # write control
    opts.config["%REDEMPTION_VERSION%"] = opts.tag
    copy_and_replace_dict_file("%s/control" % opts.packagetemp,
                               opts.config,
                               "debian/control")

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
