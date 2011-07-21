======================
A Manual to Redemption
======================

:Authors: - Christophe Grosjean
          - Martin Potier
:Organization: Wallix
:Contact: cgr@wallix.com
:Version: 0.1
:Date: 2011/07/20
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
which is libboost-test and libboost-program-options (which is programmed
to be removed).
It also relies on the following libraries: libssl, libcrypto, libdl and
libX11 (only needed for a functionnal test on genkeymap).

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
    +-- capture
    +-- core
    +-- docs
    +-- front
    +-- mod
    +-- sys
    +-- tests
    +-- utils

As of now, the top level of the directory structure of the repository looks like
the one up there.

RDP Orders
----------
::

    core/RDP/orders
    +-- RDPOrdersCommon.hpp
    +-- RDPOrdersNames.hpp
    +-- RDPOrdersPrimaryDestBlt.hpp
    +-- RDPOrdersPrimaryGlyphIndex.hpp
    +-- RDPOrdersPrimaryHeader.hpp
    +-- RDPOrdersPrimaryLineTo.hpp
    +-- RDPOrdersPrimaryMemBlt.hpp
    +-- RDPOrdersPrimaryOpaqueRect.hpp
    +-- RDPOrdersPrimaryPatBlt.hpp
    +-- RDPOrdersPrimaryScrBlt.hpp
    +-- RDPOrdersSecondaryBmpCache.hpp
    +-- RDPOrdersSecondaryBrushCache.hpp
    +-- RDPOrdersSecondaryColorCache.hpp
    +-- RDPOrdersSecondaryGlyphCache.hpp
    +-- RDPOrdersSecondaryHeader.hpp

Orders are the elements of the RDP protocol carrying the information, either for
drawing on a screen (drawing orders) or to manipulate cache (cache orders).

They are put together in the orders folder and classify in multiple levels according
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

    mod
    +-- cli
    +-- internal
    +-- null
    +-- rdp
    +-- transitory
    +-- vnc
    +-- xup

Modules are not very well defined for now, it is a work in progress.
The most recent part is internal. VNC provides access to a VNC backend.

On the conceptual level, the goal of a module is to decide which other module to
launch when it ends.

cli module
..........
cli uses information provided by clients at connection to decide whether it should
continue on the login box, or connect immediately to some remote server, continue on
internal module, and so on.

Internal Modules
................
::

    mod/internal/
    +-- bouncer2.hpp
    +-- close.hpp
    +-- dialog.hpp
    +-- internal_mod.hpp
    +-- login.hpp
    +-- test_card.hpp
    +-- test_internal.hpp
    +-- widget.cpp
    +-- widget.hpp
    +-- widget_window_login.hpp

Most internal modules are here for historical reasons; they are not certified
to be usable anymore since the ongoing work on Redemption still modifies and
obsoletes interfaces.

.. figure:: test_card_redemption.png
    :width: 66 %
    :alt: An example of internal module: test card.

    An example of internal module: test card.

Two internal modules were developped recently to provide small functionnal and
visual tests: ``bouncer2.hpp`` and ``test_card.hpp``.

The first is design to display a bouncing cube, which flees the mouse arrow.
The second a complete test card designed to test the base orders' color -- a lot
of trouble comes for color handling and it can reveal very userful to check with
a test card. In the preceding figure, we used several ``OpaqueRect``, ``MemBlt``
and ``GlyphIndex``.

Null module
...........
This module is never called. It is used to reinitialise module states to nothing,
it can receive any data from any clients without complaining.

Transitory module
.................
It is used as a placeholder when waiting for information from authentifier to choose
the next module.

Capture
-------
::

    capture/
    +-- capture.hpp
    +-- nativecapture.hpp
    +-- staticcapture.hpp

As a proxy, Redemption does not only redirect traffic, it does also capture raw
RDP sessions! You can save sessions on hard drive and replay them later. This is
useful in a context of debugging first if you are developping a RDP server or a
RDP client but also if you want to provide a authenticating proxy to a number of
computers (the way `Wallix use it with the WAB`_).

.. _Wallix use it with the WAB: http://www.wallix.com/index.php/products/wallix-adminbastion

Tests
-----
::

    tests/
    +-- compression.perf
    +-- fixtures
    +-- ftest_short_session_to_bouncing_bitmap.cpp
    +-- test_bitmap_cache.cpp
    +-- test_bitmap.cpp
    +-- test_bitmap_perf.cpp
    +-- test_colors.cpp
    +-- test_compact_to_aligned.cpp
    +-- test_config.cpp
    +-- test_context_as_map.cpp
    +-- test_dico.cpp
    +-- test_font.cpp
    +-- test_keymap.cpp
    +-- test_mod.hpp
    +-- test_orders.hpp
    +-- test_primary_order_dest_blt.cpp
    +-- test_primary_order_glyph_index.cpp
    +-- test_primary_order_line_to.cpp
    +-- test_primary_order_mem_blt.cpp
    +-- test_primary_order_opaque_rect.cpp
    +-- test_primary_order_pat_blt.cpp
    +-- test_primary_order_scr_blt.cpp
    +-- test_rect.cpp
    +-- test_region.cpp
    +-- test_rsa_keys.cpp
    +-- test_secondary_order_col_cache.cpp
    +-- test_stream.cpp
    +-- test_strings.cpp
    +-- test_transport.cpp
    +-- test_urt.cpp
    +-- test_widget.cpp
    +-- test_write_over_file.cpp
    +-- xrdp.ini

Tests -- both unit and functionnal tests -- are very important to have an idea of the 
amount of latent bugs of your program, and we are very attached to providing as
many tests as we can.

They are very useful for the developper too, because they show parts of the code in use
thus indicating how to use it and what they should do. Unfortunately, the program is
not tested enough, mainly because the scarce human ressources are devoted to feature adding
and structural cleaning. We need help ! 

.. code-block:: c++

