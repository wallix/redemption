#!/usr/bin/python3 -O

import glob
import os
from collections import OrderedDict

#project_root = '../..'
project_root = '.'

includes = set([
    'src/',
    'src/system/linux/',
    'src/system/linux/system/',
    'projects/redemption_configs/redemption_src/',
    'src/capture/ocr/',
    'tests/includes/',
])

disable_tests = set([
    'tests/utils/crypto/test_ssl_mod_exp_direct.cpp',
    'tests/test_meta_protocol2.cpp',
])

disable_srcs = set([
    'src/main/redrec.cpp', # special case in Jamroot
    'src/system/linux/system/redemption_unit_tests.cpp',
    'src/system/linux/system/register_error_exception.cpp',
    'src/system/linux/system/test_framework.cpp',
    'src/utils/log_as_syslog.cpp',
    'src/utils/log_as_logemasm.cpp',
    'src/utils/log_as_logprint.cpp',
    'src/utils/log_as_logtest.cpp',
])

src_deps = dict((
    ('src/acl/module_manager.hpp', glob.glob('src/acl/module_manager/*.cpp')),
))

src_requirements = dict((
    ('src/lib/scytale.cpp', '<include>$(REDEMPTION_TEST_PATH)/includes'), # for lcg_random
    ('src/main/rdpheadless.cpp', '<include>$(REDEMPTION_TEST_PATH)/includes'), # for lcg_random
    ('src/main/rdp_client.cpp', '<include>$(REDEMPTION_TEST_PATH)/includes'), # for lcg_random
    ('src/main/main_client_redemption.cpp', '<include>$(REDEMPTION_TEST_PATH)/includes'), # for fixed_random
    ('tests/includes/test_only/front/fake_front.cpp', '<include>$(REDEMPTION_TEST_PATH)/includes'),
    ('tests/includes/test_only/fake_graphic.cpp', '<include>$(REDEMPTION_TEST_PATH)/includes'),
    ('src/capture/video_recorder.cpp', '<cxxflags>-Wno-deprecated-declarations'),
))

dir_requirements = dict((
    # ('tests/sashimi', '<cxxflags>-Wno-format <cxxflags>-Wno-format-security'),
))

target_requirements = dict((
    # ('libredrec', '<library>log.o'),
))

# because include a .cpp file...
remove_requirements = dict((
    ('tests/capture/test_capture.cpp', '<library>src/capture/capture.o'),
    ('tests/utils/test_rle.cpp', '<library>src/utils/rle.o'),
))

# This is usefull if several source files have the same name to disambiguate tests
target_pre_renames = dict((
#    ('tests/core/RDP/test_pointer.cpp', 'test_rdp_pointer'),
))

target_renames = dict((
    ('main', 'rdpproxy'),
    ('do_recorder', 'redrec'),
    ('ini_checker', 'rdpinichecker'),
    ('rdp_client', 'rdpclient'),
))

target_nosyslog = set([
    'proxy_recorder',
    'rdpinichecker',
    'rdpclient',
    #'main_client_redemption',
    #'rdpheadless',
])

#coverage_requirements = dict((
#))

dir_nocoverage = set([
    'tests/server/',
    'tests/system/common/',
    'tests/system/emscripten/',
    'tests/client_mods'
])
file_nocoverage = set(glob.glob('tests/*.cpp'))

sys_lib_assoc = dict((
    ('png.h', '<library>png'),
    ('krb5.h', '<library>krb5'),
    ('gssapi/gssapi.h', '<library>gssapi_krb5'),
    ('snappy-c.h', '<library>snappy'),
    ('zlib.h', '<library>z <define>ZLIB_CONST'),
))
sys_lib_prefix = (
    ('libavformat/', '<library>ffmpeg'),
    ('openssl/', '<library>crypto'),
)

