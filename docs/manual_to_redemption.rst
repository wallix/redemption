======================
A Manual to Redemption
======================

:Authors: - Christophe Grosjean
          - Martin Potier
:Organization: Wallix
:Contact: cgr@wallix.com
:Version: 0.2
:Date: 2013/05/15
:Copyright: Public
:Abstract: This paper is a short developper manual
    for understanding the structure of Redemption
    the versatile RDP proxy. It goes through RDP
    orders, recording mechanism and internal modules.

.. raw:: pdf

    PageBreak

.. note:: Redemption is actively developped and was just released. This
    manual is intended to help any developper wanting to join the effort.
    This manual *will* be deprecated in parts, so keeping it up to date is
    crucial. Nonetheless, it will provide enough knowledge on Redemption to
    get started.

.. raw:: pdf

    PageBreak

About Redemption
++++++++++++++++

Redemption is a versatile RDP proxy, meaning one will connect
to remote desktops through Redemption. This allows to centralize
remote connection creating a single end point for several desktops.
About the versatile part, Redemption is able to connect to VNC
servers (more protocols to come), permitting connection from Windows®
to Linux, for instance. It is only known to run on Linux (Ubuntu, Debian,
Archlinux, either 32 or 64 bits architecture) and no effort were made
to port it to Win32 or to MacOS. It may run on FreeBSD will little effort
but there were no attempts, for now, feel free to give it a try.

You can have a try on the README to learn more.


Getting Started
+++++++++++++++

This section introduces you to fetching, building and running Redemption.

Fetching
--------
Redemption is hosted on Github.com, you can get it by different means, but
for developping purpose, we recommend cloning your own git repository.
To do so, the steps are as follow

    1. Create a free account on github.com;
    2. ``fork`` the official repository (``wallix/redemption``);
       This will create, supposing that your username is henry, a
       forked repository called ``henry/redemption``;
    3. Clone this fork to your place using the classical:
       ``git clone git@github:henry/redemption`` (don't forget to
       change henry to your own username).

Now you are ready to build.

Building
--------
Redemption's main building system is bjam.
It uses a very small part of the boost libraries -- only the essential --
which is libboost-test and libboost-program-options (which is being removed).
It also relies on the following libraries: libssl, libcrypto, libpng, libdl and
libX11 (only needed for a functional test on genkeymap).

Once the build dependencies are installed, building is easy: ::

    $ cd redemption
    $ bjam

This will create a ``bin`` folder at the root of the source where all bjam's
mess is stashed. If you wonder why there are so many warnings, you can read
the answer in the README while it's building.

To install (default path ``/usr/local/bin``): ::

    $ sudo bjam install

That's it, let's run it.

Running
-------
For a local test, the usual options are ``-n`` and ``-f``. The first option
prevents Redemption from forking in the background, and the second makes sure
no other instance is running. ::

    $ sudo /usr/local/bin/rdpproxy -nf

And now what ? If everything went ok, you should be facing a waiting daemon !
You need two more things; first a client to connect to Redemption, second a
server with RDP running (a Windows server, Windows XP Pro, etc.).

Redemption uses a hook file to get its target, username and password. This file
is in ``utils/authhook.py`` and as you noticed it is written in Python. Two
dictionnaries are important, the first one stores passwords, the second one stores
the targets. Examples show the way. ::

    $ rdesktop 127.0.0.1

You can use any RDP client you like, connect from any machine you want, just use
the IP of the computer hosting Redemption.

If you are poor -- or pure, you choose -- and you don't own a server, you can
still have a try with the internal modules. ::

    $ rdesktop 127.0.0.1 -u card -p card
    $ rdesktop 127.0.0.1 -u bouncer -p bouncer

Shouldn't it run, please send us a bug notice.


An eye on the internals
+++++++++++++++++++++++

::
    .
    +-- acl
    +-- channels
    +-- capture
    +-- core
    +-- docs
    +-- front
    +-- ftests
    +-- keyboard
    +-- main
    +-- mod
    +-- sys
    +-- tests
    +-- tools
    +-- transport
    +-- utils

As of now, the top level of the directory structure of the repository looks like
the one up there.

Front
----------
::

    front/

Front is the server side of the proxy, connected to remote RDP client (mstsc, rdesktop).
To implement a Terminal Server it uses basic bricks provided by core/RDP protocol classes. 

acl
----------
::

    acl/

Redemption provides some authentication features at proxy level. It can check user identity 
and password before connecting to remote servers. Or propose a list of targets avaialble to user.
This is done remotely (by an independant python program, check tools/authhook.py for sample code).
The acl code provide provides communication facilities to remote authentication module.

RDP Orders
----------
::
   core/RDP/orders/
   ├── RDPOrdersCommon.hpp
   ├── RDPOrdersNames.hpp
   ├── RDPOrdersPrimaryDestBlt.hpp
   ├── RDPOrdersPrimaryGlyphIndex.hpp
   ├── RDPOrdersPrimaryLineTo.hpp
   ├── RDPOrdersPrimaryMemBlt.hpp
   ├── RDPOrdersPrimaryOpaqueRect.hpp
   ├── RDPOrdersPrimaryPatBlt.hpp
   ├── RDPOrdersPrimaryScrBlt.hpp
   ├── RDPOrdersSecondaryBmpCache.hpp
   ├── RDPOrdersSecondaryBrushCache.hpp
   ├── RDPOrdersSecondaryColorCache.hpp
   └── RDPOrdersSecondaryGlyphCache.hpp

