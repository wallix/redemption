#!/bin/sh

cd "$(dirname "$0")/../../.."
d=projects/browser_client_JS

./tools/bjam/gen_targets.py \
    --main $d/src/main \
    --src $d/src/red_emscripten \
    --src $d/src/redjs \
    --src $d/src/red_channels \
    --deps-src src/mod/rdp/new_mod_rdp.hpp,$d/src/red_channels/\*.cpp \
    --include $d/src/ \
    --src-system emscripten \
    --lib '' \
    --test $d/tests \
    --disable-src src/mod/rdp/rdp_metrics.hpp \
    --disable-src src/mod/metrics.hpp \
    --disable-src src/mod/rdp/channels/rail_session_manager.hpp \
    --disable-src src/mod/rdp/channels/rail_channel.hpp \
    --disable-src src/mod/rdp/channels/sespro_alternate_shell_based_launcher.hpp \
    --disable-src src/mod/rdp/channels/sespro_channel.hpp \
    --disable-src src/mod/rdp/channels/sespro_clipboard_based_launcher.hpp \
    --disable-src src/RAIL/client_execute.hpp \
    --disable-src src/core/RDP/capabilities/rail.hpp \
    --disable-src core/RDP/caches/bmpcachepersister.hpp \
| sed -E '
    /^  <variant>[^:]+:<library>dl$|^  <(covfile|variant)|^  \$\(GCOV_NO_BUILD\)|\.coverage ;$/d;
    s/^exe /exe-js /;t
    s@^test-run '$d'/@test-run-js @;t
    s/^obj ([^.]+).o/objjs \1.bc/;ta
    s/^  <library>(.*)\.o/  \1.bc/;ta
    s/^  (.*)\.o/  \1.bc/;ta
    s/^  <library>(.*)/  \1.bc/;ta
    s/^  \$\(EXE_DEPENDENCIES\)/:\n&/;t
    :a
    s@ : src/@ : $(REDEMPTION_SRC_PATH)/@g
    s@ : tests/@ : $(REDEMPTION_TEST_PATH)/@g
    s@ : projects/redemption_configs/@ : $(REDEMPTION_CONFIG_PATH)/@g
    s@ '$d'/@ @g
'
# s@^(objjs src/main/[^.]+\.bc : src/[^ ]+) ;$@\1 : <cxxflags>-fno-rtti\&\&-frtti ;@
