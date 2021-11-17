/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "core/RDP/nla/ber.hpp"
#include "core/error.hpp"
#include "utils/log.hpp"
#include "utils/hexdump.hpp"


// BER Encoding Cheat Sheet
// ========================

// BER INTEGER
// -----------
// Only the 4 first patterns are canonical,
// next 4 are using non packed integer length encoding (pretty inefficient)
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 01 XX]
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 02 HH LL]
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 03 HH MM LL]
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 04 HH MM LL]
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 81 01 XX]
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 81 02 HH LL]
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 81 03 HH MM LL]
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 81 04 HH MM LL]
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 82 00 01 XX]
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 82 00 02 HH LL]
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 82 00 03 HH MM LL]
// [CLASS_UNIV|PC_PRIMITIVE|TAG_INTEGER] [ 82 00 04 HH MM LL]

// BER OCTET STRING
// ----------------
// [CLASS_UNIV|PC_PRIMITIVE|TAG_OCTET_STRING] [ ll ] [ XX .. XX ]
// [CLASS_UNIV|PC_PRIMITIVE|TAG_OCTET_STRING] [ 81 LL ] [ XX .. XX]
// [CLASS_UNIV|PC_PRIMITIVE|TAG_OCTET_STRING] [ 82 LH LL ] [ XX .. XX]
// [CLASS_UNIV|PC_PRIMITIVE|TAG_OCTET_STRING] [ 83 LH LM LL ] [ XX .. XX]
// ll = length of string <= 127
// LL = length of string <= 255
// LHLL = length of string <= 65535
// LHLMLL = length of string 24 bits (unlikely, in credssp context it's probably an error)

namespace BER
{

namespace
{
    template<std::ptrdiff_t Size = 0>
    struct BerBuffer;

    template<class>
    struct BerBufferSize;

    template<std::ptrdiff_t Size>
    struct BerBufferSize<BerBuffer<Size>>
    {
        static constexpr ptrdiff_t value = Size;
    };

    template<std::ptrdiff_t a, std::ptrdiff_t b>
    inline constexpr std::ptrdiff_t max_v = (a < b) ? b : a;

    template<class T, class U>
    struct CommonBuffer
    {};

    template<std::ptrdiff_t Size1, std::ptrdiff_t Size2>
    struct CommonBuffer<BerBuffer<Size1>, BerBuffer<Size2>>
    {
        using type = BerBuffer<max_v<Size1, Size2>>;
    };

    struct BerBufferData
    {
        uint8_t* start;
        uint8_t* p;
    };

    template<std::ptrdiff_t Size>
    struct [[nodiscard]] BerBuffer
    {
        BerBufferData data;

        BerBufferData internal_data() const noexcept
        {
            return data;
        }

        BerBuffer<Size+1> push(uint8_t value) noexcept
        {
            *data.p = value;
            return {{data.start, data.p+1}};
        }

        uint32_t size() const noexcept
        {
            return static_cast<uint32_t>(data.p - data.start);
        }

        template<class F>
        auto operator | (F f) const noexcept
        {
            return f(*this);
        }
    };

    constexpr auto push = [](uint8_t value) noexcept {
        return [=](auto out) noexcept {
            return out.push(value);
        };
    };

    constexpr struct {
        auto operator[](bool b) const noexcept
        {
            return [=](auto true_f, auto false_f) noexcept {
                return [=](auto out) noexcept {
                    using Result = typename CommonBuffer<
                        decltype(true_f(out)),
                        decltype(false_f(out))
                    >::type;

                    if (b) {
                        return Result{true_f(out).internal_data()};
                    }
                    else {
                        return Result{false_f(out).internal_data()};
                    }
                };
            };
        }
    } if_else;


#define WRAP_F(body) [&](auto out) noexcept { return body; }

    template<std::size_t N>
    struct BerStaticBuffer
    {
        uint8_t array[N];
        uint32_t len;

        bytes_view av() const noexcept
        {
            return {array, len};
        }

        void reverse()
        {
            std::reverse(array, array+len);
        }
    };

    template<class F>
    auto create_buffer(F f) noexcept
    {
        using R = decltype(BerBuffer<0>() | f);
        BerStaticBuffer<BerBufferSize<R>::value> buffer;
        R r = BerBuffer<0>{buffer.array, buffer.array} | f;
        buffer.len = r.size();
        return buffer;
    }

