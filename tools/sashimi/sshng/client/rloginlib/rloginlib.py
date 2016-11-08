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

# Imported modules
import sys
import socket
import select
import os
import stat
from  time import time
import traceback
import re
from logger import Logger


__all__ = ["Rlogin"]

# Tunable parameters
DEBUGLEVEL = 0

# Rlogin protocol defaults
RLOGIN_PORT = 513
RCP_PORT    = 514

class Rlogin(object):
    """ Rlogin Interface Class.
    """
    def __init__(self, remote_user = None, host = None, port = 0):
        """Constructor.

        When called without arguments, create an unconnected instance.
        With a hostname argument, it connects the instance; a port
        number is optional.

        """
        self.debuglevel = DEBUGLEVEL
        self.host = host
        self.port = port
        self.sock = None
        self.temp = 0
        self.local_user = os.environ["LOGNAME"]

        if remote_user is None:
            self.remote_user = self.local_user
        else:
            self.remote_user = remote_user

        if host is not None:
            self.connect(host, port)

    def __del__(self):
        """Destructor -- close the connection."""
        self.close()

    def connect(self, host, port=0):
        """Connect to a host.

        The optional second argument is the port number, which
        defaults to the standard rlogin port (513).

        Don't try to reopen an already connected instance.

        """
        if port == 0 :
            port = RLOGIN_PORT
        msg = "getaddrinfo returns an empty list"
        for res in socket.getaddrinfo(host, port, 0, socket.SOCK_STREAM):
            addr_family, socktype, proto, dummy_canonname, skt_addr = res
            try:
                self.sock = socket.socket(addr_family, socktype, proto)
                #Now we bind the socket to a port to handle stderr
                #Note : the port is between 512 and 1024
                #The script who called this module must be launched as root
                remote_port = 1023
                while remote_port < 1024 and remote_port > 512:
                    try:
                        self.sock.bind(('', remote_port))
                        break
                    except socket.error:
                        remote_port = remote_port - 1
                    except Exception, exc:
                        Logger().debug("Exception caught %s %s\n%s" % (
                            type(exc), repr(exc), traceback.format_exc()
                        ))
                self.sock.connect(skt_addr)
            except socket.error, msg:
                if self.sock:
                    self.sock.close()
                self.sock = None
                continue
            break
        if not self.sock:
            raise socket.error, msg


    def shell_identification_procedure(self):
        #Identification procedure
        self.sock.send('\0')
        self.sock.send(self.local_user)
        self.sock.send('\0')
        self.sock.send(self.remote_user)
        self.sock.send('\0')
        self.sock.send('xterm/9600')
        self.sock.send('\0')

    def file_to_remote_procedure(self, path_source, addr_dest, path_dest):
        #Identification procedure
        self.write('\0')
        self.write('%s\0%s\0%s -t %s\0'%(self.local_user,
                                        self.remote_user,
                                        'rcp',
                                        path_dest))

        self.source(path_source, addr_dest, path_dest)

    def file_to_local_procedure(self, addr_source, path_source, path_dest):
        self.write('\0')
        self.write('%s\0%s\0%s -f %s\0'%(self.local_user,
                                        self.remote_user,
                                        'rcp',
                                        path_source))

        self.read(1)
        self.sink(addr_source, path_source, path_dest)

    def sink(self, dummy_addr_source, dummy_path_source, path_dest):
        self.write('\0')
        #First : Checking the destination of the file
        path_dest_is_dir = False
        try:
            if stat.S_IFMT(os.stat(path_dest)[stat.ST_MODE]) == stat.S_IFDIR :
                path_dest_is_dir = True
        except OSError:
            #The destination file does not exist
            #We gonna create it later
            pass

        # The data received must be end with \n
        data = self.read(1024)
        if not data.endswith('\n'):
            raise Exception("Unexpected data received")
        data = data [0:-1] #Removing the trailing \n

        mode, size, file_name = data.split(' ')

        if mode[0] is 'T':
            raise Exception("NotYetImplemented")
        if mode[0] is 'D':
            raise Exception("NotYetImplemented")

        if mode[0] is 'C':
            #Checking the mode:
            mode = mode[1:5]

            if path_dest_is_dir:
                path_dest = path_dest + file_name

            # Receiving now data.
            new_fd = open(path_dest,'w+')
            # Grab the blocksize
            blksize = os.stat(path_dest).st_blksize
            self.write('\0')
            i = 0
            size = int(size)
            while i < size:
                bytes_to_read = 0
                if (i + blksize > size):
                    bytes_to_read = size -i
                else:
                    bytes_to_read = blksize
                data = self.read(bytes_to_read)
                if len(data) != bytes_to_read:
                    raise Exception("Error While Receiving File")
                new_fd.write(data)
                i = i + blksize
            new_fd.close()

            # Setting the correct mode to the newly created file
            os.chmod(path_dest, int(mode, 8))

            # Done!
            self.write('\0')


    def source(self, path_source, addr_dest, path_dest):
        self.read(1)
        # if recursive mode is true
        # so we must test if we have a file
        # or directory
        # Note : i don't test the number of push
        # it's possible that the program crash ?
        # todo : test if the source is a file but
        # dest is directory

        mode_source = os.stat(path_source)[stat.ST_MODE]

        # if the first file is a directory
        if (stat.S_ISDIR(mode_source)):

            # create the directory
            self.create_directory_to_remote(path_source, path_dest)

            # for all contents into directory
            for files_sources in os.listdir(path_source):

                # save the names of file/directory and take the type
                files_source = path_source  + '/' + files_sources
                files_destination = path_dest + files_sources
                mode = os.stat(files_source)[stat.ST_MODE]

                # if the second file is a directory ?
                if (stat.S_ISDIR(mode)):
                    # so call source - recursively
                    #print files_source
                    self.source(files_source, addr_dest, files_destination)
                else:
                    # otherwise treat it like a file
                    self.treat_file(files_source, files_destination, addr_dest)

        # otherwise it's a simple file
        else:
            self.treat_file(path_source, path_dest, addr_dest)
        return

    def create_directory_to_remote(self, path_source, path_dest):
        # making a stat operation on the directory to transfert
        st_mode = os.stat(path_source)[stat.ST_MODE] & 07777
        st_size = 0 # it's a dierctory so the size = 0

        # looking for the name of directory
        # not the full path
        file_name = path_dest
        while file_name.__contains__('/'):
            file_name = file_name[(file_name.index('/')+1):]

        # write to remote the directory
        self.write("D%04o %d %s\n"%(st_mode, st_size, file_name))

        return

    def treat_file(self, files_source, files_dest, dummy_addr_dest):
        # Opening the file
        files = files_source
        file_fd = open(files, 'r')

        # Making a stat operation on the file to transfert
        file_st = os.stat(files)
        st_mode = file_st[stat.ST_MODE]&07777
        st_size = file_st[stat.ST_SIZE]

        # looking for the name of directory
        # not the full path
        file_name = files_dest
        while file_name.__contains__('/'):
            file_name = file_name[(file_name.index('/')+1):]

        # write to remote the file
        self.write("C%04o %ld %s\n" % (st_mode, st_size, file_name))
        data = self.read(1)

        # Fetching the blocksize
        blksize = os.stat(files).st_blksize
        i = 0
        while i < st_size:
            bytes_to_read = blksize
            if (i + blksize > st_size):
                bytes_to_read = st_size -i
                data = file_fd.read(bytes_to_read)
            if len(data) != bytes_to_read:
                raise Exception("Error While Reading Source File")
            self.write(data)
            i = i + blksize

        # Done !
        self.write('\0')
        file_fd.close()
        data = self.read(1)

    def file_from_remote_procedure(self):
        pass

    def write(self, data):
        self.msg("send %r", data)
        ret = self.sock.send(data)
        return ret

    def read(self, rsz = 1024):
        ret = self.sock.recv(rsz)
        self.msg("recv %r", ret)
        return ret

    def expect(self, ilist, timeout = None):
        indices = range(len(ilist))
        for i in indices:
            if not hasattr(ilist[i], "search"):
                ilist[i] = re.compile(ilist[i])

        if timeout is not None:
            start_time = time()

        while 1:
            data = self.read()
            for i in indices:
                m = ilist[i].search(data)
                if m:
                    e = m.end()
                    text = data[:e]
                    return (i, m, text)
            if timeout is not None:
                elapsed = time() -start_time
                if elapsed >= timeout:
                    break
                s_args = ([self.fileno()], [], [], timeout - elapsed)
                r, dummy_w, dummy_x = select.select(*s_args)
                if not r:
                    break
        return (-1, None, text)

    def close(self):
        """Close the connection."""
        if self.sock:
            self.sock.shutdown(socket.SHUT_RDWR)
            self.sock.close()

    def fileno(self):
        """Return the fileno() of the socket object used internally."""
        return self.sock.fileno()

    def msg(self, msg, *args):
        """Print a debug message, when the debug level is > 0.

        If extra arguments are present, they are substituted in the
        message using the standard string formatting operator.

        """
        if self.debuglevel > 0:
            print 'Rlogin(%s,%d):' % (self.host, self.port),
            if args:
                print msg % args
            else:
                print msg

    def set_debuglevel(self, debuglevel):
        """Set the debug level.

        The higher it is, the more debug output you get (on sys.stdout).

        """
        self.debuglevel = debuglevel

    def interact(self):
        """
        Use This function when you don't use a proxy."
        """
        import tty
        import termios
        try:
            file_fd = sys.stdin.fileno()
            old_settings = termios.tcgetattr(file_fd)
            tty.setraw(sys.stdin.fileno())
            while True:
                r_fds, dummy_w, dummy_x = select.select([self.sock, file_fd], [], [], 5)
                if self.sock in r_fds:
                    data = self.read()
                    if data == 'logout':
                        #Lost connection
                        self.write('\0')
                    sys.stdout.write(data)
                    sys.stdout.flush()
                if file_fd in r_fds:
                    self.write(sys.stdin.read(1))
        except Exception, exc:
            Logger().debug("Exception caught %s %s\n%s" % (
                type(exc), repr(exc), traceback.format_exc()
            ))
        finally:
            termios.tcsetattr(file_fd, termios.TCSADRAIN, old_settings)

def test():
    """Test program for rloginlib.

    Usage: python rloginlib.py [-d] [-f] [-r]... [host [port]]

    Default host is localhost; default port is 513.
    When specify : -f turns on file transfert.
    When -f is specify, -r turns on recursive transfert.

    """
    debuglevel = 0
    while sys.argv[1:] and sys.argv[1] == '-d':
        debuglevel = debuglevel+1
        del sys.argv[1]
    host = 'localhost'
    if sys.argv[1:]:
        host = sys.argv[1]
    port = 0
    if sys.argv[2:]:
        portstr = sys.argv[2]
        try:
            port = int(portstr)
        except ValueError:
            port = socket.getservbyname(portstr, 'tcp')

    rl = Rlogin()
    rl.set_debuglevel(debuglevel)
    rl.connect(host, port)
    rl.shell_identification_procedure()
    rl.interact()
    rl.close()

if __name__ == '__main__':
    test()
