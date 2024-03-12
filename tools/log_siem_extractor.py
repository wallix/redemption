#!/usr/bin/env python3
import os
import re
import sys
from typing import Iterable, TextIO, Callable, Any, NewType, TypeVar
from itertools import chain
from os.path import join as path_join


py_comment_regex = re.compile(r'#[^\n]*')
cpp_comment_regex = re.compile(r'//[^\n]*|/[*](?:[^*]+|[*][^/])*')
xml_comment_regex = re.compile(r'<!--([^-]+|-[^-]|--[^>])*')

T = TypeVar('T')

def identity(x: T) -> T:
    return x

# {logtype: formats}
# {'SESSION_SHARING_GUEST_DISCONNECTION': {
#   'type="AUTHENTICATION_FAILURE" method="Kerberos"'}}
Params = list[str]
OriginalOrFormatedKVS = str
DataLog = list[tuple[OriginalOrFormatedKVS, Params]]
LogFormatType = NewType('LogFormatType', dict[str, DataLog])


def read_pyfile(filename: str) -> str:
    with open(filename, encoding='utf-8') as f:
        return py_comment_regex.sub('', f.read())


def read_xmlfile(filename: str) -> str:
    with open(filename, encoding='utf-8') as f:
        return xml_comment_regex.sub('', f.read())


def read_cppfile(filename: str) -> str:
    with open(filename, encoding='utf-8') as f:
        return cpp_comment_regex.sub('', f.read())


def color_builder(ansi_color: str) -> Callable[[Any], str]:
    return lambda x: f'\x1b[{ansi_color}m{x}\x1b[0m'


def print_alert_on_list(msg: str, texts: Iterable[str], color: bool) -> None:
    colored = color_builder('33') if color else identity
    print(colored(msg), ':\n - ', '\n - '.join(texts), file=sys.stderr, sep='')


