/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2021
    Author(s): Florent Plard
*/

#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "core/error.hpp"
#include "scard/scard_pack_type_native_convertible.hpp"
#include "scard/scard_pack_type_internal.hpp"
#include "scard/scard_pack_type_native.hpp"
#include "scard/scard_pack_type_array.hpp"
#include "scard/scard_pack_type_string.hpp"
#include "scard/scard_utils.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/array_view.hpp"


///////////////////////////////////////////////////////////////////////////////


enum scard_pack_ioctl
{
    scard_pack_ioctl_establish_context = 0x00090014,
    scard_pack_ioctl_release_context = 0x00090018,
    scard_pack_ioctl_is_valid_context = 0x0009001c,
    scard_pack_ioctl_list_reader_groups_a = 0x00090020,
    scard_pack_ioctl_list_reader_groups_w = 0x00090024,
    scard_pack_ioctl_list_readers_a = 0x00090028,
    scard_pack_ioctl_list_readers_w = 0x0009002c,
    scard_pack_ioctl_introduce_reader_group_a = 0x00090050,
    scard_pack_ioctl_introduce_reader_group_w = 0x00090054,
    scard_pack_ioctl_forget_reader_group_a = 0x00090058,
    scard_pack_ioctl_forget_reader_group_w = 0x0009005c,
    scard_pack_ioctl_introduce_reader_a = 0x00090060,
    scard_pack_ioctl_introduce_reader_w = 0x00090064,
    scard_pack_ioctl_forget_reader_a = 0x00090068,
    scard_pack_ioctl_forget_reader_w = 0x0009006c,
    scard_pack_ioctl_add_reader_to_group_a = 0x00090070,
    scard_pack_ioctl_add_reader_to_group_w = 0x00090074,
    scard_pack_ioctl_remove_reader_from_group_a = 0x00090078,
    scard_pack_ioctl_remove_reader_from_group_w = 0x0009007c,
    scard_pack_ioctl_locate_cards_a = 0x00090098,
    scard_pack_ioctl_locate_cards_w = 0x0009009c,
    scard_pack_ioctl_get_status_change_a = 0x000900a0,
    scard_pack_ioctl_get_status_change_w = 0x000900a4,
    scard_pack_ioctl_cancel = 0x000900a8,
    scard_pack_ioctl_connect_a = 0x000900ac,
    scard_pack_ioctl_connect_w = 0x000900b0,
    scard_pack_ioctl_reconnect = 0x000900b4,
    scard_pack_ioctl_disconnect = 0x000900b8,
    scard_pack_ioctl_begin_transaction = 0x000900bc,
    scard_pack_ioctl_end_transaction = 0x000900c0,
    scard_pack_ioctl_state = 0x000900c4,
    scard_pack_ioctl_status_a = 0x000900c8,
    scard_pack_ioctl_status_w = 0x000900cc,
    scard_pack_ioctl_transmit = 0x000900d0,
    scard_pack_ioctl_control = 0x000900d4,
    scard_pack_ioctl_get_attrib = 0x000900d8,
    scard_pack_ioctl_set_attrib = 0x000900dc,
    scard_pack_ioctl_access_started_event = 0x000900e0,
    scard_pack_ioctl_release_started_event = 0x000900e4,
    scard_pack_ioctl_locate_cards_by_atr_a = 0x000900e8,
    scard_pack_ioctl_locate_cards_by_atr_w = 0x000900ec,
    scard_pack_ioctl_read_cache_a = 0x000900f0,
    scard_pack_ioctl_read_cache_w = 0x000900f4,
    scard_pack_ioctl_write_cache_a = 0x000900f8,
    scard_pack_ioctl_write_cache_w = 0x000900fc,
    scard_pack_ioctl_get_transmit_count = 0x00090100,
    scard_pack_ioctl_get_reader_icon = 0x00090104,
    scard_pack_ioctl_get_device_type_id = 0x00090108
};

class scard_pack_call
{
protected:
    typedef scard_pack_pointer_index_type pointer_index_type;

public:
    ///
    virtual ~scard_pack_call() = default;

