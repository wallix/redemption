#!/usr/bin/env python3
import os
import re
import sys
from typing import Iterable, Set, Dict, Tuple, TextIO, Callable, Any, NewType
from itertools import chain
from os.path import join as path_join


py_comment_regex = re.compile(r'#[^\n]*')
cpp_comment_regex = re.compile(r'//[^\n]*|/[*](?:[^*]+|[*][^/])*')
xml_comment_regex = re.compile(r'<!--([^-]+|-[^-]|--[^>])*')

identity = lambda x: x

# {logtype: formats}
# {'SESSION_SHARING_GUEST_DISCONNECTION': {
#   'type="AUTHENTICATION_FAILURE" method="Kerberos"'}}
LogFormatType = NewType('LogFormatType', Dict[str, Set[str]])


def read_pyfile(filename) -> str:
    with open(filename) as f:
        return py_comment_regex.sub('', f.read())


def read_xmlfile(filename) -> str:
    with open(filename) as f:
        return xml_comment_regex.sub('', f.read())


def read_cppfile(filename) -> str:
    with open(filename) as f:
        return cpp_comment_regex.sub('', f.read())


def color_builder(ansi_color: str) -> Callable[[Any], str]:
    return lambda x: f'\x1b[{ansi_color}m{x}\x1b[0m'


def update_dict(d: LogFormatType,
                logid_and_kvs: Iterable[Tuple[str, str]],
                patt: re.Pattern,
                colored: Callable[[str], str]) -> None:
    cat = colored('type')
    for logid, kvs in logid_and_kvs:
        data = ''.join(f' {colored(k)}={v}' for k, v in patt.findall(kvs))
        d.setdefault(logid, set()).add(f'{cat}="{logid}"{data}')


