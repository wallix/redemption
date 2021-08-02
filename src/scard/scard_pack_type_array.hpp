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
#include <limits>
#include <type_traits>
#include <vector>

#include "scard/scard_pack_common.hpp"
#include "scard/scard_utils.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/stream.hpp"


///////////////////////////////////////////////////////////////////////////////


///
typedef uint32_t scard_pack_pointer_index_type;

///
enum scard_pack_pointer_type
{
    scard_pack_pointer_type_full,
    scard_pack_pointer_type_reference,
    scard_pack_pointer_type_unique
};

///
enum scard_pack_array_type
{
    scard_pack_array_type_conformant,
    scard_pack_array_type_conformant_varying,
    scard_pack_array_type_fixed,
    scard_pack_array_type_varying
};

enum
{
    scard_pack_array_size_auto = 0
};

enum : uint32_t
{
    scard_pack_array_referent_id_base = 0x00020000,
    scard_pack_array_referent_id_null = 0x00000000
};

template<
    typename PrimitiveType,
    std::size_t MaxElementCount,
    scard_pack_array_type ArrayType,
    scard_pack_pointer_type PointerType,
    typename SizeType
>
class scard_pack_array_primitive
{
    static_assert(std::is_fundamental<PrimitiveType>::value,
        "PrimitiveType must be a fundamental type.");

private:
    typedef std::vector<
        PrimitiveType
    > container_type;

public:
    typedef scard_pack_pointer_index_type pointer_index_type;
    typedef SizeType size_type;
    typedef PrimitiveType value_type;
    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;
    typedef typename container_type::reference reference;
    typedef typename container_type::const_reference const_reference;
    typedef typename container_type::pointer pointer;
    typedef typename container_type::const_pointer const_pointer;

private:
    scard_pack_array_type   _type;
    size_type               _max_size;
    bool                    _nil;
    container_type          _elements;

public:
    ///
    scard_pack_array_primitive()
        :
        _type(ArrayType),
        _max_size(MaxElementCount == scard_pack_array_size_auto ?
            std::numeric_limits<size_type>::max() : MaxElementCount),
        _nil(true)
    {
    }

    ///
    scard_pack_array_primitive(size_type count, const value_type &value)
        :
        _type(ArrayType),
        _max_size(MaxElementCount == scard_pack_array_size_auto ?
            std::numeric_limits<size_type>::max() : MaxElementCount),
        _nil(false)
    {
        assign(count, value);
    }

    ///
    scard_pack_array_primitive(const_pointer data, size_type size)
        :
        _type(ArrayType),
        _max_size(MaxElementCount == scard_pack_array_size_auto ?
            std::numeric_limits<size_type>::max() : MaxElementCount),
        _nil(false)
    {
        assign(data, size);
    }

    ///
    iterator begin()
    {
        return _elements.begin();
    }

    ///
    iterator end()
    {
        return _elements.end();
    }

    ///
    const_iterator begin() const
    {
        return _elements.cbegin();
    }

    ///
    const_iterator end() const
    {
        return _elements.cend();
    }

    ///
    const_pointer ptr() const
    {
        return _elements.data();
    }

    ///
    pointer ptr()
    {
        return _elements.data();
    }

    ///
    array_view<value_type> data() const
    {
        return _elements;
    }

    ///
    array_view<value_type> data()
    {
        return _elements;
    }

    ///
    bool empty() const
    {
        return _elements.empty();
    }

    ///
    size_type size() const
    {
        return _elements.size();
    }

    ///
    size_type max_size() const
    {
        return _max_size;
    }

    ///
    const_reference at(std::size_t index) const
    {
        return _elements.at(index);
    }

    ///
    reference at(std::size_t index)
    {
        return _elements.at(index);
    }

    ///
    bool is_nil() const
    {
        return _nil;
    }

    ///
    void assign(size_type count, const value_type &value)
    {
        SCARD_CHECK_PREDICATE(count <= _max_size,
            ERR_SCARD);
        
        _elements.assign(count, value);
        _nil = false;
    }

    ///
    void assign(const_pointer data, size_type size)
    {
        SCARD_CHECK_PREDICATE(size <= _max_size,
            ERR_SCARD);
        
        if (data)
        {
            _elements.assign(data, data + size);
            _nil = false;
        }
        else
        {
            _elements.clear();
            _nil = true;
        }
    }