    ///
    virtual std::size_t pack(OutStream &stream) const = 0;
    
    ///
    virtual std::size_t packed_size() const = 0;

    ///
    virtual std::size_t min_packed_size() const = 0;

    ///
    virtual uint32_t io_control_code() const = 0;

    ///
    virtual void log(int level) const = 0;
};

class scard_pack_return
{
protected:
    typedef scard_pack_pointer_index_type pointer_index_type;

public:
    ///
    virtual ~scard_pack_return() = default;

    ///
    virtual std::size_t unpack(InStream &stream) = 0;

    ///
    virtual std::size_t packed_size() const
    {
        throw std::runtime_error("Not implemented");
    }

    ///
    virtual std::size_t min_packed_size() const
    {
        return sizeof(uint32_t);
    }

    ///
    virtual void log(int level) const = 0;
};


///////////////////////////////////////////////////////////////////////////////
//////// EstablishContext /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_establish_context : public scard_pack_call
{
    SCARD_PACK_DWORD   _dwScope;

public:
    ///
    scard_pack_call_establish_context(SCARD_PACK_DWORD dwScope);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_establish_context : public scard_pack_return
{
	SCARD_PACK_LONG     _ReturnCode;
	scard_pack_context  _Context;

public:
    ///
    scard_pack_return_establish_context();

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    SCARD_PACK_NATIVE_CONTEXT hContext() const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual void log(int level) const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// ReleaseContext  //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_release_context : public scard_pack_call
{
    scard_pack_context  _Context;

public:
    ///
    scard_pack_call_release_context(
        SCARD_PACK_NATIVE_CONTEXT hContext);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_release_context : public scard_pack_return
{
    SCARD_PACK_LONG    _ReturnCode;

public:
    ///
    scard_pack_return_release_context();

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual void log(int level) const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// IsValidContext  //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_is_valid_context : public scard_pack_call
{
    scard_pack_context  _Context;

public:
    ///
    scard_pack_call_is_valid_context(
        SCARD_PACK_NATIVE_CONTEXT hContext);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_is_valid_context : public scard_pack_return
{
    SCARD_PACK_LONG    _ReturnCode;

public:
    ///
    scard_pack_return_is_valid_context();

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual void log(int level) const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// Connect //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_connect : public scard_pack_call
{
    scard_pack_string   _szReader;
    scard_pack_context  _Context;
    SCARD_PACK_DWORD    _dwShareMode;
    SCARD_PACK_DWORD    _dwPreferredProtocols;

    bool                _unicode;

public:
    ///
    scard_pack_call_connect(SCARD_PACK_NATIVE_CONTEXT hContext, const char *szReader,
        SCARD_PACK_DWORD dwShareMode, SCARD_PACK_DWORD dwPreferredProtocols,
        bool unicode);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_connect : public scard_pack_return
{
    SCARD_PACK_LONG     _ReturnCode;
    scard_pack_context  _Context;
    scard_pack_handle   _Card;
    SCARD_PACK_DWORD    _dwActiveProtocol;

public:
    ///
    scard_pack_return_connect();

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    SCARD_PACK_NATIVE_CONTEXT hContext() const;

    ///
    SCARD_PACK_NATIVE_HANDLE hCard() const;

    ///
    SCARD_PACK_DWORD dwActiveProtocol() const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual void log(int level) const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// Reconnect ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_reconnect : public scard_pack_call
{
    scard_pack_context  _Context;
    scard_pack_handle   _Card;
    SCARD_PACK_DWORD    _dwShareMode;
    SCARD_PACK_DWORD    _dwPreferredProtocols;
    SCARD_PACK_DWORD    _dwInitialization;

public:
    ///
    scard_pack_call_reconnect(SCARD_PACK_NATIVE_CONTEXT hContext,
        SCARD_PACK_NATIVE_HANDLE hCard, SCARD_PACK_DWORD dwShareMode,
        SCARD_PACK_DWORD dwPreferredProtocols, SCARD_PACK_DWORD dwInitialization);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_reconnect : public scard_pack_return
{
    SCARD_PACK_LONG     _ReturnCode;
    SCARD_PACK_DWORD    _dwActiveProtocol;

public:
    ///
    scard_pack_return_reconnect();

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    SCARD_PACK_DWORD dwActiveProtocol() const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual void log(int level) const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// Disconnect ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_disconnect : public scard_pack_call
{
    scard_pack_context  _Context;
    scard_pack_handle   _Card;
    SCARD_PACK_DWORD    _dwDisposition;

public:
    ///
    scard_pack_call_disconnect(SCARD_PACK_NATIVE_CONTEXT hContext,
        SCARD_PACK_NATIVE_HANDLE hCard, SCARD_PACK_DWORD dwDisposition);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_disconnect : public scard_pack_return
{
    SCARD_PACK_LONG     _ReturnCode;

public:
    ///
    scard_pack_return_disconnect();

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual void log(int level) const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// BeginTransaction /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_begin_transaction : public scard_pack_call
{
    scard_pack_context  _Context;
    scard_pack_handle   _Card;

public:
    ///
    scard_pack_call_begin_transaction(SCARD_PACK_NATIVE_CONTEXT hContext,
        SCARD_PACK_NATIVE_HANDLE hCard);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_begin_transaction : public scard_pack_return
{
    SCARD_PACK_LONG     _ReturnCode;

public:
    ///
    scard_pack_return_begin_transaction();

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual void log(int level) const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// EndTransaction ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_end_transaction : public scard_pack_call
{
    scard_pack_context  _Context;
    scard_pack_handle   _Card;
    SCARD_PACK_DWORD    _dwDisposition;

public:
    ///
    scard_pack_call_end_transaction(SCARD_PACK_NATIVE_CONTEXT hContext,
        SCARD_PACK_NATIVE_HANDLE hCard, SCARD_PACK_DWORD dwDisposition);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_end_transaction : public scard_pack_return
{
    SCARD_PACK_LONG     _ReturnCode;

public:
    ///
    scard_pack_return_end_transaction();

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual void log(int level) const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// Status ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_status : public scard_pack_call
{
    scard_pack_context  _Context;
    scard_pack_handle   _Card;
    SCARD_PACK_LONG     _fmszReaderNamesIsNULL;
    SCARD_PACK_DWORD    _cchReaderLen;
    SCARD_PACK_DWORD    _cbAtrLen;

    bool                _unicode;

public:
    ///
    scard_pack_call_status(SCARD_PACK_NATIVE_CONTEXT hContext,
        SCARD_PACK_NATIVE_HANDLE hCard, const char *mszReaderNames,
        unsigned long *pcchReader, bool unicode);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_status : public scard_pack_return
{
    typedef scard_pack_array_primitive<
        uint8_t, 65536,
        scard_pack_array_type_conformant,
        scard_pack_pointer_type_unique,
        SCARD_PACK_DWORD
    > reader_names_array_type;

    typedef std::array<
        uint8_t, 32
    > atr_array_type;

    SCARD_PACK_LONG             _ReturnCode;
    SCARD_PACK_DWORD            _cBytes;
    reader_names_array_type     _mszReaderNames;
    SCARD_PACK_DWORD            _dwState;
    SCARD_PACK_DWORD            _dwProtocol;
    atr_array_type              _pbAtr;
    SCARD_PACK_DWORD            _cbAtrLen;

public:
    ///
    scard_pack_return_status();

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    SCARD_PACK_DWORD cBytes() const;

    ///
    bytes_view mszReaderNames() const;

    ///
    SCARD_PACK_DWORD dwState() const;

    ///
    SCARD_PACK_DWORD dwProtocol() const;

    ///
    const uint8_t * pbAtr() const;

    ///
    SCARD_PACK_DWORD cbAtrLen() const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual void log(int level) const override;
};



///////////////////////////////////////////////////////////////////////////////
//////// GetStatusChange //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_get_status_change : public scard_pack_call
{
    typedef scard_pack_array_constructed<
        scard_pack_reader_state, 10,
        scard_pack_array_type_conformant,
        scard_pack_pointer_type_full,
        SCARD_PACK_DWORD
    > reader_states_array_type;

	scard_pack_context              _Context;
    SCARD_PACK_DWORD                _dwTimeout;
    SCARD_PACK_DWORD                _cReaders;
    reader_states_array_type        _rgReaderStates;

    bool                            _unicode;

public:
    ///
    explicit scard_pack_call_get_status_change(
        SCARD_PACK_NATIVE_CONTEXT hContext, SCARD_PACK_DWORD dwTimeout,
        SCARD_READERSTATE *rgReaderStates,
        SCARD_PACK_DWORD cReaders,
        bool unicode);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_get_status_change : public scard_pack_return
{
    typedef scard_pack_array_constructed<
        scard_pack_reader_state, 10,
        scard_pack_array_type_conformant,
        scard_pack_pointer_type_full,
        SCARD_PACK_DWORD
    > reader_states_array_type;

    SCARD_PACK_LONG                 _ReturnCode;
    SCARD_PACK_DWORD                _cReaders;
    reader_states_array_type        _rgReaderStates;

public:
    ///
    explicit scard_pack_return_get_status_change();

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    SCARD_PACK_DWORD cReaders() const;

    ///
    array_view<scard_pack_reader_state> rgReaderStates() const;

    ///
    const scard_pack_reader_state & rgReaderStates(std::size_t index) const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual void log(int level) const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// Control //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_control : public scard_pack_call
{
    typedef scard_pack_array_primitive<
        uint8_t, 66560,
        scard_pack_array_type_conformant,
        scard_pack_pointer_type_unique,
        SCARD_PACK_DWORD
    > buffer_array_type;

    scard_pack_context  _Context;
    scard_pack_handle   _Card;
    SCARD_PACK_DWORD    _dwControlCode;
    SCARD_PACK_DWORD    _cbInBufferSize;
    buffer_array_type   _pvInBuffer;
    SCARD_PACK_LONG     _fpvOutBufferIsNULL;
    SCARD_PACK_DWORD    _cbOutBufferSize;

public:
    ///
    scard_pack_call_control(SCARD_PACK_NATIVE_CONTEXT hContext,
        SCARD_PACK_NATIVE_HANDLE hCard, SCARD_PACK_DWORD dwControlCode,
        const void *lpInBuffer, unsigned long cbInBufferSize,
        const void *lpOutBuffer, unsigned long cbOutBufferSize);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_control : public scard_pack_return
{
    typedef scard_pack_array_primitive<
        uint8_t, 66560,
        scard_pack_array_type_conformant,
        scard_pack_pointer_type_unique,
        SCARD_PACK_DWORD
    > buffer_array_type;

    SCARD_PACK_LONG     _ReturnCode;
    SCARD_PACK_DWORD    _cbOutBufferSize;
    buffer_array_type   _pvOutBuffer;

public:
    ///
    scard_pack_return_control();

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    SCARD_PACK_DWORD cbOutBufferSize() const;

    ///
    const uint8_t * pvOutBuffer() const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual void log(int level) const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// Transmit /////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_transmit : public scard_pack_call
{
    typedef scard_pack_array_constructed<
        scard_pack_pci, 1,
        scard_pack_array_type_fixed,
        scard_pack_pointer_type_unique,
        SCARD_PACK_DWORD
    > pci_array_type;

    typedef scard_pack_array_primitive<
        uint8_t, 66560,
        scard_pack_array_type_conformant,
        scard_pack_pointer_type_unique,
        SCARD_PACK_DWORD
    > buffer_array_type;

    scard_pack_context      _Context;
    scard_pack_handle       _Card;
    scard_pack_pci          _ioSendPci;
    SCARD_PACK_DWORD        _cbSendLength;
    buffer_array_type       _pbSendBuffer;
    pci_array_type          _pioRecvPci;
    SCARD_PACK_LONG         _fpbRecvBufferIsNULL;
    SCARD_PACK_DWORD        _cbRecvLength;

public:
    ///
    scard_pack_call_transmit(SCARD_PACK_NATIVE_CONTEXT hContext,
        SCARD_PACK_NATIVE_HANDLE hCard, const SCARD_PACK_NATIVE_PCI_HEADER *pioSendPci,
        const uint8_t *pbSendBuffer, unsigned long cbSendLength,
        SCARD_PACK_NATIVE_PCI_HEADER *pioRecvPci, const uint8_t *pbRecvBuffer,
        const unsigned long *pcbRecvLength);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_transmit : public scard_pack_return
{
    typedef scard_pack_array_constructed<
        scard_pack_pci, 1,
        scard_pack_array_type_fixed,
        scard_pack_pointer_type_unique,
        SCARD_PACK_DWORD
    > pci_array_type;
    
    typedef scard_pack_array_primitive<
        uint8_t, 66560,
        scard_pack_array_type_conformant,
        scard_pack_pointer_type_unique,
        SCARD_PACK_DWORD
    > buffer_array_type;

    SCARD_PACK_LONG     _ReturnCode;
    pci_array_type      _pioRecvPci;
    SCARD_PACK_DWORD    _cbRecvLength;
    buffer_array_type   _pbRecvBuffer;

public:
    ///
    scard_pack_return_transmit();

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    const scard_pack_pci * pioRecvPci() const;

    ///
    SCARD_PACK_DWORD cbRecvLength() const;

    ///
    const uint8_t * pbRecvBuffer() const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual void log(int level) const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// ListReaderGroups /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_list_reader_groups : public scard_pack_call
{
	scard_pack_context              _Context;
	SCARD_PACK_LONG                 _fmszGroupsIsNULL;
	SCARD_PACK_DWORD                _cchGroups;

    const bool                      _unicode;

public:
    ///
    explicit scard_pack_call_list_reader_groups(
        SCARD_PACK_NATIVE_CONTEXT hContext,
        const char *mszGroups, SCARD_PACK_DWORD pcchGroups,
        bool unicode);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_list_reader_groups : public scard_pack_return
{
    typedef scard_pack_array_primitive<
        char, 65536,
        scard_pack_array_type_conformant,
        scard_pack_pointer_type_unique,
        SCARD_PACK_DWORD
    > groups_array_type;

    SCARD_PACK_LONG                 _ReturnCode;
    SCARD_PACK_DWORD                _cBytes;
    groups_array_type               _mszGroups;

    const bool                      _unicode;

public:
    ///
    explicit scard_pack_return_list_reader_groups(
        const bool unicode);

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    SCARD_PACK_DWORD cBytes() const;

    ///
    const char * mszGroups() const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual void log(int level) const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// ListReaders //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_list_readers : public scard_pack_call
{
    typedef scard_pack_array_primitive<
        uint8_t, 65536,
        scard_pack_array_type_conformant,
        scard_pack_pointer_type_unique,
        SCARD_PACK_DWORD
    > groups_array_type;

	scard_pack_context              _Context;
    SCARD_PACK_DWORD                _cBytes;
    groups_array_type               _mszGroups;
	SCARD_PACK_LONG                 _fmszReadersIsNULL;
	SCARD_PACK_DWORD                _cchReaders;

    const bool                      _unicode;

public:
    ///
    explicit scard_pack_call_list_readers(
        SCARD_PACK_NATIVE_CONTEXT hContext,
        const char *mszGroups, char *mszReaders,
        SCARD_PACK_DWORD cchReaders,
        bool unicode);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_list_readers : public scard_pack_return
{
    typedef scard_pack_array_primitive<
        char, 65536,
        scard_pack_array_type_conformant,
        scard_pack_pointer_type_unique,
        SCARD_PACK_DWORD
    > readers_array_type;

    SCARD_PACK_LONG                 _ReturnCode;
    SCARD_PACK_DWORD                _cBytes;
    readers_array_type              _mszReaders;

    const bool                      _unicode;

public:
    ///
    explicit scard_pack_return_list_readers(
        const bool unicode);

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    SCARD_PACK_DWORD cBytes() const;

    ///
    const char * mszReaders() const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual void log(int level) const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// Cancel ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_cancel : public scard_pack_call
{
    scard_pack_context  _Context;

public:
    ///
    scard_pack_call_cancel(SCARD_PACK_NATIVE_CONTEXT hContext);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_cancel : public scard_pack_return
{
    SCARD_PACK_LONG     _ReturnCode;

public:
    ///
    scard_pack_return_cancel();

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual void log(int level) const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// GetAttrib ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_get_attrib : public scard_pack_call
{
    scard_pack_context  _Context;
    scard_pack_handle   _Card;
    SCARD_PACK_DWORD    _dwAttrId;
    SCARD_PACK_LONG     _fpbAttrIsNULL;
    SCARD_PACK_DWORD    _cbAttrLen;

public:
    ///
    scard_pack_call_get_attrib(SCARD_PACK_NATIVE_CONTEXT hContext,
        SCARD_PACK_NATIVE_HANDLE hCard, SCARD_PACK_NATIVE_DWORD dwAttrId,
        const SCARD_PACK_BYTE *pbAttr, const SCARD_PACK_NATIVE_DWORD *pcbAttrLen);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_get_attrib : public scard_pack_return
{
    typedef scard_pack_array_primitive<
        uint8_t, 66560,
        scard_pack_array_type_conformant,
        scard_pack_pointer_type_unique,
        SCARD_PACK_DWORD
    > attr_array_type;

    SCARD_PACK_LONG     _ReturnCode;
    SCARD_PACK_DWORD    _cbAttrLen;
    attr_array_type     _pbAttr;

public:
    ///
    scard_pack_return_get_attrib();

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    SCARD_PACK_DWORD cbAttrLen() const;

    ///
    const SCARD_PACK_BYTE * pbAttr() const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual void log(int level) const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// SetAttrib ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_call_set_attrib : public scard_pack_call
{
    typedef scard_pack_array_primitive<
        uint8_t, 65536,
        scard_pack_array_type_conformant,
        scard_pack_pointer_type_unique,
        SCARD_PACK_DWORD
    > attr_array_type;

    scard_pack_context  _Context;
    scard_pack_handle   _Card;
    SCARD_PACK_DWORD    _dwAttrId;
    SCARD_PACK_DWORD    _cbAttrLen;
    attr_array_type     _pbAttr;

public:
    ///
    scard_pack_call_set_attrib(SCARD_PACK_NATIVE_CONTEXT hContext,
        SCARD_PACK_NATIVE_HANDLE hCard, SCARD_PACK_DWORD dwAttrId,
        const SCARD_PACK_BYTE *pbAttr, SCARD_PACK_DWORD cbAttrLen);

    ///
    virtual std::size_t pack(OutStream &stream) const override;

    ///
    virtual uint32_t io_control_code() const override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void log(int level) const override;
};

class scard_pack_return_set_attrib : public scard_pack_return
{
    SCARD_PACK_LONG     _ReturnCode;

public:
    ///
    scard_pack_return_set_attrib();

    ///
    SCARD_PACK_NATIVE_LONG ReturnCode() const;

    ///
    virtual std::size_t unpack(InStream &stream) override;

    ///
    virtual void log(int level) const override;
};


///////////////////////////////////////////////////////////////////////////////


///
bool has_more_payload_bytes_compat(InStream &stream, SCARD_PACK_LONG return_code);

///
std::size_t pad(OutStream &stream, std::size_t data_size);

///
std::size_t unpad(InStream &stream, std::size_t data_size);

///
std::size_t get_padding_size(std::size_t data_size);

#define SCARD_PACK_CHECK_STREAM_START(stream, packable, eid) \
    check_stream_start((stream), (packable), __FILE__, __FUNCTION__, __LINE__, (eid));

#define SCARD_PACK_CHECK_STREAM_END(stream, eid) \
    check_stream_end((stream), __FILE__, __FUNCTION__, __LINE__, (eid));

///
void check_stream_start(const InStream &stream,
    const scard_pack_return *return_, const char *file,
    const char *function, int line, error_type eid);

///
void check_stream_start(const OutStream &stream,
    const scard_pack_call *call, const char *file,
    const char *function, int line, error_type eid);

///
void check_stream_end(const InStream &stream,
    const char *file, const char *function, int line,
    error_type eid);
