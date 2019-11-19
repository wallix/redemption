# Do not remove

import sys
import ctypes
import ctypes.util
import scytale.encrypter
import scytale.decrypter
import scytale.meta
import scytale.keys

# ./tools/cpp2ctypes.lua 'src/lib/scytale.hpp' 'lib.' '' 'get_hmac_key_prototype*=GETHMACKEY' 'get_trace_key_prototype*=GETTRACEKEY'

from ctypes import CFUNCTYPE, POINTER, c_char_p, c_int, c_uint64, c_ulong, c_void_p

pathlib = 'libscytale.so'

GETHMACKEY = CFUNCTYPE(c_int, c_void_p)
GETTRACEKEY = CFUNCTYPE(c_int, c_char_p, c_int, c_void_p, c_uint)

get_hmac_key_func = GETHMACKEY(keys.get_hmac_key)
get_trace_key_func = GETTRACEKEY(keys.get_trace_key)

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

lib = ctypes.CDLL(pathlib)


# char const * scytale_version();
lib.scytale_version.argtypes = []
lib.scytale_version.restype = c_char_p

# Writer
# @{
# ScytaleWriterHandle * scytale_writer_new(
#     int with_encryption, int with_checksum, const char * derivator,
#     get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn,
#     int old_scheme, int one_shot);
lib.scytale_writer_new.argtypes = [c_int, c_int, c_char_p, GETHMACKEY, GETTRACEKEY, c_int, c_int]
lib.scytale_writer_new.restype = c_void_p

# ScytaleWriterHandle * scytale_writer_new_with_test_random(
#     int with_encryption, int with_checksum, const char * derivator,
#     get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn,
#     int old_scheme, int one_shot);
lib.scytale_writer_new_with_test_random.argtypes = [c_int, c_int, c_char_p, GETHMACKEY, GETTRACEKEY, c_int, c_int]
lib.scytale_writer_new_with_test_random.restype = c_void_p

# char const * scytale_writer_get_error_message(ScytaleWriterHandle * handle);
lib.scytale_writer_get_error_message.argtypes = [c_void_p]
lib.scytale_writer_get_error_message.restype = c_char_p

# int scytale_writer_open(
#     ScytaleWriterHandle * handle,
#     char const * path, char const * hashpath, int groupid);
lib.scytale_writer_open.argtypes = [c_void_p, c_char_p, c_char_p, c_int]
lib.scytale_writer_open.restype = c_int

# int scytale_writer_write(
#     ScytaleWriterHandle * handle, uint8_t const * buffer, unsigned long len);
lib.scytale_writer_write.argtypes = [c_void_p, c_char_p, c_ulong]
lib.scytale_writer_write.restype = c_int

# \return HashHexArray
# char const * scytale_writer_get_qhashhex(ScytaleWriterHandle * handle);
lib.scytale_writer_get_qhashhex.argtypes = [c_void_p]
lib.scytale_writer_get_qhashhex.restype = c_char_p

# \return HashHexArray
# char const * scytale_writer_get_fhashhex(ScytaleWriterHandle * handle);
lib.scytale_writer_get_fhashhex.argtypes = [c_void_p]
lib.scytale_writer_get_fhashhex.restype = c_char_p

# int scytale_writer_close(ScytaleWriterHandle * handle);
lib.scytale_writer_close.argtypes = [c_void_p]
lib.scytale_writer_close.restype = c_int

# void scytale_writer_delete(ScytaleWriterHandle * handle);
lib.scytale_writer_delete.argtypes = [c_void_p]
lib.scytale_writer_delete.restype = None

# @}
# Reader
# @{
# ScytaleReaderHandle * scytale_reader_new(
#     const char * derivator,
#     get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn,
#     int old_scheme, int one_shot);
lib.scytale_reader_new.argtypes = [c_char_p, GETHMACKEY, GETTRACEKEY, c_int, c_int]
lib.scytale_reader_new.restype = c_void_p

# char const * scytale_reader_get_error_message(ScytaleReaderHandle * handle);
lib.scytale_reader_get_error_message.argtypes = [c_void_p]
lib.scytale_reader_get_error_message.restype = c_char_p

# int scytale_reader_open(
#     ScytaleReaderHandle * handle, char const * path, char const * derivator);
lib.scytale_reader_open.argtypes = [c_void_p, c_char_p, c_char_p]
lib.scytale_reader_open.restype = c_int

