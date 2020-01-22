# ./tools/cpp2ctypes.lua 'src/lib/scytale.hpp' 'lib.' '' 'get_trace_key_prototype*=GETTRACEKEY'

from ctypes import CFUNCTYPE, POINTER, c_char_p, c_int, c_longlong, c_uint32, c_uint64, c_ulong, c_void_p, c_uint

pathlib = 'libscytale.so'

lib = ctypes.CDLL(pathlib)

GETTRACEKEY = CFUNCTYPE(c_int, c_void_p, c_int, c_void_p, c_uint)

get_trace_key_func = GETTRACEKEY(keys.get_trace_key)

# char const * scytale_version();
lib.scytale_version.argtypes = []
lib.scytale_version.restype = c_char_p

# Writer
# @{
# ScytaleWriterHandle * scytale_writer_new(
#     int with_encryption, int with_checksum, const char * master_derivator,
#     uint8_t const * hmac_key, get_trace_key_prototype * trace_fn,
#     int old_scheme, int one_shot);
lib.scytale_writer_new.argtypes = [c_int, c_int, c_char_p, POINTER(c_char), GETTRACEKEY, c_int, c_int]
lib.scytale_writer_new.restype = c_void_p

# ScytaleWriterHandle * scytale_writer_new_with_test_random(
#     int with_encryption, int with_checksum, const char * master_derivator,
#     uint8_t const * hmac_key, get_trace_key_prototype * trace_fn,
#     int old_scheme, int one_shot);
lib.scytale_writer_new_with_test_random.argtypes = [c_int, c_int, c_char_p, POINTER(c_char), GETTRACEKEY, c_int, c_int]
lib.scytale_writer_new_with_test_random.restype = c_void_p

# char const * scytale_writer_get_error_message(ScytaleWriterHandle * handle);
lib.scytale_writer_get_error_message.argtypes = [c_void_p]
lib.scytale_writer_get_error_message.restype = c_char_p

# int scytale_writer_open(
#     ScytaleWriterHandle * handle,
#     char const * record_path, char const * hash_path, int groupid);
lib.scytale_writer_open.argtypes = [c_void_p, c_char_p, c_char_p, c_int]
lib.scytale_writer_open.restype = c_int

# int scytale_writer_write(
#     ScytaleWriterHandle * handle, uint8_t const * buffer, unsigned long len);
lib.scytale_writer_write.argtypes = [c_void_p, POINTER(c_char), c_ulong]
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
#     const char * master_derivator,
#     uint8_t const * hmac_key, get_trace_key_prototype * trace_fn,
#     int old_scheme, int one_shot);
lib.scytale_reader_new.argtypes = [c_char_p, POINTER(c_char), GETTRACEKEY, c_int, c_int]
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
# long long scytale_reader_read(ScytaleReaderHandle * handle,
#     uint8_t * buffer, unsigned long len);
lib.scytale_reader_read.argtypes = [c_void_p, POINTER(c_char), c_ulong]
lib.scytale_reader_read.restype = c_longlong

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

class CType_ScytaleMwrmHeader(ctypes.Structure):
    _fields_ = [
        ("version", c_int),
        ("has_checksum", c_int),
    ]

class CType_ScytaleMwrmLine(ctypes.Structure):
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

# ScytaleMwrmHeader * scytale_meta_reader_get_header(ScytaleMetaReaderHandle * handle);
lib.scytale_meta_reader_get_header.argtypes = [c_void_p]
lib.scytale_meta_reader_get_header.restype = POINTER(CType_ScytaleMwrmHeader)

# ScytaleMwrmLine * scytale_meta_reader_get_line(ScytaleMetaReaderHandle * handle);
lib.scytale_meta_reader_get_line.argtypes = [c_void_p]
lib.scytale_meta_reader_get_line.restype = POINTER(CType_ScytaleMwrmLine)
# @}

# Fdx / Tfl
# @{
# ScytaleFdxWriterHandle * scytale_fdx_writer_new(
#     int with_encryption, int with_checksum, char const* master_derivator,
#     uint8_t const * hmac_key, get_trace_key_prototype * trace_fn,
#     char const * record_path, char const * hash_path, char const * fdx_file_base,
#     int groupid, char const * sid);
lib.scytale_fdx_writer_new.argtypes = [c_int, c_int, c_char_p, POINTER(c_char), GETTRACEKEY, c_char_p, c_char_p, c_char_p, c_int, c_char_p]
lib.scytale_fdx_writer_new.restype = c_void_p

# ScytaleFdxWriterHandle * scytale_fdx_writer_new_with_test_random(
#     int with_encryption, int with_checksum, char const* master_derivator,
#     uint8_t const * hmac_key, get_trace_key_prototype * trace_fn,
#     char const * record_path, char const * hash_path, char const * fdx_file_base,
#     int groupid, char const * sid);
lib.scytale_fdx_writer_new_with_test_random.argtypes = [c_int, c_int, c_char_p, POINTER(c_char), GETTRACEKEY, c_char_p, c_char_p, c_char_p, c_int, c_char_p]
lib.scytale_fdx_writer_new_with_test_random.restype = c_void_p

# char const * scytale_fdx_get_path(ScytaleFdxWriterHandle * handle);
lib.scytale_fdx_get_path.argtypes = [c_void_p]
lib.scytale_fdx_get_path.restype = c_char_p