    void insert_at_end(std::vector<uint8_t>& head, bytes_view av)
    {
        head.insert(head.end(), av.begin(), av.end());
    }

    template<class... AVs>
    auto concat_to_vector(AVs const&... avs)
    {
        std::vector<uint8_t> head;
        head.reserve((... + avs.size()));
        (..., insert_at_end(head, avs));
        return head;
    }

    template<class F>
    auto create_reversed_vector(F f)
    {
        auto buffer = create_buffer(f);
        buffer.reverse();
        std::vector<uint8_t> head{buffer.array, buffer.array + buffer.len};
        return head;
    }

    auto backward_push_ber_len(uint32_t len) noexcept
    {
        return [=](auto out) noexcept {
            return out
            | push(uint8_t(len))
            | if_else[len > 0xFF](
                WRAP_F(out
                | push(uint8_t(len >> 8))
                | push(0x82)),
                WRAP_F(out
                | if_else[len > 0x7F](
                    WRAP_F(out | push(0x81)),
                    WRAP_F(out)
                ))
            );
        };
    };

    auto backward_push_tagged_field_header(uint32_t payload_size, uint8_t tag) noexcept
    {
        return [=](auto out) noexcept {
            return out
            | backward_push_ber_len(payload_size)
            | push(CLASS_CTXT|PC_CONSTRUCT|tag);
        };
    }

    auto backward_push_primitive_type(uint8_t tag, uint32_t payload_size) noexcept
    {
        return [=](auto out) noexcept {
            return out
            | backward_push_ber_len(payload_size)
            | push(CLASS_UNIV|PC_PRIMITIVE|tag);
        };
    }

    auto backward_push_octet_string_header(uint32_t payload_size) noexcept
    {
        return backward_push_primitive_type(TAG_OCTET_STRING, payload_size);
    }

    auto backward_push_oid_header(uint32_t payload_size) noexcept
    {
        return backward_push_primitive_type(TAG_OBJECT_IDENTIFIER, payload_size);
    }

    auto backward_push_sequence_tag_field_header(uint32_t payload_size) noexcept
    {
        return [=](auto out) noexcept {
            return out
            | backward_push_ber_len(payload_size)
            | push(CLASS_UNIV|PC_CONSTRUCT|TAG_SEQUENCE_OF);
        };
    }

    // for values < 0x80
    auto backward_push_small_integer_like(uint8_t type, uint8_t value) noexcept
    {
        return [=](auto out) noexcept {
            return out
            | push(value)
            | backward_push_primitive_type(type, 1);
        };
    }

    auto backward_push_integer_like(uint8_t type, uint32_t value) noexcept
    {
        return [=](auto out) noexcept {
            return out
            | if_else[value < 0x80](
                WRAP_F(out
                | backward_push_small_integer_like(type, uint8_t(value))),
                WRAP_F(out
                | if_else[value < 0x8000](
                    // 16bits
                    WRAP_F(out
                    | push(uint8_t(value))
                    | push(uint8_t(value >> 8))
                    | backward_push_primitive_type(type, 2)),
                    WRAP_F(out
                    | if_else[value < 0x800000](
                        // 24bits
                        WRAP_F(out
                        | push(uint8_t(value))
                        | push(uint8_t(value >> 8))
                        | push(uint8_t(value >> 16))
                        | backward_push_primitive_type(type, 3)),
                        // 32bits
                        WRAP_F(out
                        | push(uint8_t(value))
                        | push(uint8_t(value >> 8))
                        | push(uint8_t(value >> 16))
                        | push(uint8_t(value >> 24))
                        | backward_push_primitive_type(type, 4))
                    ))
                ))
            );
        };
    }

    auto backward_push_integer(uint32_t value) noexcept
    {
        return backward_push_integer_like(TAG_INTEGER, value);
    }

    auto backward_push_enumerated(uint32_t value) noexcept
    {
        return backward_push_integer_like(TAG_ENUMERATED, value);
    }
} // anonymous namespace

} // namespace BER


std::vector<uint8_t> BER::mkOptionalNegoTokensHeader(uint32_t payload_size)
{
    auto impl = [&](auto out) noexcept {
        return out
        | backward_push_octet_string_header(payload_size)
        | WRAP_F(out | backward_push_tagged_field_header(payload_size + out.size(), 0))
        | WRAP_F(out | backward_push_sequence_tag_field_header(payload_size + out.size()))
        | WRAP_F(out | backward_push_sequence_tag_field_header(payload_size + out.size()))
        | WRAP_F(out | backward_push_tagged_field_header(payload_size + out.size(), 1))
        ;
    };

    if (payload_size > 0) {
        return create_reversed_vector(impl);
    }
    return std::vector<uint8_t>();
}

