#!/usr/bin/python3 -O

import glob ;
import os ;
from collections import OrderedDict

#project_root = '../..'
project_root = '.'

includes = (
    'src/',
    'src/system/linux/',
    'src/system/linux/system/',
    'projects/redemption_configs/redemption_src/',
    'src/capture/ocr/',
    'tests/includes/',
)

disable_tests = (
    'tests/epoll.cpp',
    'tests/utils/test_executor.cpp',
    'tests/utils/crypto/test_ssl_mod_exp_direct.cpp',
    'src/system/linux/system/test_framework.cpp',
)

disable_srcs = (
    'src/system/linux/system/test_framework.cpp',
    'src/utils/log_as_syslog.cpp',
    'src/utils/log_as_logemasm.cpp',
    'src/utils/log_as_logprint.cpp',
    'src/utils/log_as_logtest.cpp',
)

src_requirements = dict((
    ('src/main/rdpheadless.cpp', '<include>$(REDEMPTION_TEST_PATH)/includes'), # for lcg_random
    ('src/main/scytale.cpp', '<include>$(REDEMPTION_TEST_PATH)/includes'), # for lcg_random
    #('libscytale', '<library>log_print.o'),
    ('tests/includes/test_only/front/fake_front.cpp', '<include>$(REDEMPTION_TEST_PATH)/includes'),
    ('tests/includes/test_only/fake_graphic.cpp', '<include>$(REDEMPTION_TEST_PATH)/includes'),
    ('src/capture/video_recorder.cpp', '<cxxflags>-Wno-deprecated-declarations'),
))

target_requirements = dict((
    ('libscytale', '<library>log.o'),
    ('libredrec', '<library>log.o'),
))

# because include a .cpp file...
remove_requirements = dict((
    ('tests/capture/test_capture.cpp', '<library>src/capture/capture.o'),
    ('tests/utils/test_rle.cpp', '<library>src/utils/rle.o'),
))

dir_requirements = dict((
    ('src/sashimi', '<cxxflags>-Wno-format <cxxflags>-Wno-format-security'),
    ('tests/sashimi', '<cxxflags>-Wno-format <cxxflags>-Wno-format-security'),
))

target_pre_renames = dict((
    ('tests/core/RDP/test_pointer.cpp', 'test_rdp_pointer'),
))

target_renames = dict((
    ('main', 'rdpproxy'),
    ('transparent', 'rdptproxy'),
    ('do_recorder', 'redrec'),
    ('ini_checker', 'rdpinichecker'),
    ('tanalyzer', 'rdptanalyzer'),
    ('rdp_client', 'rdpclient'),
    ('vnc_client', 'vncclient'),
))

#coverage_requirements = dict((
#))

dir_nocoverage = set([
    'tests/sashimi/',
    'tests/server/',
    'tests/system/common/',
    'tests/system/emscripten/',
    'tests/client_mods'
])
file_nocoverage = set(glob.glob('tests/*.cpp'))

sys_lib_assoc = dict((
    ('png.h', 'png'),
    ('krb5.h', 'krb5'),
    ('gssapi/gssapi.h', 'gssapi_krb5'),
    ('snappy-c.h', 'snappy'),
    ('zlib.h', 'z'),
))
sys_lib_prefix = (
    ('libavformat/', 'ffmpeg'),
    ('openssl/', 'crypto'),
)

user_lib_assoc = dict((
    ('program_options/program_options.hpp', 'program_options'),
    ('openssl_crypto.hpp', 'crypto'),
    ('openssl_tls.hpp', 'openssl'),
))
user_lib_prefix = (
    ('ppocr/', 'ppocr'),
)

def get_lib(inc, lib_assoc, lib_prefix):
    if inc in lib_assoc:
        return lib_assoc[inc]

    for t in lib_prefix:
        if start_with(inc, t[0]):
            return t[1]

    return None

def get_system_lib(inc):
    return get_lib(inc, sys_lib_assoc, sys_lib_prefix)

def get_user_lib(inc):
    return get_lib(inc, user_lib_assoc, user_lib_prefix)


class File:
    def __init__(self, root, path, type):
        self.root = root
        self.path = path
        self.type = type
        #self.user_includes = set()
        #self.unknown_user_includes = set()
        #self.system_includes = set()
        #self.source_deps = set()
        #self.direct_source_deps = set()
        #self.direct_lib_deps = set()
        self.all_source_deps = None # then set()
        self.all_lib_deps = None # then set()
        self.have_coverage = False
        self.used = False #


