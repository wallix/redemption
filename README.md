Master branch: [![Build Status from master](https://travis-ci.org/wallix/redemption.svg?branch=master)](https://travis-ci.org/wallix/redemption)

Future branch: [![Build Status from future](https://travis-ci.org/wallix/redemption.svg?branch=future)](https://travis-ci.org/wallix/redemption)


A RDP (Remote Desktop Protocol) proxy.

(A RDP client in `projects/ClientQtGraphicAPI`)

Support of:

- RDP client to RDP server and
- RDP client to VNC server


Dependencies
============

To compile ReDemPtion you need the following packages:
- libboost-tools-dev (contains bjam: software build tool) (https://github.com/boostorg/build)
- libboost-test-dev (unit-test dependency)
- libssl-dev
- libkrb5-dev
- libgssglue-dev (unnecessary since Ubuntu 17.10)
- libsnappy-dev
- libpng12-dev
- libffmpeg-dev (see below)
- g++ >= 7.2 or clang++ >= 5.0 or other C++17 compiler

```sh
apt install libboost-tools-dev libboost-test-dev libssl-dev libkrb5-dev libsnappy-dev libpng12-dev
```

Extra packet:
- libboost-stacktrace-dev (only with `-sBOOST_STACKTRACE=1`)

```sh
apt install libboost-stacktrace-dev
```

Submodule ($ `git submodule update --init`):
- https://github.com/wallix/program_options
- https://github.com/wallix/ppocr


## FFmpeg:

### Ubuntu:
- libavcodec-dev
- libavformat-dev
- libavutil-dev
- libswscale-dev
- libx264-dev
- libbz2-dev

```sh
apt install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libx264-dev libbz2-dev
```

<!-- ok with 53 (?) and 54 version-->
<!-- - libavcodec-ffmpeg56 -->
<!-- - libavformat-ffmpeg56 -->
<!-- - libavutil-ffmpeg54 -->
<!-- - libswscale-ffmpeg3 -->

### Other distros:
- https://github.com/FFmpeg/FFmpeg/archive/n2.5.11.tar.gz

And set environment variable (optionally)
- `export FFMPEG_INC_PATH=/my/ffmpeg/include/path`
- `export FFMPEG_LIB_PATH=/my/ffmpeg/library/path` (/!\\ without `/` terminal)
- `export FFMPEG_LINK_MODE=shared` (static or shared, shared by default)

### Note:

Disable ffmpeg with `-sNO_FFMPEG=1`.


## Environment variable setting

List with `sed -E '/\[ setvar/!d;s/.*\[ setvar ([^ ]+).*\] ;/\1/' jam/defines.jam`

    export FFMPEG_INC_PATH=$HOME/ffmpeg/includes
    bjam ....

Or

    FFMPEG_INC_PATH=$HOME/ffmpeg/includes FFMPEG_LIB_PATH=... bjam ....

Or with `-s` to bjam

    bjam -s FFMPEG_INC_PATH=$HOME/ffmpeg/includes ...

### Special compilation variables

- `BOOST_STACKTRACE=1`: (debug only) compile with `boost_stacktrace_backtrace`.

### Special runtime variables

- `REDEMPTION_FILTER_ERROR`: Only with `BOOST_STACKTRACE=1`. no backtrace for specific error (see `src/core/error.hpp`). example: `export REDEMPTION_FILTER_ERROR=ERR_TRANSPORT_NO_MORE_DATA`.


Compilation
===========

Well, that's pretty easy once you installed the required dependencies.

Just run (as user):

$ `bjam` or `bjam toolset=your-compiler` (ex: `bjam toolset=gcc`) (see https://boostorg.github.io/build/manual/develop/index.html#bbv2.overview.configuration and `tools/bjam/user-config.jam`)

Verbose tests:

$ `export REDEMPTION_LOG_PRINT=1`\
$ `bjam tests`

Note: `bjam tests.norec`, `bjam tests/front.norec` execute files directly in tests directory, but not recursively.


Compile executables without tests (as user):

$ `bjam exe libs`

and install (as administrator):

\# `bjam install`

Binaries are located by default in `/usr/local/bin`.


Use `bjam --help` for more information.


## Modes and options

$ `bjam [variant=]{release|debug|san} [cxx-options=value] [target...]`

- `release`: default
- `debug`: debug mode (compile with `-g -D_GLIBCXX_DEBUG`)
- `san`: enable sanitizers: asan, lsan, usan

- `cxx-color`: default auto never always
- `cxx-lto`: off on fat
- `cxx-relro`: default off on full
- `cxx-stack-protector`: off on strong all

(`cxx-*` options list with `sed -E 's/^feature <([^>]+)> .*/\1/;t;d' jam/cxxflags.jam`)


Run ReDemPtion
==============

To test it, executes:

$ `python tools/passthrough/passthrough.py`

\# `/usr/local/bin/rdpproxy -nf`
<!-- $ `./bin/${BJAM_TOOLSET_NAME}/${BJAM_MODE}/rdpproxy -nf` -->


Now, at that point you'll just have two servers waiting for connections
not much fun. You still have to run some RDP client to connect to proxy. Choose
whichever you like xfreerdp, rdesktop, remmina, tsclient on Linux or of course
mstsc.exe if you are on windows. All are supposed to work. If some problem
occurs just report it to us so that we can correct it.

Example with freerdp when the proxy runs on the same host as the client:

$ `xfreerdp 127.0.0.1`

A dialog box should open in which you can type a username and a password.
With default passthrough.py at least internal services should work. Try login: internal, password: internal and bouncer2 or card as device. If you want to provide such extensions
to current passthrough.py, please contribute it, it will be much appreciated.

$ `xfreerdp /u:internal /p:internal 127.0.0.1`

Generate target and lib/obj dependencies
========================================

When create a new test or when a target fail with link error:

`bjam targets.jam` for updated `targets.jam` and `projects/ClientQtGraphicAPI/redemption_deps.jam` files.

Or run `./tools/bjam/gen_targets.py > targets.jam`

Specific deps (libs, header, cpp, etc) in `./tools/bjam/gen_targets.py`.

Compile proxy_recorder
======================

Proxy recorder is a tools used to record dialog between a client and an RDP server without
any modification of the data by redemption. This allows to record reference traffic for
replaying it later. It is useful for having available new parts or the RDP protocol in a
reproducible way and replaying traffic when implementing the new orders. This tools is
not (yet) packaged with redemption and delivered as stand-alone.

It can be compiled using static c++ libraries (usefull to use the runtime on systems
where reference compiler is older) using the command line below. Links with openssl
and kerberos are still dynamic and using shared libraries.

`bjam -a -d2 toolset=gcc-7 proxy_recorder linkflags=-static-libstdc++`

Exemple call line for proxy_recorder:

`proxy_recorder --target-host 10.10.47.252 -p 3389 -P 8000 --nla-username myusername --nla-password mypassword -t dump-%d.out`

Packaging
=========

    ./tools/packager.py --build-package

- `--force-target target`: target is a file in packaging/targets
- `--force-build`


Tag and Version
===============

    ./tools/packager.py --update-version 1.2.7 --git-tag --git-push-tag --git-push


FAQ
===

Q - Why do you use bjam for ReDemPtion instead of make, cmake, scons, etc ?
---------------------------------------------------------------------------

It is simple, more that could be thought at first sight, and bjam has the major
feature over make to keep source directories clean, all build related
informations for all architecture are kept together in bin directory.

The main drawback of bjam is the smaller user base.

But keeping in mind the complexity of make (or worse autotools + make), bjam is
a great help. We also used to have an alternative cmake build system, but it was
more complex than bjam and not maintained, so was removed.

Q - How to add configuration variables in rdpproxy.ini ?
--------------------------------------------------------

Just edit config_spec.hpp (./projects/redemption_configs/configs_specs/configs/specs/config_spec.hpp).

The necessary changes should be simple using the surrounding code as exemple.

Then enter directory `projects/redemption_configs` and type `bjam`
the needed files will be generated.

