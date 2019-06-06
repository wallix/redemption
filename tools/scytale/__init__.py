# Do not remove

import sys
import ctypes
import ctypes.util
import scytale.encrypter
import scytale.decrypter
import scytale.meta
import scytale.keys

from ctypes import CFUNCTYPE, c_ulong, c_ulonglong, c_int, c_char_p, c_uint, c_uint64, c_void_p, POINTER, Structure

pathlib = 'libscytale.so'

GETHMACKEY = CFUNCTYPE(c_int, c_void_p)
GETTRACEKEY = CFUNCTYPE(c_int, c_char_p, c_int, c_void_p, c_uint)

get_hmac_key_func = GETHMACKEY(keys.get_hmac_key)
get_trace_key_func = GETTRACEKEY(keys.get_trace_key)

lib = ctypes.CDLL(pathlib)

# char const * scytale_version()
lib.scytale_version.argtypes = []
lib.scytale_version.restype = c_char_p

# RedCryptoWriterHandle * scytale_writer_new(
#    int with_encryption, int with_checksum, get_hmac_key_prototype * hmac_fn,
#    get_trace_key_prototype * trace_fn, int old_scheme, int one_shot)
lib.scytale_writer_new.argtypes = [c_int, c_int, c_char_p, GETHMACKEY, GETTRACEKEY, c_int, c_int]
lib.scytale_writer_new.restype = c_void_p

# RedCryptoWriterHandle * scytale_writer_new_with_test_random(
#    int with_encryption, int with_checksum, get_hmac_key_prototype * hmac_fn,
#    get_trace_key_prototype * trace_fn, int old_scheme, int one_shot)
lib.scytale_writer_new_with_test_random.argtypes = [c_int, c_int, c_char_p, GETHMACKEY, GETTRACEKEY, c_int, c_int]
lib.scytale_writer_new_with_test_random.restype = c_void_p

# char const * scytale_writer_error_message(RedCryptoWriterHandle * handle);
lib.scytale_writer_error_message.argtypes = [c_void_p]
lib.scytale_writer_error_message.restype = c_char_p


# int scytale_writer_open(
#   RedCryptoWriterHandle * handle, const char * path, const char * hash_path, int mode)
lib.scytale_writer_open.argtypes = [c_void_p, c_char_p, c_char_p, c_int]
lib.scytale_writer_open.restype = c_int

# int scytale_writer_write(
#   RedCryptoWriterHandle * handle, uint8_t const * buffer, unsigned long len);
lib.scytale_writer_write.argtypes = [c_void_p, c_char_p, c_ulong]
lib.scytale_writer_write.restype = c_int

# int scytale_writer_close(RedCryptoWriterHandle * handle);
lib.scytale_writer_close.argtypes = [c_void_p]
lib.scytale_writer_close.restype = c_int

# void scytale_writer_delete(RedCryptoWriterHandle * handle);
lib.scytale_writer_delete.argtypes = [c_void_p]
lib.scytale_writer_delete.restype = None

# const char * scytale_writer_qhashhex(RedCryptoWriterHandle * handle);
lib.scytale_writer_qhashhex.argtypes = [c_void_p]
lib.scytale_writer_qhashhex.restype = c_char_p

# const char * scytale_writer_fhashhex(RedCryptoWriterHandle * handle);
lib.scytale_writer_fhashhex.argtypes = [c_void_p]
lib.scytale_writer_fhashhex.restype = c_char_p


# RedCryptoReaderHandle * scytale_reader_new(
#   get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn,
#   int old_scheme, int one_shot)
lib.scytale_reader_new.argtypes = [c_char_p, GETHMACKEY, GETTRACEKEY, c_int, c_int]
lib.scytale_reader_new.restype = c_void_p

# char const * scytale_reader_error_message(RedCryptoReaderHandle * handle);
lib.scytale_reader_error_message.argtypes = [c_void_p]
lib.scytale_reader_error_message.restype = c_char_p

# int scytale_reader_open(
#   RedCryptoReaderHandle * handle, const char * path, const char * derivator)
lib.scytale_reader_open.argtypes = [c_void_p, c_char_p, c_char_p]
lib.scytale_reader_open.restype = c_int

# enum class EncryptionSchemeTypeResult
# {
#     Error = -1,
#     NoEncrypted = 0,
#     OldScheme,
#     NewScheme,
# };
# int scytale_reader_open_with_auto_detect_encryption_scheme(
#   RedCryptoReaderHandle * handle, char const * path, char const * derivator);
lib.scytale_reader_open_with_auto_detect_encryption_scheme.argtypes = [c_void_p, c_char_p, c_char_p]
lib.scytale_reader_open_with_auto_detect_encryption_scheme.restype = c_int

# int scytale_reader_read(
#   RedCryptoReaderHandle * handle, uint8_t const * buffer, unsigned long len);
lib.scytale_reader_read.argtypes = [c_void_p, c_char_p, c_ulong]
lib.scytale_reader_read.restype = c_int

# int scytale_reader_close(RedCryptoReaderHandle * handle);
lib.scytale_reader_close.argtypes = [c_void_p]
lib.scytale_reader_close.restype = c_int

