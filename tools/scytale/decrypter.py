import ctypes

class CryptoReader(object):
    def __init__(self, filename, derivator, old_scheme, one_shot):
        print("CryptoReader::__init__")
        self.handle = lib.scytale_reader_new(derivator, get_hmac_key_func, get_trace_key_func, old_scheme, one_shot)
        self.filename = filename
    
    def __del__(self):
        print("CryptoReader::__del__")
        lib.scytale_reader_delete(self.handle)
    
    def __enter__(self):
        print("CryptoReader::__enter__")
        res = lib.scytale_reader_open(self.handle, self.filename)
        if res < 0:
            raise IOError()
        return self

    def __exit__(self, type, value, traceback):
        print("CryptoReader::__exit__")
        lib.scytale_reader_close(self.handle)
#        if self.checksums is not None:
#            self.checksums.append(lib.scytale_writer_qhashhex(self.handle))
#            self.checksums.append(lib.scytale_writer_fhashhex(self.handle))

    def hash(self):
        if lib.scytale_reader_hash(self.handle, self.filename) < 0:
            raise IOError()
        qhash = lib.scytale_reader_qhashhex(self.handle)
        fhash = lib.scytale_reader_fhashhex(self.handle)
        return [qhash, fhash]

    def read(self, part_len = 4096):
        print("CryptoReader::read")
        buf = ctypes.create_string_buffer(part_len)
        
        while True:
            res = lib.scytale_reader_read(self.handle, buf, part_len)
            if res < 0:
                raise IOError()
            if res == 0:
                return
            print("read->(%d,%d)" % (res, len(buf[:res])))
            yield buf[:res]

