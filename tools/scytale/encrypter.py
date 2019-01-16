#!python2.7

class CryptoWriter(object):
    def __init__(self, encryption, checksum, filename, checksums = None, random_type = None, old_scheme=False, one_shot=False):
        print("CryptoWriter::__init__")
        if random_type == "LCG":
            self.handle = lib.scytale_writer_new_with_test_random(encryption, checksum, get_hmac_key_func, get_trace_key_func, old_scheme, one_shot)
        else:
            self.handle = lib.scytale_writer_new(encryption, checksum, get_hmac_key_func, get_trace_key_func, old_scheme, one_shot)
        self.filename = filename
        self.checksums = checksums

    def __del__(self):
        print("CryptoWriter::__del__")
        lib.scytale_writer_delete(self.handle)

    def __enter__(self):
        print("CryptoWriter::__enter__")
        res = lib.scytale_writer_open(self.handle, self.filename, 0)
        if res < 0:
            raise IOError()
        return self

    def __exit__(self, type, value, traceback):
        print("CryptoWriter::__exit__")
        lib.scytale_writer_close(self.handle)
        if self.checksums is not None:
            self.checksums.append(lib.scytale_writer_qhashhex(self.handle))
            self.checksums.append(lib.scytale_writer_fhashhex(self.handle))

    def write(self, data):
        print("CryptoWriter::write(%d)" % len(data))
        res = lib.scytale_writer_write(self.handle, data, len(data))
        if res < 0:
            raise IOError()
        return res

