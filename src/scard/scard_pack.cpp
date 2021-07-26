#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "core/error.hpp"
#include "scard/scard_pack_common.hpp"
#include "scard/scard_pack_type_native_convertible.hpp"
#include "scard/scard_pack_type_internal.hpp"
#include "scard/scard_pack_type_native.hpp"
#include "scard/scard_pack_utils.hpp"
#include "scard/scard_utils.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/array_view.hpp"

#include "scard/scard_pack.hpp"


///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//////// EstablishContext  ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_establish_context::scard_pack_call_establish_context(
    SCARD_PACK_DWORD dwScope)
    :
    _dwScope(dwScope)
{
}

std::size_t scard_pack_call_establish_context::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;

    // dwSCope
    stream.out_uint32_le(_dwScope);
    n_packed = sizeof(uint32_t);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_establish_context::io_control_code() const
{
    return scard_pack_ioctl_establish_context;
}

std::size_t scard_pack_call_establish_context::packed_size() const
{
    std::size_t size;
    
    size  = sizeof(_dwScope);
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_establish_context::min_packed_size() const
{
    std::size_t size;
    
    size  = sizeof(_dwScope);
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_establish_context::log(int level) const
{
    LOG(level, "EstablishContext_Call {");
    LOG(level, "\tdwScope: %s (0x%08" PRIX32 ")", scope_to_string(_dwScope),
        static_cast<uint32_t>(_dwScope));
    LOG(level, "}");
}


scard_pack_return_establish_context::scard_pack_return_establish_context()
    :
    _ReturnCode(0)
{
}

SCARD_PACK_NATIVE_LONG scard_pack_return_establish_context::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

SCARD_PACK_NATIVE_CONTEXT scard_pack_return_establish_context::hContext() const
{
    SCARD_PACK_NATIVE_CONTEXT hContext(scard_pack_context::none_value);
    
    _Context.to_native(hContext);

    return hContext;
}

std::size_t scard_pack_return_establish_context::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;
    pointer_index_type pointer_index(0);

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    if (has_more_payload_bytes_compat(stream, _ReturnCode))
    {
        // Context
        n_unpacked += _Context.unpack(stream, pointer_index);

        // Context (deferred)
        n_unpacked += _Context.unpack_deferred(stream, pointer_index);
    }

    // padding
    n_unpacked += unpad(stream, n_unpacked);

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

std::size_t scard_pack_return_establish_context::packed_size() const
{
    std::size_t size;
    
    size = (
        sizeof(_ReturnCode) +
        (_ReturnCode ? 0 : _Context.packed_size())
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_return_establish_context::log(int level) const
{
    LOG(level, "EstablishContext_Return {");
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////
//////// ReleaseContext ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_release_context::scard_pack_call_release_context(
    SCARD_PACK_NATIVE_CONTEXT hContext)
    :
    _Context(hContext)
{
}

std::size_t scard_pack_call_release_context::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;
    pointer_index_type pointer_index(0);

    // Context
    n_packed =  _Context.pack(stream, pointer_index);
    n_packed += _Context.pack_deferred(stream, pointer_index);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_release_context::io_control_code() const
{
    return scard_pack_ioctl_release_context;
}

std::size_t scard_pack_call_release_context::packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.packed_size()
    );
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_release_context::min_packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.min_packed_size()
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_release_context::log(int level) const
{
    LOG(level, "ReleaseContext_Call {");
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "}");
}


scard_pack_return_release_context::scard_pack_return_release_context()
    :
    _ReturnCode(0)
{
}

SCARD_PACK_NATIVE_LONG scard_pack_return_release_context::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

std::size_t scard_pack_return_release_context::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    // padding
    n_unpacked += unpad(stream, n_unpacked);

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

std::size_t scard_pack_return_release_context::packed_size() const
{
    std::size_t size;
    
    size = (
        sizeof(_ReturnCode) +
        get_padding_size(sizeof(_ReturnCode))
    );

    return size;
}

void scard_pack_return_release_context::log(int level) const
{
    LOG(level, "ReleaseContext_Return {");
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////
//////// IsValidContext ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_is_valid_context::scard_pack_call_is_valid_context(
    SCARD_PACK_NATIVE_CONTEXT hContext)
    :
    _Context(hContext)
{
}

std::size_t scard_pack_call_is_valid_context::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;
    pointer_index_type pointer_index(0);

    // Context
    n_packed =  _Context.pack(stream, pointer_index);
    n_packed += _Context.pack_deferred(stream, pointer_index);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_is_valid_context::io_control_code() const
{
    return scard_pack_ioctl_is_valid_context;
}

std::size_t scard_pack_call_is_valid_context::packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.packed_size()
    );
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_is_valid_context::min_packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.min_packed_size()
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_is_valid_context::log(int level) const
{
    LOG(level, "IsValidContext_Call {");
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "}");
}


scard_pack_return_is_valid_context::scard_pack_return_is_valid_context()
    :
    _ReturnCode(0)
{
}

SCARD_PACK_NATIVE_LONG scard_pack_return_is_valid_context::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

std::size_t scard_pack_return_is_valid_context::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    // padding
    n_unpacked += unpad(stream, n_unpacked);

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

std::size_t scard_pack_return_is_valid_context::packed_size() const
{
    std::size_t size;
    
    size = (
        sizeof(_ReturnCode) +
        get_padding_size(sizeof(_ReturnCode))
    );

    return size;
}

void scard_pack_return_is_valid_context::log(int level) const
{
    LOG(level, "IsValidContext_Return {");
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////
//////// Connect //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_connect::scard_pack_call_connect(SCARD_PACK_NATIVE_CONTEXT hContext,
    const char *szReader, SCARD_PACK_DWORD dwShareMode,
    SCARD_PACK_DWORD dwPreferredProtocols,
    bool unicode)
    :
    _szReader(szReader),
    _Context(hContext),
    _dwShareMode(dwShareMode),
    _dwPreferredProtocols(dwPreferredProtocols),
    _unicode(unicode)
{
}

std::size_t scard_pack_call_connect::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;
    pointer_index_type pointer_index(0);

    // szReader
    n_packed = _szReader.pack_pointer(stream, pointer_index);

    // Context
    n_packed += _Context.pack(stream, pointer_index);

    // dwShareMode
    stream.out_uint32_le(_dwShareMode);
    n_packed += sizeof(uint32_t);

    // dwPreferredProtocols
    stream.out_uint32_le(_dwPreferredProtocols);
    n_packed += sizeof(uint32_t);

    // szReader (deferred)
    n_packed += _szReader.pack_elements(stream);

    // Context (deferred)
    n_packed += _Context.pack_deferred(stream, pointer_index);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_connect::io_control_code() const
{
    return (_unicode ? scard_pack_ioctl_connect_w : scard_pack_ioctl_connect_a);
}

std::size_t scard_pack_call_connect::packed_size() const
{
    std::size_t size;
    
    size = (
        _szReader.packed_size() +
        _Context.packed_size() +
        sizeof(_dwShareMode) +
        sizeof(_dwPreferredProtocols)
    );
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_connect::min_packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.min_packed_size()
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_connect::log(int level) const
{
    LOG(level, "Connect_Call [unicode=%s] {", (_unicode ? "true" : "false"));
    LOG(level, "\tszReader: [%s]", bytes_to_hex_string(_szReader.data()).c_str());
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "\tdwShareMode: %s (0x%08" PRIX32 ")", share_mode_to_string(_dwShareMode),
        static_cast<uint32_t>(_dwShareMode));
    LOG(level, "\tdwPreferredProtocols: %s (0x%08" PRIX32 ")", protocols_to_string(_dwPreferredProtocols),
        static_cast<uint32_t>(_dwPreferredProtocols));
    LOG(level, "}");
}


scard_pack_return_connect::scard_pack_return_connect()
    :
    _ReturnCode(0)
{
}

SCARD_PACK_NATIVE_LONG scard_pack_return_connect::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

