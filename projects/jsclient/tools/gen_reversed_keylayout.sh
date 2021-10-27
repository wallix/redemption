#!/usr/bin/env bash

set -e

d="$(dirname "$0")"

if [[ -z "$KEYBOARD_JS_PATH" ]]; then
    KEYBOARD_JS_PATH="$d"/../../../../rdp-keyboard.js
fi

if [[ -z "$KBDLAYOUT_PATH" ]]; then
    KBDLAYOUT_PATH="$KEYBOARD_JS_PATH"/tools/kbdlayout.info
fi

source "$KEYBOARD_JS_PATH"/tools/keylayout_list.sh

PYTHONPATH="$KEYBOARD_JS_PATH/tools/:$PYTHONPATH" "$d"/gen_reversed_keylayout.py "${KBDLAYOUT_LAYOUTS[@]}" > "$d"/../src/application/reversed_layouts.js