def extract_siem_format(src_path: str, color: bool) -> Tuple[LogFormatType,   # proxy
                                                             LogFormatType,   # rdp
                                                             LogFormatType]:  # vnc
    src_path = src_path.rstrip('/')
    colored = color_builder('35') if color else identity

    proxy_logs: LogFormatType = LogFormatType({})
    vnc_logs: LogFormatType = LogFormatType({})
    rdp_logs: LogFormatType = LogFormatType({})
    rdp_and_vnc_logs: LogFormatType = LogFormatType({})
    capture_logs: LogFormatType = LogFormatType({})
    other_logs: LogFormatType = LogFormatType({})
    declared_log_ids: Set[str] = set()

    def to_process(log_extractor, kv_extractor):
        return lambda d, text: update_dict(d, log_extractor.findall(text), kv_extractor, colored)

    # log6 call with or without ternary
    log6_regex = re.compile(r'\blog6\((?:(?!\bLogId:|[?]|;).)*(?:[?])?\s*\bLogId::([A-Z0-9_]+)[^,:)]*(?::\s*LogId::([A-Z0-9_]+)\s*\)?)?,\s*(\{(?:[^)]+|[)][^;])*)?', re.DOTALL)
    # KVLog("key"_av, value) with value "..." | "..."_av | ...
    kv_log6_regex = re.compile(r'KVLog[{(]"([^"]+)"_av,\s*((?:(?!"_av|[)}]?,?\n).)*(?:"(?=_av))?)')
    log6_process = lambda d, text: \
        update_dict(d,
                    chain.from_iterable(
                        ((t[0], t[2]), (t[1], t[2])) if t[1] else ((t[0], t[2]),)
                        for t in log6_regex.findall(text)
                    ),
                    kv_log6_regex, colored)

    server_cert_regex = re.compile(r'\{\n\s*LogId::(\w+),\s*("[^"]+")')

    log_id_sesprobe_regex = re.compile(r'EXECUTABLE_LOG6_ID_AND_NAME\(\s*([A-Z0-9_]+)\s*\)([^)]*)')
    kv_sesprobe_regex = re.compile(r'"([^"]+)"()')  # capture an empty value
    sesprob_process = to_process(log_id_sesprobe_regex, kv_sesprobe_regex)

    sesman_regex = re.compile(r'self\.rdplog\.log\("([^"]+)"([^)]*)')
    kv_sesman_regex = re.compile(r',\s*(\w+)\s*=\s*(\w+|[^,)]+)')
    sesman_process = to_process(sesman_regex, kv_sesman_regex)

    log_siem_cpp_regex = re.compile(r'R"\(\[rdpproxy\].* type="([^"]+)"([^;]+)')
    kv_siem_cpp_regex = re.compile(r'(\w+)="([^"]+)"')
    siem_cpp_process = lambda d, text: \
        update_dict(d, ((m.group(1), m.group(2)) for m in log_siem_cpp_regex.finditer(text)),
                    kv_siem_cpp_regex, colored)

    log_id_enum_regex = re.compile(r'\n    f\(([A-Z0-9_]+),')

    dirpath = ''  # used in for
    cppfile_from_dirpath = lambda filename: read_cppfile(path_join(dirpath, filename))
    filter_valid_cpp = lambda filename: filename.endswith('.hpp') or filename.endswith('.cpp')

    def update(d, process, filenames):
        for text in map(cppfile_from_dirpath, filenames):
            process(d, text)

    cat = colored('type')

    for dirpath, _, filenames_ in os.walk(src_path):
        filenames = filter(filter_valid_cpp, filenames_)

        if dirpath.startswith(f'{src_path}/mod/rdp'):
            for filename in filenames:
                text = cppfile_from_dirpath(filename)
                log6_process(rdp_logs, text)
                sesprob_process(rdp_logs, text)
                if filename == 'rdp_negociation.cpp' and dirpath == f'{src_path}/mod/rdp':
                    for (logid, desc) in server_cert_regex.findall(text):
                        rdp_logs.setdefault(logid, set()).add(f'{cat}="{logid}" description={desc}')

        elif dirpath.startswith(f'{src_path}/mod/vnc'):
            update(vnc_logs, log6_process, filenames)

        elif dirpath.startswith(f'{src_path}/capture'):
            update(capture_logs, log6_process,
                   (filename for filename in filenames if filename != 'agent_data_extractor.cpp'))

        elif dirpath == f'{src_path}/acl/module_manager':
            update(other_logs, log6_process,
                   (filename for filename in filenames if filename != 'create_module_rdp.cpp' and
                                                          filename != 'create_module_vnc.cpp'))
            log6_process(rdp_logs, cppfile_from_dirpath('create_module_rdp.cpp'))
            log6_process(vnc_logs, cppfile_from_dirpath('create_module_vnc.cpp'))

        elif dirpath == f'{src_path}/acl':
            update(other_logs, log6_process,
                   (filename for filename in filenames if filename != 'connect_to_target_host.hpp'))
            text = cppfile_from_dirpath('connect_to_target_host.hpp')
            log6_process(rdp_and_vnc_logs, text)

        elif dirpath.startswith(f'{src_path}/front'):
            for text in map(cppfile_from_dirpath, filenames):
                log6_process(rdp_and_vnc_logs, text)

        elif dirpath == f'{src_path}/core':
            for filename in filenames:
                if filename == 'log_id.hpp':
                    declared_log_ids = set(log_id_enum_regex.findall(cppfile_from_dirpath(filename)))
                else:
                    text = cppfile_from_dirpath(filename)
                    log6_process(rdp_and_vnc_logs, text)

        else:
            update(other_logs, log6_process, filenames)

    siem_cpp_process(proxy_logs, read_cppfile(f'{src_path}/utils/log_siem.cpp'))
    sesman_process(proxy_logs, read_pyfile(f'{src_path}/../tools/sesman/sesmanworker/sesman.py'))

    # merge rdp_and_vnc_logs into rdp_logs and vnc_logs
    for k, l in rdp_and_vnc_logs.items():
        rdp_logs.setdefault(k, set()).update(l)
        vnc_logs.setdefault(k, set()).update(l)

    # for d in (rdp_logs, vnc_logs, capture_logs, other_logs):
    #     for k,l in d.items():
    #         print(k, '\n ', '\n  '.join(l))
    #     print('------')

    used_logs = set(chain(rdp_logs, vnc_logs, capture_logs, other_logs))
    unused_logs = declared_log_ids - used_logs
    unused_logs.remove('PROBE_STATUS')

    if unused_logs:
        print('some LogId are unused:\n -',
            '\n - '.join(unused_logs), file=sys.stderr)

    if other_logs:
        print('Some LogId are uncategorized.\n -',
            '\n - '.join(other_logs), file=sys.stderr)

    for k in chain(rdp_logs, vnc_logs):
        if k != 'TITLE_BAR':
            capture_logs.pop(k, None)

    for k, l in capture_logs.items():
        rdp_logs.setdefault(k, set()).update(l)
        vnc_logs.setdefault(k, set()).update(l)

    return proxy_logs, rdp_logs, vnc_logs


