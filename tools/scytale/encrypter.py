import os.path
import re

_re_dirdate = re.compile('^\d\d\d\d-\d\d-\d\d$')

def normalize(filename):
    """ Helper function to convert unicode filenames to utf-8

        (do nothing if filename is not unicode,
        in that case it is assumed to be ready for filesystem)
    """
    return filename.encode('utf-8') if type(filename) is str else filename

class CryptoWriter:
    def __init__(self, do_encryption, do_checksum, filename,
                 derivator=None, checksums=None, random_type=None,
                 old_scheme=False, one_shot=False, hashpath=None,
                 hmac_key=None, trace_key_func=None, groupid=None):
        # print("CryptoWriter::__init__", id(self))
        self.closed = True
        if derivator is None:
            derivator = os.path.basename(filename)

        trace_key_func = GETTRACEKEY(trace_key_func) if trace_key_func else get_trace_key_func

        if random_type != "LCG":
            self.handle = lib.scytale_writer_new(do_encryption, do_checksum, normalize(derivator), hmac_key, trace_key_func, old_scheme, one_shot)
        else:
            self.handle = lib.scytale_writer_new_with_test_random(do_encryption, do_checksum, normalize(derivator), hmac_key, trace_key_func, old_scheme, one_shot)

        if not self.handle:
            raise IOError("Decrypter: scytale_reader_new error")

        self.do_checksum = do_checksum
        self.filename = normalize(filename)
        self.checksums = checksums
        self.hashpath = hashpath
        self.groupid = groupid

    def open(self, filename=None, hashpath=None, groupid=None):
        if self.closed:
            filename = self.filename if filename is None else normalize(filename)
            hashpath = self.hashpath if hashpath is None else normalize(hashpath)

            basename = os.path.basename(filename)
            if hashpath is None:
                dirname = os.path.dirname(filename)
                datedir = os.path.basename(dirname)

                if not _re_dirdate.match(datedir):
                    datedir = ''

                hashpath = os.path.join("/var/wab/hash", datedir)

            try:
                os.mkdir(dirname)
            except OSError:
                pass

            try:
                os.mkdir(hashpath)
            except OSError:
                pass

            if groupid is None:
                groupid = 0 if self.groupid is None else self.groupid

            res = lib.scytale_writer_open(self.handle, filename, os.path.join(hashpath, basename), groupid)
            if res != 0:
                err = lib.scytale_writer_error_message(self.handle)
                raise IOError(err)
            self.closed = False

    def __del__(self):
        # print("CryptoWriter::__del__", id(self))
        lib.scytale_writer_delete(self.handle)

    def __enter__(self):
        # print("CryptoWriter::__enter__", id(self))
        self.open()
        return self

    def close(self):
        if not self.closed:
            lib.scytale_writer_close(self.handle)
            self.closed = True
        if self.checksums is not None and self.do_checksum:
            self.checksums.append(lib.scytale_writer_qhashhex(self.handle))
            self.checksums.append(lib.scytale_writer_fhashhex(self.handle))

    def __exit__(self, type, value, traceback):
        # print("CryptoWriter::__exit__", id(self))
        self.close()

    def write(self, data):
        # print("CryptoWriter::write(%d)" % len(data), id(self))
        res = lib.scytale_writer_write(self.handle, data, len(data))
        if res < 0:
            err = lib.scytale_writer_error_message(self.handle)
            raise IOError('Encrypter' + err.decode('utf-8'))
        return res