SCARD_PACK_NATIVE_CONTEXT scard_pack_return_connect::hContext() const
{
    return _Context.to_native();
}

SCARD_PACK_NATIVE_HANDLE scard_pack_return_connect::hCard() const
{
    return _Card.to_native();
}

SCARD_PACK_DWORD scard_pack_return_connect::dwActiveProtocol() const
{
    return _dwActiveProtocol;
}

std::size_t scard_pack_return_connect::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;
    pointer_index_type pointer_index(0);

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    if (has_more_payload_bytes_compat(stream, _ReturnCode))
    {
        // Context
        n_unpacked += _Context.unpack(stream, pointer_index);

        // Card
        n_unpacked += _Card.unpack(stream, pointer_index);

        // dwActiveProtocol
        _dwActiveProtocol = stream.in_uint32_le();
        n_unpacked += sizeof(uint32_t);

        // Context (deferred)
        n_unpacked += _Context.unpack_deferred(stream, pointer_index);

        // Card (deferred)
        n_unpacked += _Card.unpack_deferred(stream, pointer_index);
    }
    
    // padding
    n_unpacked += unpad(stream, n_unpacked);

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

void scard_pack_return_connect::log(int level) const
{
    LOG(level, "Connect_Return {");
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "\tCard: [%s]", bytes_to_hex_string(bytes_view(_Card.pbHandle(),
        _Card.cbHandle())).c_str());
    LOG(level, "\tdwActiveProtocol: %s (0x%08" PRIX32 ")", protocols_to_string(_dwActiveProtocol),
        static_cast<uint32_t>(_dwActiveProtocol));
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////
//////// Reconnect ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_reconnect::scard_pack_call_reconnect(
    SCARD_PACK_NATIVE_CONTEXT hContext, SCARD_PACK_NATIVE_HANDLE hCard,
    SCARD_PACK_DWORD dwShareMode, SCARD_PACK_DWORD dwPreferredProtocols,
    SCARD_PACK_DWORD dwInitialization)
    :
    _Context(hContext),
    _Card(hCard),
    _dwShareMode(dwShareMode),
    _dwPreferredProtocols(dwPreferredProtocols),
    _dwInitialization(dwInitialization)
{
}

std::size_t scard_pack_call_reconnect::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;
    pointer_index_type pointer_index(0);

    // Context
    n_packed = _Context.pack(stream, pointer_index);

    // Card
    n_packed += _Card.pack(stream, pointer_index);

    // dwShareMode
    stream.out_uint32_le(_dwShareMode);
    n_packed += sizeof(uint32_t);

    // dwPreferredProtocols
    stream.out_uint32_le(_dwPreferredProtocols);
    n_packed += sizeof(uint32_t);

    // dwInitialization
    stream.out_uint32_le(_dwInitialization);
    n_packed += sizeof(uint32_t);

    // Context (deferred)
    n_packed += _Context.pack_deferred(stream, pointer_index);

    // Card (deferred)
    n_packed += _Card.pack_deferred(stream, pointer_index);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_reconnect::io_control_code() const
{
    return scard_pack_ioctl_reconnect;
}

std::size_t scard_pack_call_reconnect::packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.packed_size() +
        _Card.packed_size() +
        sizeof(_dwShareMode) +
        sizeof(_dwPreferredProtocols) +
        sizeof(_dwInitialization)
    );
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_reconnect::min_packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.min_packed_size() +
        _Card.min_packed_size()
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_reconnect::log(int level) const
{
    LOG(level, "Reconnect_Call {");
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "\tCard: [%s]", bytes_to_hex_string(bytes_view(_Card.pbHandle(),
        _Card.cbHandle())).c_str());
    LOG(level, "\tdwShareMode: %s (0x%08" PRIX32 ")", share_mode_to_string(_dwShareMode),
        static_cast<uint32_t>(_dwShareMode));
    LOG(level, "\tdwPreferredProtocols: %s (0x%08" PRIX32 ")", protocols_to_string(_dwPreferredProtocols),
        static_cast<uint32_t>(_dwPreferredProtocols));
    LOG(level, "\tdwInitialization: 0x%08" PRIX32,
        static_cast<uint32_t>(_dwInitialization));
    LOG(level, "}");
}


scard_pack_return_reconnect::scard_pack_return_reconnect()
    :
    _ReturnCode(0)
{
}

SCARD_PACK_NATIVE_LONG scard_pack_return_reconnect::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

SCARD_PACK_DWORD scard_pack_return_reconnect::dwActiveProtocol() const
{
    return _dwActiveProtocol;
}

std::size_t scard_pack_return_reconnect::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    if (has_more_payload_bytes_compat(stream, _ReturnCode))
    {
        // dwActiveProtocol
        _dwActiveProtocol = stream.in_uint32_le();
        n_unpacked += sizeof(uint32_t);
    }

    // padding
    n_unpacked += unpad(stream, n_unpacked);

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

void scard_pack_return_reconnect::log(int level) const
{
    LOG(level, "Reconnect_Return {");
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "\tdwActiveProtocol:  %s (0x%08" PRIX32 ")", protocols_to_string(_dwActiveProtocol),
        static_cast<uint32_t>(_dwActiveProtocol));
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////
//////// Disconnect ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_disconnect::scard_pack_call_disconnect(
    SCARD_PACK_NATIVE_CONTEXT hContext, SCARD_PACK_NATIVE_HANDLE hCard,
    SCARD_PACK_DWORD dwDisposition)
    :
    _Context(hContext),
    _Card(hCard),
    _dwDisposition(dwDisposition)
{
}

std::size_t scard_pack_call_disconnect::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;
    pointer_index_type pointer_index(0);

    // Context
    n_packed = _Context.pack(stream, pointer_index);

    // Card
    n_packed += _Card.pack(stream, pointer_index);

    // dwDisposition
    stream.out_uint32_le(_dwDisposition);
    n_packed += sizeof(uint32_t);

    // Context (deferred)
    n_packed += _Context.pack_deferred(stream, pointer_index);

    // Card (deferred)
    n_packed += _Card.pack_deferred(stream, pointer_index);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_disconnect::io_control_code() const
{
    return scard_pack_ioctl_disconnect;
}

std::size_t scard_pack_call_disconnect::packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.packed_size() +
        _Card.packed_size() +
        sizeof(_dwDisposition)
    );
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_disconnect::min_packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.min_packed_size() +
        _Card.min_packed_size()
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_disconnect::log(int level) const
{
    LOG(level, "Disconnect_Call {");
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "\tCard: [%s]", bytes_to_hex_string(bytes_view(_Card.pbHandle(),
        _Card.cbHandle())).c_str());
    LOG(level, "\tdwDisposition: %s (0x%08" PRIX32 ")", disposition_to_string(_dwDisposition),
        static_cast<uint32_t>(_dwDisposition));
    LOG(level, "}");
}


scard_pack_return_disconnect::scard_pack_return_disconnect()
    :
    _ReturnCode(0)
{
}

SCARD_PACK_NATIVE_LONG scard_pack_return_disconnect::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

std::size_t scard_pack_return_disconnect::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    // padding
    n_unpacked += unpad(stream, n_unpacked);

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

void scard_pack_return_disconnect::log(int level) const
{
    LOG(level, "Disconnect_Return {");
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////
//////// BeginTransaction /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_begin_transaction::scard_pack_call_begin_transaction(
    SCARD_PACK_NATIVE_CONTEXT hContext, SCARD_PACK_NATIVE_HANDLE hCard)
    :
    _Context(hContext),
    _Card(hCard)
{
}

std::size_t scard_pack_call_begin_transaction::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;
    pointer_index_type pointer_index(0);

    // Context
    n_packed = _Context.pack(stream, pointer_index);

    // Card
    n_packed += _Card.pack(stream, pointer_index);

    // dwDisposition (present but ignored)
    stream.out_uint32_le(0);
    n_packed += sizeof(uint32_t);

    // Context (deferred)
    n_packed += _Context.pack_deferred(stream, pointer_index);

    // Card (deferred)
    n_packed += _Card.pack_deferred(stream, pointer_index);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_begin_transaction::io_control_code() const
{
    return scard_pack_ioctl_begin_transaction;
}