std::vector<uint8_t> BER::mkContextualFieldHeader(uint32_t payload_size, uint8_t tag)
{
    return create_reversed_vector(backward_push_tagged_field_header(payload_size, tag));
}

std::vector<uint8_t> BER::mkOctetStringHeader(uint32_t payload_size)
{
    return create_reversed_vector(backward_push_octet_string_header(payload_size));
}

// std::vector<uint8_t> BER::mkOidHeader(uint32_t payload_size)
// {
//     return create_reversed_vector(backward_push_oid_header(payload_size));
// }
//
// std::vector<uint8_t> BER::mkOid(bytes_view oid)
// {
//     auto buffer = create_buffer(backward_push_oid_header(checked_int(oid.size())));
//     buffer.reverse();
//
//     return concat_to_vector(buffer.av(), oid);
// }

std::vector<uint8_t> BER::mkOidField(buffer_view oid, uint8_t tag)
{
    const auto n = checked_cast<uint32_t>(oid.size());
    auto impl = [&](auto out) noexcept {
        return out
        | backward_push_oid_header(n)
        | WRAP_F(out | backward_push_tagged_field_header(n + out.size(), tag));
    };
    auto buffer = create_buffer(impl);
    buffer.reverse();

    return concat_to_vector(buffer.av(), oid);
}

std::vector<uint8_t> BER::mkMandatoryOctetStringFieldHeader(uint32_t payload_size, uint8_t tag)
{
    return create_reversed_vector([&](auto out) noexcept {
        return out
        | backward_push_octet_string_header(payload_size)
        | WRAP_F(out | backward_push_tagged_field_header(payload_size + out.size(), tag));
    });
}

std::vector<uint8_t> BER::mkOptionalOctetStringFieldHeader(uint32_t payload_size, uint8_t tag)
{
    if (payload_size > 0) {
        return mkMandatoryOctetStringFieldHeader(payload_size, tag);
    }
    return {};
}

// std::vector<uint8_t> BER::mkSmallInteger(uint8_t value)
// {
//     return create_reversed_vector(backward_push_small_integer_like(TAG_INTEGER, value));
// }

std::vector<uint8_t> BER::mkSmallIntegerField(uint8_t value, uint8_t tag)
{
    return create_reversed_vector([&](auto out) noexcept {
        return out
        | backward_push_small_integer_like(TAG_INTEGER, value)
        | WRAP_F(out | backward_push_tagged_field_header(out.size(), tag));
    });
}

// std::vector<uint8_t> mkInteger(uint32_t value)
// {
//     return create_reversed_vector(backward_push_integer(value));
// }

std::vector<uint8_t> BER::mkIntegerField(uint32_t value, uint8_t tag)
{
    return create_reversed_vector([&](auto out) noexcept {
        return out
        | backward_push_integer(value)
        | WRAP_F(out | backward_push_tagged_field_header(out.size(), tag));
    });
}


std::vector<uint8_t> BER::mkSequenceHeader(uint32_t payload_size)
{
    return create_reversed_vector(backward_push_sequence_tag_field_header(payload_size));
}

std::vector<uint8_t> BER::mkEnumeratedField(uint32_t value, uint8_t tag)
{
    return create_reversed_vector([&](auto out) noexcept {
        return out
        | backward_push_enumerated(value)
        | WRAP_F(out | backward_push_tagged_field_header(out.size(), tag));
    });
}

// std::vector<uint8_t> BER::mkAppHeader(uint32_t payload_size, uint8_t tag)
// {
//     return create_reversed_vector([&](auto out) {
//         return out
//         | backward_push_ber_len(payload_size)
//         | push(CLASS_APPL|PC_CONSTRUCT|tag);
//     });
// }


bool BER::check_ber_ctxt_tag(bytes_view s, uint8_t tag)
{
    if (s.empty()) {
        return false;
    }
    return s[0] == (CLASS_CTXT|PC_CONSTRUCT|tag);
}

