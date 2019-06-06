""" Decryption of trace files through scytale C library

"""

import ctypes
import os.path

from collections import deque

def normalize(filename):
    """ Helper function to convert unicode filenames to utf-8

        (do nothing if filename is not unicode,
        in that case it is assumed to be ready for filesystem)
    """
    return filename.encode('utf-8') if type(filename) is unicode else filename


class CryptoReader:
    def __init__(self, filename, derivator=None,
                 old_scheme=False, one_shot=False, auto_scheme=False,
                 hmac_key_func=None, trace_key_func=None):
        print("CryptoReader::__init__")
        self.filename = normalize(filename)
        self.derivator = normalize(derivator) if derivator else self.filename

        master_derivator = self.derivator

        if not old_scheme and master_derivator[-4:] == '.wrm':
            master_derivator = "%s.mwrm" % master_derivator[:-11]

        hmac_key_func = GETHMACKEY(hmac_key_func) if hmac_key_func else get_hmac_key_func
        trace_key_func = GETTRACEKEY(trace_key_func) if trace_key_func else get_trace_key_func

        self.original_old_scheme = old_scheme
        self.old_scheme = old_scheme
        self.handle = lib.scytale_reader_new(os.path.basename(master_derivator),
                                             hmac_key_func, trace_key_func,
                                             [0, 1][old_scheme], [0, 1][one_shot])
        self.fhash = None
        self.qhash = False
        self.lines = deque()
        self.closed = True
        self.auto_scheme = auto_scheme

    def __del__(self):
        # print("CryptoReader::__del__")
        lib.scytale_reader_delete(self.handle)

    def __enter__(self):
        # print("CryptoReader::__enter__")
        return self.open()

    def __exit__(self, type, value, traceback):
        # print("CryptoReader::__exit__")
        self.close()

    def is_old_scheme(self):
        return self.old_scheme

    def open(self, filename=None, auto_scheme=False):
        """ Explicit open of scytale Reader """
        if filename is None:
            filename = self.filename
        else:
            filename = normalize(filename)

        self.old_scheme = self.original_old_scheme
        if auto_scheme or self.auto_scheme:
            res = lib.scytale_reader_open_with_auto_detect_encryption_scheme(
                self.handle, filename, os.path.basename(filename))
            self._raise_if_error(res)
            self.old_scheme = (res == 1)
        else:
            res = lib.scytale_reader_open(
                self.handle, filename, os.path.basename(filename))
            self._raise_if_error(res)

        self.closed = False
        return self

    def close(self):
        """ Explicit close of scytale Reader """
        if not self.closed:
            lib.scytale_reader_close(self.handle)
            self.closed = True

    def quick_hash(self):
        """ Return quick_hash value for a crypto reader object

            (file does not need to be opened))
        """
        #        print("CryptoReader::hash")
        if not self.qhash:
            res = lib.scytale_reader_qhash(self.handle, self.filename)
            self._raise_if_error(res)
            self.qhash = lib.scytale_reader_qhashhex(self.handle)
        return self.qhash

    def full_hash(self):
        """ Return full_hash value for a crypto reader object

            (file does not need to be opened))
        """
        if not self.fhash:
            res = lib.scytale_reader_fhash(self.handle, self.filename)
            self._raise_if_error(res)
            self.fhash = lib.scytale_reader_fhashhex(self.handle)
        return self.fhash

    def readline(self):
        """Read until a newline is found,

            returns line terminated by a newline except for last line
            which will miss the final '\n'
        """
        buf = ctypes.create_string_buffer(128)
        while len(self.lines) < 2:
            res = lib.scytale_reader_read(self.handle, buf, 128)
            self._raise_if_error(res)
            if res == 0:
                if len(self.lines) == 1:
                    item = self.lines.popleft()
                    return item
                return ""

            raw = buf.raw
            if len(buf) != res:
                raw = buf.raw[:res];
            if self.lines:
                self.lines.extend(raw.split('\n'))
                item0 = self.lines.popleft()
                item1 = self.lines.popleft()
                self.lines.appendleft(item0+item1)
            else:
                self.lines.extend(raw.split('\n'))

        # print("read->(%d,%d)" % (res, len(buf[:res])))
        item = self.lines.popleft()
        return item+'\n'

    def read(self, expected_len):
        """Read a block of data of expected_len size

        """
        buf = ctypes.create_string_buffer(expected_len)
        ok_len = 0
        while ok_len < expected_len:
            res = lib.scytale_reader_read(
                self.handle,
                (ctypes.c_char * (expected_len - ok_len)).from_buffer(buf, ok_len),
                expected_len - ok_len)
            self._raise_if_error(res)
            if res == 0:
                return buf[:ok_len]
            ok_len += res
        # print("read->(%d,%d)" % (res, len(buf[:res])))
        return buf

    def readbuffer(self, buf, expected_len):
        return lib.scytale_reader_read(self.handle, buf, expected_len)

    def readall(self, part_len=4096):
        """Returns an iterator that will read the entire file

           every block of data returned will be of at most part_len size
        """
        # print("CryptoReader::__iter__")
        buf = ctypes.create_string_buffer(part_len)

        while True:
            res = lib.scytale_reader_read(self.handle, buf, part_len)
            self._raise_if_error(res)
            if res == 0:
                return
            # print("read->(%d,%d)" % (res, len(buf[:res])))
            yield buf[:res]

    def _raise_if_error(self, res):
        if res < 0:
            err = lib.scytale_reader_error_message(self.handle)
            raise IOError("Decrypter: " + err)