std::size_t scard_pack_call_begin_transaction::packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.packed_size() +
        _Card.packed_size() +
        sizeof(uint32_t)
    );
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_begin_transaction::min_packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.min_packed_size() +
        _Card.min_packed_size()
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_begin_transaction::log(int level) const
{
    LOG(level, "BeginTransaction_Call {");
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "\tCard: [%s]", bytes_to_hex_string(bytes_view(_Card.pbHandle(),
        _Card.cbHandle())).c_str());
    LOG(level, "}");
}


scard_pack_return_begin_transaction::scard_pack_return_begin_transaction()
    :
    _ReturnCode(0)
{
}

SCARD_PACK_NATIVE_LONG scard_pack_return_begin_transaction::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

std::size_t scard_pack_return_begin_transaction::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    // padding
    n_unpacked += unpad(stream, n_unpacked);

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

void scard_pack_return_begin_transaction::log(int level) const
{
    LOG(level, "BeginTransaction_Return {");
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////
//////// EndTransaction ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_end_transaction::scard_pack_call_end_transaction(
    SCARD_PACK_NATIVE_CONTEXT hContext, SCARD_PACK_NATIVE_HANDLE hCard,
    SCARD_PACK_DWORD dwDisposition)
    :
    _Context(hContext),
    _Card(hCard),
    _dwDisposition(dwDisposition)
{
}

std::size_t scard_pack_call_end_transaction::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;
    pointer_index_type pointer_index(0);

    // Context
    n_packed = _Context.pack(stream, pointer_index);

    // Card
    n_packed += _Card.pack(stream, pointer_index);

    // dwDisposition
    stream.out_uint32_le(_dwDisposition);
    n_packed += sizeof(uint32_t);

    // Context (deferred)
    n_packed += _Context.pack_deferred(stream, pointer_index);

    // Card (deferred)
    n_packed += _Card.pack_deferred(stream, pointer_index);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_end_transaction::io_control_code() const
{
    return scard_pack_ioctl_end_transaction;
}

std::size_t scard_pack_call_end_transaction::packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.packed_size() +
        _Card.packed_size() +
        sizeof(_dwDisposition)
    );
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_end_transaction::min_packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.min_packed_size() +
        _Card.min_packed_size()
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_end_transaction::log(int level) const
{
    LOG(level, "EndTransaction_Call {");
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "\tCard: [%s]", bytes_to_hex_string(bytes_view(_Card.pbHandle(),
        _Card.cbHandle())).c_str());
    LOG(level, "\tdwDisposition: %" PRIu32, _dwDisposition);
    LOG(level, "}");
}


scard_pack_return_end_transaction::scard_pack_return_end_transaction()
    :
    _ReturnCode(0)
{
}

SCARD_PACK_NATIVE_LONG scard_pack_return_end_transaction::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

std::size_t scard_pack_return_end_transaction::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    // padding
    n_unpacked += unpad(stream, n_unpacked);

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

void scard_pack_return_end_transaction::log(int level) const
{
    LOG(level, "EndTransaction_Return {");
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////
//////// Status ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_status::scard_pack_call_status(
    SCARD_PACK_NATIVE_CONTEXT hContext, SCARD_PACK_NATIVE_HANDLE hCard,
    const char *mszReaderNames, unsigned long *pcchReader,
    bool unicode)
    :
    _Context(hContext),
    _Card(hCard),
    _fmszReaderNamesIsNULL(mszReaderNames ? 0 : 1),
    _cchReaderLen(pcchReader ? *pcchReader : 0),
    _cbAtrLen(0),
    _unicode(unicode)
{
}

std::size_t scard_pack_call_status::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;
    pointer_index_type pointer_index(0);

    // Context
    n_packed = _Context.pack(stream, pointer_index);

    // Card
    n_packed += _Card.pack(stream, pointer_index);

    // fmszReaderNamesIsNULL
    stream.out_sint32_le(_fmszReaderNamesIsNULL);
    n_packed += sizeof(int32_t);

    // cchReaderLen
    stream.out_uint32_le(_cchReaderLen);
    n_packed += sizeof(uint32_t);

    // cbAtrLen
    stream.out_uint32_le(_cbAtrLen);
    n_packed += sizeof(uint32_t);

    // Context (deferred)
    n_packed += _Context.pack_deferred(stream, pointer_index);

    // Card (deferred)
    n_packed += _Card.pack_deferred(stream, pointer_index);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_status::io_control_code() const
{
    return (_unicode ? scard_pack_ioctl_status_w : scard_pack_ioctl_status_a);
}

std::size_t scard_pack_call_status::packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.packed_size() +
        _Card.packed_size() +
        sizeof(_fmszReaderNamesIsNULL) +
        sizeof(_cchReaderLen) +
        sizeof(_cbAtrLen)
    );
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_status::min_packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.min_packed_size() +
        _Card.min_packed_size() +
        sizeof(_fmszReaderNamesIsNULL) +
        sizeof(_cchReaderLen) +
        sizeof(_cbAtrLen)
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_status::log(int level) const
{
    LOG(level, "Status_Call [unicode=%s] {", (_unicode ? "true" : "false"));
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "\tCard: [%s]", bytes_to_hex_string(bytes_view(_Card.pbHandle(),
        _Card.cbHandle())).c_str());
    LOG(level, "\tfmszReaderNamesIsNULL: %" PRId32, _fmszReaderNamesIsNULL);
    LOG(level, "\tcchReaderLen: %" PRIu32, _cchReaderLen);
    LOG(level, "\tcbAtrLen: %" PRIu32, _cbAtrLen);
    LOG(level, "}");
}


scard_pack_return_status::scard_pack_return_status()
    :
    _ReturnCode(0),
    _cBytes(0),
    _dwState(0),
    _dwProtocol(0),
    _cbAtrLen(0)
{
    _pbAtr.fill(0);
}

SCARD_PACK_NATIVE_LONG scard_pack_return_status::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

SCARD_PACK_DWORD scard_pack_return_status::cBytes() const
{
    return _cBytes;
}

bytes_view scard_pack_return_status::mszReaderNames() const
{
    return _mszReaderNames.data();
}

SCARD_PACK_DWORD scard_pack_return_status::dwState() const
{
    return _dwState;
}

SCARD_PACK_DWORD scard_pack_return_status::dwProtocol() const
{
    return _dwProtocol;
}

const uint8_t * scard_pack_return_status::pbAtr() const
{
    return _pbAtr.data();
}

SCARD_PACK_DWORD scard_pack_return_status::cbAtrLen() const
{
    return _cbAtrLen;
}

std::size_t scard_pack_return_status::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;
    pointer_index_type pointer_index(0);

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    if (has_more_payload_bytes_compat(stream, _ReturnCode))
    {
        // cBytes
        n_unpacked += _mszReaderNames.unpack_size(stream, _cBytes);

        // mszReaderNames
        n_unpacked += _mszReaderNames.unpack_pointer(stream, pointer_index);

        // dwState
        _dwState = stream.in_uint32_le();
        n_unpacked += sizeof(uint32_t);

        // dwProtocol
        _dwProtocol = stream.in_uint32_le();
        n_unpacked += sizeof(uint32_t);

        // pbAtr
        stream.in_copy_bytes(_pbAtr.data(), _pbAtr.size());
        n_unpacked += _pbAtr.size();

        // cbAtrLen
        _cbAtrLen = stream.in_uint32_le();
        n_unpacked += sizeof(uint32_t);

        SCARD_CHECK_PREDICATE(_cbAtrLen <= _pbAtr.max_size(),
            ERR_SCARD);

        // mszReaderNames (deferred)
        n_unpacked += _mszReaderNames.unpack_elements(stream, _cBytes);
    }

    // padding
    n_unpacked += unpad(stream, n_unpacked);

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

