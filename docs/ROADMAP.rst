==================
Redemption ROADMAP
==================

:Authors: - Christophe Grosjean
          - Martin Potier
:Organization: Wallix
:Contact: christophe.grosjean@wallix.com
:Version: 0.1
:Date: 2011/07/20
:Copyright: Public

Unification of protocol layers:

    Protocol layers should be common to client and server. The current
    code contains two concurrent implementation of the protocol layers.

    Protocol layer should be *free* of system calls.

Create independant libraries with protocol layers:

    Unified protocol layers should be easy to reuse in any client or
    server implementation.

    Ideally, we could take rdesktop and replace its implementation of the
    protocol layers without breaking it (that's just an image).

Create a few sample server applications using Redemption:

    It allows the use of Redemption as a test suite for other implementations
    of RDP clients (including Microsoft's :p)

    It *does* mean a huge cleanup of the widget layer!

Extend RDP support level:

    First, it implies including all the RDP5 extensions

    It includes also moving on to RDP6 and RDP7 (higher available today)

Make pluggable interface usable:

    Redemption aims to be a "any to any" proxy, using RDP as its core protocol.
    It implies developping plugins to support new protocols (either frontend or
    backend).

    If Emacs make coffee, Redemption must make T: it must be able to connect one
    to many (like broadcasting a server screen to many clients) and many to one
    (like connecting to several servers from one client using Redemption).
    Yes, for now it's classified as dream. A first serious step would be to write
    down ideas of implementation, after fixing the pluggable interface.

Autogeneration of documentation:

    There's a README, there's a tiny Manual to Redemption, there must also be
    automated documentation of the source code (ideally a reStructuredText based
    renderer).

    If no options are available, write a little parser.

Write a standard authhook:

    authhook should use a real configuration file, and nobody should have to edit the
    authhook source code.

Fix the X backend:

    It used to work before forking from xrdp, but I guess it should be rewrited from
    scratch. Maybe one should take a look a the current xrdp code before doing it.
 
Remove libboost-program-options:

    Why? It is now used to parse program options, configuration file and create a nice
    online help. This tool is not entirely satisfying since it does not allow editing
    the configuration file (we would like to configure Redemption through a Redemption
    internal module) -- on top of the fact that it's awkward to use for configuration
    file.

Create a manual page for Redemption:

    That's self-explanatory, one could use reST to create the man page.

Include permanent debug switch:

    Redemption should be able to be more or less verbose and log different parts of the
    code when asked to at launch. Recompiling should not be necessary for that purpose.

Merge some internal modules:

    close, dialog and login internal module should merge.
