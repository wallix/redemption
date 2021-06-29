#!/usr/bin/env bash

wget --no-verbose http://www.kbdlayout.info/ -O- \
| grep '[0-9a-fA-F]\{8\}' -o \
| sort -u \
| while read klid ; do
    echo $klid
    metadata=$(wget --no-verbose http://www.kbdlayout.info/$klid -O- | sed -E '
        /KLID|Layout Display Name/!d
        s/.*>([0-9a-f]{8}) \(([^)]+).*/  <metadata KLID="\1" LocaleName="\2"/
        t
        s#^\s*<tr><th>Layout Display Name</th><td>([^<]+).*# LayoutDisplayName="\1"/>#
        q')
    metadata=${metadata/$'\n'/}
    echo "$metadata"
    wget --no-verbose http://www.kbdlayout.info/$klid/download/xml -O- | sed "2i$metadata" > $klid.xml
done