    ///
    std::size_t packed_size() const
    {
        std::size_t size;

        // pointer
        size = sizeof(uint32_t);

        if (!_nil)
        {
            // array header
            switch (_type)
            {
                case scard_pack_array_type_conformant:
                {
                    size += sizeof(uint32_t);
                    break;
                }
                case scard_pack_array_type_varying:
                {
                    size += (2 * sizeof(uint32_t));
                    break;
                }
                case scard_pack_array_type_conformant_varying:
                {
                    size += (3 * sizeof(uint32_t));
                    break;
                }
                case scard_pack_array_type_fixed:
                default:
                    break;
            }

            const auto elements_size = _elements.size() * sizeof(value_type);
            
            // array elements
            size += elements_size;

            // padding
            size += get_padding_size(elements_size);
        }
        
        return size;
    }

    ///
    std::size_t min_packed_size() const
    {
        return (
            sizeof(uint32_t)
        );
    }

    ///
    std::size_t pack_size(OutStream &stream, size_type size) const
    {
        std::size_t n_packed;

        SCARD_CHECK_PREDICATE(size == _elements.size(),
            ERR_SCARD);

        // size (4 bytes)
        stream.out_uint32_le(size);
        n_packed = sizeof(uint32_t);

        return n_packed;
    }

    ///
    std::size_t pack_pointer(OutStream &stream,
        pointer_index_type &pointer_index) const
    {
        std::size_t n_packed;

        // referent ID (4 bytes)
        stream.out_uint32_le(_nil ? scard_pack_array_referent_id_null :
            scard_pack_array_referent_id_base + 4 * pointer_index++);
        n_packed = sizeof(uint32_t);

        return n_packed;
    }

    ///
    std::size_t pack_elements(OutStream &stream) const
    {
        return pack_elements(stream, _elements.size());
    }

    ///
    std::size_t pack_elements(OutStream &stream, size_type count) const
    {
        SCARD_CHECK_PREDICATE(count <= _elements.size(),
            ERR_SCARD);

        if (_nil)
        {
            return 0;
        }

        std::size_t n_packed(0);

        // array header
        if ((_type == scard_pack_array_type_conformant)
            || (_type == scard_pack_array_type_conformant_varying))
        {
            // Maximum Count (4 bytes) (see [C706] §14.3.3.2 and §14.3.3.4)
            stream.out_uint32_le(_elements.size());
            n_packed += sizeof(uint32_t);
        }
        if ((_type == scard_pack_array_type_varying)
            || (_type == scard_pack_array_type_conformant_varying))
        {
            // Offset (4 bytes) (see [C706] §14.3.3.3 and §14.3.3.4)
            stream.out_uint32_le(0);
            n_packed += sizeof(uint32_t);

            // Actual Count (4 bytes) (see [C706] §14.3.3.2 and §14.3.3.4)
            stream.out_uint32_le(_elements.size());
            n_packed += sizeof(uint32_t);
        }

        const auto elements_size = _elements.size();
        const auto data_size = elements_size * sizeof(value_type);

        // array elements
        stream.out_copy_bytes(reinterpret_cast<const uint8_t *>(
            _elements.data()), data_size);
        n_packed += elements_size;

        // padding
        n_packed += pad(stream, elements_size);

        return n_packed;
    }

    ///
    std::size_t unpack_size(InStream &stream, size_type &size)
    {
        std::size_t n_unpacked;

        // size (4 bytes)
        const auto size_ = stream.in_uint32_le();
        n_unpacked = sizeof(uint32_t);

        SCARD_CHECK_PREDICATE(size_ <= _max_size,
            ERR_SCARD);
        
        size = size_;

        return n_unpacked;
    }

    ///
    std::size_t unpack_pointer(InStream &stream,
        pointer_index_type &/*pointer_index*/)
    {
        std::size_t n_unpacked;

        // referent ID (4 bytes)
        const auto referent_id = stream.in_uint32_le();
        n_unpacked = sizeof(uint32_t);

        _nil = (referent_id == scard_pack_array_referent_id_null);

        return n_unpacked;
    }