###
### Get files
###

sources = []
mains = []
libs = []
tests = []

def append_file(a, root, path, type):
    if root[:2] == './':
        root = root[2:]
    if path[:2] == './':
        path = path[2:]
    root = root.replace('//', '/')
    path = path.replace('//', '/')
    a.append(File(root, path, type))

def get_files(a, dirpath):
    for root, dirs, files in os.walk(project_root + '/' + dirpath):
        for name in files:
            if name[-4:] == '.hpp':
                append_file(a, root, root+'/'+name, 'H')
            elif name[-4:] == '.cpp':
                append_file(a, root, root+'/'+name, 'C')
            elif name[-2:] == '.h':
                append_file(a, root, root+'/'+name, 'H')
            elif name[-2:] == '.c':
                append_file(a, root, root+'/'+name, 'C')
            elif name[-3:] == '.hh':
                append_file(a, root, root+'/'+name, 'H')
            elif name[-3:] == '.cc':
                append_file(a, root, root+'/'+name, 'C')

def start_with(str, prefix):
    return str[:len(prefix)] == prefix

for d in (
    "acl",
    "capture",
    "core",
    "front",
    "gdi",
    "keyboard",
    "mod",
    "regex",
    "sashimi",
    "transport",
    "utils"
):
    get_files(sources, 'src/'+d)
get_files(sources, 'src/system/linux/system')
get_files(sources, 'tests/includes/test_only')
for path in glob.glob('src/main/*.hpp'):
    append_file(sources, 'src/main', path, 'H')

for path in glob.glob('src/main/*.cpp'):
    if path == 'src/main/redrec.cpp':
        continue
    a = mains
    if path in ('src/main/scytale.cpp', 'src/main/do_recorder.cpp'):
        a = libs
    append_file(a, 'src/main', path, 'C')

files_on_tests = []
get_files(files_on_tests, 'tests')
tests = [f for f in files_on_tests \
    if f.type != 'H' \
    and not start_with(f.path, 'tests/includes/') \
    and not start_with(f.path, 'tests/system/common/') \
    and not start_with(f.path, 'tests/system/emscripten/system/') \
    and not start_with(f.path, 'tests/web_video/') \
    and f.path != 'tests/test_meta_protocol2.cpp' \
    and f.path not in disable_tests
]

sources = [f for f in sources if f.path not in disable_srcs]

extra_srcs = (
    ('src/configs/config.hpp', File(
        'projects/redemption_configs/redemption_src',
        'projects/redemption_configs/redemption_src/configs/config.hpp',
        'H')
    ),
    ('src/configs/config.cpp', File(
        'projects/redemption_configs/redemption_src',
        'projects/redemption_configs/redemption_src/configs/config.cpp',
        'C')
    ),
)

for t in extra_srcs:
    sources.append(t[1])

#for k,f in tests:
    #print(k, f.root)

kpath = lambda f: f.path
sources = sorted(sources, key=kpath)
mains = sorted(mains, key=kpath)
libs = sorted(libs, key=kpath)
tests = sorted(tests, key=kpath)

def tuple_files(l):
    return [(f.path, f) for f in l]

all_files = OrderedDict(tuple_files(tests))
all_files.update(tuple_files(sources))
all_files.update(tuple_files(mains))
all_files.update(tuple_files(libs))
all_files.update(extra_srcs)

#print([f.path for f in sources])
#print("------")


###
### Get user includes
###

def get_includes(path):
    user_includes = []
    system_includes = []
    unknown_user_includes = []
    with open(path) as f:
        for line in f:
            line = line.lstrip()
            if len(line) and line[0] == '#':
                line = line[1:].lstrip()
                if line[:7] == 'include':
                    line = line[7:].lstrip()
                    if len(line) and line[0] == '"':
                        inc = line[1:line.rfind('"')]
                        found = False
                        for dir_name in includes:
                            file_name = dir_name+inc
                            if file_name in all_files:
                                user_includes.append(all_files[file_name])
                                found = True
                                break
                        if not found:
                            file_name = path[0:path.rfind('/')] + '/' + inc
                            if file_name in all_files:
                                user_includes.append(all_files[file_name])
                            unknown_user_includes.append(inc)
                    if len(line) and line[0] == '<':
                        system_includes.append(line[1:line.rfind('>')])
    return set(user_includes), set(system_includes), set(unknown_user_includes)

