/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2012-2022
*   Author(s): Jonathan Poelen
*/

#include "capture/utils/pattern_searcher.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "core/error.hpp"
#include "utils/log.hpp"

#include <cstring>

#include <hs/hs.h>


struct PatternSearcher::D
{
    struct InternalPatternFound
    {
        chars_view pattern;
        unsigned long long call_id;
        unsigned long long offset_data;
    };

    unsigned long long nb_call;
    unsigned long long total_len;
    hs_database_t* db;
    hs_scratch_t* scratch;
    hs_stream_t** streams;
    PatternFound* scan_results;
    InternalPatternFound* internal_scan_results;
    unsigned nb_pattern;
    unsigned nb_stream;
    unsigned nb_pattern_kill;
    bool is_kbd;

    array_view<hs_stream_t*> av_streams() const noexcept
    {
        return array_view{streams, nb_stream};
    }
};

PatternSearcher::PatternSearcher(
    array_view<CapturePattern> cap_patterns_kill,
    array_view<CapturePattern> cap_patterns_notify,
    CapturePattern::CaptureType cap_type)
{
    // each pattern of type cap_type
    auto for_each = [&](auto f) {
        for (auto cap_patterns : {cap_patterns_kill, cap_patterns_notify}) {
            for (CapturePattern cap_pattern : cap_patterns) {
                if (cap_pattern.is(cap_type)) {
                    f(cap_pattern);
                }
            }
        }
    };

    unsigned nb_kill = 0;
    for (CapturePattern cap_pattern : cap_patterns_kill) {
        if (cap_pattern.is(cap_type)) {
            ++nb_kill;
        }
    }

    using PatternType = CapturePattern::PatternType;

    constexpr unsigned str_type_mask = (1u << unsigned(PatternType::str))
                                     | (1u << unsigned(PatternType::exact_str));

    constexpr unsigned exact_type_mask = (1u << unsigned(PatternType::exact_reg))
                                       | (1u << unsigned(PatternType::exact_str));

    const bool is_kbd = (cap_type == CapturePattern::CaptureType::kbd);

    const unsigned reg_flags = (is_kbd
                                ? HS_FLAG_SOM_LEFTMOST
                                : HS_FLAG_SINGLEMATCH)
                             | HS_FLAG_UTF8 | HS_FLAG_DOTALL;

    const unsigned reg_mode = is_kbd
                            ? HS_MODE_STREAM | HS_MODE_SOM_HORIZON_SMALL
                            : HS_MODE_BLOCK;

    // count cap_type pattern and total string len
    unsigned len = 0;
    std::size_t str_len = 0;
    std::size_t transformed_str_len = 0;
    for_each([&](CapturePattern cap_pattern){
        ++len;
        auto flag = 1u << unsigned(cap_pattern.pattern_type());
        str_len += cap_pattern.pattern().size();
        if (str_type_mask & flag) {
            // some characters are escaped
            transformed_str_len += cap_pattern.pattern().size();
        }
        if (exact_type_mask & flag) {
            // push '^(' ... ')$'
            transformed_str_len += 4;
        }
    });
    transformed_str_len += str_len;

    if (!len) {
        return ;
    }

    static_assert(sizeof(char*) >= sizeof(unsigned));

    struct CompileArrays
    {
        char const** expressions;
        unsigned* flags;
        unsigned* ids;
        char* patterns;
    };

    // allocate memory for hs_compile_multi
    const CompileArrays compile_arrays = [=]{
        std::size_t compile_mem_len = sizeof(char*) * len       // pattern table
                                    + sizeof(unsigned) * len    // flags
                                    + sizeof(unsigned) * len    // ids
                                    + transformed_str_len + len // patterns
                                    ;
        void* compile_mem = ::operator new(compile_mem_len);
        auto* exprs = static_cast<char const**>(compile_mem);
        auto* flags = reinterpret_cast<unsigned*>(exprs + len);
        auto* ids = flags + len;
        auto* patterns = reinterpret_cast<char*>(ids + len);
        return CompileArrays{exprs, flags, ids, patterns};
    }();
    SCOPE_EXIT(::operator delete(compile_arrays.expressions));

    auto compile_arrays_it = compile_arrays;


    static_assert(alignof(hs_stream_t*) >= alignof(PatternSearcher::PatternFound));

    // allocate memory
    std::size_t mem_len = sizeof(D)
                        + sizeof(PatternFound) * len
                        + sizeof(D::InternalPatternFound) * len
                        + str_len + len                     // pattern strings + '\0'
                        ;
    mem_len += is_kbd ? sizeof(hs_stream_t*) * len : 0;     // streams

    d = new((::operator new(mem_len))) D{
        0,
        0,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        len,
        0,
        nb_kill,
        is_kbd,
    };

    hs_stream_t** streams = reinterpret_cast<hs_stream_t**>(d + 1);
    PatternFound* scan_results = reinterpret_cast<PatternFound*>(streams + (is_kbd ? len : 0));
    D::InternalPatternFound* internal_scan_results = reinterpret_cast<D::InternalPatternFound*>(scan_results + len);
    char* str_patterns = reinterpret_cast<char*>(internal_scan_results + len);

    d->streams = streams;
    d->scan_results = scan_results;
    d->internal_scan_results = internal_scan_results;

    unsigned id = 0;
    // init arrays
    for_each([&](CapturePattern cap_pattern) {
        auto patt = cap_pattern.pattern();

        *internal_scan_results++ = {
            {str_patterns, patt.size()},
            0,
            0,
        };

        memcpy(str_patterns, patt.data(), patt.size());
        str_patterns += patt.size();
        *str_patterns++ = '\0';

        *compile_arrays_it.expressions++ = compile_arrays_it.patterns;
        *compile_arrays_it.flags++ = reg_flags;
        *compile_arrays_it.ids++ = id;

        ++id;

        auto flag = 1u << unsigned(cap_pattern.pattern_type());
        if (exact_type_mask & flag) {
            // push '^(' ... ')$'
            *compile_arrays_it.patterns++ = '^';
            *compile_arrays_it.patterns++ = '(';
        }

        if (str_type_mask & flag) {
            // escape some characters
            for (char c : patt) {
                switch (c) {
                    case '{':
                    case '}':
                    case '[':
                    case ']':
                    case '(':
                    case ')':
                    case '|':
                    case '\\':
                    case '^':
                    case '$':
                    case '.':
                    case '?':
                    case '+':
                    case '*':
                        *compile_arrays_it.patterns++ = '\\';
                }
                *compile_arrays_it.patterns++ = c;
            }
        }
        else {
            memcpy(compile_arrays_it.patterns, patt.data(), patt.size());
            compile_arrays_it.patterns += patt.size();
        }

        if (exact_type_mask & flag) {
            // push '^(' ... ')$'
            *compile_arrays_it.patterns++ = ')';
            *compile_arrays_it.patterns++ = '$';
        }

        *compile_arrays_it.patterns++ = '\0';
    });

    hs_compile_error_t *compileErr;
    hs_error_t err;

    // compile expressions
    err = hs_compile_multi(compile_arrays.expressions, compile_arrays.flags, compile_arrays.ids,
                           len, reg_mode, nullptr, &d->db, &compileErr);

    while (err != HS_SUCCESS) {
        if (compileErr->expression < 0) {
            LOG(LOG_ERR, "PatternsSearcher::PatternsSearcher(): %s", compileErr->message);
            hs_free_compile_error(compileErr);
            return;
        }

        auto pos = static_cast<unsigned>(compileErr->expression);
        LOG(LOG_ERR, "PatternsSearcher::PatternsSearcher(): '%s' failed compilation with error: %s",
            compile_arrays.expressions[pos], compileErr->message);
        hs_free_compile_error(compileErr);

        // remove malformed pattern
        --d->nb_pattern;
        memmove(compile_arrays.expressions+pos, compile_arrays.expressions+pos+1, d->nb_pattern - pos);
        compile_arrays.expressions[pos] = compile_arrays.expressions[d->nb_pattern];
        d->internal_scan_results[pos] = d->internal_scan_results[d->nb_pattern];
        if (pos < d->nb_pattern_kill) {
            --d->nb_pattern_kill;
        }

        err = hs_compile_multi(compile_arrays.expressions, compile_arrays.flags, compile_arrays.ids,
                               d->nb_pattern, reg_mode, nullptr, &d->db, &compileErr);
    }

    // init scratch
    err = hs_alloc_scratch(d->db, &d->scratch);

    if (err != HS_SUCCESS) {
        LOG(LOG_ERR, "PatternsSearcher::PatternsSearcher(): could not allocate scratch space");
        throw Error(ERR_MEMORY_ALLOCATION_FAILED);
    }

    // open streams
    if (is_kbd) {
        // TODO used only 1 stream for any kill pattern
        len = d->nb_pattern;
        d->nb_stream = len;
        memset(streams, 0, sizeof(*streams) * len);
        for (std::size_t i = 0; i < len; ++i) {
            hs_error_t err = hs_open_stream(d->db, 0, streams);
            ++streams;
            if (err != HS_SUCCESS) {
                LOG(LOG_ERR, "PatternsSearcher::PatternsSearcher(): unable to open stream");
                throw Error(ERR_MEMORY_ALLOCATION_FAILED);
            }
        }
    }
}