static bool peek_length(bytes_view s, bool verbose, const char * message, uint32_t & length, bytes_view & ret)
{
    if (s.empty()) {
        LOG_IF(verbose, LOG_ERR, "%s: Ber parse error", message);
        return false;
    }

    uint32_t extraSize = 1;
    length = s[0];
    if (length > 0x80) {
        switch (length){
        case 0x81:
            if (s.size() < 2) {
                LOG_IF(verbose, LOG_ERR, "%s: not enough byte for length on 2 bytes", message);
                return false;
            }
            length = s[1];
            extraSize = 2;
            break;
        case 0x82:
            if (s.size() < 3) {
                LOG_IF(verbose, LOG_ERR, "%s: not enough byte for length on 3 bytes", message);
                return false;
            }
            length = static_cast<uint32_t>(s[2] | (s[1] << 8)); // uint16_be()
            extraSize = 3;
            break;
        default:
            LOG_IF(verbose, LOG_ERR, "%s: Ber parse error, length=0x%x", message, length);
            return false;
        }
    }

    if (s.size() < extraSize + length) {
        LOG_IF(verbose, LOG_ERR,
            "%s: Ber Not enough data for length on %u bytes(need=%u have=%zu)",
            message, extraSize, extraSize + length, s.size());
        return false;
    }

    ret = s.drop_front(extraSize);
    return true;
}


std::pair<uint32_t, bytes_view> BER::pop_length(bytes_view s, const char * message)
{
    bytes_view queue;
    uint32_t len;
    if (!peek_length(s, true, message, len, queue)) {
        throw Error(ERR_CREDSSP_TS_REQUEST);
    }

    return {len, queue};
}

bool BER::check_ber_app_tag(bytes_view s, uint8_t tag, bool verbose, const char * message, bytes_view & body)
{
    if (s.empty()) {
        return false;
    }

    if (s[0] != (CLASS_APPL|PC_CONSTRUCT|tag)) {
        return false;
    }

    uint32_t len;
    bytes_view queue;
    if (!peek_length(s.drop_front(1), verbose, message, len, queue))
        return false;

    body = bytes_view(queue.data(), len);
    return true;
}


bool BER::peek_construct_tag(bytes_view s, uint8_t & tag, bool verbose, const char * message, bytes_view & queue) {
    if (s.empty()) {
        LOG_IF(verbose, LOG_ERR, "%s: Ber data truncated", message);
        return false;
    }

    tag = s[0] & TAG_MASK;
    bytes_view tmp = s.drop_front(1);

    uint32_t len;
    if (!peek_length(tmp, true, message, len, queue)) {
        return false;
    }

    queue = queue.first(len);
    return true;
}


static bool peek_check_tag(bytes_view s, uint8_t tag, bool verbose, const char * message, bytes_view & queue)
{
    if (s.empty()) {
        LOG_IF(verbose, LOG_ERR, "%s: Ber data truncated", message);
        return false;
    }

    uint8_t tag_byte = s[0];
    if (tag_byte != tag) {
        LOG_IF(verbose, LOG_ERR, "%s: Ber unexpected tag", message);
        return false;
    }

    queue = s.drop_front(1);
    return true;
}

static bytes_view pop_check_tag(bytes_view s, uint8_t tag, const char * message)
{
    bytes_view ret;
    if (!peek_check_tag(s, tag, true, message, ret)) {
        throw Error(ERR_CREDSSP_TS_REQUEST);
    }

    return ret;
}

std::pair<uint32_t, bytes_view> BER::pop_tag_length(bytes_view s, uint8_t tag, const char * message)
{
    return pop_length(pop_check_tag(s, tag, message), message);
}

// std::pair<uint32_t, bytes_view> pop_app_tag(bytes_view s, uint8_t tag, const char * message) {
//     return pop_length(pop_check_tag(s, (CLASS_APPL|PC_CONSTRUCT|tag), message), message);
// }

namespace BER
{
namespace {
    std::pair<uint32_t, bytes_view> pop_integer_or_enumerated(
        bytes_view s, const char * message, uint8_t tag)
    {
        assert(tag == TAG_INTEGER || tag == TAG_ENUMERATED);

        auto [byte, queue] = pop_tag_length(s, CLASS_UNIV | PC_PRIMITIVE | tag, message);

        if (byte < 1 || byte > 4) {
            LOG(LOG_ERR, "%s: Ber unexpected %s length %u",
                message, (tag == TAG_INTEGER) ? "integer" : "enumerated", byte);
            throw Error(ERR_CREDSSP_TS_REQUEST);
        }

        // Now bytes contains length of integer value
        if (queue.size() < byte) {
            LOG(LOG_ERR, "%s: Ber %s data truncated %u",
                message, (tag == TAG_INTEGER) ? "integer" : "enumerated", byte);
            throw Error(ERR_CREDSSP_TS_REQUEST);
        }

        InStream in_s(queue);
        switch (byte) {
        default:
            break;
        case 3:
            return {in_s.in_uint24_be(), queue.drop_front(3)};
        case 2:
            return {in_s.in_uint16_be(), queue.drop_front(2)};
        case 1:
            return {in_s.in_uint8(), queue.drop_front(1)};
        }
        return {in_s.in_uint32_be(), queue.drop_front(4)};
    }
} // namespace BER
} // anonymous namespace

