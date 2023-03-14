#!/usr/bin/env bash

set -e

cd "$(dirname "$0")/../../.."
dirbase=projects/qtclient

./tools/bjam/gen_targets.py \
    --implicit tests \
    --main $dirbase/src \
    --test $dirbase/tests \
    --include $dirbase/src \
    --lib '' \
    --sys-lib-prefix-flags 'QtCore/:;<library>libQtCore' \
    --sys-lib-prefix-flags 'QtGui/:;<library>libQtGui' \
    --sys-lib-prefix-flags 'QtWidgets/:;<library>libQtWidgets' \
    --sys-lib-prefix-flags 'QtNetwork/:;<library>libQtNetwork' \
    --user-lib-flags 'src/utils/log.hpp:;<library>log.o' \
    $(find $dirbase/src -mindepth 1 -type d | sed 's/^/--src /') \
| sed -E '
    s#'"$dirbase"'/?##g;t
    s# src/([-_/\w]+\.cpp)# $(REDEMPTION_SRC_PATH)/\1#g
    s# tests/([-_/\w.]+\.cpp)# $(REDEMPTION_TEST_PATH)/\1#g
    s# '$d'/# #g
'