def extract_siem_format(src_path: str, color: bool) -> tuple[LogFormatType,   # proxy
                                                             LogFormatType,   # rdp
                                                             LogFormatType]:  # vnc
    src_path = src_path.rstrip('/')
    colored = color_builder('35') if color else identity
    cat = colored('type')

    def update_dict(d: LogFormatType,
                    logid_and_kvs: Iterable[tuple[str, str]],
                    patt: re.Pattern) -> None:
        for logid, kvs in logid_and_kvs:
            kvs_list = patt.findall(kvs)
            data = ''.join(f' {colored(k)}={v}' for k, v in kvs_list)
            data = f'{cat}="{logid}"{data}'
            d.setdefault(logid, []).append((data, [k for k, v in kvs_list]))

    proxy_logs: LogFormatType = LogFormatType({})
    vnc_logs: LogFormatType = LogFormatType({})
    rdp_logs: LogFormatType = LogFormatType({})
    rdp_and_vnc_logs: LogFormatType = LogFormatType({})
    capture_logs: LogFormatType = LogFormatType({})
    other_logs: LogFormatType = LogFormatType({})
    declared_log_ids: set[str] = set()

    def to_process(log_extractor, kv_extractor):  # noqa: ANN001, ANN202
        return lambda d, text: update_dict(d, log_extractor.findall(text), kv_extractor)

    # log6 call with or without ternary
    log6_regex = re.compile(r'\blog6\((?:(?!\bLogId:|[?]|;).)*(?:[?])?\s*\bLogId::([A-Z0-9_]+)[^,:)]*(?::\s*LogId::([A-Z0-9_]+)\s*\)?)?,\s*(?:KVLogList)?(\{(?:[^)]+|[)][^;])*)?', re.DOTALL)
    # KVLog("key"_av, value) with value "..." | "..."_av | ...
    kv_log6_regex = re.compile(r'KVLog[{(]"([^"]+)"_av,\s*((?:(?!"_av|[)}]?,?\n).)*(?:"(?=_av))?)')
    def log6_process(d, text):  # noqa: ANN001, ANN202
        return update_dict(d,
                           chain.from_iterable(
                               ((t[0], t[2]), (t[1], t[2])) if t[1] else ((t[0], t[2]),)
                               for t in log6_regex.findall(text)
                           ),
                           kv_log6_regex)

    server_cert_regex = re.compile(r'\{\n\s*LogId::(\w+),\s*("[^"]+")')

    log_id_sesprobe_regex = re.compile(r'EXECUTABLE_LOG6_ID_AND_NAME\(\s*([A-Z0-9_]+)\s*\)([^)]*)')
    kv_sesprobe_regex = re.compile(r'"([^"]+)"()')  # capture an empty value
    sesprob_process = to_process(log_id_sesprobe_regex, kv_sesprobe_regex)

    sesman_regex = re.compile(r'self\.rdplog\.log\("([^"]+)"([^)]*)')
    kv_sesman_regex = re.compile(r',\s*(\w+)\s*=\s*(\w+|[^,)]+)')
    sesman_process = to_process(sesman_regex, kv_sesman_regex)

    log_siem_cpp_regex = re.compile(r'R"\(\[rdpproxy\].* type="([^"]+)"([^;]+)')
    kv_siem_cpp_regex = re.compile(r'(\w+)="([^"]+)"')
    def siem_cpp_process(d, text):  # noqa: ANN001, ANN202
        return update_dict(d, ((m.group(1), m.group(2)) for m in
                               log_siem_cpp_regex.finditer(text)),
                           kv_siem_cpp_regex)

    log_id_enum_regex = re.compile(r'\n    f\(([A-Z0-9_]+),')

    dirpath = ''  # used in for
    def cppfile_from_dirpath(filename: str) -> str:
        return read_cppfile(path_join(dirpath, filename))

    def filter_valid_cpp(filename: str) -> bool:
        return filename.endswith(('.hpp', '.cpp'))

    def update(d, process, filenames):  # noqa: ANN001, ANN202
        for text in map(cppfile_from_dirpath, filenames):
            process(d, text)

    for dirpath, _, filenames_ in os.walk(src_path):
        filenames = filter(filter_valid_cpp, filenames_)

        if dirpath.startswith(f'{src_path}/mod/rdp'):
            for filename in filenames:
                text = cppfile_from_dirpath(filename)
                log6_process(rdp_logs, text)
                sesprob_process(rdp_logs, text)
                if filename == 'rdp_negociation.cpp' and dirpath == f'{src_path}/mod/rdp':
                    for (logid, desc) in server_cert_regex.findall(text):
                        data = f'{cat}="{logid}" {colored("description")}={desc}'
                        rdp_logs.setdefault(logid, []).append((data, ['description']))

        elif dirpath.startswith(f'{src_path}/mod/vnc'):
            update(vnc_logs, log6_process, filenames)

        elif dirpath.startswith(f'{src_path}/capture'):
            update(capture_logs, log6_process,
                   (filename for filename in filenames if filename != 'agent_data_extractor.cpp'))

        elif dirpath == f'{src_path}/acl/module_manager':
            update(other_logs, log6_process,
                   (filename for filename in filenames
                    if filename not in {'create_module_rdp.cpp', 'create_module_vnc.cpp'}))
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
        rdp_logs.setdefault(k, []).extend(l)
        vnc_logs.setdefault(k, []).extend(l)

    # for d in (rdp_logs, vnc_logs, capture_logs, other_logs):
    #     for k,l in d.items():
    #         print(k, '\n ', '\n  '.join(l))
    #     print('------')

    used_logs = set(chain(rdp_logs, vnc_logs, capture_logs, other_logs))
    unused_logs = declared_log_ids - used_logs
    unused_logs.remove('PROBE_STATUS')

    if unused_logs:
        print_alert_on_list('Some LogId are unused', unused_logs, color)

    if other_logs:
        print_alert_on_list('Some LogId are uncategorized', other_logs, color)

    for k in chain(rdp_logs, vnc_logs):
        if k != 'TITLE_BAR':
            capture_logs.pop(k, None)

    for k, l in capture_logs.items():
        rdp_logs.setdefault(k, []).extend(l)
        vnc_logs.setdefault(k, []).extend(l)

    # remove duplicate
    for d in (proxy_logs, rdp_logs, vnc_logs):
        for logid, values in d.items():
            dejavu = set()
            datalogs = []
            d[logid] = datalogs
            for datalog in values:
                k = (logid, datalog[0])
                if k in dejavu:
                    continue
                datalogs.append(datalog)
                dejavu.add(k)

    return proxy_logs, rdp_logs, vnc_logs