void scard_pack_return_status::log(int level) const
{
    LOG(level, "Status_Return {");
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "\tcBytes: %" PRIu32, _cBytes);
    LOG(level, "\tmszReaderNames: %s", bytes_to_hex_string(_mszReaderNames.data()).c_str());
    LOG(level, "\tdwState: %s (0x%08" PRIX32 ")", card_state_to_string(_dwState),
        static_cast<uint32_t>(_dwState));
    LOG(level, "\tdwProtocol: %s (0x%08" PRIX32 ")", protocols_to_string(_dwProtocol),
        static_cast<uint32_t>(_dwProtocol));
    LOG(level, "\tpbAtr: %s", bytes_to_hex_string(_pbAtr).c_str());
    LOG(level, "\tcbAtrLen: %" PRIu32, _cbAtrLen);
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////
//////// GetStatusChange //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_get_status_change::scard_pack_call_get_status_change(
    SCARD_PACK_NATIVE_CONTEXT hContext, SCARD_PACK_DWORD dwTimeout,
    SCARD_READERSTATE *rgReaderStates, SCARD_PACK_DWORD cReaders,
    bool unicode)
    :
    _Context(hContext),
    _dwTimeout(dwTimeout),
    _cReaders(cReaders),
    _rgReaderStates(rgReaderStates, cReaders),
    _unicode(unicode)
{
}

std::size_t scard_pack_call_get_status_change::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;
    pointer_index_type pointer_index(0);

    // Context
    n_packed = _Context.pack(stream, pointer_index);

    // dwTimeout
    stream.out_uint32_le(_dwTimeout);
    n_packed += sizeof(uint32_t);

    // cReaders
    n_packed += _rgReaderStates.pack_size(stream, _cReaders);

    // rgReaderStates
    n_packed += _rgReaderStates.pack_pointer(stream, pointer_index);

    // Context (deferred)
    n_packed += _Context.pack_deferred(stream, pointer_index);

    // rgReaderStates (deferred)
    n_packed += _rgReaderStates.pack_elements(stream, _cReaders, pointer_index);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_get_status_change::io_control_code() const
{
    return (_unicode ? scard_pack_ioctl_get_status_change_w
        : scard_pack_ioctl_get_status_change_a);
}

std::size_t scard_pack_call_get_status_change::packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.packed_size() +
        sizeof(_dwTimeout) +
        sizeof(_cReaders) +
        _rgReaderStates.packed_size()
    );
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_get_status_change::min_packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.min_packed_size() +
        sizeof(_dwTimeout) +
        sizeof(_cReaders) +
        _rgReaderStates.min_packed_size()
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_get_status_change::log(int level) const
{
    LOG(level, "GetStatusChange_Call [unicode=%s] {", (_unicode ? "true" : "false"));
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "\tdwTimeout: %" PRIu32, _dwTimeout);
    LOG(level, "\tcReaders: %" PRIu32, _cReaders);
    LOG(level, "\trgReaderStates: [");
    for (const auto &reader_state : _rgReaderStates)
    {
        LOG(level, "\t\t{");
        LOG(level, "\t\t\tdwCurrentState: %s (0x%08" PRIX32 ")",
            reader_state_to_string(reader_state.dwCurrentState()),
            reader_state.dwCurrentState());
        LOG(level, "\t\t\tdwEventState: %s (0x%08" PRIX32 ")",
            reader_state_to_string(reader_state.dwEventState()),
            reader_state.dwEventState());
        LOG(level, "\t\t\tcbAtr: %" PRIu32, reader_state.cbAtr());
        LOG(level, "\t\t\trgbAtr: [%s]", bytes_to_hex_string(reader_state.rgbAtr()).c_str());
        LOG(level, "\t\t}");
    }
    LOG(level, "\t]");
    LOG(level, "}");
}


scard_pack_return_get_status_change::scard_pack_return_get_status_change()
    :
    _ReturnCode(0),
    _cReaders(0)
{
}

SCARD_PACK_NATIVE_LONG scard_pack_return_get_status_change::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

SCARD_PACK_DWORD scard_pack_return_get_status_change::cReaders() const
{
    return _cReaders;
}

array_view<scard_pack_reader_state>
scard_pack_return_get_status_change::rgReaderStates() const
{
    return _rgReaderStates.data();
}

const scard_pack_reader_state &
scard_pack_return_get_status_change::rgReaderStates(std::size_t index) const
{
    return _rgReaderStates.at(index);
}

std::size_t scard_pack_return_get_status_change::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;
    pointer_index_type pointer_index(0);

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    // cReaders
    n_unpacked += _rgReaderStates.unpack_size(stream, _cReaders);

    // rgReaderStates
    n_unpacked += _rgReaderStates.unpack_pointer(stream, pointer_index);

    // rgReaderStates (deferred)
    n_unpacked += _rgReaderStates.unpack_elements(stream, _cReaders, pointer_index);

    // FIXME Fix for probable bug in FreeRDP (version 2.3.0 at the time of writing)
    //
    // FreeRDP appears to be aligning (induly) the whole array of ReaderState_Return
    // structures to the 4-byte boundary. HOWEVER, when calculating the padding size
    // it seems to consider the number of items in the array instead of the total size
    // occupied by the items (which would have made more sense).
    //
    // Here we fix it by discarding any spurious padding data encountered:
    {
        const auto padding_size = get_padding_size(n_unpacked);
        const auto remaining_size = stream.in_remain();

        if (remaining_size > padding_size)
        {
            // calculate size of spurious padding data as FreeRDP does
            const auto data_size = _rgReaderStates.size();
            const auto spurious_padding_data_size = ((data_size % 4) ? (4 - (data_size % 4)) : 0);
            
            // skip spurious padding data
            stream.in_skip_bytes(spurious_padding_data_size);
            n_unpacked += spurious_padding_data_size;
        }

        // padding
        n_unpacked += unpad(stream, n_unpacked);
    }
    // This should be the correct code:
    //{
    //// padding
    //n_unpacked += unpad(stream, n_unpacked);
    //}

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

void scard_pack_return_get_status_change::log(int level) const
{
    LOG(level, "GetStatusChange_Return {");
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "\tcReaders: %" PRIu32, _cReaders);
    LOG(level, "\trgReaderStates: [");
    for (const auto &reader_state : _rgReaderStates)
    {
        LOG(level, "\t\t{");
        LOG(level, "\t\t\tdwCurrentState: %s (0x%08" PRIX32 ")",
            reader_state_to_string(reader_state.dwCurrentState()),
            reader_state.dwCurrentState());
        LOG(level, "\t\t\tdwEventState: %s (0x%08" PRIX32 ")",
            reader_state_to_string(reader_state.dwEventState()),
            reader_state.dwEventState());
        LOG(level, "\t\t\tcbAtr: %" PRIu32, reader_state.cbAtr());
        LOG(level, "\t\t\trgbAtr: [%s]", bytes_to_hex_string(reader_state.rgbAtr()).c_str());
        LOG(level, "\t\t}");

    }
    LOG(level, "\t]");
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////
//////// Control //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_control::scard_pack_call_control(SCARD_PACK_NATIVE_CONTEXT hContext,
    SCARD_PACK_NATIVE_HANDLE hCard, SCARD_PACK_DWORD dwControlCode,
    const void *lpInBuffer, unsigned long cbInBufferSize,
    const void *lpOutBuffer, unsigned long cbOutBufferSize)
    :
    _Context(hContext),
    _Card(hCard),
    _dwControlCode(dwControlCode),
    _cbInBufferSize(cbInBufferSize),
    _pvInBuffer(reinterpret_cast<const uint8_t *>(lpInBuffer), cbInBufferSize),
    _fpvOutBufferIsNULL(lpOutBuffer ? 0 : 1),
    _cbOutBufferSize(lpOutBuffer ? cbOutBufferSize : 0)
{
}

