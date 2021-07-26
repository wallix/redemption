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

#include <cstdint>
#include <string_view>
#include <vector>

#include "scard/scard_pack_common.hpp"
#include "scard/scard_pack_type_internal.hpp"
#include "scard/scard_pack_type_native.hpp"
#include "scard/scard_pack_type_array.hpp"
#include "scard/scard_pack_type_string.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/bytes_view.hpp"


///////////////////////////////////////////////////////////////////////////////


template< typename NativeType>
class scard_pack_type_native_convertible
{
public:
    typedef scard_pack_pointer_index_type pointer_index_type;
    typedef NativeType native_type;
    typedef NativeType * native_pointer;
    typedef const NativeType * native_const_pointer;
    typedef NativeType & native_reference;
    typedef const NativeType & native_const_reference;

    ///
    virtual ~scard_pack_type_native_convertible() = default;

    ///
    virtual std::size_t pack(OutStream &stream,
        pointer_index_type &pointer_index) const = 0;

    ///
    virtual std::size_t pack_deferred(OutStream &stream,
        pointer_index_type &pointer_index) const = 0;
    
    ///
    virtual std::size_t unpack(InStream &stream,
        pointer_index_type &pointer_index) = 0;

    ///
    virtual std::size_t unpack_deferred(InStream &stream,
        pointer_index_type &pointer_index) = 0;
    
    ///
    virtual std::size_t packed_size() const = 0;

    ///
    virtual std::size_t min_packed_size() const = 0;

    ///
    virtual void from_native(native_const_reference native_value) = 0;

    ///
    virtual void to_native(native_reference native_value) const = 0;

    ///
    virtual std::string repr() const = 0;

    ///
    virtual void log(int level) const = 0;
};


///////////////////////////////////////////////////////////////////////////////
//////// scard_pack_context  //////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_context : public scard_pack_type_native_convertible<SCARD_PACK_NATIVE_CONTEXT>
{
    typedef scard_pack_array_primitive<
        uint8_t, 16,
        scard_pack_array_type_conformant,
        scard_pack_pointer_type_unique,
        SCARD_PACK_DWORD
    > context_array_type;

    SCARD_PACK_DWORD        _cbContext;
    context_array_type      _pbContext;

public:
    enum { none_value = 0 };

    ///
    scard_pack_context();

    ///
    scard_pack_context(native_type native_context);

    ///
    SCARD_PACK_DWORD cbContext() const;

    ///
    context_array_type::const_pointer pbContext() const;

    ///
    bytes_view Context() const;

    ///
    virtual std::size_t pack(OutStream &stream,
        pointer_index_type &pointer_index) const override;

    ///
    virtual std::size_t pack_deferred(OutStream &stream,
        pointer_index_type &pointer_index) const override;

    ///
    virtual std::size_t unpack(InStream &stream,
        pointer_index_type &pointer_index) override;

    ///
    virtual std::size_t unpack_deferred(InStream &stream,
        pointer_index_type &pointer_index) override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void from_native(native_const_reference native_value) override;

    ///
    virtual void to_native(native_reference native_value) const override;

    ///
    native_type to_native() const;

    ///
    virtual void log(int level) const override;

    ///
    virtual std::string repr() const override;
};

