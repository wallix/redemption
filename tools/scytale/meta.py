#python2.7

""" Encryption of trace files through scytale C library

"""

import ctypes
import os.path
from collections import deque
from scytale.decrypter import CryptoReader


class MwrmHeader:
    def __init__(self, version, has_checksum):
        self.version = version
        self.has_checksum = has_checksum

class MwrmLine:
    def __init__(self, filename, start_time, stop_time, with_hash, qhash, fhash, stat):
        self.filename = filename
        self.qhash = qhash
        self.fhash = fhash
        self.start_time = start_time
        self.stop_time = stop_time
        self.with_hash = with_hash
        self.stat = stat

class MwrmFileStat:
    def __init__(self, size, mode, uid, gid, dev, ino, mtime, ctime):
        self.st_size = size
        self.st_mode = mode
        self.st_uid = uid
        self.st_gid = gid
        self.st_dev = dev
        self.st_ino = ino
        self.st_mtime = mtime
        self.st_ctime = ctime

class MetaReader:
    """ Class used to read a mwrm file
    """

    def __init__(self, reader=None):
        self.reader = reader;
        self.header = None
        self.handle = None

    def __del__(self):
        self.close()

    def __enter__(self):
        return self.open()

    def __exit__(self, type, value, traceback):
        self.close()

    def open(self, reader=None):
        """ Explicit open of scytale Meta Reader """
        if reader is None:
            reader = self.reader
        self.reader = None

        if not isinstance(reader, CryptoReader):
            raise IOError("reader parameter is not CryptoReader")

        self.close()

        self.handle = lib.scytale_meta_reader_new(reader.handle)
        if self.handle is None:
            raise IOError("Bad allocation of MetaReader")

        return self

    def close(self):
        """ Explicit close of scytale Reader """
        if self.handle is not None:
            lib.scytale_meta_reader_delete(self.handle)
            self.handle = None
            self.header = None
        self.reader = None

    def _raise_if_error(self, res):
        if res != 0:
            err = lib.scytale_meta_reader_message(self.handle)
            raise IOError("MetaReader: " + err)

    def read_mwrm_header(self):
        """ Read the header of file
        """
        res = lib.scytale_meta_reader_read_header(self.handle)
        self._raise_if_error(res)
        header = lib.scytale_meta_reader_get_header(self.handle).contents
        self.header = MwrmHeader(
            version = int(header.version),
            has_checksum = bool(header.has_checksum))
        return self.header

    def iter_mwrm_line(self):
        """ Returns an iterator that will read the meta line
        """
        res = lib.scytale_meta_reader_read_line(self.handle)
        self._raise_if_error(res)

        while True:
            yield self._get_line()

            res = lib.scytale_meta_reader_read_line(self.handle)
            if res != 0:
                if lib.scytale_meta_reader_read_line_eof(self.handle):
                    return
                self._raise_if_error(res)

    def read_hash_file(self, version, has_checksum):
        """ Read a hash file
        """
        self.header = MwrmHeader(version = version, has_checksum = has_checksum)
        res = lib.scytale_meta_reader_read_hash(self.handle, version, has_checksum)
        self._raise_if_error(res)

        return self._get_line()

    def get_header(self):
        """ Return MwrmHeader object
        """
        return self.header

    def _get_line(self):
        mwrm_line = lib.scytale_meta_reader_get_line(self.handle).contents
        wrm_stat = None if self.header.version == 1 else MwrmFileStat(
            size = int(mwrm_line.size),
            mode = int(mwrm_line.mode),
            uid = int(mwrm_line.uid),
            gid = int(mwrm_line.gid),
            dev = int(mwrm_line.dev),
            ino = int(mwrm_line.ino),
            mtime = int(mwrm_line.mtime),
            ctime = int(mwrm_line.ctime))
        # copy string
        return MwrmLine(
            filename = ''.join(mwrm_line.filename),
            start_time = int(mwrm_line.start_time),
            stop_time = int(mwrm_line.stop_time),
            with_hash = bool(mwrm_line.with_hash),
            fhash = ''.join(mwrm_line.fhash),
            qhash = ''.join(mwrm_line.qhash),
            stat = wrm_stat)