def extract_doc_siem(docfile: str) -> Tuple[LogFormatType,   # proxy
                                            LogFormatType,   # rdp
                                            LogFormatType]:  # vnc
    type_regex = re.compile(r'<para><literal>(rdpproxy: \[rdpproxy\]|(?:rdpproxy: )?\[(?:RDP|VNC) Session\])'
                            r' (?:(?!type=|<).)*type=["”]([^"”]+)["”][^<]*',
                            re.DOTALL)

    proxy_logs: LogFormatType = LogFormatType({})
    rdp_logs: LogFormatType = LogFormatType({})
    vnc_logs: LogFormatType = LogFormatType({})

    d = {
        'rdpproxy: [rdpproxy]': proxy_logs,
        'rdpproxy: [RDP Session]': rdp_logs,
        'rdpproxy: [VNC Session]': vnc_logs,
        '[RDP Session]': rdp_logs,
        '[VNC Session]': vnc_logs,
    }

    for m in type_regex.finditer(read_xmlfile(docfile)):
        d[m.group(1)].setdefault(m.group(2), set()).add(m.group(0))

    return proxy_logs, rdp_logs, vnc_logs


class DocSiemChecker:
    def __init__(self,
                 proxy_logs: LogFormatType,
                 rdp_logs: LogFormatType,
                 vnc_logs: LogFormatType,
                 color: bool,
                 ) -> None:
        self.proxy_logs = proxy_logs
        self.rdp_logs = rdp_logs
        self.vnc_logs = vnc_logs

        self.proxy_ids = set(proxy_logs.keys())
        self.rdp_ids = set(rdp_logs.keys())
        self.vnc_ids = set(vnc_logs.keys())

        self.colored = color_builder('31;1') if color else identity
        self.colored2 = color_builder('34;3;1') if color else identity

    def check_bad_quote(self,
                        doc_proxy_logs: LogFormatType,
                        doc_rdp_logs: LogFormatType,
                        doc_vnc_logs: LogFormatType,
                        ) -> str:
        it = chain(doc_proxy_logs.values(),
                   doc_rdp_logs.values(),
                   doc_vnc_logs.values())
        bad_logs = set(log for log in chain.from_iterable(it) if '”' in log)
        if bad_logs:
            l = '\n - '.join(sorted(bad_logs))
            return f'Log with ” instead of ":\n - {l}'
        return ''

    def check_missing_or_unknown(self,
                                 doc_proxy_logs: LogFormatType,
                                 doc_rdp_logs: LogFormatType,
                                 doc_vnc_logs: LogFormatType,
                                 ) -> bool:
        doc_proxy_ids = doc_proxy_logs.keys()
        doc_rdp_ids = doc_rdp_logs.keys()
        doc_vnc_ids = doc_vnc_logs.keys()

        proxy_missing = self.proxy_ids - doc_proxy_ids
        rdp_missing = self.rdp_ids - doc_rdp_ids
        vnc_missing = self.vnc_ids - doc_vnc_ids

        proxy_unknown = doc_proxy_ids - self.proxy_ids
        rdp_unknown = doc_rdp_ids - self.rdp_ids
        vnc_unknown = doc_vnc_ids - self.vnc_ids

        colored = self.colored
        colored2 = self.colored2

        msgs = []

        def append_missing(msg, l, d):
            if l:
                list_format = lambda l: "\n      ".join(sorted(l))
                formats = ''.join(f'\n - {colored(k)}:\n      {list_format(d[k])}' for k in sorted(l))
                msgs.append(f'{colored2(msg)}:{formats}')

        append_missing('Missing log SIEM for [rdpproxy]', proxy_missing, self.proxy_logs)
        append_missing('Missing log SIEM for [RDP Session]', rdp_missing, self.rdp_logs)
        append_missing('Missing log SIEM for [VNC Session]', vnc_missing, self.vnc_logs)

        def append_unknown(msg, l):
            if l:
                elems = '\n - '.join(map(colored, sorted(l)))
                msgs.append(f'{colored2(msg)}:\n - {elems}')

        append_unknown('Unknown log SIEM for [rdpproxy]', proxy_unknown)
        append_unknown('Unknown log SIEM for [RDP Session]', rdp_unknown)
        append_unknown('Unknown log SIEM for [VNC Session]', vnc_unknown)

        return '\n\n'.join(msgs)


