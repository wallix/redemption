#!/usr/bin/env python3
import re
import sys
import argparse
from pathlib import Path


parser = argparse.ArgumentParser(description='Log splitter')
parser.add_argument('-l', '--local', action='store_true', help='file without rdpproxy prefix')
parser.add_argument('-s', '--suffix', help='specify suffix for output filename', default='.log')
parser.add_argument('-e', '--echo', action='store_true', help='echo to stdout')
parser.add_argument('filenames', nargs='*')

args = parser.parse_args()

auto_name: bool = True
suffix: str = args.suffix
cp_to_stdout: bool = args.echo

# Dec  8 15:51:22 INFO (211075/211075) -- ReDemPtion 10.4.31 starting
# Dec  8 15:51:22 rdpproxy[211075]: INFO (211075/211075) -- ReDemPtion 10.4.31 starting
id_extractor = (re.compile(r' \((\d+)[^)]+\)()') if args.local
                else re.compile(r' rdpproxy\[(\d+)\]: (\w+) \([^)]+\)'))

files = {}

def read_file(f, prefix: str, suffix: str, cp_to_stdout: bool) -> None:
    for line in f:
        m = id_extractor.search(line)
        if m:
            pid = m.group(1)
            sid = int(pid)
            output = files.get(sid)
            if not output:
                filename = f'{prefix}{pid}{suffix}'
                print(f'new file: {filename}')
                output = open(filename, 'w', encoding='utf-8')  # noqa: SIM115
                files[sid] = output
            start, stop = m.span()
            s1 = line[:start]
            s2 = line[stop:]
            output.write(s1)
            output.write(s2)
            if cp_to_stdout:
                print(s1, s2, sep='', end='')

if not args.filenames:
    read_file(sys.stdin, 'stdin.', suffix, cp_to_stdout=True)
else:
    for filename in args.filenames:
        if filename == '-':
            read_file(sys.stdin, 'stdin.', suffix, cp_to_stdout=True)
        else:
            p = Path(filename)
            with p.open() as f:
                read_file(f, f'{p.parent / p.stem}-', suffix, cp_to_stdout)

for f in files.values():
    f.close()
