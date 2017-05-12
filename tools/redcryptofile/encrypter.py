#!python2.7

class CryptoWriter(object):
    def __init__(self, encryption, checksum, filename):
        print("CryptoWriter::__init__")
        self.handle = lib.redcryptofile_writer_new(0, 0, get_hmac_key_func, get_trace_key_func)
        self.filename = filename
    
    def __del__(self):
        print("CryptoWriter::__del__")
        lib.redcryptofile_writer_delete(self.handle)
    
    def __enter__(self):
        print("CryptoWriter::__enter__")
        lib.redcryptofile_writer_open(self.handle, self.filename)
        return self

    def __exit__(self, type, value, traceback):
        print("CryptoWriter::__exit__")
        lib.redcryptofile_writer_close(self.handle)
        self.qhashhex = lib.redcryptofile_writer_qhashhex(self.handle)[:]
        self.fhashhex = lib.redcryptofile_writer_fhashhex(self.handle)[:]
        
    def write(self, data, length):
        print("CryptoWriter::write")
        return lib.redcryptofile_writer_write(self.handle, data, length)

