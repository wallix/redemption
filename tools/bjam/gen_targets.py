#!/usr/bin/env python3

import glob
import os
import sys
from collections import OrderedDict

#project_root = '../..'
project_root = '.'

includes = set((
    'src/',
    'projects/redemption_configs/redemption_src/',
    'src/capture/ocr/',
    'tests/includes/',
))

disable_tests = set((
    # 'tests/utils/crypto/test_ssl_mod_exp_direct.cpp',
    # 'tests/test_meta_protocol2.cpp',
    'tests/utils/test_divers.cpp',
))

disable_srcs = set((
    'src/main/redrec.cpp', # special case in Jamroot
    'tests/includes/test_only/test_framework/impl/register_exception.cpp',
    'tests/includes/test_only/test_framework/impl/test_framework.cpp',
    'tests/includes/test_only/test_framework/redemption_unit_tests.cpp',
    'tests/includes/test_only/log_as_logtest.cpp',
    'src/utils/log_as_syslog.cpp',
    'src/utils/log_as_logprint.cpp',
))

src_deps = dict((
    ('src/acl/module_manager.hpp', glob.glob('src/acl/module_manager/*.cpp')),
    ('src/proxy_recorder/proxy_recorder.hpp', ['src/proxy_recorder/proxy_recorder.cpp']),
    ('src/utils/primitives/primitives_internal.hpp', ['src/utils/primitives/primitives_sse2.cpp']),

))

class Dep:
    def __init__(self, linkflags=None, cxxflags=None):
        self.linkflags = linkflags or []
        self.cxxflags = cxxflags or []
    def union(self, other):
        return Dep(
            linkflags=self.linkflags + other.linkflags,
            cxxflags=self.cxxflags + other.cxxflags)

inc_test_dep = Dep(cxxflags=[
    '<include>$(REDEMPTION_TEST_PATH)/includes', # for lcg_random or fixed_random
    '<variant>debug:<define>SHOW_DEBUG_TRACES=1',
    '<variant>san:<define>SHOW_DEBUG_TRACES=1',
])

src_requirements = dict((
    ('src/lib/scytale.cpp', inc_test_dep),
    ('src/capture/ocr/main/ppocr_extract_text.cpp', Dep(linkflags=['<library>log_print.o'])),
    # otherwise std::bad_alloc or mmap error
    ('tests/front/test_front.cpp', Dep(linkflags=['<noopstacktrace>on'])),
))

obj_requirements = dict((
    ('tests/includes/test_only/test_framework/working_directory.cpp', Dep(cxxflags=[
        '<variant>debug:<define>RED_COMPILE_TYPE=debug',
        '<variant>release:<define>RED_COMPILE_TYPE=release',
        '<variant>san:<define>RED_COMPILE_TYPE=san',
    ])),
))

dir_requirements = dict((
))

target_requirements = dict((
    # ('libredrec', ['<library>log.o']),
))

# because include a .cpp file...
remove_requirements = dict((
    # ('tests/capture/test_capture.cpp', ['<library>src/capture/capture.o']),
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
    ('proxy_recorder_cli', 'proxy_recorder'),
    ('nla_server_cli', 'nla_server'),
))

target_nosyslog = set((
    'proxy_recorder',
    'nla_server',
    'rdpinichecker',
    'rdpclient',
    #'main_client_redemption',
))

#coverage_requirements = dict((
#))

dir_nocoverage = set((
    'tests/server',
    'tests/system/common',
    'tests/system/emscripten',
    'tests/client_mods',
    'tests/lib',
))
file_nocoverage = set(glob.glob('tests/*.cpp'))

