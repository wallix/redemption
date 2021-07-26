#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string_view>

#include "scard_pack_common.hpp"
#include "scard/scard_pack_type_native_convertible.hpp"
#include "scard/scard_pack_type_array.hpp"
#include "scard/scard_pack_utils.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/bytes_view.hpp"


///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//////// scard_pack_context ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_context::scard_pack_context()
    : _cbContext(0)
{
}

scard_pack_context::scard_pack_context(native_type native_context)
    : _cbContext(0)
{
    from_native(native_context);
}

SCARD_PACK_DWORD scard_pack_context::cbContext() const
{
    return _cbContext;
}

scard_pack_context::context_array_type::const_pointer
scard_pack_context::pbContext() const
{
    return _pbContext.ptr();
}

bytes_view scard_pack_context::Context() const
{
    return _pbContext.data();
}

std::size_t scard_pack_context::pack(OutStream &stream,
    pointer_index_type &pointer_index) const
{
    std::size_t n_packed;

    // cbContext
    n_packed = _pbContext.pack_size(stream, _cbContext);

    // pbContext
    n_packed += _pbContext.pack_pointer(stream, pointer_index);

    return n_packed;
}

std::size_t scard_pack_context::pack_deferred(OutStream &stream,
    pointer_index_type &/*pointer_index*/) const
{
    std::size_t n_packed;

    // pbContext (deferred)
    n_packed = _pbContext.pack_elements(stream, _cbContext);

    return n_packed;
}

std::size_t scard_pack_context::unpack(InStream &stream,
    pointer_index_type &pointer_index)
{
    std::size_t n_unpacked;

    // cbContext
    n_unpacked = _pbContext.unpack_size(stream, _cbContext);

    // pbContext
    n_unpacked += _pbContext.unpack_pointer(stream, pointer_index);

    return n_unpacked;
}

std::size_t scard_pack_context::unpack_deferred(InStream &stream,
    pointer_index_type &/*pointer_index*/)
{
    std::size_t n_unpacked;

    // pbContext (deferred)
    n_unpacked = _pbContext.unpack_elements(stream, _cbContext);

    return n_unpacked;
}

std::size_t scard_pack_context::packed_size() const
{
    return (
        sizeof(_cbContext) +
        _pbContext.packed_size()
    );
}

std::size_t scard_pack_context::min_packed_size() const
{
    return (
        sizeof(_cbContext) +
        _pbContext.min_packed_size()
    );
}

void scard_pack_context::from_native(native_const_reference native_value)
{
    _pbContext.assign(reinterpret_cast<const uint8_t *>(&native_value),
        sizeof(native_type));
    _cbContext = sizeof(native_type);
}

void scard_pack_context::to_native(native_reference native_value) const
{
    SCARD_CHECK_PREDICATE((_cbContext == sizeof(native_type)) &&
        (_pbContext.size() == sizeof(native_type)),
        ERR_SCARD);

    std::memcpy(reinterpret_cast<uint8_t *>(&native_value),
        _pbContext.ptr(), sizeof(native_type));
}

scard_pack_context::native_type scard_pack_context::to_native() const
{
    SCARD_CHECK_PREDICATE((_cbContext == sizeof(native_type)) &&
        (_pbContext.size() == sizeof(native_type)),
        ERR_SCARD);
    
    return *reinterpret_cast<const native_type *>(_pbContext.ptr());
}

void scard_pack_context::log(int level) const
{
    LOG(level, "Context {");
    LOG(level, "\tcbContext: [%s]", bytes_to_hex_string(
        bytes_view(_pbContext.ptr(), _cbContext)).c_str());
    LOG(level, "\tpbContext: [%s]", bytes_to_hex_string(_pbContext.data()).c_str());
    LOG(level, "}");
}

std::string scard_pack_context::repr() const
{
    std::ostringstream stream;

    stream
        << "cbContext=" << _cbContext
        << " "
        << "pbContext=" << bytes_to_hex_string(_pbContext.data())
    ;

    return stream.str();
}


///////////////////////////////////////////////////////////////////////////////
//////// scard_pack_handle ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_handle::scard_pack_handle()
    : _cbHandle(0)
{
}

scard_pack_handle::scard_pack_handle(native_type native_handle)
    : _cbHandle(0)
{
    from_native(native_handle);
}

SCARD_PACK_DWORD scard_pack_handle::cbHandle() const
{
    return _cbHandle;
}

scard_pack_handle::handle_array_type::const_pointer
scard_pack_handle::pbHandle() const
{
    return _pbHandle.ptr();
}

bytes_view scard_pack_handle::Handle() const
{
    return _pbHandle.data();
}