std::pair<uint32_t, bytes_view> BER::pop_integer(bytes_view s, const char * message)
{
    return pop_integer_or_enumerated(s, message, TAG_INTEGER);
}

std::pair<uint32_t, bytes_view> BER::pop_integer_field(bytes_view s, uint8_t tag, const char * message)
{
    auto [length, queue] = pop_tag_length(s, CLASS_CTXT|PC_CONSTRUCT|tag, message);
    if (queue.size() < length) {
        LOG(LOG_ERR, "%s: Ber tagged integer field truncated", message);
        throw Error(ERR_CREDSSP_TS_REQUEST);
    }
    return pop_integer(queue, message);
}

uint32_t BER::read_mandatory_integer(InStream & stream, uint8_t tag, const char * message)
{
    auto [length1, queue1] = BER::pop_tag_length(stream.remaining_bytes(), CLASS_CTXT|PC_CONSTRUCT|tag, message);
    stream.in_skip_bytes(stream.in_remain()-queue1.size());
    (void)length1;

    auto [ret, queue2] = BER::pop_integer(stream.remaining_bytes(), message);
    stream.in_skip_bytes(stream.in_remain()-queue2.size());
    return ret;
}

uint32_t BER::read_optional_integer(InStream & stream, uint8_t tag, const char * message)
{
    if (BER::check_ber_ctxt_tag(stream.remaining_bytes(), tag)) {
        return read_mandatory_integer(stream, tag, message);
    }
    return 0;
}

std::pair<uint32_t, bytes_view> BER::pop_enumerated(bytes_view s, const char * message)
{
    return pop_integer_or_enumerated(s, message, TAG_ENUMERATED);
}

uint32_t BER::read_mandatory_enumerated(InStream & stream, uint8_t tag, const char *message)
{
    auto [length1, queue1] = BER::pop_tag_length(stream.remaining_bytes(), CLASS_CTXT|PC_CONSTRUCT|tag, message);
    stream.in_skip_bytes(stream.in_remain()-queue1.size());
    (void)length1;

    auto [ret, queue2] = BER::pop_enumerated(stream.remaining_bytes(), message);
    stream.in_skip_bytes(stream.in_remain()-queue2.size());
    return ret;
}

void BER::read_optional_enumerated(InStream & stream, uint8_t tag, uint32_t & ret, const char * message)
{
    if (BER::check_ber_ctxt_tag(stream.remaining_bytes(), tag)) {
        ret = read_mandatory_enumerated(stream, tag, message);
    }
}


std::vector<uint8_t> BER::read_mandatory_octet_string(InStream & stream, uint8_t tag, const char * message)
{
    auto [length1, queue1] = BER::pop_tag_length(stream.remaining_bytes(), CLASS_CTXT|PC_CONSTRUCT|tag, message);
    stream.in_skip_bytes(stream.in_remain()-queue1.size());
    (void)length1;

    auto [length2, queue2] = BER::pop_tag_length(stream.remaining_bytes(), CLASS_UNIV|PC_PRIMITIVE|TAG_OCTET_STRING, message);
    stream.in_skip_bytes(stream.in_remain()-queue2.size());

    auto av = stream.in_skip_bytes(length2);
    return {av.data(), av.data()+av.size()};
}

std::vector<uint8_t> BER::read_optional_octet_string(InStream & stream, uint8_t tag, const char * message)
{
    if (BER::check_ber_ctxt_tag(stream.remaining_bytes(), tag)) {
        return read_mandatory_octet_string(stream, tag, message);
    }
    return {};
}

