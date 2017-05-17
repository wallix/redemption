#!python2.7

class CryptoWriter(object):
    def __init__(self, encryption, checksum, filename, checksums = None, random_type = None):
        print("CryptoWriter::__init__")
        if random_type == "LCG":
            self.handle = lib.redcryptofile_writer_new_with_test_random(encryption, checksum, get_hmac_key_func, get_trace_key_func)
        else:
            self.handle = lib.redcryptofile_writer_new(encryption, checksum, get_hmac_key_func, get_trace_key_func)
        self.filename = filename
        self.checksums = checksums

    def __del__(self):
        print("CryptoWriter::__del__")
        lib.redcryptofile_writer_delete(self.handle)

    def __enter__(self):
        print("CryptoWriter::__enter__")
        res = lib.redcryptofile_writer_open(self.handle, self.filename, 0)
        if res < 0:
            raise IOError()
        return self

    def __exit__(self, type, value, traceback):
        print("CryptoWriter::__exit__")
        lib.redcryptofile_writer_close(self.handle)
        if self.checksums is not None:
            self.checksums.append(lib.redcryptofile_writer_qhashhex(self.handle))
            self.checksums.append(lib.redcryptofile_writer_fhashhex(self.handle))

    def write(self, data):
        print("CryptoWriter::write(%d)" % len(data))
        res = lib.redcryptofile_writer_write(self.handle, data, len(data))
        if res < 0:
            raise IOError()
        return res

