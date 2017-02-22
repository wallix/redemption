==================
Redemption ROADMAP
==================

:Authors: - Christophe Grosjean
          - Martin Potier
:Organization: Wallix
:Contact: christophe.grosjean@wallix.com
:Version: 0.2
:Date: 2012/11/14
:Copyright: Public

Unification of protocol layers:
===============================
    +--------------------------------------------------------------------+---------+
    | Protocol layers should be common to client and server. The current |         |
    | code contains two concurrent implementation of the protocol layers.| DONE    |
    +--------------------------------------------------------------------+---------+
    | Protocol layer should be *free* of system calls.                   | DONE    |
    +--------------------------------------------------------------------+---------+ 

Extend RDP support level:
=========================

    +--------------------------------------------------------------------+---------+
    | First, it implies including all the RDP5 extensions                | ONGOING |
    +--------------------------------------------------------------------+---------+
    | It includes also moving on to RDP6 and RDP7 (or higher available)  | NOTDONE |
    +--------------------------------------------------------------------+---------+

Create independant libraries with protocol layers:
==================================================
    +--------------------------------------------------------------------+---------+
    | Unified protocol layers should be easy to reuse in any client or   | ONGOING |
    | server implementation.                                             |         | 
    +--------------------------------------------------------------------+---------+
    | Ideally, we could take rdesktop and replace its implementation of  | NOTDONE | 
    | the protocol layers without breaking it (that's just an image).    |         |
    +--------------------------------------------------------------------+---------+

ReDemPtion as an application server
===================================

    +--------------------------------------------------------------------+---------+
    | Better keyboard support with Windows standard behavior             | DONE    |
    +--------------------------------------------------------------------+---------+
    | It *does* mean a huge cleanup of the widget layer!                 | ONGOING |
    +--------------------------------------------------------------------+---------+
    | close, dialog and login internal module should merge.              | ONGOING |
    +--------------------------------------------------------------------+---------+
    | work on selector to make it more efficient (avoid redrawing all    | ONGOING |
    | screen at each refresh.                                            |         |
    +--------------------------------------------------------------------+---------+
    | Work on some abstract widget based description of boxes            | ONGOING |
    +--------------------------------------------------------------------+---------+
    | Allow the use of Redemption as a test suite for other              | NOTDONE |
    | implementations of RDP clients (including Microsoft's :p)          |         |
    +--------------------------------------------------------------------+---------+


Documentation and support
=========================
    +--------------------------------------------------------------------+---------+
    | Automated technical documentation:                                 |         |
    | There's a README, there's a tiny Manual to Redemption, there must  |         |
    | also be automated documentation of the source code (ideally a      | STALLED |
    | reStructuredText based renderer).                                  |         |
    | If no options are available, write a little parser.                |         |
    +--------------------------------------------------------------------+---------+
    | Creation of a standard unix man page:                              |         |
    | That's self-explanatory, one could use reST to create the man page.| NOTDONE |
    +--------------------------------------------------------------------+---------+
    | Permanent debug switches:                                          |         |
    | Redemption should be able to be more or less verbose and log       |         |
    | different parts of the code when asked to at launch. Recompiling   | DONE    |
    | should not be necessary for that purpose.                          |         |
    +--------------------------------------------------------------------+---------+

Session recording:
==================
    +--------------------------------------------------------------------+---------+
    | Redemption should be able to record any session in an internal file|         |
    | format similar to RDP orders encoding.                             | DONE    |
    +--------------------------------------------------------------------+---------+
    | ReDemPtion should be able to take periodic snapshots of server     | DONE    |
    | screen to some image format (PNG).                                 |         |
    +--------------------------------------------------------------------+---------+
    | Some external utility should be able to extract datas from native  | DONE    |
    | recordings                                                         |         |
    +--------------------------------------------------------------------+---------+
    | Keyboard input should be stored as text inside native recordings   | NOTDONE |
    +--------------------------------------------------------------------+---------+

Write a standard authhook:
==========================
    +--------------------------------------------------------------------+---------+
    | authhook should use a real configuration file, and nobody should   |         |
    | have to edit the authhook source code.                             | NOTDONE |
    +--------------------------------------------------------------------+---------+

Fix backends:
==================
    +--------------------------------------------------------------------+---------+
    | X Backend used to work before forking from xrdp, but I guess it    |         |
    | should be rewritten from scratch. Maybe one should take a look at  | STALLED |
    |  the current xrdp code before doing it.                            |         |
    +--------------------------------------------------------------------+---------+
    | VNC backend is much better but still need support for many         | ONGOING |
    | compression algorithms.                                            |         | 
    +--------------------------------------------------------------------+---------+
    | VNC copy/paste (more limited than RDP's) should work               | NEARLY  |
    |                                                                    | DONE    |
    +--------------------------------------------------------------------+---------+

Remove libboost-program-options:
================================
    +--------------------------------------------------------------------+---------+
    | Why? It is now used to parse program options, configuration file   |         |
    | and create a nice online help. This tool is not entirely satisfying| NEARLY  |
    |  since it does not allow editing the configuration file (we would  | DONE    |
    | like to configure Redemption through a Redemption internal module) |         |
    | -- on top of the fact that it's awkward to use for configuration   |         |
    | file.                                                              |         |
    +--------------------------------------------------------------------+---------+

Make pluggable interface usable:
================================
    +--------------------------------------------------------------------+---------+
    | Redemption aims to be a "any to any" proxy, using RDP as its core  | ONGOING |
    | protocol.                                                          |         |
    | It implies developping plugins to support new protocols (either    |         |
    | frontend or backend).                                              |         |
    +--------------------------------------------------------------------+---------+
    | If Emacs make coffee, Redemption must make T: it must be able to   | NOTDONE |
    | connect one to many (like broadcasting a server screen to many     |         |
    | clients) and many to one (like connecting to several servers from  |         |
    | one client using Redemption). Yes, for now it's classified as      |         |
    | dream. A first serious step would be to write down ideas of        |         |
    | implementation, after fixing the pluggable interface.              |         |
    +--------------------------------------------------------------------+---------+

    
