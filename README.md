Master branch: [![Build Status from master](https://travis-ci.org/wallix/redemption.svg?branch=master)](https://travis-ci.org/wallix/redemption)

Future branch: [![Build Status from future](https://travis-ci.org/wallix/redemption.svg?branch=future)](https://travis-ci.org/wallix/redemption)


Dependencies
============

To compile ReDemPtion you need the following packages:
- libboost-dev (contains bjam : software build tool) (http://sourceforge.net/projects/boost/files/boost/)
- libboost-test-dev (unit-test dependency)
- libssl-dev
- libkrb5-dev
- libgssglue-dev
- libsnappy-dev
- libpng12-dev
- libffmpeg-dev (see below)
- g++ >= 4.9 or clang++ >= 3.5 or other C++14 compiler

<!--Optionally:
- python (python-dev)-->

Submodule ($ `git submodule update --init`):
- https://github.com/wallix/program_options
- https://github.com/wallix/ppocr

## FFmpeg:

### Ubuntu (16.04):
<!-- BEGIN optional if NO_FFMPEG ? (note: ok with 53 (?) and 54 version)-->
- libavcodec-dev
- libavformat-dev
- libavutil-dev
- libswscale-dev
- libavcodec-ffmpeg56
- libavformat-ffmpeg56
- libavutil-ffmpeg54
- libswscale-ffmpeg3
<!-- - libbz2-dev -->
<!-- END optional -->

### Other distros:
- https://github.com/FFmpeg/FFmpeg/archive/n2.5.11.tar.gz

And set environment variable (optionally)
- `export FFMPEG_INC_PATH=/my/ffmpeg/include/path`
- `export FFMPEG_LIB_PATH=/my/ffmpeg/library/path` (/!\\ without `/` terminal)
- `export FFMPEG_LINK_MODE=shared` (static or shared, shared by default)

## Environment variable setting

List with `grep '\[ setvar' jam/defines.jam`


Generate target and lib/obj dependencies
========================================

When create a new test or when a target fail with link error:

`bjam targets.jam` for updated `targets.jam` file.

Specific deps (libs, header, cpp, etc) in `./tools/bjam/gen_targets.py`.


FAQ
===

Q - Why did you fork xrdp instead of contributing to the project ?
------------------------------------------------------------------

Many reasons. The most important one being we wished to change so many things
for our usage, we needed to have control on the project roadmap.

In details, a very important goal was to unit test as much of the program as
possible, and to achieve something simpler than xrdp.

The good part was there were obvious constraints of xrdp that we could remove:

- ReDemPtion had no need to be fully Windows compatible, enabling us:
    * to use a simple (and rock solid) listen and fork model instead of threads,
    * to remove abstract system independent compatibility layer,
    * to replace dynamically loaded back-end modules by simpler static ones, etc.

- several parts of xrdp are trying to optimize program performance through
use of complex macros, code duplication, specialized program paths (like
avoiding memory duplication when color model does not change), etc. We had a
very suspicious look over those. They may or may not enhance performances.
As there was no test to check benefits, it was hard to tell. These optimizations
also made tests harder to write. We just chose to simplify, removing them
whenever possible even if we may reintroduce some at a later time, when
performance tests will be available.

- we intend to merge the client and the server part of xrdp in one unified
library where client and server are (guaranteed) compatible with each other.
The method to do this was merging files containing client and server parts
to have matching client and server code side by side. That implied many changes
in code architecture, while xrdp clearly kept rdesktop inherited client code
very similar to original code (and it has it's advantages as it makes porting
new code of rdesktop to xrdp easier).

And once we decided to fork, it was also easy to choose other more drastic
changes that made our life easier like porting to C++ or going for a simpler
build system.


Q - Why did you use C++ for ReDemPtion instead of plain C ?
------------------------------------------------------------
Any of the two could do the trick, but it appeared that a few components of
STL or the boost softwares were handy, mostly for easing unit testing and
build process.

Really we do not use much of C++ special features like templates or libraries.
We intend to keep the code that way to avoid the number 1 pitfall of C++,
bury a program behind so many abstraction layers that nobody can understand
any more what this program is doing and how it's doing it. By the way, you can
achieve the same kind of troubles with C using enough macro abstraction layers.


Q - Why do you use bjam for ReDemPtion instead of make, cmake, scons, etc ?
---------------------------------------------------------------------------

It is simple, more that could be thought at first sight, and bjam has the major
feature over make to keep source directories clean, all build related
informations for all architecture are kept together in bin directory.

The main drawback of bjam is the smaller user base.

But keeping in mind the complexity of make (or worse autotools + make), bjam is
a great help. We also used to have an alternative cmake build system, but it was
more complex than bjam and not maintained, so was removed.


Q - Fine, now how do I compile and run ReDemPtion ?
---------------------------------------------------
Well, that's pretty easy once you installed the required dependencies.

Just run (as user):

$ `bjam` or `bjam toolset=gcc`, `bjam toolset=clang` or `bjam toolset=your-compiler` (see http://www.boost.org/build/doc/html/bbv2/overview/configuration.html)

Compile executables without tests (as user):

$ `bjam exe libs`

and install (as administrator):

\# `bjam install`

Binaries are located in /usr/local/bin.

To test it, executes:

$ `python tools/passthrough/passthrough.py`

\# `/usr/local/bin/rdpproxy -nf`

Now, at that point you'll just have two servers waiting for connections
not much fun. You still have to run some RDP client to connect to proxy. Choose
whichever you like xfreerdp, rdesktop, remmina, tsclient on Linux or of course
mstsc.exe if you are on windows. All are supposed to work. If some problem
occurs just report it to us so that we can correct it.

Example with freerdp when the proxy runs on the same host as the client:

$ `xfreerdp 127.0.0.1`

A dialog box should open in which you can type a username and a password.
With default authhook at least internal services should work. Try login: bouncer
and password: bouncer, or login: card and password: card. To access your own
remote RDP hosts you'll of course have to configure them in authhook.py.
Hopefully at some time in the future these won't be hardcoded, but authhook.py
will access to some configuration file. If you want to provide such extensions
to current authhook.py, please contribute it, it will be much appreciated.

You can also bypass login dialog box and go directly to the RDP server by
providing a login and a password from command line.

$ `xfreerdp -u 'bouncer' -p 'bouncer' 127.0.0.1`


Q - Why did you choose this special "headers only" coding style ?
-----------------------------------------------------------------

It may look surprising, but the first consideration doing so is maintenance.
When we change functions or methods, we are only doing it in one place **the
header** instead of two places, the header and the implementation of the
function. As at this stage, API is far from being stable and we very often
change functions signatures or move them around, not having to do it in two
places is nice.

It also has some other advantages:

- it forces avoiding loops between classes, as this style just can't work
with such loops.

- distribution of headers only libraries is the easiest possible. Just include
them. No target dependent binary, no link necessary.

- optimizing compilers - like gcc - can perform a better job when they are not
restricted by the linker to some artificial horizon. Even if modern compilers
are trying to address this issue, no link is *still* better.

On the other hands there is also drawbacks, the worst one being that
compilation time is much longer. Separate compilation is not an issue with a
project this small. Hopefully independant "headers only" libraries will be
extracted before it becomes one.

Note by CGR: I *know* that separate compilation is the right way. If you don't
believe me check here:

http://stackoverflow.com/questions/5283841/why-compile-to-an-object-file-first