# enum class EncryptionSchemeTypeResult
# {
#     Error = -1,
#     NoEncrypted = 0,
#     OldScheme,
#     NewScheme,
# };
# \result EncryptionSchemeTypeResult
# int scytale_reader_open_with_auto_detect_encryption_scheme(
#     ScytaleReaderHandle * handle, char const * path, char const * derivator);
lib.scytale_reader_open_with_auto_detect_encryption_scheme.argtypes = [c_void_p, c_char_p, c_char_p]
lib.scytale_reader_open_with_auto_detect_encryption_scheme.restype = c_int

# < 0: error, 0: eof, >0: length read
# int scytale_reader_read(ScytaleReaderHandle * handle, uint8_t * buffer, unsigned long len);
lib.scytale_reader_read.argtypes = [c_void_p, c_char_p, c_ulong]
lib.scytale_reader_read.restype = c_int

# int scytale_reader_close(ScytaleReaderHandle * handle);
lib.scytale_reader_close.argtypes = [c_void_p]
lib.scytale_reader_close.restype = c_int

# void scytale_reader_delete(ScytaleReaderHandle * handle);
lib.scytale_reader_delete.argtypes = [c_void_p]
lib.scytale_reader_delete.restype = None

# int scytale_reader_fhash(ScytaleReaderHandle * handle, const char * file);
lib.scytale_reader_fhash.argtypes = [c_void_p, c_char_p]
lib.scytale_reader_fhash.restype = c_int

# int scytale_reader_qhash(ScytaleReaderHandle * handle, const char * file);
lib.scytale_reader_qhash.argtypes = [c_void_p, c_char_p]
lib.scytale_reader_qhash.restype = c_int

# \return HashHexArray
# const char * scytale_reader_get_qhashhex(ScytaleReaderHandle * handle);
lib.scytale_reader_get_qhashhex.argtypes = [c_void_p]
lib.scytale_reader_get_qhashhex.restype = c_char_p

# \return HashHexArray
# const char * scytale_reader_get_fhashhex(ScytaleReaderHandle * handle);
lib.scytale_reader_get_fhashhex.argtypes = [c_void_p]
lib.scytale_reader_get_fhashhex.restype = c_char_p

# @}
# Meta reader
# @{
# ScytaleMetaReaderHandle * scytale_meta_reader_new(ScytaleReaderHandle * reader);
lib.scytale_meta_reader_new.argtypes = [c_void_p]
lib.scytale_meta_reader_new.restype = c_void_p

# char const * scytale_meta_reader_get_error_message(ScytaleMetaReaderHandle * handle);
lib.scytale_meta_reader_get_error_message.argtypes = [c_void_p]
lib.scytale_meta_reader_get_error_message.restype = c_char_p

# int scytale_meta_reader_read_hash(
#     ScytaleMetaReaderHandle * handle, int version, int has_checksum);
lib.scytale_meta_reader_read_hash.argtypes = [c_void_p, c_int, c_int]
lib.scytale_meta_reader_read_hash.restype = c_int

# int scytale_meta_reader_read_header(ScytaleMetaReaderHandle * handle);
lib.scytale_meta_reader_read_header.argtypes = [c_void_p]
lib.scytale_meta_reader_read_header.restype = c_int

# int scytale_meta_reader_read_line(ScytaleMetaReaderHandle * handle);
lib.scytale_meta_reader_read_line.argtypes = [c_void_p]
lib.scytale_meta_reader_read_line.restype = c_int

# int scytale_meta_reader_read_line_eof(ScytaleMetaReaderHandle * handle);
lib.scytale_meta_reader_read_line_eof.argtypes = [c_void_p]
lib.scytale_meta_reader_read_line_eof.restype = c_int

# void scytale_meta_reader_delete(ScytaleMetaReaderHandle * handle);
lib.scytale_meta_reader_delete.argtypes = [c_void_p]
lib.scytale_meta_reader_delete.restype = None

# ScytaleMwrmHeader * scytale_meta_reader_get_header(ScytaleMetaReaderHandle * handle);
lib.scytale_meta_reader_get_header.argtypes = [c_void_p]
lib.scytale_meta_reader_get_header.restype = POINTER(CtypeMwrmHeader)

# ScytaleMwrmLine * scytale_meta_reader_get_line(ScytaleMetaReaderHandle * handle);
lib.scytale_meta_reader_get_line.argtypes = [c_void_p]
lib.scytale_meta_reader_get_line.restype = POINTER(CtypeMwrmLine)