user_lib_assoc = dict((
    ('program_options/program_options.hpp', '<library>program_options'),
    ('src/core/error.hpp', '<variant>debug:<library>dl <variant>san:<library>dl'), # Boost.stacktrace
    ('openssl_crypto.hpp', '<library>crypto'),
    ('openssl_tls.hpp', '<library>openssl'),
))
user_lib_prefix = (
    ('ppocr/', '<library>ppocr'),
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

def HFile(d, f):
    return File(d, d+'/'+f, 'H')

def CFile(d, f):
    return File(d, d+'/'+f, 'C')

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

def get_type(name):
    if name[-4:] == '.hpp':
        return 'H'
    elif name[-4:] == '.cpp':
        return 'C'
    elif name[-2:] == '.h':
        return 'H'
    elif name[-2:] == '.c':
        return 'C'
    elif name[-3:] == '.hh':
        return 'H'
    elif name[-3:] == '.cc':
        return 'C'

def get_files(a, dirpath):
    for root, dirs, files in os.walk(project_root + '/' + dirpath):
        for name in files:
            type = get_type(name)
            if type:
                append_file(a, root, root+'/'+name, type)

def start_with(str, prefix):
    return str[:len(prefix)] == prefix

get_files(sources, "src/acl")
get_files(sources, "src/capture")
get_files(sources, "src/client_redemption")
get_files(sources, "src/core")
get_files(sources, "src/front")
get_files(sources, "src/gdi")
get_files(sources, "src/keyboard")
get_files(sources, "src/lib")
get_files(sources, "src/mod")
get_files(sources, "src/regex")
get_files(sources, "src/system/linux/system")
get_files(sources, "src/transport")
get_files(sources, "src/utils")
get_files(sources, 'tests/includes/test_only')

for t in ((mains, 'src/main'), (libs, 'src/lib')):
    for path in glob.glob(t[1] + "/*.hpp"):
        append_file(sources, t[1], path, 'H')
    for path in glob.glob(t[1] + "/*.cpp"):
        append_file(t[0], t[1], path, 'C')

files_on_tests = []
get_files(files_on_tests, 'tests')
tests = [f for f in files_on_tests \
    if f.type != 'H' \
    and not start_with(f.path, 'tests/includes/') \
    and not start_with(f.path, 'tests/system/emscripten/system/') \
    and not start_with(f.path, 'tests/web_video/') \
    and f.path not in disable_tests
]

libs = [f for f in libs if f.path not in disable_srcs]
mains = [f for f in mains if f.path not in disable_srcs]
sources = [f for f in sources if f.path not in disable_srcs]
sources += [f for f in files_on_tests \
    if f.type == 'H' \
    and not start_with(f.path, 'tests/includes/') \
    and not start_with(f.path, 'tests/system/emscripten/system/') \
    and not start_with(f.path, 'tests/web_video/')
]

extra_srcs = (
    ('src/configs/config.hpp', HFile(
        'projects/redemption_configs/redemption_src',
        'configs/config.hpp')
    ),
    ('src/configs/config.cpp', CFile(
        'projects/redemption_configs/redemption_src',
        'configs/config.cpp')
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
                            file_name = os.path.normpath(dir_name+inc)
                            if file_name in all_files:
                                user_includes.append(all_files[file_name])
                                found = True
                                break
                        if not found:
                            file_name = os.path.normpath(path[0:path.rfind('/')] + '/' + inc)
                            if file_name in all_files:
                                user_includes.append(all_files[file_name])
                            else:
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
        if pf.path in src_deps:
            for path in src_deps[pf.path]:
                deps.append(all_files[path])
        for ext in ('.cpp', '.cc'):
            cpp_name = pf.path[:-4]+ext
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
print('# run `bjam targets.jam`')
print('# or')
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
    for dep in f.all_lib_deps:
        a.append(dep)
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

generate('exe', [f for f in mains if (get_target(f) not in target_nosyslog)], '$(EXE_DEPENDENCIES)')
generate('exe', [f for f in mains if (get_target(f) in target_nosyslog)], '$(EXE_DEPENDENCIES_NO_SYSLOG)')
print()

generate('lib', libs, '$(LIB_DEPENDENCIES)\n  <library>log.o', lambda f: 'lib'+get_target(f))
for f in libs:
    print('obj ', mark_target(f.path), '.lib.o :\n  ', inject_variable_prefix(f.path), '\n:\n  $(LIB_DEPENDENCIES)', sep='')
    requirement_action(f, lambda r: print(' ', r))
    print(';')
print()

generate('test-run', tests, '', unprefixed_file)
print()

generate_obj(sources)
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

explicit_no_rec = []
for name,aliases in dir_tests.items():
    name += '.norec'
    explicit_no_rec.append(name)
    print('alias ', mark_target(name), ' :\n  ', '\n  '.join(aliases), '\n;', sep='')


# alias for recursive directory
dir_rec_tests = OrderedDict()
for name,aliases in dir_tests.items():
    dir_rec_tests.setdefault(name, []).append(name+'.norec')

for name,aliases in dir_tests.items():
    dirname = os.path.dirname(name)
    if dirname:
        dir_rec_tests.get(dirname, []).append(name)

explicit_rec = []
for name,aliases in dir_rec_tests.items():
    explicit_rec.append(name)
    print('alias ', mark_target(name), ' :\n  ', '\n  '.join(aliases), '\n;', sep='')

print('explicit\n  ', '\n  '.join(explicit_no_rec), '\n;', sep='')
print('explicit\n  ', '\n  '.join(explicit_rec),    '\n;', sep='')


import sys
for f in all_files.values():
    if f.type == 'C' and not f.used and \
    f.path != 'src/capture/ocr/display_learning.cc' and \
    f.path != 'src/capture/ocr/extract_text.cc' and \
    f.path != 'src/capture/ocr/learning.cc' and \
    f.path != 'src/capture/ocr/ppocr_extract_text.cpp':
        print('\x1B[1;31m', f.path, ' is unused\x1B[0m', file=sys.stderr, sep='')
