#!/usr/bin/bash

set -e

if (( $# != 1 )); then
    echo 'build_pages.sh dirname'
    exit 1
fi

d="$(realpath -m $1)"
[[ -e $d ]] || mkdir -m "$d"

cd "$(dirname "$0")"
../../../tools/conf_migration_tool/conf_migrate.py --dump=json > "$d"/migrate.json
root=$(realpath -m "$PWD"/..)
ln -s "$root"/pages/config.html "$root"/autogen/doc/*.json "$root"/../../docs/theme/*.png "$d"

cd "$d"
python -m http.server
