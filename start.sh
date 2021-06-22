#!/bin/sh

if ! which ffmpeg 2>/dev/null; then
  echo "installing ffmpeg"
  apt -qq install -y ffmpeg
  echo "ffmpeg installed"
fi

python3 /usr/local/share/passthrough/passthrough.py &
rdpproxy -f
sleep infinity