std::size_t scard_pack_call_control::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;
    pointer_index_type pointer_index(0);

    // Context
    n_packed = _Context.pack(stream, pointer_index);

    // Card
    n_packed += _Card.pack(stream, pointer_index);

    // dwControlCode
    stream.out_uint32_le(_dwControlCode);
    n_packed += sizeof(uint32_t);

    // cbInBufferSize
    n_packed += _pvInBuffer.pack_size(stream, _cbInBufferSize);

    // pvInBuffer
    n_packed += _pvInBuffer.pack_pointer(stream, pointer_index);

    // fpvOutBufferIsNULL
    stream.out_uint32_le(_fpvOutBufferIsNULL);
    n_packed += sizeof(uint32_t);

    // cbOutBufferSize
    stream.out_uint32_le(_cbOutBufferSize);
    n_packed += sizeof(uint32_t);

    // Context (deferred)
    n_packed += _Context.pack_deferred(stream, pointer_index);

    // Card (deferred)
    n_packed += _Card.pack_deferred(stream, pointer_index);

    // pvInBuffer (deferred)
    n_packed += _pvInBuffer.pack_elements(stream, _cbInBufferSize);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_control::io_control_code() const
{
    return scard_pack_ioctl_control;
}

std::size_t scard_pack_call_control::packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.packed_size() +
        _Card.packed_size() +
        sizeof(_dwControlCode) +
        sizeof(_cbInBufferSize) +
        _pvInBuffer.packed_size() +
        sizeof(_fpvOutBufferIsNULL) +
        sizeof(_cbOutBufferSize)
    );
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_control::min_packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.min_packed_size() +
        _Card.min_packed_size() +
        sizeof(_dwControlCode) +
        sizeof(_cbInBufferSize) +
        _pvInBuffer.min_packed_size() +
        sizeof(_fpvOutBufferIsNULL) +
        sizeof(_cbOutBufferSize)
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_control::log(int level) const
{
    LOG(level, "Control_Call");
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "\tCard: [%s]", bytes_to_hex_string(bytes_view(_Card.pbHandle(),
        _Card.cbHandle())).c_str());
    LOG(level, "\tdwControlCode: 0x%08" PRIX32, static_cast<uint32_t>(_dwControlCode));
    LOG(level, "\tcbInBufferSize: %" PRIu32, _cbInBufferSize);
    LOG(level, "\tpvInBuffer: %s", bytes_to_hex_string(_pvInBuffer.data()).c_str());
    LOG(level, "\tfpvOutBufferIsNULL: %" PRId32, _fpvOutBufferIsNULL);
    LOG(level, "\tcbOutBufferSize: %" PRIu32, _cbOutBufferSize);
    LOG(level, "}");
}


scard_pack_return_control::scard_pack_return_control()
    :
    _ReturnCode(0),
    _cbOutBufferSize(0)
{
}

SCARD_PACK_NATIVE_LONG scard_pack_return_control::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

SCARD_PACK_DWORD scard_pack_return_control::cbOutBufferSize() const
{
    return _cbOutBufferSize;
}

const uint8_t * scard_pack_return_control::pvOutBuffer() const
{
    return _pvOutBuffer.ptr();
}

std::size_t scard_pack_return_control::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;
    pointer_index_type pointer_index(0);

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    if (has_more_payload_bytes_compat(stream, _ReturnCode))
    {
        // cbOutBufferSize
        n_unpacked += _pvOutBuffer.unpack_size(stream, _cbOutBufferSize);

        // pvOutBuffer
        n_unpacked += _pvOutBuffer.unpack_pointer(stream, pointer_index);

        // pvOutBuffer (deferred)
        n_unpacked += _pvOutBuffer.unpack_elements(stream, _cbOutBufferSize);
    }

    // padding
    n_unpacked += unpad(stream, n_unpacked);

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

void scard_pack_return_control::log(int level) const
{
    LOG(level, "Control_Return {");
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "\tcbOutBufferSize: %" PRIu32, _cbOutBufferSize);
    LOG(level, "\tpvOutBuffer: %s", bytes_to_hex_string(_pvOutBuffer.data()).c_str());
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////
//////// Transmit /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_transmit::scard_pack_call_transmit(
    SCARD_PACK_NATIVE_CONTEXT hContext, SCARD_PACK_NATIVE_HANDLE hCard,
    const SCARD_PACK_NATIVE_PCI_HEADER *pioSendPci, const uint8_t *pbSendBuffer,
    unsigned long cbSendLength, SCARD_PACK_NATIVE_PCI_HEADER *pioRecvPci,
    const uint8_t *pbRecvBuffer, const unsigned long *pcbRecvLength)
    :
    _Context(hContext),
    _Card(hCard),
    _ioSendPci(pioSendPci),
    _cbSendLength(cbSendLength),
    _pbSendBuffer(pbSendBuffer, cbSendLength),
    _fpbRecvBufferIsNULL(pbRecvBuffer ? 0 : 1),
    _cbRecvLength((pbRecvBuffer && pcbRecvLength) ? *pcbRecvLength : 0)
{
    if (pioRecvPci)
    {
        _pioRecvPci.assign(1, scard_pack_native_pci(pioRecvPci));
    }
}

std::size_t scard_pack_call_transmit::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;
    scard_pack_pointer_index_type pointer_index(0);

    // Context
    n_packed = _Context.pack(stream, pointer_index);

    // Card
    n_packed += _Card.pack(stream, pointer_index);

    // ioSendPci
    n_packed += _ioSendPci.pack(stream, pointer_index);

    // cbSendLength
    n_packed += _pbSendBuffer.pack_size(stream, _cbSendLength);

    // pbSendBuffer
    n_packed += _pbSendBuffer.pack_pointer(stream, pointer_index);

    // pioRecvPci
    n_packed += _pioRecvPci.pack_pointer(stream, pointer_index);

    // fpbRecvBufferIsNULL
    stream.out_uint32_le(_fpbRecvBufferIsNULL);
    n_packed += sizeof(uint32_t);

    // cbRecvLength
    stream.out_uint32_le(_cbRecvLength);
    n_packed += sizeof(uint32_t);

    // Context (deferred)
    n_packed += _Context.pack_deferred(stream, pointer_index);

    // Card (deferred)
    n_packed += _Card.pack_deferred(stream, pointer_index);

    // ioSendPci (deferred)
    n_packed += _ioSendPci.pack_deferred(stream, pointer_index);

    // pbSendBuffer (deferred)
    n_packed += _pbSendBuffer.pack_elements(stream, _cbSendLength);

    // pioRecvPci (deferred)
    n_packed += _pioRecvPci.pack_elements(stream, pointer_index);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_transmit::io_control_code() const
{
    return scard_pack_ioctl_transmit;
}

std::size_t scard_pack_call_transmit::packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.packed_size() +
        _Card.packed_size() +
        _ioSendPci.packed_size() +
        sizeof(_cbSendLength) +
        _pbSendBuffer.packed_size() +
        _pioRecvPci.packed_size() +
        sizeof(_fpbRecvBufferIsNULL) +
        sizeof(_cbRecvLength)
    );
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_transmit::min_packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.min_packed_size() +
        _Card.min_packed_size() +
        _ioSendPci.min_packed_size() +
        sizeof(_cbSendLength) +
        _pbSendBuffer.min_packed_size() +
        _pioRecvPci.min_packed_size() +
        sizeof(_fpbRecvBufferIsNULL) +
        sizeof(_cbRecvLength)
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_transmit::log(int level) const
{
    LOG(level, "Transmit_Call {");
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "\tCard: [%s]", bytes_to_hex_string(bytes_view(_Card.pbHandle(),
        _Card.cbHandle())).c_str());
    LOG(level, "\tioSendPci: %s", _ioSendPci.repr().c_str());
    LOG(level, "\tcbSendLength: %" PRIu32, _cbSendLength);
    LOG(level, "\tpbSendBuffer: [%s]", bytes_to_hex_string(_pbSendBuffer.data()).c_str());
    LOG(level, "\tpioRecvPci: [");
    for (const auto &pci : _pioRecvPci)
    {
        LOG(level, "\t\t{");
        LOG(level, "\t\t\tdwProtocol: %s (0x%08" PRIX32 ")",
            protocols_to_string(pci.dwProtocol()),
            pci.dwProtocol());
        LOG(level, "\t\t\tcbExtraBytes: %" PRIu32, pci.cbExtraBytes());
        LOG(level, "\t\t\tpbExtraBytes: [%s]", bytes_to_hex_string(pci.ExtraBytes()).c_str());
        LOG(level, "\t\t}");
    }
    LOG(level, "\t]");
    LOG(level, "\tfpbRecvBufferIsNULL: %" PRId32, _fpbRecvBufferIsNULL);
    LOG(level, "\tcbRecvLength: %" PRIu32, _cbRecvLength);
    LOG(level, "}");
}