///////////////////////////////////////////////////////////////////////////////
//////// scard_pack_handle  ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_handle : public scard_pack_type_native_convertible<SCARD_PACK_NATIVE_HANDLE>
{
    typedef scard_pack_array_primitive<
        uint8_t, 16,
        scard_pack_array_type_conformant,
        scard_pack_pointer_type_unique,
        SCARD_PACK_DWORD
    > handle_array_type;

    SCARD_PACK_DWORD        _cbHandle;
    handle_array_type       _pbHandle;

public:
    enum { none_value = 0 };

    ///
    scard_pack_handle();

    ///
    scard_pack_handle(native_type native_handle);

    ///
    SCARD_PACK_DWORD cbHandle() const;

    ///
    handle_array_type::const_pointer pbHandle() const;

    ///
    bytes_view Handle() const;

    ///
    virtual std::size_t pack(OutStream &stream,
        pointer_index_type &pointer_index) const override;

    ///
    virtual std::size_t pack_deferred(OutStream &stream,
        pointer_index_type &pointer_index) const override;

    ///
    virtual std::size_t unpack(InStream &stream,
        pointer_index_type &pointer_index) override;

    ///
    virtual std::size_t unpack_deferred(InStream &stream,
        pointer_index_type &pointer_index) override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    virtual void from_native(native_const_reference native_value) override;

    ///
    virtual void to_native(native_reference native_value) const override;

    ///
    native_type to_native() const;

    ///
    virtual void log(int level) const override;

    ///
    virtual std::string repr() const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// scard_pack_reader_state  /////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_reader_state : public scard_pack_type_native_convertible<SCARD_READERSTATE>
{
    typedef std::array<
        uint8_t, 36
    > atr_array_type;

    scard_pack_string   _szReader;
    SCARD_PACK_DWORD    _dwCurrentState;
    SCARD_PACK_DWORD    _dwEventState;
    SCARD_PACK_DWORD    _cbAtr;
    atr_array_type      _rgbAtr;

public:
    ///
    scard_pack_reader_state();

    ///
    scard_pack_reader_state(native_const_reference native_value);

    ///
    virtual ~scard_pack_reader_state() = default;

    ///
    std::string_view Reader() const;

    ///
    SCARD_PACK_DWORD dwCurrentState() const;

    ///
    SCARD_PACK_DWORD dwEventState() const;

    ///
    SCARD_PACK_DWORD cbAtr() const;

    ///
    bytes_view rgbAtr() const;

    ///
    virtual std::size_t pack(OutStream &stream,
        pointer_index_type &pointer_index) const override;

    ///
    virtual std::size_t pack_deferred(OutStream &stream,
        pointer_index_type &pointer_index) const override;

    ///
    virtual std::size_t unpack(InStream &stream,
        pointer_index_type &pointer_index) override;

    ///
    virtual std::size_t unpack_deferred(InStream &stream,
        pointer_index_type &pointer_index) override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    /*
    virtual std::size_t max_packed_size() const override;
    */

    ///
    virtual void from_native(native_const_reference native_value) override;

    ///
    virtual void to_native(native_reference native_value) const override;

    ///
    virtual void log(int level) const override;

    ///
    virtual std::string repr() const override;
};


///////////////////////////////////////////////////////////////////////////////
//////// scard_pack_pci ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class scard_pack_pci : public scard_pack_type_native_convertible<scard_pack_native_pci>
{
    typedef scard_pack_array_primitive<
        uint8_t, 1024,
        scard_pack_array_type_conformant,
        scard_pack_pointer_type_unique,
        SCARD_PACK_DWORD
    > extra_bytes_array_type;

    SCARD_PACK_DWORD        _dwProtocol;
    SCARD_PACK_DWORD        _cbExtraBytes;
    extra_bytes_array_type  _pbExtraBytes;

public:
    ///
    scard_pack_pci();

    ///
    scard_pack_pci(native_const_reference native_value);

    ///
    virtual ~scard_pack_pci() = default;

    ///
    SCARD_PACK_DWORD dwProtocol() const;

    ///
    SCARD_PACK_DWORD cbExtraBytes() const;

    ///
    extra_bytes_array_type::const_pointer pbExtraBytes() const;

    ///
    bytes_view ExtraBytes() const;

    ///
    virtual std::size_t pack(OutStream &stream,
        pointer_index_type &pointer_index) const override;

    ///
    virtual std::size_t pack_deferred(OutStream &stream,
        pointer_index_type &pointer_index) const override;

    ///
    virtual std::size_t unpack(InStream &stream,
        pointer_index_type &pointer_index) override;

    ///
    virtual std::size_t unpack_deferred(InStream &stream,
        pointer_index_type &pointer_index) override;

    ///
    virtual std::size_t packed_size() const override;

    ///
    virtual std::size_t min_packed_size() const override;

    ///
    /*
    virtual std::size_t max_packed_size() const override;
    */

    ///
    virtual void from_native(native_const_reference native_value) override;

    ///
    virtual void to_native(native_reference native_value) const override;

    ///
    void to_native(SCARD_PACK_NATIVE_PCI_HEADER *native_value) const;

    ///
    virtual void log(int level) const override;

    ///
    virtual std::string repr() const override;
};