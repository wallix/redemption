#!/usr/bin/python -O
# -*- coding: utf-8 -*-
#
# Copyright (c) 2011 WALLIX, SARL. All rights reserved.
#
# Licensed computer software. Property of WALLIX.
# Product Name : Sphinx
# Author(s)    : Tristan de Cacqueray <tdc@wallix.com>
# Id           : $Id$
# URL          : $URL$

"""
Crypto trace interface
"""

import struct
import redcryptofile

class TraceIntegrityError(Exception):
    pass

REDCRYPTOFILE_MAGIC = 0x4D464357

class CryptoTrace:
    """ New crypto trace format writer """
    def __init__(self, path, mode):
        self._fd = redcryptofile.open(path, mode)

        if self._fd == -1:
            raise TraceIntegrityError("Invalid CryptoTrace")
    def flush(self):
        flush_ret = redcryptofile.flush(self._fd)
        if flush_ret == -1:
            raise TraceIntegrityError("Flush failed")
        return flush_ret
    def write(self, data):
#        print "CryptoTrace.write"
        write_ret = redcryptofile.write(self._fd, data)
#        print "CryptoTrace.write ->", write_ret
        if write_ret == -1:
            raise TraceIntegrityError("Write failed")
        return write_ret
    def read(self, sz):
        read_ret = redcryptofile.read(self._fd, sz)
        if read_ret == -1:
            raise TraceIntegrityError("Read failed")
        return read_ret
    def close(self):
        close_ret = redcryptofile.close(self._fd)
        if close_ret == -1:
            raise TraceIntegrityError("Close failed")
        return close_ret
    def __del__(self):
        try:
            self.close()
        except Exception, e:
            pass
#            import traceback
#            print "<<<%s>>>" % traceback.format_exc(e)

class Trace:
    """ Trace reader """
    def __init__(self, path):
        with open(path, 'r') as f:
            encrypted = struct.unpack('I', f.read(4))[0] == REDCRYPTOFILE_MAGIC

        if encrypted:
            self.handle = CryptoTrace(path, 'r')
        else:
            self.handle = open(path, 'r')

    def read(self, sz):
        return self.handle.read(sz)

    def readall(self):
        buf = []
        while True:
            data = self.handle.read(4096)
            if data == '':
                break
            buf.append(data)
        return ''.join(buf)

    def __del__(self):
        if self.handle:
            if type(self.handle) == file:
                self.handle.close()
            else:
                self.handle.__del__()