std::size_t scard_pack_handle::pack(OutStream &stream,
    pointer_index_type &pointer_index) const
{
    std::size_t n_packed;

    // cbHandle
    n_packed = _pbHandle.pack_size(stream, _cbHandle);

    // pbHandle
    n_packed += _pbHandle.pack_pointer(stream, pointer_index);

    return n_packed;
}

std::size_t scard_pack_handle::pack_deferred(OutStream &stream,
    pointer_index_type &/*pointer_index*/) const
{
    std::size_t n_packed;

    // pbHandle (deferred)
    n_packed = _pbHandle.pack_elements(stream, _cbHandle);

    return n_packed;
}

std::size_t scard_pack_handle::unpack(InStream &stream,
    pointer_index_type &pointer_index)
{
    std::size_t n_unpacked;

    // cbHandle
    n_unpacked = _pbHandle.unpack_size(stream, _cbHandle);

    // pbHandle
    n_unpacked += _pbHandle.unpack_pointer(stream, pointer_index);

    return n_unpacked;
}

std::size_t scard_pack_handle::unpack_deferred(InStream &stream,
    pointer_index_type &/*pointer_index*/)
{
    std::size_t n_unpacked;

    // pbHandle (deferred)
    n_unpacked = _pbHandle.unpack_elements(stream, _cbHandle);

    return n_unpacked;
}

std::size_t scard_pack_handle::packed_size() const
{
    return (
        sizeof(_cbHandle) +
        _pbHandle.packed_size()
    );
}

std::size_t scard_pack_handle::min_packed_size() const
{
    return (
        sizeof(_cbHandle) +
        _pbHandle.min_packed_size()
    );
}

void scard_pack_handle::from_native(native_const_reference native_value)
{
    _pbHandle.assign(reinterpret_cast<const uint8_t *>(&native_value),
        sizeof(native_type));
    _cbHandle = sizeof(native_type);
}

void scard_pack_handle::to_native(native_reference native_value) const
{
    SCARD_CHECK_PREDICATE((_cbHandle == sizeof(native_type)) &&
        (_pbHandle.size() == sizeof(native_type)),
        ERR_SCARD);

    std::memcpy(reinterpret_cast<uint8_t *>(&native_value),
        _pbHandle.ptr(), sizeof(native_type));
}

scard_pack_handle::native_type scard_pack_handle::to_native() const
{
    SCARD_CHECK_PREDICATE((_cbHandle == sizeof(native_type)) &&
        (_pbHandle.size() == sizeof(native_type)),
        ERR_SCARD);
    
    return *reinterpret_cast<const native_type *>(_pbHandle.ptr());
}

void scard_pack_handle::log(int level) const
{
    LOG(level, "Handle {");
    LOG(level, "\tcbHandle: [%s]", bytes_to_hex_string(
        bytes_view(_pbHandle.ptr(), _cbHandle)).c_str());
    LOG(level, "\tpbHandle: [%s]", bytes_to_hex_string(_pbHandle.data()).c_str());
    LOG(level, "}");
}

std::string scard_pack_handle::repr() const
{
    std::ostringstream stream;

    stream
        << "cbHandle=" << _cbHandle
        << " "
        << "pbHandle=" << bytes_to_hex_string(_pbHandle.data())
    ;

    return stream.str();
}


///////////////////////////////////////////////////////////////////////////////
//////// scard_pack_reader_state //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_reader_state::scard_pack_reader_state()
{
}

scard_pack_reader_state::scard_pack_reader_state(native_const_reference native_value)
{
    from_native(native_value);
}

std::string_view scard_pack_reader_state::Reader() const
{
    return std::string_view(_szReader.ptr(), _szReader.size());
}

SCARD_PACK_DWORD scard_pack_reader_state::dwCurrentState() const
{
    return _dwCurrentState;
}

SCARD_PACK_DWORD scard_pack_reader_state::dwEventState() const
{
    return _dwEventState;
}

SCARD_PACK_DWORD scard_pack_reader_state::cbAtr() const
{
    return _cbAtr;
}

bytes_view scard_pack_reader_state::rgbAtr() const
{
    return _rgbAtr;
}

std::size_t scard_pack_reader_state::pack(OutStream &stream,
    pointer_index_type &pointer_index) const
{
    std::size_t n_packed;

    // szReader
    n_packed = _szReader.pack_pointer(stream, pointer_index);

    // dwCurrentState (4 bytes)
    stream.out_uint32_le(_dwCurrentState);
    n_packed += sizeof(uint32_t);

    // dwEventState (4 bytes)
    stream.out_uint32_le(_dwEventState);
    n_packed += sizeof(uint32_t);

    // cbAtr (4 bytes)
    stream.out_uint32_le(_cbAtr);
    n_packed += sizeof(uint32_t);

    // rgbAtr (36 bytes)
    stream.out_copy_bytes(_rgbAtr.data(), _rgbAtr.max_size());
    n_packed += _rgbAtr.max_size();

    return n_packed;
}