sys_lib_assoc = dict((
    ('png.h', Dep(
        linkflags=['<library>png'])),
    ('krb5.h', Dep(
        linkflags=['<library>krb5'])),
    ('gssapi/gssapi.h', Dep(
        linkflags=['<library>gssapi_krb5'])),
    ('snappy-c.h', Dep(
        linkflags=['<library>snappy'])),
    ('zlib.h', Dep(
        linkflags=['<library>z'],
        cxxflags=['<define>ZLIB_CONST'])),
    ('openssl/ssl.h', Dep(
        linkflags=['<library>openssl'])),
    ('boost/stacktrace.hpp', Dep(
        linkflags=['$(BOOST_STACKTRACE_LINKFLAGS)'],
        cxxflags=['$(BOOST_STACKTRACE_CXXFLAGS)'])),
))
sys_lib_prefix = (
    ('libavformat/', Dep(
        linkflags=['<library>ffmpeg'],
        cxxflags=['$(FFMPEG_CXXFLAGS)'])),
    ('libavcodec/', Dep(
        cxxflags=['$(FFMPEG_CXXFLAGS)'])),
    ('openssl/', Dep(
        linkflags=['<library>crypto'])),
)


user_lib_assoc = dict((
    ('program_options/program_options.hpp', Dep(
        linkflags=['<library>program_options'])),
))

user_lib_prefix = (
    ('ppocr/', Dep(
        linkflags=['<library>ppocr'])),
)

def get_lib(inc, lib_assoc, lib_prefix):
    if inc in lib_assoc:
        return lib_assoc[inc]

    for t in lib_prefix:
        if startswith(inc, t[0]):
            if not (len(t) > 2 and inc in t[2]):
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
        #self.direct_link_deps = set()
        #self.direct_cxx_deps = set()
        self.all_source_deps = None # then set()
        self.all_link_deps = None # then set()
        self.all_cxx_deps = None # then set()
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
exes = []
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
    f = File(root, path, type)
    a.append(f)
    return f

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

def startswith(str, prefix):
    return str[:len(prefix)] == prefix

def paths_in(paths, filters):
    return [pf for pf in paths if pf.path in filters]

def paths_not_in(paths, filters):
    return [pf for pf in paths if pf.path not in filters]

filter_targets = []

argv_gen = (arg for arg in sys.argv)
default_options = {}
has_set_arg = False
next(argv_gen)
try:
    set_arg = lambda name: lambda arg: default_options.setdefault(name, arg)
    def add_deps_src(arg):
        a = arg.split(',')
        src_deps[a[0]] = glob.glob(a[1])
    def add_disable_src(arg):
        disable_srcs.add(arg)
        base = arg[-4:]
        if base == '.hpp':
            disable_srcs.add(arg[:-3] + 'cpp')
    options = {
        '--src': lambda arg: get_files(sources, arg),
        '--main': set_arg('main'),
        '--lib': set_arg('lib'),
        '--test': set_arg('test'),
        '--src-system': set_arg('system'),
        '--disable-src': add_disable_src,
        '--deps-src': add_deps_src,
        '--include': lambda arg: includes.add(arg),
    }
    while True:
        arg = next(argv_gen)
        if arg == '-h' or arg == '--help':
            for k in options.keys():
                print(k, '[path]')
        lbd = options.get(arg)
        if lbd:
            lbd(next(argv_gen))
            has_set_arg = True
        else:
            filter_targets.append(arg)
            filter_targets += argv_gen
            break
except StopIteration:
    pass

# remove src_deps key in disable_srcs
new_src_deps = []
for (k,deps) in src_deps.items():
    if k not in disable_srcs:
        new_deps = [x for x in deps if x not in disable_srcs]
        if new_deps:
            new_src_deps.append((k,new_deps))
src_deps = dict(new_src_deps)

src_mains = default_options.get('main', 'src/main')
src_libs = default_options.get('lib', 'src/lib')
src_tests = default_options.get('test', 'tests')
system_type = default_options.get('system', 'linux')

includes.add('src/system/' + system_type + '/')

for d in os.listdir('src'):
    if d not in (
        'ftests',
        'main',
        'lib',
        'system',
    ):
        get_files(sources, 'src/' + d)