scard_pack_return_transmit::scard_pack_return_transmit()
    :
    _ReturnCode(0),
    _cbRecvLength(0)
{
}

SCARD_PACK_NATIVE_LONG scard_pack_return_transmit::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

const scard_pack_pci * scard_pack_return_transmit::pioRecvPci() const
{
    return (_pioRecvPci.empty() ? nullptr : &(_pioRecvPci.at(0)));
}

SCARD_PACK_DWORD scard_pack_return_transmit::cbRecvLength() const
{
    return _cbRecvLength;
}

const uint8_t * scard_pack_return_transmit::pbRecvBuffer() const
{
    return _pbRecvBuffer.ptr();
}

std::size_t scard_pack_return_transmit::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;
    scard_pack_pointer_index_type pointer_index(0);

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    if (has_more_payload_bytes_compat(stream, _ReturnCode))
    {
        // pioRecvPci
        n_unpacked += _pioRecvPci.unpack_pointer(stream, pointer_index);

        // cbRecvLength
        n_unpacked += _pbRecvBuffer.unpack_size(stream, _cbRecvLength);

        // pbRecvBuffer
        n_unpacked += _pbRecvBuffer.unpack_pointer(stream, pointer_index);
        
        // pioRecvPci (deferred)
        n_unpacked += _pioRecvPci.unpack_elements(stream, 1, pointer_index);

        // pbRecvBuffer (deferred)
        n_unpacked += _pbRecvBuffer.unpack_elements(stream, _cbRecvLength);
    }

    // padding
    n_unpacked += unpad(stream, n_unpacked);

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

void scard_pack_return_transmit::log(int level) const
{
    LOG(level, "Transmit_Return {");
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "\tpioRecvPci: [");
    for (const auto &pci : _pioRecvPci)
    {
        LOG(level, "\t\t{");
        LOG(level, "\t\t\tdwProtocol: %s (0x%08" PRIX32 ")",
            protocols_to_string(pci.dwProtocol()),
            pci.dwProtocol());
        LOG(level, "\t\t\tcbExtraBytes: %" PRIu32, pci.cbExtraBytes());
        LOG(level, "\t\t\tpbExtraBytes: [%s]", bytes_to_hex_string(pci.ExtraBytes()).c_str());
        LOG(level, "\t\t}");
    }
    LOG(level, "\t]");
    LOG(level, "\tcbRecvLength: %" PRIu32, _cbRecvLength);
    LOG(level, "\tpbRecvBuffer: [%s]", bytes_to_hex_string(_pbRecvBuffer.data()).c_str());
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////
//////// ListReaderGroups /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_list_reader_groups::scard_pack_call_list_reader_groups(
    SCARD_PACK_NATIVE_CONTEXT hContext,
    const char *mszGroups, SCARD_PACK_DWORD cchGroups,
    bool unicode)
    :
    _Context(hContext),
    _fmszGroupsIsNULL(mszGroups ? 0 : 1),
    _cchGroups(cchGroups),
    _unicode(unicode)
{
}

std::size_t scard_pack_call_list_reader_groups::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;
    pointer_index_type pointer_index(0);

    // Context
    n_packed =  _Context.pack(stream, pointer_index);

    // fmszGroupsIsNULL
    stream.out_sint32_le(_fmszGroupsIsNULL);
    n_packed += sizeof(int32_t);

    // cchGroups
    stream.out_uint32_le(_cchGroups);
    n_packed += sizeof(uint32_t);

    // Context (deferred)
    n_packed += _Context.pack_deferred(stream, pointer_index);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_list_reader_groups::io_control_code() const
{
    return (_unicode ? scard_pack_ioctl_list_reader_groups_w : scard_pack_ioctl_list_reader_groups_a);
}

std::size_t scard_pack_call_list_reader_groups::packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.packed_size() +
        sizeof(_fmszGroupsIsNULL) +
        sizeof(_cchGroups)
    );
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_list_reader_groups::min_packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.min_packed_size() +
        sizeof(_fmszGroupsIsNULL) +
        sizeof(_cchGroups)
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_list_reader_groups::log(int level) const
{
    LOG(level, "ListReaderGroups_Call [unicode=%s] {", (_unicode ? "true" : "false"));
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "\tfmszGroupsIsNULL: %" PRId32, _fmszGroupsIsNULL);
    LOG(level, "\tcchGroups: %" PRIu32, _cchGroups);
    LOG(level, "}");
}


scard_pack_return_list_reader_groups::scard_pack_return_list_reader_groups(
    const bool unicode)
    :
    _ReturnCode(0),
    _cBytes(0),
    _unicode(unicode)
{
}

SCARD_PACK_NATIVE_LONG scard_pack_return_list_reader_groups::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

SCARD_PACK_DWORD scard_pack_return_list_reader_groups::cBytes() const
{
    return _cBytes;
}

const char * scard_pack_return_list_reader_groups::mszGroups() const
{
    return _mszGroups.ptr();
}

std::size_t scard_pack_return_list_reader_groups::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;
    pointer_index_type pointer_index(0);

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    if (has_more_payload_bytes_compat(stream, _ReturnCode))
    {
        // cBytes
        n_unpacked += _mszGroups.unpack_size(stream, _cBytes);

        // mszGroups
        n_unpacked += _mszGroups.unpack_pointer(stream, pointer_index);

        // mszGroups (deferred)
        n_unpacked += _mszGroups.unpack_elements(stream, _cBytes);
    }

    // padding
    n_unpacked += unpad(stream, n_unpacked);

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