def extract_doc_siem(docfile: str) -> tuple[LogFormatType,   # proxy
                                            LogFormatType,   # rdp
                                            LogFormatType]:  # vnc
    type_regex = re.compile(r'<para><literal>(rdpproxy: \[rdpproxy\]|(?:rdpproxy: )?\[(?:RDP|VNC) Session\])'
                            r' (?:(?!type=|<).)*type=["”]([^"”]+)["”]([^<]*)',
                            re.DOTALL)
    kv_siem_cpp_regex = re.compile(r'(\w+)=["”][^"”]+["”]')

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

    dejavu = set()

    for m in type_regex.finditer(read_xmlfile(docfile)):
        data = m.group(0)
        if data in dejavu:
            continue
        kvs = kv_siem_cpp_regex.findall(m.group(0).replace('”', '"'))
        d[m.group(1)].setdefault(m.group(2), []).append((data, kvs))
        dejavu.add(data)

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
        self.colored_key = color_builder('35') if color else identity

    def check_bad_quote(self,  # noqa: PLR6301
                        doc_proxy_logs: LogFormatType,
                        doc_rdp_logs: LogFormatType,
                        doc_vnc_logs: LogFormatType,
                        ) -> str:
        it = chain(doc_proxy_logs.values(),
                   doc_rdp_logs.values(),
                   doc_vnc_logs.values())
        bad_logs = {datalog[0] for datalog in chain.from_iterable(it) if '”' in datalog[0]}
        if bad_logs:
            l = '\n - '.join(sorted(bad_logs))
            return f'Log with ” instead of ":\n - {l}'
        return ''

    def check_missing_or_unknown(self,
                                 doc_proxy_logs: LogFormatType,
                                 doc_rdp_logs: LogFormatType,
                                 doc_vnc_logs: LogFormatType,
                                 ) -> str:
        doc_proxy_ids = doc_proxy_logs.keys()
        doc_rdp_ids = doc_rdp_logs.keys()
        doc_vnc_ids = doc_vnc_logs.keys()

        proxy_missing = self.proxy_ids - doc_proxy_ids
        rdp_missing = self.rdp_ids - doc_rdp_ids
        vnc_missing = self.vnc_ids - doc_vnc_ids

        proxy_unknown = doc_proxy_ids - self.proxy_ids
        rdp_unknown = doc_rdp_ids - self.rdp_ids
        vnc_unknown = doc_vnc_ids - self.vnc_ids

        proxy_known = doc_proxy_ids & self.proxy_ids
        rdp_known = doc_rdp_ids & self.rdp_ids
        vnc_known = doc_vnc_ids & self.vnc_ids

        colored = self.colored
        colored2 = self.colored2
        colored_key = self.colored_key

        msgs = []

        def append_missing(msg, l, d):  # noqa: ANN001, ANN202
            if l:
                sep = "\n      "
                formats = ''.join(f'\n - {colored(k)}:\n      {sep.join({data for data,_ in d[k]})}'
                                  for k in sorted(l))
                msgs.append(f'{colored2(msg)}:{formats}')

        append_missing('Missing log SIEM for [rdpproxy]', proxy_missing, self.proxy_logs)
        append_missing('Missing log SIEM for [RDP Session]', rdp_missing, self.rdp_logs)
        append_missing('Missing log SIEM for [VNC Session]', vnc_missing, self.vnc_logs)

        def append_unknown(msg, l):  # noqa: ANN001, ANN202
            if l:
                elems = '\n - '.join(map(colored, sorted(l)))
                msgs.append(f'{colored2(msg)}:\n - {elems}')

        append_unknown('Unknown log SIEM for [rdpproxy]', proxy_unknown)
        append_unknown('Unknown log SIEM for [RDP Session]', rdp_unknown)
        append_unknown('Unknown log SIEM for [VNC Session]', vnc_unknown)

        proxy_keys = ', '.join(colored_key(key) for key in (
            'psid', 'user', 'type',
        ))
        session_keys = ', '.join(colored_key(key) for key in (
            'session_id', 'client_ip', 'target_ip', 'user', 'device', 'service', 'account', 'type',
        ))
        def append_malformated_params(msg, ids, doc_logs, logs, keys):  # noqa: ANN001, ANN202
            def format_line(s, keys):
                s = ', '.join(colored_key(x) for x in s)
                if keys:
                    return f'{keys}, {s}' if s else keys
                return s

            def to_str(logs, keys):  # noqa: ANN001, ANN202
                return '\n    '.join(set(format_line(datalog[1], keys) for datalog in logs))

            for logid in sorted(ids):
                a = to_str(logs[logid], keys)
                b = to_str(doc_logs[logid], '')
                # if a != b:
                if sorted(a) != sorted(b):  # ignore key ordering
                    msg2 = f'{msg} with {logid}'
                    msgs.append(f'{colored2(msg2)}:\n  source code:\n    {a}\n  doc:\n    {b}')

        append_malformated_params('SIEM Parameters differ for [rdpproxy]',
                                  (logid for logid in proxy_known if logid not in {
                                      'DISCONNECT', 'INCOMING_CONNECTION', 'TIME_METRICS'}),
                                  doc_proxy_logs, self.proxy_logs, proxy_keys)
        append_malformated_params('SIEM Parameters differ for [RDP Session]',
                                  rdp_known, doc_rdp_logs, self.rdp_logs, session_keys)
        append_malformated_params('SIEM Parameters differ for [VNC Session]',
                                  vnc_known, doc_vnc_logs, self.vnc_logs, session_keys)

        return '\n\n'.join(msgs)


