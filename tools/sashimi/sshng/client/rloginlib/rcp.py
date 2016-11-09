#!/usr/bin/python -O
#
# Copyright (c) 2008 WALLIX, SAS. All rights reserved.
#
# Licensed computer software. Property of WALLIX.
# Product Name: Sphinx
# Author(s) : Olivier Hervieu <olivier.hervieu@wallix.com>
# Id: $Id$
# URL: $URL$
# Module description: Rlogin Library
#
"""
This Module is a simple rcp client, written entirely in python
"""

# Imported modules
import rloginlib
from optparse import OptionParser

def test():
    """Test program for rcplib.

    Usage: python rloginlib.py [-d] [-r] file1 file2 
      -d  Turns on Debug Information
      -r  If any of the source files are directories, rcp copies each subtree 
          rooted at that name; in this case the destination must be a directory.
    """



    parser = OptionParser()
    parser.add_option("-d", "--debug", action="store_true", dest="debug",
                        help = "Set Debug On",
                        default=0)
    parser.add_option("-r", "--recursive", action="store_true", dest="recursive",
                        help = "Set Recursive Transfer On", 
                        default=False)
    parser.add_option("-x", "--encrypt", action="store_true", dest="encrypt",
                        help = "Set Encryption On", 
                        default=False)
    parser.add_option("-p", "--preserve", action="store_true", dest="preserve",
                        help = "Attempt to preserve in mod. times and modes of the source file",
                        default=False)
    parser.add_option("-f", "--from" , dest = "source" ,
                        help = "File(s) Source",
                        default=None)
    parser.add_option("-t", "--to"   ,
                        help = "File(s) Destination",
                        default=None)

    (options, dummy_args) = parser.parse_args()


    # Check args -f and -t are mandatory
    if options.to is None or options.source is None:
        raise Exception("-f and -t flags are mandatory")


    # Check from and to args
    try:
        # addr_source can to be <host>@<ip> or empty
        # path_source is the path where we can find the file
        # or directory
        addr_source, path_source = options.source.split(':')
    except ValueError:
        if len(options.source.split('@'))>1:
            raise Exception("Invalid Syntax for -f flag")
        # options.source is local (the format is not "user@dest:path"
        else:
            addr_source = ''
            path_source = options.source

    try:
        # addr_dest can to be <host>@<ip> or empty
        # path_destination is the path where we send the file
        # or directory
        addr_dest, path_dest = options.to.split(':')
    except ValueError:
        if len(options.to.split('@'))>1:
            raise Exception("Invalid Syntax for -t flag")
        else:
            addr_dest = ''
            path_dest = options.to

    # if source and destination are empty
    if addr_dest == addr_source == '':
        raise Exception("Invalid Syntax")

    if addr_dest != '' and addr_source != '':
        remote_to_remote(addr_source, path_source, addr_dest, path_dest)

    # we send from local to remote
    if addr_dest != '' and addr_source == '':
        user, dest = addr_dest.split('@')
        rl = rloginlib.Rlogin(user, dest, 514)
        rl.set_debuglevel(options.debug)
        rl.file_to_remote_procedure(path_source, addr_dest, path_dest)

    # we get file from remote to local
    if addr_dest == '' and addr_source != '':
        user, dest = addr_source.split('@')
        rl = rloginlib.Rlogin(user, dest, 514)
        rl.set_debuglevel(options.debug)
        rl.file_to_local_procedure(addr_source, path_source, path_dest)

    rl.close()


def remote_to_remote(source, path_source, dest, path_dest):
    pass

if __name__ == '__main__':    
    test()