get_files(sources, "src/system/" + system_type + "/system")
get_files(sources, 'tests/includes/test_only')

for t in ((mains, src_mains), (libs, src_libs)):
    if not t[1]:
        continue
    for path in glob.glob(t[1] + "/*.hpp"):
        append_file(sources, t[1], path, 'H')
    for path in glob.glob(t[1] + "/*.cpp"):
        f = append_file(sources, t[1], path, 'C')
        t[0].append(f)

if has_set_arg:
    ocr_mains = []
else :
    ocr_mains = [f for f in sources if startswith(f.path, 'src/capture/ocr/main/')]

files_on_tests = []

def is_exclude_path(path):
    return startswith(path, 'tests/includes/') \
        or startswith(path, 'tests/system/'
                      + (system_type == 'linux' and 'emscripten' or 'linux')
                      + '/system/') \
        or startswith(path, 'tests/web_video/') \

if not filter_targets:
    if src_tests:
        get_files(files_on_tests, src_tests)
        tests = [f for f in files_on_tests \
            if f.type != 'H' \
            and not is_exclude_path(f.path) \
            and f.path not in disable_tests
        ]
    libs = paths_not_in(libs, disable_srcs)
    mains = paths_not_in(mains, disable_srcs)
else:
    mains = paths_in(mains, filter_targets)
    libs = paths_in(libs, filter_targets)