std::size_t scard_pack_reader_state::pack_deferred(OutStream &stream,
    pointer_index_type &pointer_index) const
{
    std::size_t n_packed;

    // szReader (deferred)
    n_packed = _szReader.pack_elements(stream, pointer_index);

    return n_packed;
}

std::size_t scard_pack_reader_state::unpack(InStream &stream,
    pointer_index_type &/*pointer_index*/)
{
    std::size_t n_unpacked;

    // szReader
    // (ignored on input, see [MS-RDPECS] §2.2.1.11)

    // dwCurrentState (4 bytes)
    _dwCurrentState = stream.in_uint32_le();
    n_unpacked = sizeof(uint32_t);

    // dwEventState (4 bytes)
    _dwEventState = stream.in_uint32_le();
    n_unpacked += sizeof(uint32_t);

    // cbAtr (4 bytes)
    _cbAtr = stream.in_uint32_le();
    n_unpacked += sizeof(uint32_t);

    SCARD_CHECK_PREDICATE(_cbAtr <= _rgbAtr.max_size(),
        ERR_SCARD);

    // rgbAtr (36 bytes)
    stream.in_copy_bytes(_rgbAtr.data(), _rgbAtr.max_size());
    n_unpacked += _rgbAtr.max_size();

    return n_unpacked;
}

std::size_t scard_pack_reader_state::unpack_deferred(InStream &/*stream*/,
    pointer_index_type &/*pointer_index*/)
{
    // szReader (deferred)
    // (ignored on input, see [MS-RDPECS] §2.2.1.11)

    return 0;
}

std::size_t scard_pack_reader_state::packed_size() const
{
    return (
        _szReader.packed_size() +
        sizeof(uint32_t) +
        sizeof(uint32_t) +
        sizeof(uint32_t) +
        _rgbAtr.max_size()
    );
}

std::size_t scard_pack_reader_state::min_packed_size() const
{
    return (
        _szReader.min_packed_size() +
        sizeof(uint32_t) +
        sizeof(uint32_t) +
        sizeof(uint32_t) +
        _rgbAtr.max_size()
    );
}

void scard_pack_reader_state::from_native(native_const_reference native_value)
{
    _szReader.assign(native_value.szReader,
        (native_value.szReader ? std::strlen(native_value.szReader) + 1 : 0));
    _dwCurrentState = native_value.dwCurrentState;
    _dwEventState = native_value.dwEventState;
    _cbAtr = native_value.cbAtr;

    SCARD_CHECK_PREDICATE(_cbAtr <= _rgbAtr.max_size(),
        ERR_SCARD);
    
    std::memcpy(_rgbAtr.data(), native_value.rgbAtr, _rgbAtr.max_size());
}

void scard_pack_reader_state::to_native(native_reference native_value) const
{
    //native_value.szReader = _Reader.ptr();
	//native_value.pvUserData = nullptr;
	native_value.dwCurrentState = _dwCurrentState;
	native_value.dwEventState = _dwEventState;
	native_value.cbAtr = _cbAtr;
    
	std::memcpy(native_value.rgbAtr, _rgbAtr.data(), _rgbAtr.max_size());
}

void scard_pack_reader_state::log(int level) const
{
    LOG(level, "ReaderState {");
    LOG(level, "\tdwCurrentState: %s (0x%08" PRIX32 ")",
        reader_state_to_string(_dwCurrentState), _dwCurrentState);
    LOG(level, "\tdwEventState: %s (0x%08" PRIX32 ")",
        reader_state_to_string(_dwEventState), _dwEventState);
    LOG(level, "\tcbAtr: %" PRIu32, _cbAtr);
    LOG(level, "\trgbAtr: [%s]", bytes_to_hex_string(_rgbAtr).c_str());
    LOG(level, "}");
}

std::string scard_pack_reader_state::repr() const
{
    std::ostringstream stream;

    stream
        << "dwCurrentState=" << _dwCurrentState
        << " "
        << "dwEventState=" << _dwEventState
        << " "
        << "cbAtr=" << _cbAtr
        << " "
        << "rgbAtr=" << bytes_to_hex_string(_rgbAtr)
    ;

    return stream.str();
}


///////////////////////////////////////////////////////////////////////////////
//////// scard_pack_pci ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_pci::scard_pack_pci()
    : _cbExtraBytes(0)
{
}

scard_pack_pci::scard_pack_pci(native_const_reference native_value)
    : _cbExtraBytes(0)
{
    from_native(native_value);
}

SCARD_PACK_DWORD scard_pack_pci::dwProtocol() const
{
    return _dwProtocol;
}