# \param direction  Mwrm3::Direction
# ScytaleTflWriterHandler * scytale_fdx_writer_open_tfl(
#     ScytaleFdxWriterHandle * handle, char const * filename, int direction);
lib.scytale_fdx_writer_open_tfl.argtypes = [c_void_p, c_char_p, c_int]
lib.scytale_fdx_writer_open_tfl.restype = c_void_p

# int scytale_tfl_writer_write(
#     ScytaleTflWriterHandler * handle, uint8_t const * buffer, unsigned long len);
lib.scytale_tfl_writer_write.argtypes = [c_void_p, POINTER(c_char), c_ulong]
lib.scytale_tfl_writer_write.restype = c_int

# len should be 32
# \param transfered_status  Mwrm3::TransferedStatus
# int scytale_tfl_writer_close(
#     ScytaleTflWriterHandler * handle, int transfered_status,
#     uint8_t const* sig, unsigned long len);
lib.scytale_tfl_writer_close.argtypes = [c_void_p, c_int, POINTER(c_char), c_ulong]
lib.scytale_tfl_writer_close.restype = c_int

# int scytale_tfl_writer_cancel(ScytaleTflWriterHandler * handle);
lib.scytale_tfl_writer_cancel.argtypes = [c_void_p]
lib.scytale_tfl_writer_cancel.restype = c_int

# \return HashHexArray
# char const * scytale_fdx_writer_get_qhashhex(ScytaleFdxWriterHandle * handle);
lib.scytale_fdx_writer_get_qhashhex.argtypes = [c_void_p]
lib.scytale_fdx_writer_get_qhashhex.restype = c_char_p

# \return HashHexArray
# char const * scytale_fdx_writer_get_fhashhex(ScytaleFdxWriterHandle * handle);
lib.scytale_fdx_writer_get_fhashhex.argtypes = [c_void_p]
lib.scytale_fdx_writer_get_fhashhex.restype = c_char_p

# \return 1 if not open, 0 is ok, -1 if error
# int scytale_fdx_writer_close(ScytaleFdxWriterHandle * handle);
lib.scytale_fdx_writer_close.argtypes = [c_void_p]
lib.scytale_fdx_writer_close.restype = c_int

# int scytale_fdx_writer_delete(ScytaleFdxWriterHandle * handle);
lib.scytale_fdx_writer_delete.argtypes = [c_void_p]
lib.scytale_fdx_writer_delete.restype = c_int

# char const * scytale_fdx_writer_get_error_message(
#     ScytaleFdxWriterHandle * handle);
lib.scytale_fdx_writer_get_error_message.argtypes = [c_void_p]
lib.scytale_fdx_writer_get_error_message.restype = c_char_p
# @}

# Mwrm3 Reader
# @{
# ScytaleMwrm3ReaderHandle * scytale_mwrm3_reader_new(ScytaleReaderHandle * reader);
lib.scytale_mwrm3_reader_new.argtypes = [c_void_p]
lib.scytale_mwrm3_reader_new.restype = c_void_p

# 'u': uint64_t  'i': int64_t  's': {char*, uint32_t}  'B': {uint8_t*, uint32_t}
class CType_ScytaleMwrm3ReaderData(ctypes.Structure):
    _fields_ = [
        ("type", c_uint32),
        ("fmt", c_char_p),
        ("data", c_void_p),
    ]

# return nullptr if error or truncated file (truncated if reader is No Error + eof())
# ScytaleMwrm3ReaderData const* scytale_mwrm3_reader_read_next(
#     ScytaleMwrm3ReaderHandle * handle);
lib.scytale_mwrm3_reader_read_next.argtypes = [c_void_p]
lib.scytale_mwrm3_reader_read_next.restype = POINTER(CType_ScytaleMwrm3ReaderData)

# char const * scytale_mwrm3_reader_get_error_message(ScytaleMwrm3ReaderHandle * handle);
lib.scytale_mwrm3_reader_get_error_message.argtypes = [c_void_p]
lib.scytale_mwrm3_reader_get_error_message.restype = c_char_p

# int scytale_mwrm3_reader_delete(ScytaleMwrm3ReaderHandle * handle);
lib.scytale_mwrm3_reader_delete.argtypes = [c_void_p]
lib.scytale_mwrm3_reader_delete.restype = c_int
# @}


def set_proxy(proxy):
    scytale.keys.proxy = proxy

def print_version():
    print("library version: %s" % lib.b2s(lib.scytale_version()))
    print("script version: %s" % version.SCYTALE_VERSION)


scytale.encrypter.lib = lib
scytale.encrypter.GETTRACEKEY = GETTRACEKEY
scytale.encrypter.get_trace_key_func = get_trace_key_func

scytale.decrypter.lib = lib
scytale.decrypter.GETTRACEKEY = GETTRACEKEY
scytale.decrypter.get_trace_key_func = get_trace_key_func

scytale.meta.lib = lib
scytale.keys.lib = lib

SCYTALE_VERSION = 2

CryptoReader = scytale.decrypter.CryptoReader
CryptoWriter = scytale.encrypter.CryptoWriter
MetaReader = scytale.meta.MetaReader
