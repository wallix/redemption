#!/bin/sh

cd "$(dirname "$0")/../../.."
d=projects/browser_client_JS

./tools/bjam/gen_targets.py \
    --main $d/src/main \
    --src $d/src/red_emscripten \
    --src $d/src/redjs \
    --include $d/src/ \
    --src-system emscripten \
    --lib '' \
    --test '' \
| sed -E '
    /^  <variant>[^:]+:<library>dl$|^  <(covfile|variant)|^  \$\(GCOV_NO_BUILD\)|\.coverage ;$/d;
    s/^exe /exe-js /;t
    s/^obj ([^.]+).o/objjs \1.bc/;ta
    s/^  <library>(.*)\.o/  \1.bc/;ta
    s/^  (.*)\.o/  \1.bc/;ta
    s/^  <library>(.*)/  \1.bc/;ta
    s/^  \$\(EXE_DEPENDENCIES\)/:\n&/;t
    :a
    s@ : src/@ : $(REDEMPTION_SRC_PATH)/@g
    s@ : tests/@ : $(REDEMPTION_TEST_PATH)/@g
    s@ : projects/redemption_configs/@ : $(REDEMPTION_CONFIG_PATH)/@g
    s@ : '$d'/@ : @g
    s@^(objjs '$d'/src/main/[^.]+\.bc : src/[^ ]+) ;$@\1 : <cxxflags>-fno-rtti\&\&-frtti ;@
'