def print_siem_format(proxy_logs: LogFormatType,
                      rdp_logs: LogFormatType,
                      vnc_logs: LogFormatType) -> None:
    def show(logs, prefix):  # noqa: ANN001, ANN202
        for log in sorted(datalog[0] for values in logs.values() for datalog in values):
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

    def show(name, logs):  # noqa: ANN001, ANN202
        siem = (f"('{siem_type.lower().replace('_', '-')}', '{siem_type}')," for siem_type in sorted(logs))
        print('\n', name, ' = (\n    ', "\n    ".join(siem), '\n)',
              sep='', file=output)

    show('SIEM_FILTERS_RDP_PROXY', proxy_logs)
    show('SIEM_FILTERS_RDP_SESSION', rdp_logs)
    show('SIEM_FILTERS_VNC_SESSION', vnc_logs)


def check_docs(xml_doc_files: Iterable[str],
               doc_checker: DocSiemChecker,
               no_check_missing_or_unknown: bool,
               no_check_quote: bool,
               color: bool) -> bool:
    color_file = color_builder('33') if color else identity
    insert_nl = False
    for docfile in xml_doc_files:
        doc_siems = extract_doc_siem(docfile)
        err1 = '' if no_check_missing_or_unknown \
            else doc_checker.check_missing_or_unknown(*doc_siems)
        err2 = '' if no_check_quote \
            else doc_checker.check_bad_quote(*doc_siems)
        if err1 or err2:
            nl = '\n\n' if insert_nl else ''
            sep = '\n\n' if err1 and err2 else ''
            print(f'{nl}{color_file(docfile)}:\n\n{err1}{sep}{err2}')
            insert_nl = True

    return not insert_nl


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='Log SIEM extractor')
    parser.add_argument('-l', '--list-format', action='store_true')
    parser.add_argument('-c', '--check-doc', action='append', metavar='PATH')
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
        check_docs(args.check_doc,
                   DocSiemChecker(*siems, args.color),
                   no_check_missing_or_unknown=args.no_check_missing_or_unknown,
                   no_check_quote=args.no_check_quote,
                   color=args.color)
        has_action = True

    if not has_action or args.python_ids:
        print_log_siem_constants(*siems, args.output_python)
