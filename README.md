<!-- Master branch: [![Build Status from master](https://travis-ci.org/wallix/redemption.svg?branch=master)](https://travis-ci.org/wallix/redemption) -->

<!-- Future branch: [![Build Status from future](https://travis-ci.org/wallix/redemption.svg?branch=future)](https://travis-ci.org/wallix/redemption) -->


A RDP (Remote Desktop Protocol) proxy.

(A RDP client in `projects/ClientQtGraphicAPI`, a RDP web client in `projects/jsclient`)

Support of:

- RDP client to RDP server and
- RDP client to VNC server

<!-- https://github.com/jonathanpoelen/gh-md-toc -->
<!-- toc -->
1. [Dependencies](#dependencies)
    1. [FFmpeg](#ffmpeg)
        1. [Ubuntu / Debian](#ubuntu--debian)
        2. [Other distros](#other-distros)
2. [Compilation](#compilation)
    1. [Special runtime variables (shell variable)](#special-runtime-variables-shell-variable)
    2. [Setting build variables](#setting-build-variables)
    3. [Modes and options](#modes-and-options)
3. [Run ReDemPtion](#run-redemption)
4. [Convert .mwrm/.wrm capture to video](#convert-mwrmwrm-capture-to-video)
5. [Generate target and lib/obj dependencies](#generate-target-and-libobj-dependencies)
6. [Compile proxy_recorder](#compile-proxy_recorder)
7. [Packaging](#packaging)
8. [Tag and Version](#tag-and-version)
9. [Test files](#test-files)
10. [FAQ](#faq)
    1. [Q - Why do you use bjam for ReDemPtion instead of make, cmake, scons, etc ?](#q---why-do-you-use-bjam-for-redemption-instead-of-make-cmake-scons-etc-)
    2. [Q - How to add configuration variables in rdpproxy.ini ?](#q---how-to-add-configuration-variables-in-rdpproxyini-)
<!-- /toc -->

# Dependencies

To compile ReDemPtion you need the following packages:
- libboost-tools-dev (contains bjam and b2: software build tool) (https://github.com/boostorg/build)
- libboost-test-dev (unit-test dependency)
- zlib1g-dev
- libssl-dev
- libkrb5-dev
- libgssglue-dev (unnecessary since Ubuntu 17.10 and Debian 10)
- libsnappy-dev
- libpng-dev (libpng12-dev (< Debian 10))
- libbz2-dev
- libffmpeg-dev (see below)
- g++ >= 8.0 or clang++ >= 5.0 or other C++17 compiler

```sh
apt install libboost-tools-dev libboost-test-dev libssl-dev libkrb5-dev libsnappy-dev libpng-dev libbz2-dev
```

Submodule ($ `git submodule update --init`):
- https://github.com/wallix/program_options
- https://github.com/wallix/ppocr

On Ubuntu SNAPPY dev files are broken and `SNAPPY_MAJOR`, `SNAPPY_MINOR` and `SNAPPY_PATCHLEVEL` macros are not defined.
The simplest way to fix that is editing `/usr/include/snappy-stubs-public.h` and define these above `SNAPPY_VERSION`
like below (change values depending on your snappy package).

```bash
$ apt show libsnappy-dev | grep Version
Version: 1.1.7-1
```

```C
#define SNAPPY_MAJOR 1
#define SNAPPY_MINOR 1
#define SNAPPY_PATCHLEVEL 7
#define SNAPPY_VERSION \
    ((SNAPPY_MAJOR << 16) | (SNAPPY_MINOR << 8) | SNAPPY_PATCHLEVEL)
```


## FFmpeg

### Ubuntu / Debian

- libavcodec-dev
- libavformat-dev
- libavutil-dev
- libswscale-dev
- libx264-dev

```sh
apt install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libx264-dev
```

<!-- ok with 53 (?) and 54 version-->
<!-- - libavcodec-ffmpeg56 -->
<!-- - libavformat-ffmpeg56 -->
<!-- - libavutil-ffmpeg54 -->
<!-- - libswscale-ffmpeg3 -->

### Other distros

- https://github.com/FFmpeg/FFmpeg/archive/n2.5.11.tar.gz

And set the [build variables](#setting-build-variables) (optionally)
- `FFMPEG_INC_PATH=/my/ffmpeg/include/path`
- `FFMPEG_LIB_PATH=/my/ffmpeg/library/path` (/!\\ without `/` terminal)
- `FFMPEG_LINK_MODE=shared` (static or shared, shared by default)

Note: Disable ffmpeg with `NO_FFMPEG=1`.


# Compilation

([Instruction for Debian 9](https://github.com/wallix/redemption/issues/34#issuecomment-471322759))

Well, that's pretty easy once you installed the required dependencies.

Just run (as user):

$ `bjam` or `bjam toolset=your-compiler` (ex: `bjam toolset=gcc`) (see https://boostorg.github.io/build/manual/develop/index.html#bbv2.overview.configuration and `tools/bjam/user-config.jam`)

Verbose tests:

$ `export REDEMPTION_LOG_PRINT=1`\
$ `bjam tests`

- `REDEMPTION_LOG_PRINT=e` for error and debug only
- `REDEMPTION_LOG_PRINT=d` for debug only
- `REDEMPTION_LOG_PRINT=w` for other than info

Note: `bjam tests.norec`, `bjam tests/front.norec` execute files directly in tests directory, but not recursively.


Compile executables without tests (as user):

$ `bjam exe libs`

and install (as administrator):

\# `bjam install`

Binaries are located by default in `/usr/local/bin`. For a user install, see [setting build variables](#setting-build-variables).


Use `bjam --help` for more information.


## Special runtime variables (shell variable)

- `REDEMPTION_FILTER_ERROR` (Only if `BOOST_STACKTRACE != 0`): No backtrace for specific error (see `src/core/error.hpp`). example: `export REDEMPTION_FILTER_ERROR=ERR_TRANSPORT_NO_MORE_DATA,ERR_SEC`.


## Setting build variables

List with `bjam env_help`.

    bjam -s FFMPEG_INC_PATH=$HOME/ffmpeg/includes ...

Or with a shell variable

    export FFMPEG_INC_PATH=$HOME/ffmpeg/includes
    bjam ....


## Modes and options

$ `bjam [variant=]{release|debug|san} [-s cxx_option=value] [target...]`

- `release`: default
- `debug`: debug mode
- `san`: debug + sanitizers (asan, lsan, usan)

- `-s cxx_color`: default auto never always
- `-s cxx_lto`: off on fat linker-plugin
- `-s cxx_relro`: default off on full
- `-s cxx_stack_protector`: off on strong all
- ...

Complet list with `bjam cxx_help`.


# Run ReDemPtion

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

$ `xfreerdp /v:127.0.0.1`

$ `xfreerdp /v:127.0.0.1 /u:username@target_ip /p:password`

A dialog box should open in which you can type a username and a password.
With default passthrough.py at least internal services should work. Try login: internal, password: internal and bouncer2 or card as device. If you want to provide such extensions
to current passthrough.py, please contribute it, it will be much appreciated.

$ `xfreerdp /u:internal@internal /p:internal 127.0.0.1`


# Convert .mwrm/.wrm capture to video

`.mwrm` and `.wrm` are native capture formats in `capture_flags=2` in `rdpproxy.ini`.

    redrec -f --video-codec mp4 -i file.mwrm -o output_prefix

Note: `rdpproxy --print-default-ini` show a default ini file.


# Generate target and lib/obj dependencies

When create a new test or when a target fail with link error:

`bjam targets.jam` for updated `targets.jam` and `projects/ClientQtGraphicAPI/redemption_deps.jam` files.

Or run `./tools/bjam/gen_targets.py > targets.jam`

Specific deps (libs, header, cpp, etc) in `./tools/bjam/gen_targets.py`.


# Compile proxy_recorder

Proxy recorder is a tools used to record dialog between a client and an RDP server without
any modification of the data by redemption. This allows to record reference traffic for
replaying it later. It is useful for having available new parts or the RDP protocol in a
reproducible way and replaying traffic when implementing the new orders. This tools is
not (yet) packaged with redemption and delivered as stand-alone.

It can be compiled using static c++ libraries (usefull to use the runtime on systems
where reference compiler is older) using the command line below. Links with openssl
and kerberos are still dynamic and using shared libraries.

    bjam -a -d2 toolset=gcc-7 proxy_recorder linkflags=-static-libstdc++

Exemple call line for proxy_recorder:

    proxy_recorder --target-host 10.10.47.252 -p 3389 -P 8000 --nla-username myusername --nla-password mypassword -t dump-%d.out


# Packaging

    ./tools/packager.py --build-package

- `--force-target target`: target is a file in packaging/targets
- `--force-build`


# Tag and Version

    ./tools/packager.py --update-version 1.2.7 --git-tag --git-push-tag --git-push

- `--add-changelog`: if you want to add some specific changelogs


# Test files

See [test_framework directory](tests/includes/test_only/test_framework).


# FAQ

## Q - Why do you use bjam for ReDemPtion instead of make, cmake, scons, etc ?

It is simple, more that could be thought at first sight, and bjam has the major
feature over make to keep source directories clean, all build related
informations for all architecture are kept together in bin directory.

The main drawback of bjam is the smaller user base.

But keeping in mind the complexity of make (or worse autotools + make), bjam is
a great help. We also used to have an alternative cmake build system, but it was
more complex than bjam and not maintained, so was removed.

## Q - How to add configuration variables in rdpproxy.ini ?

Just edit config_spec.hpp (`projects/redemption_configs/configs_specs/configs/specs/config_spec.hpp`).

The necessary changes should be simple using the surrounding code as exemple.

Then enter directory `projects/redemption_configs` and type `bjam`
the needed files will be generated.