typedef std::vector<uint8_t> BerOID;
bool BER::peek_oid(bytes_view s, bool verbose, const char * message, BerOID & oid, bytes_view & queue)
{
    uint32_t len;
    bytes_view q1, q2;

    if (!peek_check_tag(s, CLASS_UNIV|PC_PRIMITIVE|TAG_OBJECT_IDENTIFIER, verbose, message, q1))
        return false;

    if (!peek_length(q1, verbose, message, len, q2))
        return false;

    oid = {q2.data(), q2.data() + len};
    queue = q2.drop_front(len);
    return true;
}

std::pair<BerOID, bytes_view> BER::pop_oid(bytes_view s, const char * message)
{
    BerOID oid;
    bytes_view queue;

    if (!peek_oid(s, true, message, oid, queue)) {
        throw Error(ERR_CREDSSP_TS_REQUEST);
    }

    return {oid, queue};
}

BerOID BER::pop_oid(InStream & s, const char * message)
{
    auto [bytes, queue] = pop_tag_length(s.remaining_bytes(), CLASS_UNIV | PC_PRIMITIVE | TAG_OBJECT_IDENTIFIER, message);

    if (queue.size() < bytes) {
        LOG(LOG_ERR, "%s: Ber oid data truncated %u", message, bytes);
        throw Error(ERR_CREDSSP_TS_REQUEST);
    }

    s.in_skip_bytes(s.in_remain() - queue.size());

    BerOID ret = s.in_skip_bytes(bytes).as<std::vector<uint8_t>>();
    return ret;
}


namespace BER
{

namespace serial
{

namespace
{

auto integer(uint32_t value, uint8_t tag) noexcept
{
    return [=](auto out) noexcept {
        auto n = out.size();
        return out
        | backward_push_integer(value)
        | WRAP_F(out | backward_push_tagged_field_header(out.size() - n, tag));
    };
}

auto push_bytes(bytes_view value) noexcept
{
    return [=](auto out) noexcept {
        return out.push_bytes(value);
    };
}

auto string(bytes_view value, uint8_t tag) noexcept
{
    return [=](auto out) noexcept {
        auto n = out.size();
        return out
        | push_bytes(value)
        | backward_push_octet_string_header(value.size())
        | WRAP_F(out | backward_push_tagged_field_header(out.size() - n, tag));
    };
}

auto nego_tokens(bytes_view value) noexcept
{
    return [=](auto out) noexcept {
        auto n = out.size();
        return out
        | push_bytes(value)
        | backward_push_octet_string_header(value.size())
        | WRAP_F(out | backward_push_tagged_field_header(out.size() - n, 0))
        | WRAP_F(out | backward_push_sequence_tag_field_header(out.size() - n))
        | WRAP_F(out | backward_push_sequence_tag_field_header(out.size() - n))
        | WRAP_F(out | backward_push_tagged_field_header(out.size() - n, 1))
        ;
    };
}

auto sequence_header() noexcept
{
    return [=](auto out) noexcept {
        return out | backward_push_sequence_tag_field_header(out.size());
    };
}

template<class F>
auto optional(bool b, F f) noexcept
{
    return [=](auto out) noexcept {
        using Result = decltype(f(out));
        if (b) {
            return f(out);
        }
        return Result{out.internal_data()};
    };
}

template<std::size_t Size, std::size_t BufferCount>
struct [[nodiscard]] ProcessSizeBuffer
{
    ProcessSizeBuffer(int /*dummy*/ = 0) {}

    ProcessSizeBuffer<Size+1, BufferCount> push(uint8_t value) noexcept;

    ProcessSizeBuffer<Size, BufferCount+1> push_bytes(bytes_view value) noexcept;

    int internal_data() const noexcept;

    static uint32_t size() noexcept;

    template<class F>
    auto operator | (F f) const noexcept -> decltype(f(*this));
};

template<class>
struct ProcessSizes;

template<std::size_t Size, std::size_t BufferCount>
struct ProcessSizes<ProcessSizeBuffer<Size, BufferCount>>
{
    static constexpr std::size_t static_size = Size;
    static constexpr std::size_t buffer_count = BufferCount;
};


struct [[nodiscard]] ProcessWritableBuffer
{
    struct Data
    {
        uint32_t previous_size;
        uint8_t* start;
        uint8_t* p;
        bytes_view* end_stack;

        bytes_view reversed_buffer() noexcept
        {
            std::reverse(start, p);
            return {start, p};
        }
    };

    Data data;

    Data internal_data() const noexcept
    {
        return data;
    }

    uint32_t size() const noexcept
    {
        return data.previous_size;
    }

