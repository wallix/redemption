#!/bin/bash

set -exu

cd "$(dirname "$0")/.."

if ! [[ "$(<include/main/version.hpp)" =~ 'VERSION "'([^\"]+) ]]; then
  echo Version not found >&2
  exit 1
fi
version="${BASH_REMATCH[1]}"

[[ -d debian ]] && rm -rf debian
cp -R packaging/debian debian
sed -i "s/%VERSION%/$version/g" debian/*

dpkg-buildpackage -b -tc -us -uc -r
rm -rf debian
