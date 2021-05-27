#!/bin/sh
python3 /usr/local/share/passthrough/passthrough.py &
rdpproxy -nf
