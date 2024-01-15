#!/usr/bin/env sh

cd "$(dirname "$0")"/../..

errcode=0
ruff=${RUFF:-ruff}

# sources only (exclude tests)
"$ruff" --preview --target-version=py311 "$@" \
    tools/*.py \
    tools/conf_migration_tool/*.py \
    tools/gen_locale/ \
    tools/i18n/ \
    tools/icap_validator/ \
    tools/passthrough/ \
    tools/sesman/*.py \
    tools/sesman/benchmark/ \
    tools/sesman/sesmanworker/ \
    || errcode=$?

# tests only (exclude sources)
# S104: hardcoded-bind-all-interfaces
"$ruff" --preview --target-version=py311 --ignore=S104 "$@" \
    tools/conf_migration_tool/tests \
    tools/sesman/tests \
    || errcode=$?

exit $errcode