sources = paths_not_in(sources, disable_srcs)
sources += [pf for pf in files_on_tests \
    if pf.type == 'H'
    and not is_exclude_path(pf.path)
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
ocr_mains = sorted(ocr_mains, key=kpath)
mains = sorted(mains, key=kpath)
libs = sorted(libs, key=kpath)
tests = sorted(tests, key=kpath)

def tuple_files(l):
    return [(f.path, f) for f in l]

all_files = OrderedDict(tuple_files(tests))
all_files.update(tuple_files(sources))
all_files.update(tuple_files(ocr_mains))
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
                            elif file_name in disable_srcs:
                                found = True
                                break
                        if not found:
                            file_name = os.path.normpath(path[0:path.rfind('/')] + '/' + inc)
                            if file_name in all_files:
                                user_includes.append(all_files[file_name])
                            elif file_name not in disable_srcs:
                                unknown_user_includes.append(inc)
                    if len(line) and line[0] == '<':
                        system_includes.append(line[1:line.rfind('>')])
    return set(user_includes), set(system_includes), set(unknown_user_includes)

for name, f in all_files.items():
    f.user_includes, f.system_includes, f.unknown_user_includes = get_includes(f.path)

# remove .cpp include from user_includes
for f in tests:
    l = ['<library>{fp}o'.format(fp=finc.path[:-3]) for finc in f.user_includes if finc.path.endswith('.cpp')]
    if l:
        remove_requirements.setdefault(f.path, []).extend(l)

#for f in sources:
    #print("--", f.path, [a.path for a in f.user_includes], f.system_includes, f.unknown_user_includes)


###
### Get deps (cpp, hpp, lib)
###

for name, f in all_files.items():
    direct_link_deps = set()
    direct_cxx_deps = set()

    def append(direct_link_deps, direct_cxx_deps, l, k):
        if k in l:
            d = l[k]
            direct_link_deps.update(d.linkflags)
            direct_cxx_deps.update(d.cxxflags)

    append(direct_link_deps, direct_cxx_deps, src_requirements, f.path)
    append(direct_link_deps, direct_cxx_deps, dir_requirements, f.root)
    append(direct_link_deps, direct_cxx_deps, obj_requirements, f.path)

    direct_source_deps = set()
    for pf in f.user_includes:
        if pf.path in src_deps:
            for path in src_deps[pf.path]:
                direct_source_deps.add(all_files[path])
        for ext in ('.cpp', '.cc'):
            cpp_name = pf.path[:-4]+ext
            if cpp_name in all_files:
                direct_source_deps.add(all_files[cpp_name])
    f.direct_source_deps = direct_source_deps

    def append(direct_link_deps, direct_cxx_deps, d):
        if d:
            direct_link_deps.update(d.linkflags)
            direct_cxx_deps.update(d.cxxflags)

    for name in f.system_includes:
        append(direct_link_deps, direct_cxx_deps, get_system_lib(name))
    for name in f.unknown_user_includes:
        append(direct_link_deps, direct_cxx_deps, get_user_lib(name))
    for pf in f.user_includes:
        append(direct_link_deps, direct_cxx_deps, get_user_lib(pf.path))

    f.direct_link_deps = direct_link_deps
    f.direct_cxx_deps = direct_cxx_deps


empty_set = set()
def compute_all_source_deps(f):
    if f.all_source_deps is None:
        srcs = f.direct_source_deps
        libs = f.direct_link_deps
        cxxs = f.direct_cxx_deps
        incs = f.user_includes
        f.direct_source_deps = empty_set
        f.direct_link_deps = empty_set
        f.direct_cxx_deps = empty_set
        f.user_includes = empty_set

        all_source_deps = srcs.copy()
        all_cxx_deps = cxxs.copy()
        all_link_deps = libs.copy()

        for pf in srcs:
            compute_all_source_deps(pf)
            if pf.all_source_deps is not None:
                all_source_deps.update(pf.all_source_deps)
                all_link_deps.update(pf.all_link_deps)

        for pf in incs:
            compute_all_source_deps(pf)
            if pf.all_source_deps is not None:
                all_source_deps.update(pf.all_source_deps)
                all_link_deps.update(pf.all_link_deps)
                all_cxx_deps.update(pf.all_cxx_deps)

        f.direct_source_deps = srcs
        f.direct_link_deps = libs
        f.direct_cxx_deps = cxxs
        f.user_includes = incs

        f.all_source_deps = all_source_deps
        f.all_link_deps = all_link_deps
        f.all_cxx_deps = all_cxx_deps


for f in all_files.values():
    compute_all_source_deps(f)


###
### Generate
###

print("""#
# DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN
# run `bjam targets.jam`
# or
# run `tools/bjam/gen_targets.py`
#
""")

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

def generate(type, files, requirements, get_target_cb = get_target):
    for f in files:
        f.used = True
        src = f.path
        target = get_target_cb(f)
        deps = get_sources_deps(f, type, f)
        deps += f.all_link_deps
        deps += f.all_cxx_deps
        deps += requirements
        if target in target_requirements:
            deps += target_requirements[target]

        if type == 'test-run':
            if f.root in dir_nocoverage or f.path in file_nocoverage:
                deps.append('$(GCOV_NO_BUILD)')
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
            src = cpp_to_obj(f)
        elif type == 'lib':
            src += '.lib.o'

        print(type, ' ', target, ' :\n  ', src, '\n:', sep='')

        # <library> following by <...other...>
        deps = sorted(set(deps), key=lambda s: s if s.startswith('<library>') else 'Z'+s)

        if f.path in remove_requirements:
            l = remove_requirements[f.path]
            for s in deps:
                if s not in l:
                    print(' ', s)
        else:
            for s in deps:
                print(' ', s)
        print(';')

def inject_variable_prefix(path):
    if startswith(path, 'src/'):
        path = '$(REDEMPTION_SRC_PATH)' + path[3:]
    elif startswith(path, 'tests/'):
        path = '$(REDEMPTION_TEST_PATH)' + path[5:]
    elif startswith(path, 'projects/redemption_configs/'):
        path = '$(REDEMPTION_CONFIG_PATH)' + path[27:]
    return path

def generate_obj(files):
    for f in files:
        if f.type == 'C' and f != app_path_cpp:
            print('obj', cpp_to_obj(f), ':', inject_variable_prefix(f.path), end='')
            if f.all_cxx_deps:
                print(' :', ' '.join(sorted(f.all_cxx_deps)), end='')
            if f.path.startswith('tests/includes/test_only/'):
                if not f.all_cxx_deps:
                    print(' :', end='')
                print(' $(CXXFLAGS_TEST)', end='')
            print(' ;')

# filter target
if filter_targets:
    tests = []
    keep_paths = sys.argv[1:]
    mains = paths_in(mains, filter_targets)
    libs = paths_in(libs, filter_targets)
    sources = set()
    ocr_mains = []
    for f in libs:
        sources.update(f.all_source_deps)
    for f in mains:
        sources.update(f.all_source_deps)

generate('exe', [f for f in mains if (get_target(f) not in target_nosyslog)],
         ['$(EXE_DEPENDENCIES)'])
generate('exe', [f for f in mains if (get_target(f) in target_nosyslog)],
         ['$(EXE_DEPENDENCIES_NO_SYSLOG)'])
generate('exe', ocr_mains, ['$(GCOV_NO_BUILD)'])
print()

generate('lib', libs, ['$(LIB_DEPENDENCIES)', '<library>log.o'], lambda f: 'lib'+get_target(f))
for f in libs:
    print('obj ', f.path, '.lib.o :\n  ', inject_variable_prefix(f.path), '\n:\n  $(LIB_DEPENDENCIES)', sep='')
    if f.all_cxx_deps:
        print(' ', '\n  '.join(f.all_cxx_deps))
    print(';')
print()

generate('test-run', tests, [], unprefixed_file)
print()

if has_set_arg:
    generate_obj(f for f in sources if f.used)
else:
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

coverages = []
for k,t in test_targets_counter.items():
    if t[0] == 1:
        f = t[1]
        unprefixed = unprefixed_file(f)
        print('alias', k, ':', unprefixed, ';')
        if f.have_coverage:
            coverages.append((k,unprefixed))

if coverages:
    print('if $(ENABLE_COVERAGE) = "on"')
    print('{')
    for t in coverages:
        print('alias ', t[0]+'.coverage', ' : ', t[1]+'.coverage', ' ;', sep='')
    print('}')

# alias by directory
dir_tests = OrderedDict()
for f in tests:
    dir_tests.setdefault(f.root, []).append(unprefixed_file(f))

explicit_no_rec = []
for name,aliases in dir_tests.items():
    name += '.norec'
    explicit_no_rec.append(name)
    print('alias ', name, ' :\n  ', '\n  '.join(aliases), '\n;', sep='')


# alias for recursive directory
dir_rec_tests = OrderedDict()
for name,aliases in dir_tests.items():
    dir_rec_tests.setdefault(name, []).append(name+'.norec')
    dirname = os.path.dirname(name)
    if dirname:
        dir_rec_tests.setdefault(dirname, []).append(name)

explicit_rec = []
for name,aliases in dir_rec_tests.items():
    explicit_rec.append(name)
    sorted(aliases)
    print('alias ', name, ' :\n  ', '\n  '.join(aliases), '\n;', sep='')

if explicit_no_rec:
    print('explicit\n  ', '\n  '.join(explicit_no_rec), '\n;', sep='')
if explicit_rec:
    print('explicit\n  ', '\n  '.join(explicit_rec),    '\n;', sep='')


if not filter_targets and not has_set_arg:
    for f in all_files.values():
        if f.type == 'C' and not f.used and \
        f.path != 'src/capture/ocr/display_learning.cc' and \
        f.path != 'src/capture/ocr/extract_text.cc' and \
        f.path != 'src/capture/ocr/learning.cc' and \
        f.path != 'src/capture/ocr/ppocr_extract_text.cpp':
            print('\x1B[1;31m', f.path, ' is unused\x1B[0m', file=sys.stderr, sep='')