    ///
    std::size_t unpack_elements(InStream &stream)
    {
        return unpack_elements(stream, _elements.size());
    }

    ///
    std::size_t unpack_elements(InStream &stream, size_type /*count*/)
    {
        if (_nil)
        {
            return 0;
        }
        
        std::size_t n_unpacked(0);
        uint32_t size(_max_size);

        // array header
        if ((_type == scard_pack_array_type_conformant)
            || (_type == scard_pack_array_type_conformant_varying))
        {
            // Maximum Count (4 bytes) (see [C706] §14.3.3.2 and §14.3.3.4)
            size = stream.in_uint32_le();
            n_unpacked += sizeof(uint32_t);
        }
        if ((_type == scard_pack_array_type_varying)
            || (_type == scard_pack_array_type_conformant_varying))
        {
            // Offset (4 bytes) (see [C706] §14.3.3.3 and §14.3.3.4)
            const auto offset = stream.in_uint32_le();
            n_unpacked += sizeof(uint32_t);

            SCARD_CHECK_PREDICATE(offset == 0,
                ERR_SCARD);

            // Actual Count (4 bytes) (see [C706] §14.3.3.2 and §14.3.3.4)
            size = stream.in_uint32_le();
            n_unpacked += sizeof(uint32_t);
        }

        SCARD_CHECK_PREDICATE(size <= _max_size,
            ERR_SCARD);

        // allocate memory
        _elements.resize(size);

        const auto data_size = size * sizeof(value_type);

        // array elements
        stream.in_copy_bytes(reinterpret_cast<uint8_t *>(
            _elements.data()), data_size);
        n_unpacked += data_size;

        // padding
        n_unpacked += unpad(stream, size);

        return n_unpacked;
    }

    ///
    void log(int level) const
    {
        LOG(level, "%s", bytes_to_hex_string(_elements).c_str());
    }

private:
    ///
    std::size_t pad(OutStream &stream, std::size_t referent_size) const
    {
        std::size_t padding_size;

        // calculate padding size
        padding_size = get_padding_size(referent_size);

        // do pad
        // stream.out_skip_bytes not used here to prevent leaks
        auto size = padding_size;
        while (size--)
        {
            stream.out_uint8(0x00);
        }

        return padding_size;
    }

    ///
    std::size_t unpad(InStream &stream, std::size_t referent_size) const
    {
        std::size_t padding_size;

        // calculate padding size
        padding_size = get_padding_size(referent_size);

        // do unpad
        stream.in_skip_bytes(padding_size);

        return padding_size;
    }

    ///
    std::size_t get_padding_size(std::size_t referent_size) const
    {
        return ((referent_size % 4) ? (4 - (referent_size % 4)) : 0);
    }
};

template<
    typename VirtualType,
    std::size_t MaxElementCount,
    scard_pack_array_type ArrayType,
    scard_pack_pointer_type PointerType,
    typename SizeType
>
class scard_pack_array_constructed
{
private:
    typedef std::vector<
        VirtualType
    > container_type;

    typedef typename VirtualType::native_type native_type;
    typedef typename VirtualType::native_const_pointer native_const_pointer;

public:
    typedef scard_pack_pointer_index_type pointer_index_type;
    typedef SizeType size_type;
    typedef VirtualType value_type;
    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;
    typedef typename container_type::reference reference;
    typedef typename container_type::const_reference const_reference;
    typedef typename container_type::pointer pointer;
    typedef typename container_type::const_pointer const_pointer;

private:
    scard_pack_array_type   _type;
    size_type               _max_size;
    bool                    _nil;
    container_type          _elements;

public:
    ///
    scard_pack_array_constructed()
        :
        _type(ArrayType),
        _max_size(MaxElementCount == scard_pack_array_size_auto ?
            std::numeric_limits<size_type>::max() : MaxElementCount),
        _nil(true)
    {
    }

    ///
    scard_pack_array_constructed(size_type count, const value_type &value)
        :
        _type(ArrayType),
        _max_size(MaxElementCount == scard_pack_array_size_auto ?
            std::numeric_limits<size_type>::max() : MaxElementCount),
        _nil(false)
    {
        assign(count, value);
    }