for name, f in all_files.items():
    f.user_includes, f.system_includes, f.unknown_user_includes = get_includes(f.path)

#for f in sources:
    #print("--", f.path, [a.path for a in f.user_includes], f.system_includes, f.unknown_user_includes)


###
### Get deps (cpp, hpp, lib)
###

for name, f in all_files.items():
    deps = []
    for pf in f.user_includes:
        cpp_name = pf.path[:-4]+'.cpp'
        if cpp_name in all_files:
            deps.append(all_files[cpp_name])
        cpp_name = pf.path[:-4]+'.cc'
        if cpp_name in all_files:
            deps.append(all_files[cpp_name])
    f.direct_source_deps = set(deps)

    deps = []
    for name in f.system_includes:
        syslib = get_system_lib(name)
        if syslib:
            deps.append(syslib)
    for name in f.unknown_user_includes:
        syslib = get_user_lib(name)
        if syslib:
            deps.append(syslib)
    for pf in f.user_includes:
        syslib = get_user_lib(pf.path)
        if syslib:
            deps.append(syslib)
    f.direct_lib_deps = set(deps)


def compute_all_source_deps(f, empty_array = []):
    if f.all_source_deps is None:
        srcs = f.direct_source_deps
        libs = f.direct_lib_deps
        incs = f.user_includes
        f.direct_source_deps = empty_array
        f.direct_lib_deps = empty_array
        f.user_includes = empty_array

        all_source_deps = list(srcs)
        all_lib_deps = list(libs)

        for pf in srcs:
            compute_all_source_deps(pf)
            if pf.all_source_deps is not None:
                all_source_deps += pf.all_source_deps
                all_lib_deps += pf.all_lib_deps

        for pf in incs:
            compute_all_source_deps(pf)
            if pf.all_source_deps is not None:
                all_source_deps += pf.all_source_deps
                all_lib_deps += pf.all_lib_deps

        f.direct_source_deps = srcs
        f.direct_lib_deps = libs
        f.user_includes = incs

        f.all_source_deps = set(all_source_deps)
        f.all_lib_deps = set(all_lib_deps)


for f in all_files.values():
    compute_all_source_deps(f)


###
### Generate
###

print('# ')
print('# DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN')
print('# run `tools/bjam/gen_targets.py > targets.jam`')
print('# ')

def get_target(f):
    if f.path in target_pre_renames:
        return target_pre_renames[f.path]
    iright = f.path.rfind('.')
    ileft = f.path.rfind('/')
    target = f.path[ileft+1:iright]
    if target in target_renames:
        target = target_renames[target]
    return target

def unprefixed_file(f):
    return f.path[:f.path.rfind('.')]

def cpp_to_obj(f):
    return unprefixed_file(f)+'.o'

app_path_cpp = all_files['src/core/app_path.cpp']
#log_hpp = all_files['src/utils/log.hpp']

def get_sources_deps(f, cat, exclude):
    a = []
    for pf in f.all_source_deps:
        pf.used = True
        if pf == app_path_cpp:
            if cat == 'test-run':
                a.append('<library>app_path_test.o')
            else:
                a.append('<library>app_path_exe.o')
        elif pf != exclude:
            a.append('<library>'+cpp_to_obj(pf))
    return a

def requirement_action(f, act):
    if f.path in src_requirements:
        act(src_requirements[f.path])
    if f.root in dir_requirements:
        act(dir_requirements[f.root])

def get_requirements(f):
    a = []
    for name in f.all_lib_deps:
        a.append('<library>'+name)
    requirement_action(f, lambda r: a.append(r))
    return a

all_targets = []
def mark_target(target, dep = None):
    all_targets.append([target, dep])
    return target