# void scytale_reader_delete(RedCryptoReaderHandle * handle);
lib.scytale_reader_delete.argtypes = [c_void_p]
lib.scytale_reader_delete.restype = None

# int scytale_reader_qhash(RedCryptoReaderHandle * handle, const char * file);
lib.scytale_reader_qhash.argtypes = [c_void_p, c_char_p]
lib.scytale_reader_qhash.restype = c_int

# int scytale_reader_fhash(RedCryptoReaderHandle * handle, const char * file);
lib.scytale_reader_fhash.argtypes = [c_void_p, c_char_p]
lib.scytale_reader_fhash.restype = c_int

# const char * scytale_qhashhex(RedCryptoReaderHandle * handle);
lib.scytale_reader_qhashhex.argtypes = [c_void_p]
lib.scytale_reader_qhashhex.restype = c_char_p

# const char * scytale_fhashhex(RedCryptoReaderHandle * handle);
lib.scytale_reader_fhashhex.argtypes = [c_void_p]
lib.scytale_reader_fhashhex.restype = c_char_p


# RedCryptoMetaReaderHandle * scytale_meta_reader_new(RedCryptoReaderHandle * reader);
lib.scytale_meta_reader_new.argtypes = [c_void_p]
lib.scytale_meta_reader_new.restype = c_void_p

# char const * scytale_meta_reader_message(RedCryptoMetaReaderHandle * handle);
lib.scytale_meta_reader_message.argtypes = [c_void_p]
lib.scytale_meta_reader_message.restype = c_char_p

# int scytale_meta_reader_read_hash(
#   RedCryptoMetaReaderHandle * handle, int version, int has_checksum);
lib.scytale_meta_reader_read_hash.argtypes = [c_void_p, c_int, c_int]
lib.scytale_meta_reader_read_hash.restype = c_int

# int scytale_meta_reader_read_header(RedCryptoMetaReaderHandle * handle);
lib.scytale_meta_reader_read_header.argtypes = [c_void_p]
lib.scytale_meta_reader_read_header.restype = c_int

# int scytale_meta_reader_read_line(RedCryptoMetaReaderHandle * handle);
lib.scytale_meta_reader_read_line.argtypes = [c_void_p]
lib.scytale_meta_reader_read_line.restype = c_int

# int scytale_meta_reader_read_line_eof(RedCryptoMetaReaderHandle * handle);
lib.scytale_meta_reader_read_line_eof.argtypes = [c_void_p]
lib.scytale_meta_reader_read_line_eof.restype = c_int

# void scytale_meta_reader_delete(RedCryptoMetaReaderHandle * handle);
lib.scytale_meta_reader_delete.argtypes = [c_void_p]
lib.scytale_meta_reader_delete.restype = None

class CtypeMwrmHeader(Structure):
    _fields_ = [
        ("version", c_int),
        ("has_checksum", c_int),
    ]

class CtypeMwrmLine(Structure):
    _fields_ = [
        ("filename", c_char_p),
        ("size", c_uint64),
        ("mode", c_uint64),
        ("uid", c_uint64),
        ("gid", c_uint64),
        ("dev", c_uint64),
        ("ino", c_uint64),
        ("mtime", c_uint64),
        ("ctime", c_uint64),
        ("start_time", c_uint64),
        ("stop_time", c_uint64),
        ("with_hash", c_int),
        ("qhash", c_char_p),
        ("fhash", c_char_p),
    ]

# RedCryptoMwrmHeader * scytale_meta_reader_get_header(RedCryptoMetaReaderHandle * handle);
lib.scytale_meta_reader_get_header.argtypes = [c_void_p]
lib.scytale_meta_reader_get_header.restype = POINTER(CtypeMwrmHeader)

# RedCryptoMwrmLine * scytale_meta_reader_get_line(RedCryptoMetaReaderHandle * handle);
lib.scytale_meta_reader_get_line.argtypes = [c_void_p]
lib.scytale_meta_reader_get_line.restype = POINTER(CtypeMwrmLine)


scytale.encrypter.lib = lib
scytale.encrypter.GETHMACKEY = GETHMACKEY
scytale.encrypter.GETTRACEKEY = GETTRACEKEY
scytale.encrypter.get_hmac_key_func = get_hmac_key_func
scytale.encrypter.get_trace_key_func = get_trace_key_func

scytale.decrypter.lib = lib
scytale.decrypter.GETHMACKEY = GETHMACKEY
scytale.decrypter.GETTRACEKEY = GETTRACEKEY
scytale.decrypter.get_hmac_key_func = get_hmac_key_func
scytale.decrypter.get_trace_key_func = get_trace_key_func

scytale.meta.lib = lib

def set_proxy(proxy):
    scytale.keys.proxy = proxy

def print_version():
    print("library version: %s" % lib.scytale_version())
    print("script version: %s" % version.SCYTALE_VERSION)

CryptoReader = scytale.decrypter.CryptoReader
CryptoWriter = scytale.encrypter.CryptoWriter
MetaReader = scytale.meta.MetaReader
Verifier = scytale.verifier.Verifier