std::size_t scard_pack_return_list_reader_groups::packed_size() const
{
    std::size_t size;
    
    size = (
        sizeof(_ReturnCode) +
        (_ReturnCode ? 0 : sizeof(_cBytes) + _mszGroups.packed_size())
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_return_list_reader_groups::log(int level) const
{
    LOG(level, "ListReaderGroups_Return [unicode=%s] {", (_unicode ? "true" : "false"));
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "\tcBytes: %" PRIu32, _cBytes);
    LOG(level, "\tmszGroups: [%s]", bytes_to_hex_string(_mszGroups.data()).c_str());
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////
//////// ListReaders  /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_list_readers::scard_pack_call_list_readers(
    SCARD_PACK_NATIVE_CONTEXT hContext,
    const char *mszGroups, char *mszReaders,
    SCARD_PACK_DWORD cchReaders,
    bool unicode)
    :
    _Context(hContext),
    _cBytes(mszGroups ? (std::strlen(mszGroups) + 1) : 0),
    _mszGroups(reinterpret_cast<const uint8_t *>(mszGroups), _cBytes),
    _fmszReadersIsNULL(mszReaders ? 0 : 1),
    _cchReaders(cchReaders),
    _unicode(unicode)
{
}

std::size_t scard_pack_call_list_readers::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;
    pointer_index_type pointer_index(0);

    // Context
    n_packed =  _Context.pack(stream, pointer_index);

    // cBytes
    n_packed += _mszGroups.pack_size(stream, _cBytes);

    // mszGroups
    n_packed += _mszGroups.pack_pointer(stream, pointer_index);

    // fmszReadersIsNULL
    stream.out_sint32_le(_fmszReadersIsNULL);
    n_packed += sizeof(int32_t);

    // cchReaders
    stream.out_uint32_le(_cchReaders);
    n_packed += sizeof(uint32_t);

    // Context (deferred)
    n_packed += _Context.pack_deferred(stream, pointer_index);

    // mszGroups (deferred)
    n_packed += _mszGroups.pack_elements(stream, _cBytes);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_list_readers::io_control_code() const
{
    return (_unicode ? scard_pack_ioctl_list_readers_w : scard_pack_ioctl_list_readers_a);
}

std::size_t scard_pack_call_list_readers::packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.packed_size() +
        sizeof(_cBytes) +
        _mszGroups.packed_size() +
        sizeof(_fmszReadersIsNULL) +
        sizeof(_cchReaders)
    );
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_list_readers::min_packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.min_packed_size() +
        sizeof(_cBytes) +
        _mszGroups.min_packed_size() +
        sizeof(_fmszReadersIsNULL) +
        sizeof(_cchReaders)
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_list_readers::log(int level) const
{
    LOG(level, "ListReaders_Call [unicode=%s] {", (_unicode ? "true" : "false"));
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "\tcBytes: %" PRIu32, _cBytes);
    LOG(level, "\tmszGroups: [%s]", bytes_to_hex_string(_mszGroups.data()).c_str());
    LOG(level, "\tfmszReadersIsNULL: %" PRId32, _fmszReadersIsNULL);
    LOG(level, "\tcchReaders: %" PRIu32, _cchReaders);
    LOG(level, "}");
}


scard_pack_return_list_readers::scard_pack_return_list_readers(
    const bool unicode)
    :
    _ReturnCode(0),
    _cBytes(0),
    _unicode(unicode)
{
}

SCARD_PACK_NATIVE_LONG scard_pack_return_list_readers::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

SCARD_PACK_DWORD scard_pack_return_list_readers::cBytes() const
{
    return _cBytes;
}

const char * scard_pack_return_list_readers::mszReaders() const
{
    return _mszReaders.ptr();
}

std::size_t scard_pack_return_list_readers::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;
    pointer_index_type pointer_index(0);

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    if (has_more_payload_bytes_compat(stream, _ReturnCode))
    {
        // cBytes
        n_unpacked += _mszReaders.unpack_size(stream, _cBytes);

        // mszReaders
        n_unpacked += _mszReaders.unpack_pointer(stream, pointer_index);

        // mszReaders (deferred)
        n_unpacked += _mszReaders.unpack_elements(stream, _cBytes);
    }

    // padding
    n_unpacked += unpad(stream, n_unpacked);

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

std::size_t scard_pack_return_list_readers::packed_size() const
{
    std::size_t size;
    
    size = (
        sizeof(_ReturnCode) +
        (_ReturnCode ? 0 : sizeof(_cBytes) + _mszReaders.packed_size())
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_return_list_readers::log(int level) const
{
    LOG(level, "ListReaders_Return [unicode=%s] {", (_unicode ? "true" : "false"));
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "\tcBytes: %" PRIu32, _cBytes);
    LOG(level, "\tmszReaders: [%s]", bytes_to_hex_string(_mszReaders.data()).c_str());
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////
//////// Cancel ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_cancel::scard_pack_call_cancel(SCARD_PACK_NATIVE_CONTEXT hContext)
    :
    _Context(hContext)
{
}

std::size_t scard_pack_call_cancel::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;
    pointer_index_type pointer_index(0);

    // Context
    n_packed = _Context.pack(stream, pointer_index);

    // Context (deferred)
    n_packed += _Context.pack_deferred(stream, pointer_index);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_cancel::io_control_code() const
{
    return scard_pack_ioctl_cancel;
}

std::size_t scard_pack_call_cancel::packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.packed_size()
    );
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_cancel::min_packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.min_packed_size()
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_cancel::log(int level) const
{
    LOG(level, "Cancel_Call {");
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "}");
}


scard_pack_return_cancel::scard_pack_return_cancel()
    :
    _ReturnCode(0)
{
}

SCARD_PACK_NATIVE_LONG scard_pack_return_cancel::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

std::size_t scard_pack_return_cancel::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    // padding
    n_unpacked += unpad(stream, n_unpacked);

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

void scard_pack_return_cancel::log(int level) const
{
    LOG(level, "Cancel_Return {");
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////
//////// GetAttrib ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_get_attrib::scard_pack_call_get_attrib(
    SCARD_PACK_NATIVE_CONTEXT hContext, SCARD_PACK_NATIVE_HANDLE hCard,
    SCARD_PACK_NATIVE_DWORD dwAttrId, const SCARD_PACK_BYTE *pbAttr,
    const SCARD_PACK_NATIVE_DWORD *pcbAttrLen)
    :
    _Context(hContext),
    _Card(hCard),
    _dwAttrId(dwAttrId),
    _fpbAttrIsNULL(pbAttr ? 0 : 1),
    _cbAttrLen((pbAttr && pcbAttrLen) ? *pcbAttrLen : 0)
{
}

std::size_t scard_pack_call_get_attrib::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;
    pointer_index_type pointer_index(0);

    // Context
    n_packed = _Context.pack(stream, pointer_index);

    // Card
    n_packed += _Card.pack(stream, pointer_index);

    // dwAttrId
    stream.out_uint32_le(_dwAttrId);
    n_packed += sizeof(uint32_t);

    // fpbAttrIsNULL
    stream.out_sint32_le(_fpbAttrIsNULL);
    n_packed += sizeof(int32_t);

    // cbAttrLen
    stream.out_uint32_le(_cbAttrLen);
    n_packed += sizeof(uint32_t);

    // Context (deferred)
    n_packed += _Context.pack_deferred(stream, pointer_index);

    // Card (deferred)
    n_packed += _Card.pack_deferred(stream, pointer_index);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_get_attrib::io_control_code() const
{
    return scard_pack_ioctl_get_attrib;
}

std::size_t scard_pack_call_get_attrib::packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.packed_size() +
        _Card.packed_size() +
        sizeof(_dwAttrId) +
        sizeof(_fpbAttrIsNULL) +
        sizeof(_cbAttrLen)
    );
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_get_attrib::min_packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.min_packed_size() +
        _Card.min_packed_size() +
        sizeof(_dwAttrId) +
        sizeof(_fpbAttrIsNULL) +
        sizeof(_cbAttrLen)
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_get_attrib::log(int level) const
{
    LOG(level, "GetAttrib_Call {");
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "\tCard: [%s]", bytes_to_hex_string(bytes_view(_Card.pbHandle(),
        _Card.cbHandle())).c_str());
    LOG(level, "\tdwAttrId: %s (0x%08" PRIX32 ")", attribute_to_string(_dwAttrId),
        static_cast<uint32_t>(_dwAttrId));
    LOG(level, "\tfpbAttrIsNULL: %" PRId32, _fpbAttrIsNULL);
    LOG(level, "\tcbAttrLen: %" PRIu32, _cbAttrLen);
    LOG(level, "}");
}


scard_pack_return_get_attrib::scard_pack_return_get_attrib()
    :
    _ReturnCode(0),
    _cbAttrLen(0)
{
}

SCARD_PACK_NATIVE_LONG scard_pack_return_get_attrib::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

SCARD_PACK_DWORD scard_pack_return_get_attrib::cbAttrLen() const
{
    return _cbAttrLen;
}

const SCARD_PACK_BYTE * scard_pack_return_get_attrib::pbAttr() const
{
    return _pbAttr.ptr();
}