def generate(type, files, requirements, get_target_cb = get_target):
    for f in files:
        f.used = True
        src = f.path
        target = get_target_cb(f)
        deps = get_sources_deps(f, type, f)
        deps += get_requirements(f)
        if target in target_requirements:
            deps.append(target_requirements[target])

        if type == 'test-run':
            if f.root in dir_nocoverage or f.path in file_nocoverage:
                deps.append('<covflag>nocover $(GCOV_NO_BUILD)')
            else:
                iright = len(f.root)
                base = 'src/' + src[6:iright+1] + src[iright+6:-3]
                for ext in ('hpp', 'cpp', 'h', 'c'):
                    if base+ext in all_files:
                        deps.append('<covfile>'+base+ext)
                        f.have_coverage = True
            #if src in coverage_requirements:
                #deps.append(coverage_requirements[src])
            src = inject_variable_prefix(f.path)
        elif type == 'exe':
            src = inject_variable_prefix(f.path)
        elif type == 'lib':
            src += '.lib.o'

        print(type, ' ', mark_target(target), ' :\n  ', src, '\n:', sep='')

        if requirements:
            print(' ', requirements)

        if f.path in remove_requirements:
            l = remove_requirements[f.path]
            for s in sorted(deps):
                if s not in l:
                    print(' ', s)
        else:
            for s in sorted(deps):
                print(' ', s)
        print(';')

def inject_variable_prefix(path):
    if start_with(path, 'src/'):
        path = '$(REDEMPTION_SRC_PATH)' + path[3:]
    elif start_with(path, 'tests/'):
        path = '$(REDEMPTION_TEST_PATH)' + path[5:]
    elif start_with(path, 'projects/redemption_configs/'):
        path = '$(REDEMPTION_CONFIG_PATH)' + path[27:]
    return path

def generate_obj(files):
    for f in files:
        if f.type == 'C' and f != app_path_cpp:
            print('obj', mark_target(cpp_to_obj(f)), ':', inject_variable_prefix(f.path), end='')
            requirement_action(f, lambda r: print(' :', r, end=''))
            print(' ;')

generate('exe', mains, '$(EXE_DEPENDENCIES)')
print()

generate('lib', libs, '$(LIB_DEPENDENCIES)', lambda f: 'lib'+get_target(f))
for f in libs:
    print('obj ', mark_target(f.path), '.lib.o :\n  ', inject_variable_prefix(f.path), '\n:\n  $(LIB_DEPENDENCIES)', sep='')
    requirement_action(f, lambda r: print(' ', r))
    print(';')
print()

generate('test-run', tests, '', unprefixed_file)
print()

generate_obj(sources)
print()
generate_obj(libs)
print()

###
### Test alias
###

# alias by name
test_targets = [(get_target(f), [0, f]) for f in tests]
test_targets_counter = OrderedDict(test_targets)
for t in test_targets:
    test_targets_counter[t[0]][0] += 1

for k,t in test_targets_counter.items():
    if t[0] == 1:
        f = t[1]
        unprefixed = unprefixed_file(f)
        print('alias', mark_target(k, unprefixed), ':', unprefixed, ';')
        if f.have_coverage:
            print('alias ',
                  mark_target(k+'.coverage', unprefixed), ' : ',
                  mark_target(unprefixed+'.coverage'), ' ;', sep='')

# alias by directory
dir_tests = OrderedDict()
for f in tests:
    dir_tests.setdefault(f.root, []).append(unprefixed_file(f))

for name,aliases in dir_tests.items():
    print('explicit', name, ';')
    print('alias ', mark_target(name), ' :\n  ', '\n  '.join(aliases), '\n;', sep='')

print('explicit tests.full ;')
print('alias tests.full :')
for name in dir_tests.keys():
    print(' ', name)
print(';')

# explicit sashimi
print('explicit ')
for target,dep in all_targets:
    if -1 != target.find("/sashimi/") or (dep and -1 != dep.find("/sashimi/")):
        print(' ', target)
print(';')

import sys
for f in all_files.values():
    if f.type == 'C' and not f.used and \
    f.path != 'src/capture/ocr/display_learning.cc' and \
    f.path != 'src/capture/ocr/extract_text.cc' and \
    f.path != 'src/capture/ocr/learning.cc' and \
    f.path != 'src/capture/ocr/ppocr_extract_text.cpp':
        if not start_with(f.path, 'src/sashimi/'):
            print('\x1B[1;31m', f.path, ' is unused\x1B[0m', file=sys.stderr, sep='')
        else:
            print(f.path, 'is unused', file=sys.stderr)
