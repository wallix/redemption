# Dynamic analyzer

- `valgrind [-d binary-directory] [testnames...]`: run tests in `binary-directory` (bin/gcc/debug by default) with valgrind and print a little diagnostic error (see ./tools/valgrind-filter/valgrind-output-color and ./tools/valgrind-filter/valgrind-ignore-ssl-snappy)
- `bt [-hqsef] [--help] command [gdb-args...]`: show backtrace with `gdb`.
- `gdb_trace [-c] command`: show function trace with `gdb`
- `tmalloc command [args]`: run command with libtmalloc (cf: LD_PRELOAD=../tmalloc/libtmalloc.so), see ./tools/tmalloc
- `sanitize-address [bjam-args]`: run `bjam asan` with `LD_LIBRARY_PATH=/usr/lib/gcc-snapshot/lib`

# Static analyzer

- `cppcheck [cppcheck-args]`: run cppcheck on main/*
- `cppcheck-full [cppcheck-args]`: run cppcheck
- `scan-build`: run `scan-build` on each executable
- `unused_files`: show unesed files (files without `#include`).

# Source corrector

- `clang-tidy [clang-tidy-args]`: run `clang-tidy`. Used `-fix` flag for fix detected errors.