std::size_t scard_pack_return_get_attrib::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;
    pointer_index_type pointer_index(0);

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    if (has_more_payload_bytes_compat(stream, _ReturnCode))
    {
        // cbAttrLen
        n_unpacked += _pbAttr.unpack_size(stream, _cbAttrLen);

        // pbAttr
        n_unpacked += _pbAttr.unpack_pointer(stream, pointer_index);

        // pbAttr (deferred)
        n_unpacked += _pbAttr.unpack_elements(stream, _cbAttrLen);
    }

    // padding
    n_unpacked += unpad(stream, n_unpacked);

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

void scard_pack_return_get_attrib::log(int level) const
{
    LOG(level, "GetAttrib_Return {");
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "\tcbAttrLen: %" PRIu32, _cbAttrLen);
    LOG(level, "\tpbAttr: [%s]", bytes_to_hex_string(_pbAttr.data()).c_str());
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////
//////// SetAttrib ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_call_set_attrib::scard_pack_call_set_attrib(
    SCARD_PACK_NATIVE_CONTEXT hContext, SCARD_PACK_NATIVE_HANDLE hCard,
    SCARD_PACK_DWORD dwAttrId, const SCARD_PACK_BYTE *pbAttr,
    SCARD_PACK_DWORD cbAttrLen)
    :
    _Context(hContext),
    _Card(hCard),
    _dwAttrId(dwAttrId),
    _cbAttrLen(cbAttrLen),
    _pbAttr(pbAttr, cbAttrLen)
{
}

std::size_t scard_pack_call_set_attrib::pack(OutStream &stream) const
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_packed;
    pointer_index_type pointer_index(0);

    // Context
    n_packed = _Context.pack(stream, pointer_index);

    // Card
    n_packed += _Card.pack(stream, pointer_index);

    // dwAttrId
    stream.out_uint32_le(_dwAttrId);
    n_packed += sizeof(uint32_t);

    // cbAttrLen
    n_packed += _pbAttr.pack_size(stream, _cbAttrLen);

    // pbAttr
    n_packed += _pbAttr.pack_pointer(stream, pointer_index);

    // Context (deferred)
    n_packed += _Context.pack_deferred(stream, pointer_index);

    // Card (deferred)
    n_packed += _Card.pack_deferred(stream, pointer_index);

    // pbAttr (deferred)
    n_packed += _pbAttr.pack_elements(stream, _cbAttrLen);

    // padding
    n_packed += pad(stream, n_packed);

    return n_packed;
}

uint32_t scard_pack_call_set_attrib::io_control_code() const
{
    return scard_pack_ioctl_set_attrib;
}

std::size_t scard_pack_call_set_attrib::packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.packed_size() +
        _Card.packed_size() +
        sizeof(_dwAttrId) +
        sizeof(_cbAttrLen) +
        _pbAttr.packed_size()
    );
    size += get_padding_size(size);

    return size;
}

std::size_t scard_pack_call_set_attrib::min_packed_size() const
{
    std::size_t size;
    
    size = (
        _Context.min_packed_size() +
        _Card.min_packed_size() +
        sizeof(_dwAttrId) +
        sizeof(_cbAttrLen) +
        _pbAttr.min_packed_size()
    );
    size += get_padding_size(size);

    return size;
}

void scard_pack_call_set_attrib::log(int level) const
{
    LOG(level, "SetAttrib_Call {");
    LOG(level, "\tContext: [%s]", bytes_to_hex_string(bytes_view(_Context.pbContext(),
        _Context.cbContext())).c_str());
    LOG(level, "\tCard: [%s]", bytes_to_hex_string(bytes_view(_Card.pbHandle(),
        _Card.cbHandle())).c_str());
    LOG(level, "\tdwAttrId: %s (0x%08" PRIX32 ")", attribute_to_string(_dwAttrId),
        static_cast<uint32_t>(_dwAttrId));
    LOG(level, "\tcbAttrLen: %" PRIu32, _cbAttrLen);
    LOG(level, "\tpbAttr: [%s]", bytes_to_hex_string(_pbAttr.data()).c_str());
    LOG(level, "}");
}


scard_pack_return_set_attrib::scard_pack_return_set_attrib()
    :
    _ReturnCode(0)
{
}

SCARD_PACK_NATIVE_LONG scard_pack_return_set_attrib::ReturnCode() const
{
    return static_cast<unsigned>(_ReturnCode);
}

std::size_t scard_pack_return_set_attrib::unpack(InStream &stream)
{
    SCARD_PACK_CHECK_STREAM_START(stream, this, ERR_SCARD);

    std::size_t n_unpacked;

    // ReturnCode
    _ReturnCode = stream.in_sint32_le();
    n_unpacked = sizeof(int32_t);

    // padding
    n_unpacked += unpad(stream, n_unpacked);

    SCARD_PACK_CHECK_STREAM_END(stream,  ERR_SCARD);

    return n_unpacked;
}

void scard_pack_return_set_attrib::log(int level) const
{
    LOG(level, "SetAttrib_Return {");
    LOG(level, "\tReturnCode: %s (0x%08" PRIX32 ")", return_code_to_string(_ReturnCode),
        static_cast<uint32_t>(_ReturnCode));
    LOG(level, "}");
}


///////////////////////////////////////////////////////////////////////////////


// FIXME Fix for probable bug in FreeRDP (version 2.3.0 at the time of writing)
//
// FreeRDP appears not to be encoding the whole Return structure in cases where
// the ReturnCode is an error ReturnCode. The whole Return structure should always
// be encoded not matter what the ReturnCode is.
bool has_more_payload_bytes_compat(InStream &stream, SCARD_PACK_LONG return_code)
{
    if (!return_code)
    {
        return true;
    }

    // calculate padding size
    const auto padding_size = get_padding_size(sizeof(return_code));

    // get the remaining byte count
    const auto remaining_size = stream.in_remain();

    return (remaining_size > padding_size);
}

std::size_t pad(OutStream &stream, std::size_t data_size)
{
    std::size_t padding_size;

    // calculate padding size
    padding_size = get_padding_size(data_size);

    // do pad
    // stream.out_skip_bytes not used here to prevent leaks
    auto size = padding_size;
    while (size--)
    {
        stream.out_uint8(0x00);
    }

    return padding_size;
}

std::size_t unpad(InStream &stream, std::size_t data_size)
{
    std::size_t padding_size;

    // calculate padding size
    padding_size = get_padding_size(data_size);

    // do unpad
    stream.in_skip_bytes(padding_size);

    return padding_size;
}

std::size_t get_padding_size(std::size_t data_size)
{
    return ((data_size % 8) ? (8 - (data_size % 8)) : 0);
}

void check_stream_start(const InStream &stream, const scard_pack_return *return_,
    const char */*file*/, const char *function, int line, error_type eid)
{
    assert(return_);

    const auto min_expected = return_->min_packed_size();
    const auto n_remaining = stream.in_remain();
    
    if (n_remaining < min_expected)
    {
        LOG(LOG_ERR, "%s(%d): ill-sized input stream: "
            "min_expected=%zu remains=%zu",
            function, line, min_expected, n_remaining);

        throw Error(eid);
    }
}

void check_stream_start(const OutStream &stream, const scard_pack_call *call,
    const char */*file*/, const char *function, int line, error_type eid)
{
    assert(call);

    const auto max_expected = call->packed_size();
    const auto n_remaining = stream.tailroom();
    
    if (n_remaining < max_expected)
    {
        LOG(LOG_ERR, "%s(%d): ill-sized output stream: "
            "max_expected=%zu remains=%zu",
            function, line, max_expected, n_remaining);

        throw Error(eid);
    }
}

void check_stream_end(const InStream &stream,
    const char */*file*/, const char *function, int line,
    error_type eid)
{
    const auto n_remaining = stream.in_remain();

    if (n_remaining)
    {
        LOG(LOG_ERR, "%s(%d): ill-sized intput stream: "
            "remains=%zu",
            function, line, n_remaining);

        throw Error(eid);
    }
}