Orders are the elements of the RDP protocol carrying the information, either for
drawing on a screen (drawing orders) or to manipulate cache (cache orders).

They are put together in the orders folder and classified in multiple levels according
to the RDP protocol (Primary, Secondary, ...). Of course -- and quite infortunately
-- all existing orders are not implemented, yet there's enough to make Redemption
work.

.. important::
    In Redemption, an order is an object. It holds a coherent set of information and
    is able to ``receive()`` (decode) and ``emit()`` (encode) itself from a binary stream.

Details on the orders of the RDP protocol can be found both in the official documentation 
of the protocol from Microsoft, which is now freely available and in Redemption in the
form of comments at the most convenient place: where it is used; thus each order file
contains a generous portion of comments quoting the official documentation. Feel free
to peek through it.

Of course you are welcomed to contribute parts of the documentation
that would be missing from the comments. Please note that we also tried to clarify dark
spots of the official documentation.

Primary Drawing Orders
......................
Primary drawing orders carry information on how to draw elements. Their name start
with ``RDPOrdersPrimary`` plus the name of the order. The common header for all
primary orders is in the ``RDPOrdersPrimaryHeader.hpp`` file.

Secondary Drawing Orders
........................
Secondary drawing orders handle cache operations. Their name start with
``RDPOrdersSecondary`` followed by the name of the order. The common header for all
secondary orders is in the ``RDPOrdersSecondaryHeader.hpp`` file.

Modules
-------
::

   mod/
   +-- cli
   +-- draw_api.hpp
   +-- internal
   +-- mod_api.hpp
   +-- null
   +-- rdp
   +-- transitory
   +-- vnc
   +-- xup

Modules are not very well defined for now, it is a work in progress.
The most recent part is internal. VNC provides access to a VNC backend.

On the conceptual level, modules manage connexion opened to some backend (server).

cli module
..........
cli is a pseudo modules that receive information provided by client on statup.


Internal Modules
................
::

   mod/internal/
   +-- bouncer2.hpp
   +-- close.hpp
   +-- dialog.hpp
   +-- internal_mod.hpp
   +-- login.hpp
   +-- selector.hpp
   +-- test_card.hpp
   +-- test_internal.hpp
   +-- widget
   +-- widget2

Internal modules are the internal server side of redemption. They are used to
provide basic interaction betwwen user and proxy. They rely only on the included 
internal widget library (soon to be replaced ny widget2, hopefully easier to 
maintain and extend).


.. figure:: test_card_redemption.png
    :width: 66 %
    :alt: An example of internal module: test card.

    An example of internal module: test card.

Two internal modules were developped recently to provide small functionnal and
visual tests: ``bouncer2.hpp`` and ``test_card.hpp``.

The first is designed to display a bouncing cube, which flees the mouse arrow.
The second a complete test card designed to test the base orders' color -- a lot
of trouble historically came from color handling and it was very useful to check
with a test card. In the preceding figure, we used several internal RDP drawing 
orders namely ``OpaqueRect``, ``MemBlt`` and ``GlyphIndex``.

Null module
...........
This module is never supposed to be called. It is used as a Null object to 
reinitialise module states to nothing, it can receive any data from any clients
without complaining (but that should obviously not happen).

Transitory module
.................
Transitory is used as a placeholder when waiting for information from authentifier
to choose the next module (Null module could probably be also used for that purpose).

Capture
-------
::

    capture/
    +-- capture.hpp
    +-- nativecapture.hpp
    +-- staticcapture.hpp

As a proxy, Redemption does not only redirect traffic, it does also capture raw
RDP sessions! You can save sessions on hard drive and replay them later or create
still images. This is really useful when debugging first if you are developping
a RDP server or a RDP client but also if you want to provide an authenticating proxy
to a number of computers (the way `Wallix use it with the WAB`_).

.. _Wallix use it with the WAB: http://www.wallix.com/index.php/products/wallix-adminbastion

Utils
-------
::

General purpose C++ source code used by Redemption. Feel free to reuse or modify any parts
in your own project if it pleases you in any way.


Tools
-------
::

Gather all usefull scripts for redemption management: sample python authentifier, 
log parsing tools, source code generators, and so on.


Tests
-----
::

    tests/
    ftests/

Tests -- both unit and functionnal tests -- are very important to have an idea of the 
amount of latent bugs of your program, and we are very attached to providing as
many tests as we can.

They are very useful for the developper too, because they show parts of the code in use
thus indicating how to use it and what they should do.

As you may have guessed, unit tests are put in tests/ and it's subdirectories and
functional tests in ftests. 

We are currently working on providing some coverage data for the project using gcov.
To have a clear view of wich parts are or aren't tested.


.. code-block:: c++