SCARD_PACK_DWORD scard_pack_pci::cbExtraBytes() const
{
    return _cbExtraBytes;
}

scard_pack_pci::extra_bytes_array_type::const_pointer
scard_pack_pci::pbExtraBytes() const
{
    return _pbExtraBytes.ptr();
}

bytes_view scard_pack_pci::ExtraBytes() const
{
    return _pbExtraBytes.data();
}

std::size_t scard_pack_pci::pack(OutStream &stream,
    pointer_index_type &pointer_index) const
{
    std::size_t n_packed;

    // dwProtocol (4 bytes)
    stream.out_uint32_le(_dwProtocol);
    n_packed = sizeof(uint32_t);

    // cbExtraBytes
    n_packed += _pbExtraBytes.pack_size(stream, _cbExtraBytes);

    // pbExtraBytes
    n_packed += _pbExtraBytes.pack_pointer(stream, pointer_index);

    return n_packed;
}

std::size_t scard_pack_pci::pack_deferred(OutStream &stream,
    pointer_index_type &/*pointer_index*/) const
{
    std::size_t n_packed;

    // pbExtraBytes (deferred)
    n_packed = _pbExtraBytes.pack_elements(stream, _cbExtraBytes);

    return n_packed;
}

std::size_t scard_pack_pci::unpack(InStream &stream,
    pointer_index_type &pointer_index)
{
    std::size_t n_unpacked;

    // dwProtocol (4 bytes)
    _dwProtocol = stream.in_uint32_le();
    n_unpacked = sizeof(uint32_t);

    // cbExtraBytes
    n_unpacked += _pbExtraBytes.unpack_size(stream, _cbExtraBytes);

    // pbExtraBytes
    n_unpacked += _pbExtraBytes.unpack_pointer(stream, pointer_index);

    return n_unpacked;
}

std::size_t scard_pack_pci::unpack_deferred(InStream &stream,
    pointer_index_type &/*pointer_index*/)
{
    std::size_t n_unpacked;
    
    // pbExtraBytes (deferred)
    n_unpacked = _pbExtraBytes.unpack_elements(stream, _cbExtraBytes);

    return n_unpacked;
}

std::size_t scard_pack_pci::packed_size() const
{
    return (
        sizeof(_dwProtocol) +
        sizeof(_cbExtraBytes) +
        _pbExtraBytes.packed_size()
    );
}

std::size_t scard_pack_pci::min_packed_size() const
{
    return (
        sizeof(_dwProtocol) +
        sizeof(_cbExtraBytes) +
        _pbExtraBytes.min_packed_size()
    );
}

void scard_pack_pci::from_native(native_const_reference native_value)
{
    const auto &header = native_value.header();
    const auto &body = native_value.body();

    _dwProtocol = header.dwProtocol;
    _pbExtraBytes.assign(body.data(), body.size());
    _cbExtraBytes = body.size();
}

void scard_pack_pci::to_native(native_reference native_value) const
{
    native_value.header(_dwProtocol, _cbExtraBytes);
    native_value.body(_pbExtraBytes.data());
}

void scard_pack_pci::to_native(SCARD_PACK_NATIVE_PCI_HEADER *native_value) const
{
    if (native_value)
    {
        SCARD_CHECK_PREDICATE(native_value->cbPciLength
            >= sizeof(SCARD_PACK_NATIVE_PCI_HEADER), ERR_SCARD);

        native_value->dwProtocol = _dwProtocol;
        std::memcpy(native_value + sizeof(SCARD_PACK_NATIVE_PCI_HEADER), _pbExtraBytes.ptr(),
            native_value->cbPciLength - sizeof(SCARD_PACK_NATIVE_PCI_HEADER));
        native_value->cbPciLength = sizeof(SCARD_PACK_NATIVE_PCI_HEADER) + _cbExtraBytes;
    }
}

void scard_pack_pci::log(int level) const
{
    LOG(level, "SCardIO_Request {");
    LOG(level, "\tdwProtocol: %s (0x%08" PRIX32 ")", protocols_to_string(_dwProtocol),
        static_cast<uint32_t>(_dwProtocol));
    LOG(level, "\tcbExtraBytes: %" PRIu32, _cbExtraBytes);
    LOG(level, "\tpbExtraBytes: [%s]", bytes_to_hex_string(_pbExtraBytes.data()).c_str());
    LOG(level, "}");
}

std::string scard_pack_pci::repr() const
{
    std::ostringstream stream;

    stream
        << "dwProtocol=" << _dwProtocol
        << " "
        << "cbExtraBytes=" << _cbExtraBytes
        << " "
        << "pbExtraBytes=" << bytes_to_hex_string(_pbExtraBytes.data())
    ;

    return stream.str();
}