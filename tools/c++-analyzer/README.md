# Dynamic analyzer

- `valgrind [-d binary-directory] [testnames...]`: run tests in `binary-directory` (bin/gcc/debug by default) with valgrind and print a little diagnostic error (see `./tools/valgrind-filter/valgrind-output-color` and `./tools/valgrind-filter/valgrind-ignore-ssl-snappy`).
- `bt [-hqsef] [--help] command [-- [gdb-args...]]`: show the backtrace with `gdb`.
- `gtrace [-cph] [--help] command [-- [gdb-args...]]`: show functions trace with `gdb`.
Ex: `./gtrace -c -- ./bin/gcc/debug/test_in_meta_sequence_transport -ex 'b test_in_meta_sequence_transport.cpp:340' | ./bt -fs`
- `gbt [-hcpqs] [--help] command line [-- [gdb-args...]]`: alias for `./gtrace [-cp] -n $command -- -ex "b $command_filename.cpp:$line" | ./bt -f[qs]`
- `tmalloc command [args]`: run command with libtmalloc (cf: `LD_PRELOAD=../tmalloc/libtmalloc.so`), see `./tools/tmalloc`.
- `sanitize-address [bjam-args]`: run `bjam asan` with `LD_LIBRARY_PATH=/usr/lib/gcc-snapshot/lib`.

# Static analyzer

- `cppcheck [cppcheck-args]`: run `cppcheck` on `main/*`.
- `cppcheck-full [cppcheck-args]`: run `cppcheck`.
- `scan-build`: run `scan-build` on each executable.
- `unused_files`: show unesed files (files without `#include`).

# Source corrector

- `clang-tidy [clang-tidy-args]`: run `clang-tidy`. Used `-fix` flag for fix detected errors.
