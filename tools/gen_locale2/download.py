#!/usr/bin/env python3

from urllib.request import (urlopen, URLError)
import re

lcid_re = re.compile('[0-9a-fA-F]{8}')
klid_re = re.compile('.*>([0-9a-f]{8}) \(([^)]+).')
ldname_re = re.compile('\n\s*<tr><th>Layout Display Name</th><td>([^<]+).*')

ids = set()

def get_url(url):
    print(url)
    with urlopen(url) as f:
        return f.read().decode()

for lcid in lcid_re.findall(get_url('http://www.kbdlayout.info/')):
    if lcid in ids:
        continue
    page = get_url(f'http://www.kbdlayout.info/{lcid}')
    klids = klid_re.findall(page)
    ldnames = ldname_re.findall(page)
    for klid, ldname in zip(klids, ldnames):
        ids.add(lcid)
        xml = get_url(f'http://www.kbdlayout.info/{klid[0]}/download/xml')
        i = xml.index('\n')
        with open(f'{klid[0]}.xml', 'w') as output:
            output.write(xml[:i])
            output.write(f'\n  <metadata KLID="{klid[0]}" LocaleName="{klid[1]}" LayoutDisplayName="{ldname}"/>')
            output.write(xml[i:])