PatternSearcher::~PatternSearcher()
{
    if (d) {
        auto* scratch = d->scratch;
        if (d->is_kbd) {
            for (hs_stream_t* stream : d->av_streams()) {
                hs_close_stream(stream, scratch, nullptr, nullptr);
            }
        }
        hs_free_scratch(scratch);
        hs_free_database(d->db);
        ::operator delete(static_cast<void*>(d));
    }
}

bool PatternSearcher::has_pattern() const
{
    return d && d->nb_pattern > 0;
}

void PatternSearcher::reset_kbd_streams()
{
    assert(d);
    auto* scratch = d->scratch;
    for (hs_stream_t* stream : d->av_streams()) {
        hs_reset_stream(stream, 0, scratch, nullptr, nullptr);
    }
}

array_view<PatternSearcher::PatternFound> PatternSearcher::scan(chars_view str)
{
    assert(d);
    struct Ctx
    {
        unsigned long long call_id;
        unsigned stream_id;
        unsigned nb_pattern_kill;
        PatternSearcher::PatternFound* scan_results;
        D::InternalPatternFound* internal_scan_results;
    };

    auto matcher = [](auto check_stream){
        return [](
            unsigned id, unsigned long long from, unsigned long long /*to*/,
            unsigned /*flags*/, void *pctx
        ) {
            Ctx& ctx = *static_cast<Ctx*>(pctx);

            if constexpr (decltype(check_stream)::value) {
                if (id != ctx.stream_id) {
                    return 0;
                }
            }

            auto& cache = ctx.internal_scan_results[id];
            if (cache.call_id != ctx.call_id) {
                cache.call_id = ctx.call_id;
                *ctx.scan_results++ = {
                    cache.pattern,
                    id,
                    id < ctx.nb_pattern_kill,
                    from,
                };
            }

            return 0; // continue matching
        };
    };

    Ctx ctx{
        ++d->nb_call,
        0,
        d->nb_pattern_kill,
        d->scan_results,
        d->internal_scan_results,
    };

    auto* scratch = d->scratch;

    if (d->is_kbd) {
        for (hs_stream_t* stream : d->av_streams()) {
            hs_scan_stream(stream, str.data(), checked_int(str.size()),
                           0, scratch, matcher(std::true_type()), &ctx);
            ++ctx.stream_id;
        }

        unsigned long long total_len = d->total_len + str.size();

        for (PatternFound& patt : writable_array_view{d->scan_results, ctx.scan_results}) {
            hs_reset_stream(d->streams[patt.id], 0, scratch, nullptr, nullptr);
            auto& internal_result = ctx.internal_scan_results[patt.id];
            patt.match_len = total_len - (patt.match_len + internal_result.offset_data);
            internal_result.offset_data = total_len;
        }

        d->total_len = total_len;
    }
    else {
        hs_scan(d->db, str.data(), checked_int(str.size()),
                0, scratch, matcher(std::false_type()), &ctx);
    }

    return {d->scan_results, ctx.scan_results};
}