    ///
    scard_pack_array_constructed(native_const_pointer data, size_type size)
        :
        _type(ArrayType),
        _max_size(MaxElementCount == scard_pack_array_size_auto ?
            std::numeric_limits<size_type>::max() : MaxElementCount),
        _nil(false)
    {
        assign(data, size);
    }

    ///
    iterator begin()
    {
        return _elements.begin();
    }

    ///
    iterator end()
    {
        return _elements.end();
    }

    ///
    const_iterator begin() const
    {
        return _elements.cbegin();
    }

    ///
    const_iterator end() const
    {
        return _elements.cend();
    }

    array_view<value_type> data() const
    {
        return _elements;
    }

    array_view<value_type> data()
    {
        return _elements;
    }

    ///
    bool empty() const
    {
        return _elements.empty();
    }

    ///
    size_type size() const
    {
        return _elements.size();
    }

    ///
    size_type max_size() const
    {
        return _max_size;
    }

    ///
    const_reference at(std::size_t index) const
    {
        return _elements.at(index);
    }

    ///
    reference at(std::size_t index)
    {
        return _elements.at(index);
    }

    ///
    bool is_nil() const
    {
        return _nil;
    }

    ///
    void assign(size_type count, const value_type &value)
    {
        SCARD_CHECK_PREDICATE(count <= _max_size,
            ERR_SCARD);
        
        _elements.assign(count, value);
        _nil = false;
    }

    ///
    void assign(native_const_pointer data, size_type size)
    {
        SCARD_CHECK_PREDICATE(size <= _max_size,
            ERR_SCARD);
        
        if (data)
        {
            _elements.assign(data, data + size);
            _nil = false;
        }
        else
        {
            _elements.clear();
            _nil = true;
        }
    }

    ///
    std::size_t packed_size() const
    {
        std::size_t size;

        // pointer
        size = sizeof(uint32_t);

        if (!_nil)
        {
            // array header
            switch (_type)
            {
                case scard_pack_array_type_conformant:
                {
                    size += sizeof(uint32_t);
                    break;
                }
                case scard_pack_array_type_varying:
                {
                    size += (2 * sizeof(uint32_t));
                    break;
                }
                case scard_pack_array_type_conformant_varying:
                {
                    size += (3 * sizeof(uint32_t));
                    break;
                }
                case scard_pack_array_type_fixed:
                default:
                    break;
            }
            
            // array elements
            {
                for (const auto &element : _elements)
                {
                    size += element.packed_size();
                }
            }
        }

        return size;
    }

    ///
    std::size_t min_packed_size() const
    {
        return (
            sizeof(uint32_t)
        );
    }

    ///
    std::size_t pack_size(OutStream &stream, size_type size) const
    {
        std::size_t n_packed;

        SCARD_CHECK_PREDICATE(size == _elements.size(),
            ERR_SCARD);

        // size (4 bytes)
        stream.out_uint32_le(size);
        n_packed = sizeof(uint32_t);

        return n_packed;
    }

    ///
    std::size_t pack_pointer(OutStream &stream,
        pointer_index_type &pointer_index) const
    {
        std::size_t n_packed;

        // referent ID (4 bytes)
        stream.out_uint32_le(_nil ? scard_pack_array_referent_id_null :
            scard_pack_array_referent_id_base + 4 * pointer_index++);
        n_packed = sizeof(uint32_t);

        return n_packed;
    }

    ///
    std::size_t pack_elements(OutStream &stream,
        pointer_index_type &pointer_index) const
    {
        return pack_elements(stream, _elements.size(), pointer_index);
    }

    ///
    std::size_t pack_elements(OutStream &stream, size_type count,
        pointer_index_type &pointer_index) const
    {
        SCARD_CHECK_PREDICATE(count <= _elements.size(),
            ERR_SCARD);

        if (_nil)
        {
            return 0;
        }

        std::size_t n_packed(0);

        // array header
        if ((_type == scard_pack_array_type_conformant)
            || (_type == scard_pack_array_type_conformant_varying))
        {
            // Maximum Count (4 bytes) (see [C706] §14.3.3.2 and §14.3.3.4)
            stream.out_uint32_le(_elements.size());
            n_packed += sizeof(uint32_t);
        }
        if ((_type == scard_pack_array_type_varying)
            || (_type == scard_pack_array_type_conformant_varying))
        {
            // Offset (4 bytes) (see [C706] §14.3.3.3 and §14.3.3.4)
            stream.out_uint32_le(0);
            n_packed += sizeof(uint32_t);

            // Actual Count (4 bytes) (see [C706] §14.3.3.2 and §14.3.3.4)
            stream.out_uint32_le(_elements.size());
            n_packed += sizeof(uint32_t);
        }

        // array elements
        for (const auto &element : _elements)
        {
            n_packed += element.pack(stream, pointer_index);
        }
        for (const auto &element : _elements)
        {
            n_packed += element.pack_deferred(stream, pointer_index);
        }

        return n_packed;
    }