    ProcessWritableBuffer push(uint8_t value) noexcept
    {
        *data.p = value;
        return {data.previous_size + 1, data.start, data.p + 1, data.end_stack};
    }

    ProcessWritableBuffer push_bytes(bytes_view value) noexcept
    {
        *(data.end_stack - 1) = data.reversed_buffer();
        *(data.end_stack - 2) = {value.begin(), value.end()};
        return {checked_int(data.previous_size + value.size()), data.p, data.p, data.end_stack - 2};
    }

    template<class F>
    ProcessWritableBuffer operator | (F f) const noexcept
    {
        return f(*this);
    }
};

std::vector<uint8_t> bytes_to_vec(array_view<bytes_view> ranges)
{
    std::size_t len = 0;
    for (auto range : ranges) {
        len += range.size();
    }

    std::vector<uint8_t> vec(static_cast<std::size_t>(len));
    auto* p = vec.data();
    for (auto range : ranges) {
        std::memcpy(p, range.data(), range.size());
        p += range.size();
    }

    return vec;
}

template<class F>
struct Pipe
{
    F f;

    template<class T>
    auto operator | (T x) noexcept
    {
        return f(x);
    }
};

template<class... Fs>
std::vector<uint8_t> make_vector(Fs... fs) noexcept
{
    using Sizes = ProcessSizes<decltype((Pipe<Fs>{fs} | ... | ProcessSizeBuffer<0, 0>{}))>;
    uint8_t buffer[Sizes::static_size];

    constexpr auto stack_size = Sizes::buffer_count * 2 + 1u;
    bytes_view stack[stack_size];

    auto buf = (Pipe<Fs>{fs} | ... | ProcessWritableBuffer{{0, buffer, buffer, stack + stack_size}});
    *--buf.data.end_stack = buf.data.reversed_buffer();
    return bytes_to_vec({buf.data.end_stack, stack + stack_size});
}

}

}

template<std::size_t Size1, std::size_t BufferCount1, std::size_t Size2, std::size_t BufferCount2>
struct CommonBuffer<serial::ProcessSizeBuffer<Size1, BufferCount1>, serial::ProcessSizeBuffer<Size2, BufferCount2>>
{
    using type = serial::ProcessSizeBuffer<max_v<Size1, Size2>, max_v<BufferCount1, BufferCount2>>;
};

template<>
struct CommonBuffer<serial::ProcessWritableBuffer, serial::ProcessWritableBuffer>
{
    using type = serial::ProcessWritableBuffer;
};

std::vector<uint8_t> emitTSRequest(uint32_t version,
                                   bytes_view negoTokens,
                                   bytes_view authInfo,
                                   bytes_view pubKeyAuth,
                                   uint32_t error_code,
                                   bytes_view clientNonce,
                                   bool nonce_initialized,
                                   bool verbose)
{
    auto vec = serial::make_vector(
        serial::sequence_header(),
        serial::integer(version, 0),
        serial::optional(negoTokens.size(), serial::nego_tokens(negoTokens)),
        serial::optional(authInfo.size(), serial::string(authInfo, 2)),
        serial::optional(pubKeyAuth.size(), serial::string(pubKeyAuth, 3)),
        serial::optional((version >= 3 && version == 5) && error_code != 0,
            serial::integer(error_code, 4)),
        serial::optional(version >= 5 && nonce_initialized,
            serial::string(clientNonce, 5))
    );

    if (verbose) {
        LOG(LOG_INFO, "TSRequest hexdump ---------------------------------");
        LOG(LOG_INFO, "TSRequest version %u ------------------------------", version);
        LOG(LOG_INFO, "TSRequest negoTokens ------------------------------");
        hexdump_d(negoTokens);
        LOG(LOG_INFO, "TSRequest authInfo --------------------------------");
        hexdump_d(authInfo);
        LOG(LOG_INFO, "TSRequest pubkeyAuth ------------------------------");
        hexdump_d(pubKeyAuth);
        LOG(LOG_INFO, "TSRequest error_code %u ---------------------------", error_code);
        LOG(LOG_INFO, "TSRequest clientNonce -----------------------------");
        hexdump_d(clientNonce);

        LOG(LOG_INFO, "emit TSRequest full dump---------------------------");
        hexdump_d(vec);
        LOG(LOG_INFO, "emit TSRequest hexdump -DONE-----------------------");
    }

    return vec;
}

}

#undef WRAP_F
