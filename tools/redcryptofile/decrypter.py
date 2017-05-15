import ctypes

class CryptoReader(object):
    def __init__(self, filename):
        print("CryptoReader::__init__")
        self.handle = lib.redcryptofile_reader_new(get_hmac_key_func, get_trace_key_func)
        self.filename = filename
    
    def __del__(self):
        print("CryptoReader::__del__")
        lib.redcryptofile_reader_delete(self.handle)
    
    def __enter__(self):
        print("CryptoReader::__enter__")
        res = lib.redcryptofile_reader_open(self.handle, self.filename)
        if res < 0:
            raise IOError()
        return self

    def __exit__(self, type, value, traceback):
        print("CryptoReader::__exit__")
        lib.redcryptofile_reader_close(self.handle)
#        if self.checksums is not None:
#            self.checksums.append(lib.redcryptofile_writer_qhashhex(self.handle))
#            self.checksums.append(lib.redcryptofile_writer_fhashhex(self.handle))
        
    def read(self, part_len = 4096):
        print("CryptoReader::read")
        buf = ctypes.create_string_buffer(part_len)
        
        while True:
            res = lib.redcryptofile_reader_read(self.handle, buf, part_len)
            if res < 0:
                raise IOError()
            if res == 0:
                return
            yield buf[:res]