    ///
    std::size_t unpack_size(InStream &stream, size_type &size)
    {
        std::size_t n_unpacked;

        // size (4 bytes)
        const auto size_ = stream.in_uint32_le();
        n_unpacked = sizeof(uint32_t);

        SCARD_CHECK_PREDICATE(size_ <= _max_size,
            ERR_SCARD);
        
        size = size_;

        return n_unpacked;
    }

    ///
    std::size_t unpack_pointer(InStream &stream,
        pointer_index_type &/*pointer_index*/)
    {
        std::size_t n_unpacked;

        // referent ID (4 bytes)
        const auto referent_id = stream.in_uint32_le();
        n_unpacked = sizeof(uint32_t);

        _nil = (referent_id == scard_pack_array_referent_id_null);

        return n_unpacked;
    }

    ///
    std::size_t unpack_elements(InStream &stream,
        pointer_index_type &pointer_index)
    {
        return unpack_elements(stream, _elements.size(), pointer_index);
    }

    ///
    std::size_t unpack_elements(InStream &stream, size_type /*count*/,
        pointer_index_type &pointer_index)
    {
        if (_nil)
        {
            return 0;
        }

        std::size_t n_unpacked(0);
        uint32_t size(_max_size);

        // array header
        if ((_type == scard_pack_array_type_conformant)
            || (_type == scard_pack_array_type_conformant_varying))
        {
            // Maximum Count (4 bytes) (see [C706] §14.3.3.2 and §14.3.3.4)
            size = stream.in_uint32_le();
            n_unpacked += sizeof(uint32_t);
        }
        if ((_type == scard_pack_array_type_varying)
            || (_type == scard_pack_array_type_conformant_varying))
        {
            // Offset (4 bytes) (see [C706] §14.3.3.3 and §14.3.3.4)
            const auto offset = stream.in_uint32_le();
            n_unpacked += sizeof(uint32_t);

            SCARD_CHECK_PREDICATE(offset == 0,
                ERR_SCARD);

            // Actual Count (4 bytes) (see [C706] §14.3.3.2 and §14.3.3.4)
            size = stream.in_uint32_le();
            n_unpacked += sizeof(uint32_t);
        }

        SCARD_CHECK_PREDICATE(size <= _max_size,
            ERR_SCARD);

        // allocate memory
        _elements.resize(size);

        // array elements
        for (auto &element : _elements)
        {
            n_unpacked += element.unpack(stream, pointer_index);
        }
        for (auto &element : _elements)
        {
            n_unpacked += element.unpack_deferred(stream, pointer_index);
        }

        return n_unpacked;
    }

    ///
    void log(int level) const
    {
        for (const auto &element : _elements)
        {
            element.log(level);
        }
    }

private:
    ///
    std::size_t pad(OutStream &stream, std::size_t referent_size) const
    {
        std::size_t padding_size;

        // calculate padding size
        padding_size = get_padding_size(referent_size);

        // do pad
        // stream.out_skip_bytes not used here to prevent leaks
        auto size = padding_size;
        while (size--)
        {
            stream.out_uint8(0x00);
        }

        return padding_size;
    }

    ///
    std::size_t unpad(InStream &stream, std::size_t referent_size) const
    {
        std::size_t padding_size;

        // calculate padding size
        padding_size = get_padding_size(referent_size);

        // do unpad
        stream.in_skip_bytes(padding_size);

        return padding_size;
    }

    ///
    std::size_t get_padding_size(std::size_t referent_size) const
    {
        return ((referent_size % 4) ? (4 - (referent_size % 4)) : 0);
    }
};