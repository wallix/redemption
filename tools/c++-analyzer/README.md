# Dynamic analyzer

- `valgrind [-d binary-directory] [testnames...]`: run tests from `binary-directory` (bin/gcc/debug by default) with valgrind and print a little diagnostic error (see `./tools/valgrind-filter/valgrind-output-color` and `./tools/valgrind-filter/valgrind-ignore-ssl-snappy`).
- `bt [-hqsef] [--help] command [-- [gdb-args...]]`: show the backtrace with `gdb`.
- `gtrace [-cph] [--help] command [-- [gdb-args...]]`: show functions trace with `gdb`.
Ex: `./gtrace -c -- ./bin/gcc/debug/test_in_meta_sequence_transport -ex 'b test_in_meta_sequence_transport.cpp:340' | ./bt -fs`
- `gbt [-hcpqs] [--help] command line [-- [gdb-args...]]`: alias for `./gtrace [-cp] -n $command -- -ex "b $command_filename.cpp:$line" | ./bt -f[qs]`
- `tmalloc command [args]`: run command with libtmalloc (cf: `LD_PRELOAD=../tmalloc/libtmalloc.so`), see `./tools/tmalloc`.

# Static analyzer

- `cppcheck [cppcheck-args]`: run `cppcheck` on `main/*`.
- `cppcheck-full [cppcheck-args]`: run `cppcheck`.
- `scan-build [-h | jamdir [targers pattern-filter-cmd]]]`: run `scan-build` on each executable.
- `unused_files`: show unesed files (files without `#include`).
- `todo_extractor files...`: extract comment with BUG, ALERT, ATTENTION, DANGER, HACK, SECURITYFIXME, DEPRECATED, TASK, TODO, TBD, WARNING, CAUTION, NOTE, NOTICE, TEST, TESTING, PERFORMANCE, PERF

# Source corrector

- `clang-tidy [clang-tidy-args]`: run `clang-tidy`. Used `-fix` flag for fix detected errors.

# Sanitizers

- Add the ligne below at the end of your '.bashrc'.

alias bjam="ASAN_OPTIONS=detect_stack_use_after_return=1:detect_leaks=1 LSAN_OPTIONS=suppressions=./tools/c++-analyzer/suppr-leak-asan.txt bjam"

## ASan

 - help: `ASAN_OPTIONS=help=1 ./exe`
 - `export ASAN_OPTIONS=detect_stack_use_after_return=1:detect_leaks=1`

NOTE: `detect_stack_use_after_return=1` can be very slow.

## UBSan

http://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html

 -  `export UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1`

## Options

- Compile with -g and -fno-omit-frame-pointer
- `export UBSAN_OPTIONS=print_stacktrace=1`
- Make sure llvm-symbolizer binary is in PATH (example: `export ASAN_SYMBOLIZER_PATH=/usr/lib/llvm-3.8/bin/llvm-symbolizer`).

# Utils

## include_tree

```bash
include_tree [-v filters='file1;file2'] [-v nolist=1] [-v notree=1] [-v oneline=1] [-v headeronly=1] files
```

### Examples

#### Zsh

```zsh
set extendedglob
./tools/c++-analyzer/include_tree -v oneline=1 src/**/*~src/keyboard*(^/) tests/includes/test-only/**/*(^/) tests/mod/rdp/test_rdp.cpp
```

#### Bash

```bash
find src -type f -exec ./tools/c++-analyzer/include_tree -v oneline=1 {} +
```
