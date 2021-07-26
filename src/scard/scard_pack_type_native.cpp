#include <cstdint>

#include "utils/sugar/bytes_view.hpp"

#include "scard_pack_type_native.hpp"


///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//////// scard_pack_native_pci ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


scard_pack_native_pci::scard_pack_native_pci()
    :
    _value(sizeof(SCARD_PACK_NATIVE_PCI_HEADER), 0)
{
}

scard_pack_native_pci::scard_pack_native_pci(
    const SCARD_PACK_NATIVE_PCI_HEADER *native_value)
{
    if (native_value)
    {
        const auto value_ptr = reinterpret_cast<const uint8_t *>(native_value);
        const auto value_size = native_value->cbPciLength;

        // yes, we are really about to do this
        // see https://docs.microsoft.com/en-us/windows/win32/api/winscard/nf-winscard-scardtransmit
        _value.assign(value_ptr, value_ptr + value_size);
    }
    else
    {
        _value.assign(sizeof(SCARD_PACK_NATIVE_PCI_HEADER), 0);
    }
}

const SCARD_PACK_NATIVE_PCI_HEADER & scard_pack_native_pci::header() const
{
    return *(reinterpret_cast<const SCARD_PACK_NATIVE_PCI_HEADER *>(_value.data()));
}

void scard_pack_native_pci::header(unsigned long dwProtocol, unsigned long cbPciLength)
{
    const auto value_ptr = reinterpret_cast<SCARD_PACK_NATIVE_PCI_HEADER *>(_value.data());

    value_ptr->dwProtocol = dwProtocol;
    value_ptr->cbPciLength = cbPciLength;
}

bytes_view scard_pack_native_pci::body() const
{
    return bytes_view((_value.size() == sizeof(SCARD_PACK_NATIVE_PCI_HEADER)) ?
            nullptr : _value.data() + sizeof(SCARD_PACK_NATIVE_PCI_HEADER),
        _value.size() - sizeof(SCARD_PACK_NATIVE_PCI_HEADER));
}

void scard_pack_native_pci::body(bytes_view value)
{
    // resize container for holding the new body data
    _value.resize(sizeof(SCARD_PACK_NATIVE_PCI_HEADER) + value.size());

    // copy body data
    std::copy(value.begin(), value.end(), _value.begin()
        + sizeof(SCARD_PACK_NATIVE_PCI_HEADER));
}

bytes_view scard_pack_native_pci::data() const
{
    return _value;
}