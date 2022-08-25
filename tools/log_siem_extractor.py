#!/usr/bin/env python3
import os
import re
import sys
from typing import Iterable, Iterator, Callable, List, Set
from itertools import chain
from os.path import join as path_join


log_id_regex = re.compile(r'\bLogId::([A-Z0-9_]+)')
log_id_sesprobe_regex = re.compile(r'EXECUTABLE_LOG6_ID_AND_NAME\(([A-Z0-9_]+)')
sesman_regex = re.compile(r'self.rdplog.log\("([^"]+)"')
log_id_enum_regex = re.compile(r'\n    f\(([A-Z0-9_]+),')
log_siem_cpp_regex = re.compile(r' type="([^"]+)"')

def readall(filename) -> str:
    with open(filename) as f:
        return f.read()

def extract_log_id(filename: str) -> List[str]:
    return log_id_regex.findall(readall(filename))

def extract_log_id_rdp(filename: str) -> Iterator[str]:
    text = readall(filename)
    return chain(
        log_id_regex.findall(text),
        log_id_sesprobe_regex.findall(text),
    )

def extract_log_id_from_files(dirpath: str, filenames: Iterable[str],
                              log_id_extractor: Callable[[str], Iterable[str]] = extract_log_id
                              ) -> Iterator[str]:
    return chain.from_iterable(
        log_id_extractor(path_join(dirpath, filename))
            for filename in filenames
                if filename.endswith('.hpp') or filename.endswith('.cpp')
    )

def print_log_siem_constants(src_path: str) -> None:
    src_path = src_path.rstrip('/')

    vnc_log_ids: Set[str] = set()
    rdp_log_ids: Set[str] = set()
    capture_log_ids: Set[str] = set()
    other_log_ids: Set[str] = set()
    declared_log_ids: Set[str] = set()

    for dirpath, _, filenames in os.walk('src'):
        if dirpath.startswith(f'{src_path}/mod/rdp'):
            rdp_log_ids.update(extract_log_id_from_files(dirpath, filenames, extract_log_id_rdp))
        elif dirpath.startswith(f'{src_path}/mod/vnc'):
            vnc_log_ids.update(extract_log_id_from_files(dirpath, filenames))
        elif dirpath.startswith(f'{src_path}/capture'):
            capture_log_ids.update(extract_log_id_from_files(
                dirpath, (filename for filename in filenames if filename != 'agent_data_extractor.cpp')))
        elif dirpath == f'{src_path}/acl/module_manager':
            other_log_ids.update(extract_log_id_from_files(
                dirpath, (filename for filename in filenames if filename != 'create_module_rdp.cpp' and
                                                                filename != 'create_module_vnc.cpp')))
            rdp_log_ids.update(extract_log_id(f'{src_path}/acl/module_manager/create_module_rdp.cpp'))
            vnc_log_ids.update(extract_log_id(f'{src_path}/acl/module_manager/create_module_vnc.cpp'))
        elif dirpath == f'{src_path}/acl':
            other_log_ids.update(extract_log_id_from_files(
                dirpath, (filename for filename in filenames if filename != 'connect_to_target_host.hpp')))
            log_ids = extract_log_id(f'{src_path}/acl/connect_to_target_host.hpp')
            rdp_log_ids.update(log_ids)
            vnc_log_ids.update(log_ids)
        elif dirpath.startswith(f'{src_path}/front'):
            log_ids = (*extract_log_id_from_files(dirpath, filenames),)
            rdp_log_ids.update(log_ids)
            vnc_log_ids.update(log_ids)
        elif dirpath == f'{src_path}/core':
            log_ids = (*extract_log_id_from_files(
                dirpath, (filename for filename in filenames if filename != 'session.cpp' and
                                                                filename != 'log_id.hpp')),)
            rdp_log_ids.update(log_ids)
            vnc_log_ids.update(log_ids)
            log_ids = (*extract_log_id(f'{src_path}/core/session.cpp'),)
            rdp_log_ids.update(log_ids)
            vnc_log_ids.update(log_ids)
            declared_log_ids = set(log_id_enum_regex.findall(readall(f'{src_path}/core/log_id.hpp')))
        else:
            other_log_ids.update(extract_log_id_from_files(dirpath, filenames))

    used_logs = set(chain(rdp_log_ids, vnc_log_ids, capture_log_ids, other_log_ids))
    unused_logs = declared_log_ids - used_logs

    if unused_logs:
        print('some LogId are unused:\n -',
            '\n - '.join(unused_logs), file=sys.stderr)
        exit(1)

    if other_log_ids:
        print('some LogId are uncategorized.\n -',
            '\n - '.join(other_log_ids), file=sys.stderr)
        exit(2)

    capture_log_ids -= rdp_log_ids
    capture_log_ids -= vnc_log_ids
    capture_log_ids.add('TITLE_BAR')

    rdp_log_ids.update(capture_log_ids)
    rdp_log_ids.remove('PROBE_STATUS')
    vnc_log_ids.update(capture_log_ids)
    vnc_log_ids.remove('PROBE_STATUS')

    log_proxy = set(log_siem_cpp_regex.findall(readall('src/utils/log_siem.cpp')))
    log_proxy.update(sesman_regex.findall(readall('tools/sesman/sesmanworker/sesman.py')))

    print("""\
#######################################################################
## DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN ##
#######################################################################

# see redemption/tools/log_siem_extractor.py
    """)

    def show(name, logs):
        print(name, ' = (\n    \'', "',\n    '".join(sorted(logs)), '\',\n)\n', sep='')

    show('SIEM_RDP_PROXY', log_proxy)
    show('SIEM_RDP_SESSION', rdp_log_ids)
    show('SIEM_VNC_SESSION', vnc_log_ids)


if __name__ == '__main__':
    print_log_siem_constants(sys.argv[1] if len(sys.argv) >= 2 else 'src')