def print_siem_format(proxy_logs: LogFormatType,
                      rdp_logs: LogFormatType,
                      vnc_logs: LogFormatType) -> None:
    def show(logs, prefix):
        for log in sorted(chain.from_iterable(logs.values())):
            print(prefix, log)
        print()

    show(proxy_logs, '[rdpproxy] ...')
    show(rdp_logs, '[RDP Session] ...')
    show(vnc_logs, '[VNC Session] ...')


def print_log_siem_constants(proxy_logs: Iterable[str],
                             rdp_logs: Iterable[str],
                             vnc_logs: Iterable[str],
                             output: TextIO = sys.stdout) -> None:
    print("""\
# #####################################################################
# # DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN #
# #####################################################################

# see redemption/tools/log_siem_extractor.py""", file=output)

    def show(name, logs):
        siem = (f"('{siem_type.lower().replace('_', '-')}', '{siem_type}')," for siem_type in sorted(logs))
        print('\n', name, ' = (\n    ', "\n    ".join(siem), '\n)',
              sep='', file=output)

    show('SIEM_FILTERS_RDP_PROXY', proxy_logs)
    show('SIEM_FILTERS_RDP_SESSION', rdp_logs)
    show('SIEM_FILTERS_VNC_SESSION', vnc_logs)


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='Log SIEM extractor')
    parser.add_argument('-l', '--list-format', action='store_true')
    parser.add_argument('-c', '--check-doc', action='append')
    parser.add_argument('-p', '--python-ids', action='store_true')
    parser.add_argument('-o', '--output-python', default=sys.stdout, type=argparse.FileType('w'))
    parser.add_argument('-q', '--no-check-quote', action='store_true')
    parser.add_argument('-m', '--no-check-missing-or-unknown', action='store_true')
    parser.add_argument('--color', action='store_true')
    parser.add_argument('path', nargs='?', default='src')

    args = parser.parse_args()
    has_action = False

    siems = extract_siem_format(args.path, args.color)

    if args.list_format:
        print_siem_format(*siems)
        has_action = True

    if args.check_doc:
        color_file = color_builder('33') if args.color else identity
        doc_checker = DocSiemChecker(*siems, args.color)
        insert_nl = False
        for docfile in args.check_doc:
            doc_siems = extract_doc_siem(docfile)
            err1 = '' if args.no_check_missing_or_unknown \
                else doc_checker.check_missing_or_unknown(*doc_siems)
            err2 = '' if args.no_check_quote \
                else doc_checker.check_bad_quote(*doc_siems)
            if err1 or err2:
                nl = '\n\n' if insert_nl else ''
                sep = '\n\n' if err1 and err2 else ''
                print(f'{nl}{color_file(docfile)}:\n\n{err1}{sep}{err2}')
                insert_nl = True
        has_action = True

    if not has_action or args.python_ids:
        print_log_siem_constants(*siems, args.output_python)