# @}
# Key
# @{
# ScytaleKeyHandle * scytale_key_new(const char * masterkeyhex);
lib.scytale_key_new.argtypes = [c_char_p]
lib.scytale_key_new.restype = c_void_p

# const char * scytale_key_derivate(
#     ScytaleKeyHandle * handle, const uint8_t * derivator, unsigned long len);
lib.scytale_key_derivate.argtypes = [c_void_p, c_char_p, c_ulong]
lib.scytale_key_derivate.restype = c_char_p

# void scytale_key_delete(ScytaleKeyHandle * handle);
lib.scytale_key_delete.argtypes = [c_void_p]
lib.scytale_key_delete.restype = None

# \return HashHexArray
# const char * scytale_key_get_master(ScytaleKeyHandle * handle);
lib.scytale_key_get_master.argtypes = [c_void_p]
lib.scytale_key_get_master.restype = c_char_p

# \return HashHexArray
# const char * scytale_key_get_derivated(ScytaleKeyHandle * handle);
lib.scytale_key_get_derivated.argtypes = [c_void_p]
lib.scytale_key_get_derivated.restype = c_char_p

# @}
# Tfl
# @{
# \param test_random for reproductible test
# ScytaleFdxWriterHandle * scytale_fdx_writer_new(
#     char const * path, char const * hashpath, int groupid, char const * sid,
#     int with_encryption, int with_checksum, char const* master_derivator,
#     get_hmac_key_prototype * hmac_fn, get_trace_key_prototype * trace_fn,
#     int test_random);
lib.scytale_fdx_writer_new.argtypes = [c_char_p, c_char_p, c_int, c_char_p, c_int, c_int, c_char_p, GETHMACKEY, GETTRACEKEY, c_int]
lib.scytale_fdx_writer_new.restype = c_void_p

# ScytaleTflWriterHandler * scytale_fdx_writer_open_tfl(
#     ScytaleFdxWriterHandle * handle, char const * filename);
lib.scytale_fdx_writer_open_tfl.argtypes = [c_void_p, c_char_p]
lib.scytale_fdx_writer_open_tfl.restype = c_void_p

# int scytale_tfl_writer_write(
#     ScytaleTflWriterHandler * tfl, uint8_t const * buffer, unsigned long len);
lib.scytale_tfl_writer_write.argtypes = [c_void_p, c_char_p, c_ulong]
lib.scytale_tfl_writer_write.restype = c_int

# int scytale_fdx_writer_close_tfl(
#     ScytaleFdxWriterHandle * handle, ScytaleTflWriterHandler * tfl);
lib.scytale_fdx_writer_close_tfl.argtypes = [c_void_p, c_void_p]
lib.scytale_fdx_writer_close_tfl.restype = c_int

# int scytale_fdx_writer_cancel_tfl(
#     ScytaleFdxWriterHandle * handle, ScytaleTflWriterHandler * tfl);
lib.scytale_fdx_writer_cancel_tfl.argtypes = [c_void_p, c_void_p]
lib.scytale_fdx_writer_cancel_tfl.restype = c_int

# int scytale_fdx_writer_delete(ScytaleFdxWriterHandle * handle);
lib.scytale_fdx_writer_delete.argtypes = [c_void_p]
lib.scytale_fdx_writer_delete.restype = c_int

# char const * scytale_fdx_writer_get_error_message(
#     ScytaleFdxWriterHandle * handle);
lib.scytale_fdx_writer_get_error_message.argtypes = [c_void_p]
lib.scytale_fdx_writer_get_error_message.restype = c_char_p

# @}


scytale.encrypter.lib = lib
scytale.encrypter.get_hmac_key_func = get_hmac_key_func
scytale.encrypter.get_trace_key_func = get_trace_key_func

scytale.decrypter.lib = lib
scytale.decrypter.get_hmac_key_func = get_hmac_key_func
scytale.decrypter.get_trace_key_func = get_trace_key_func

scytale.meta.lib = lib
scytale.keys.lib = lib

def set_proxy(proxy):
    scytale.keys.proxy = proxy

def print_version():
    print("library version: %s" % lib.b2s(lib.scytale_version()))
    print("script version: %s" % version.SCYTALE_VERSION)


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

SCYTALE_VERSION = 2

CryptoReader = scytale.decrypter.CryptoReader
CryptoWriter = scytale.encrypter.CryptoWriter
MetaReader = scytale.meta.MetaReader
