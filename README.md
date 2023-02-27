<!-- Master branch: [![Build Status from master](https://travis-ci.org/wallix/redemption.svg?branch=master)](https://travis-ci.org/wallix/redemption) -->

<!-- Future branch: [![Build Status from future](https://travis-ci.org/wallix/redemption.svg?branch=future)](https://travis-ci.org/wallix/redemption) -->

# About ReDemPtion

Redemption is a versatile RDP proxy, meaning one will connect to remote desktops through Redemption.
This allows to centralize remote connection creating a single end point for several desktops.
About the versatile part, Redemption is able to connect to VNC servers, permitting connection from Windows® to Linux, for instance.
It is only known to run on Linux (Ubuntu, Debian, Arch Linux) and no effort were made to port it to Win32, MacOS or FreeBSD.

Redemption is also able to record sessions in .wrm/.mwrm files and then convert them to mp4 video.

The project also contains 2 RDP clients:

- A desktop client in `projects/qtclient` (supports RDP and VNC)
- A web client in `projects/jsclient` that uses a websocket (option that must be enabled in the proxy configuration).


<!-- https://github.com/jonathanpoelen/gh-md-toc -->
<!-- toc -->
1. [Compilation](#compilation)
    1. [Dependencies](#dependencies)
    2. [FFmpeg](#ffmpeg)
        1. [Ubuntu / Debian](#ubuntu--debian)
        2. [Other distros](#other-distros)
    3. [Compile from source](#compile-from-source)
    4. [Run tests](#run-tests)
        1. [Verbose tests](#verbose-tests)
        2. [Error in test_snappy.cpp](#error-in-test_snappycpp)
        3. [Error in test_video_capture.cpp](#error-in-test_video_capturecpp)
    5. [Modes and options](#modes-and-options)
        1. [Setting build variables](#setting-build-variables)
            1. [Local installation](#local-installation)
            2. [Musl libc](#musl-libc)
    6. [Add .cpp file](#add-cpp-file)
2. [Run Redemption](#run-redemption)
3. [Session recording](#session-recording)
    1. [Convert .mwrm/.wrm capture to video](#convert-mwrmwrm-capture-to-video)
4. [Compile proxy_recorder](#compile-proxy_recorder)
5. [Packaging](#packaging)
6. [Tag and Version](#tag-and-version)
7. [Test files](#test-files)
8. [FAQ](#faq)
    1. [Q - Why do you use bjam for Redemption instead of make, cmake, scons, etc ?](#q---why-do-you-use-bjam-for-redemption-instead-of-make-cmake-scons-etc-)
    2. [Q - How to add configuration variables in rdpproxy.ini ?](#q---how-to-add-configuration-variables-in-rdpproxyini-)
<!-- /toc -->


# Compilation

For automatic compilation, a Dockerfile is available. This one is based on Ubuntu, but other linux systems are supported like Debian or Alpine. For the latter, `-s MUSL_LIBC=1` must be added on the line containing `bjam` and the `libgettext` package must be added in the installed package.

The following is for manual installation.


## Dependencies

To compile Redemption you need the following packages:
- libboost-tools-dev (contains bjam and b2: software build tool) (https://www.bfgroup.xyz/b2/)
- libboost-test-dev (unit-test dependency)
- zlib1g-dev
- libssl-dev
- libkrb5-dev
- libsnappy-dev
- libpng-dev
- libbz2-dev
- libhyperscan-dev
- libffmpeg-dev (see below)
- libgettext (when musl is used)
- g++ >= 8.0 or clang++ >= 7.0 or other C++17 compiler

```sh
apt install libboost-tools-dev libboost-test-dev libssl-dev libkrb5-dev libsnappy-dev libpng-dev libbz2-dev libhyperscan-dev
```

Submodule ($ `git submodule update --init`):

- https://github.com/wallix/ppocr


## FFmpeg

If your goal is to use the proxy without doing any video conversion (the recording is still available), you can disable FFmpeg by adding `NO_FFMPEG=1` to your environment variables before compiling.

For more information on available variables, see [Setting build variables](#setting-build-variables).

### Ubuntu / Debian

Package:

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

- https://github.com/FFmpeg/FFmpeg/archive/n2.5.11.tar.gz (or later)

And set the [build variables](#setting-build-variables) (optionally)
- `FFMPEG_INC_PATH=/my/ffmpeg/include/path`
- `FFMPEG_LIB_PATH=/my/ffmpeg/library/path` (/!\\ without `/` terminal)
- `FFMPEG_LINK_MODE=shared` (static or shared, shared by default)


## Compile from source

([Instruction for Debian 9](https://github.com/wallix/redemption/issues/34#issuecomment-471322759)).

Well, that's pretty easy once you installed the required dependencies.

Go to the redemption folder then just run (as user):

```sh
bjam exe libs
```

(Always put a target name, otherwise the tests will also be compiled and executed).

Then install (as administrator):

```sh
bjam install
```

`bjam install` depends on `exe libs`, if you have enough rights, it is not necessary to run the first command.

Binaries are located by default in `/usr/local/bin`. For a user install, see [Local Installation](#local-installation).

You can choose your compiler and its version by adding `toolset=${compiler}` on the command line: `bjam toolset=gcc exe libs` / `bjam toolset=clang-16 exe libs` (see https://www.bfgroup.xyz/b2/manual/main/index.html#bbv2.overview.configuration and `tools/bjam/user-config.jam`).

You will find bjam auto-completion files for bash and zsh in the `tools/bjam` folder.

Note: If you intend to make changes to the proxy, it is not necessary to reinstall it each time, the executable created in the `bin` folder can be run directly. It is also possible to install only resources via `bjam install-resources` without installing the executable and library.


## Run tests

Tests are compiled and run with `bjam tests`. You can also compile everything (exe, lib, tests + runtime) with just `bjam` (without specifying a target).

Each test file has its corresponding target and can be run independently:

```sh
bjam tests/utils/test_rect  # for tests/utils/test_rect.cpp
bjam test_rect              # same
```

In addition, each folder has an associated target name:

```sh
bjam tests/utils        # all tests in tests/utils/
bjam tests/utils.norec  # all tests in tests/utils/, but not recursively (exclude sub-directories)
```

For more user-friendly test output, you can look at what's in `tools/bjam`.


### Verbose tests

By default, tests do not display log messages. These can be enabled by playing with the `REDEMPTION_LOG_PRINT` environment variable.

```sh
export REDEMPTION_LOG_PRINT=1
bjam tests
```

- `REDEMPTION_LOG_PRINT=1` enable all logs
- `REDEMPTION_LOG_PRINT=e` for error and debug only
- `REDEMPTION_LOG_PRINT=d` for debug only
- `REDEMPTION_LOG_PRINT=w` for other than info

In debug mode and if the `BOOST_STACKTRACE` option is set, the `Error` constructor displays a call stack. This can be controlled with the environment variable `REDEMPTION_FILTER_ERROR` which contains a comma separated list of values to ignore.

- `REDEMPTION_FILTER_ERROR=` no filter
- `REDEMPTION_FILTER_ERROR='*'` filter all
- `REDEMPTION_FILTER_ERROR=ERR_TRANSPORT_NO_MORE_DATA,ERR_SEC` filter specific errors (see `src/core/error.hpp`).


### Error in test_snappy.cpp

Under some versions of Ubuntu, Snappy dev files are broken and `SNAPPY_MAJOR`, `SNAPPY_MINOR` and `SNAPPY_PATCHLEVEL` macros are not defined.
The simplest way to fix that is editing `/usr/include/snappy-stubs-public.h` and define these above `SNAPPY_VERSION`.

Like below (change values depending on your snappy package).

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


### Error in test_video_capture.cpp

The tests on the video generation check the size of the output files. This way of doing is not perfect and the result depends on the version of ffmpeg and the quality of the encoders. There is no other way than to change the range of possible values.


## Modes and options

Bjam is configured to offer 3 compilation modes:

- `release`: default
- `debug`: Compile in debug mode
- `san`: Compile in debug mode + sanitizers (asan, lsan, usan)

The mode is selected by adding `variant=${mode}` or simple `${mode}` to the bjam command line.

There are also several variables for setting compiler options:

- `-s cxx_color`: default auto never always
- `-s cxx_lto`: off on fat linker-plugin
- `-s cxx_relro`: default off on full
- `-s cxx_stack_protector`: off on strong all
- ...

Complete list with `bjam cxx_help`.

Finally, `bjam` provides `cxxflags` and `linkflags` to add options to the compiler and linker. This is useful for example to remove warnings with the latest openssl versions.

```sh
bjam variant=debug -s lto=on cxxflags='-Wno-deprecated-declarations' targets...
```

By default, `bjam` compiles everything into a folder named `bin`, you can change this with `--build-dir=new-path`.


### Setting build variables

List with `bjam env_help`.

These variables can be used as environment variables or passed on the bjam command line with `-s varname=value`.

Example with ffmpeg:

```sh
bjam -s FFMPEG_INC_PATH=$HOME/ffmpeg/includes ....
# or
FFMPEG_INC_PATH=$HOME/ffmpeg/includes bjam ....
# or
export FFMPEG_INC_PATH=$HOME/ffmpeg/includes bjam ....
bjam ....
```

#### Local installation

If you have already compiled anything before this step, it is best to delete your `bin` folder (everything will be recompiled) or the `app_path_exe.o` file inside.

The paths to the installed files can be listed with `bjam env_help`. The minimum requirement is the following (change the `install_path` variable to your liking).

```sh
install_path="$HOME/redemption"
export PREFIX="$install_path"/usr/local
export ETC_PREFIX="$install_path"/etc
export VAR_PREFIX="$install_path"/var
export SESSION_PREFIX="$install_path"/var/lib
export PID_PATH="$install_path"/var/run
bjam ....
```

#### Musl libc

```sh
bjam -s MUSL_LIBC=1 ....
```

## Add .cpp file

The compiled files are referenced in `targets.jam`. This is a file that is automatically generated via `./tools/bjam/gen_targets.py` and is updated with `bjam targets.jam` or `./tools/bjam/gen_targets.py > targets.jam`.

`bjam targets.jam` also updated `projects/qtclient/redemption_deps.jam`.

When you added a .cpp file or there is a link error, remember to run `bjam targets.jam`.


# Run Redemption

For a local test, the usual options are `-n` and `-f`.
The first option prevents Redemption from forking in the background and the second makes sure
no other instance is running.

```sh
rdpproxy -nf
```
<!-- $ `bin/${BJAM_TOOLSET_NAME}/${BJAM_MODE}/rdpproxy -nf` -->

And now what ? If everything went ok, you should be facing a waiting daemon !
You need two more things; first a client to connect to Redemption, second a
server with RDP running (a Windows server, Windows XP Pro, etc.).

Redemption uses a hook file to get its target, username and password. This file
is `tools/passthrough/passthrough.py` communicates with rdpproxy through a unix socket.
This is referenced as "authentifier" in proxy logs.

```sh
./tools/passthrough/passthrough.py
```

Now, at that point you'll just have two servers waiting for connections
not much fun. You still have to run some RDP client to connect to proxy. Choose
whichever you like xfreerdp, rdesktop, remmina, tsclient on Linux or of course
mstsc.exe if you are on windows. All are supposed to work. If some problem
occurs just report it to us so that we can correct it.

Examples with freerdp when the proxy runs on the same host as the client:

```sh
xfreerdp /v:127.0.0.1
```

A dialog box should open in which you can type a device ip, username and a password.

These 3 fields can be pre-filled by configuring the connection identifiers sent by your RDP client. The target ip must be put with the login in the form `username@target_ip`:

```sh
xfreerdp /v:127.0.0.1 /u:username@10.10.43.11 /p:password
```

With default passthrough.py at least internal services should work. Try login `internal@bouncer2` or `internal@card`.

```sh
xfreerdp /v:127.0.0.1 /u:internal@bouncer2
```

`passthrough.py` is made to be edited and only provides the bare minimum.


# Session recording

To enable session recording, the line containing `is_rec` in `passthrough.py` must be uncommented.


## Convert .mwrm/.wrm capture to video

`.mwrm` and `.wrm` are native capture formats in `capture_flags=2` in `rdpproxy.ini`.

    redrec -f --video-codec mp4 -i file.mwrm -o output_prefix

Note: `rdpproxy --print-default-ini` show a default ini file.


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

## Q - Why do you use bjam for Redemption instead of make, cmake, scons, etc ?

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
