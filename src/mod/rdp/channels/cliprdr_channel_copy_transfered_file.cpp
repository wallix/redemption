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
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou, Clément Moroldo
*/

#include "mod/rdp/channels/cliprdr_channel_copy_transfered_file.hpp"
#include "std17/charconv.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/array_view.hpp"

#include <array>
#include <limits>
#include <iterator>


std::string TflSuffixGenerator::name_at(uint64_t i)
{
    constexpr int count_max_digit_prefix = 6;

    char path[
        1 +
        count_max_digit_prefix +
        std::numeric_limits<uint64_t>::digits10 + 2 +
        4
    ];

    char* p = path + 1;

    for (auto* pend = p + count_max_digit_prefix; p < pend; ++p) {
        *p = '0';
    }

    std::to_chars_result const chars_result = std::to_chars(p, end(path), i);
    std::ptrdiff_t const len = std::distance(p, chars_result.ptr);

    auto prefix_len = (len >= count_max_digit_prefix ? 0 : count_max_digit_prefix - len);
    auto* first = p - prefix_len;
    auto* last = p + len;
    *--first = ',';
    *last++ = '.';
    *last++ = 't';
    *last++ = 'f';
    *last++ = 'l';

    return std::string(first, last);
}


FdxNameGenerator::FdxNameGenerator(
    std::string_view path, std::string_view hashpath, std::string_view sid)
{
    constexpr array_view_const_char fdx_suffix = ".fdx"_av;

    auto const filename_len = sid.size() + fdx_suffix.size();

    str_append(this->record_path, path, '/', sid, fdx_suffix);
    str_append(this->hash_path, hashpath, array_view(this->record_path).last(filename_len+1));

    this->pos_end_record_suffix = this->record_path.size() - fdx_suffix.size();
    this->pos_end_hash_suffix = this->hash_path.size() - fdx_suffix.size();
    this->pos_start_filename = this->pos_end_record_suffix - sid.size();
}

void FdxNameGenerator::next_tfl()
{
    auto const suffix_filename = this->tfl_suffix_generator.next();

    this->record_path.erase(this->pos_end_record_suffix);
    this->record_path += suffix_filename;

    this->hash_path.erase(this->pos_end_hash_suffix);
    this->hash_path += suffix_filename;
}

std::string_view FdxNameGenerator::get_current_filename() const noexcept
{
    std::string_view sv = this->record_path;
    sv.remove_prefix(this->pos_start_filename);
    return sv;
}




FdxCapture::FdxCapture(CryptoContext& cctx, Random& rnd, Fstat& fstat, ReportError report_error)
: cctx(cctx)
, rnd(rnd)
, fstat(fstat)
, out_crypto_transport(cctx, rnd, fstat, std::move(report_error))
{}

void FdxCapture::open(std::string_view path, std::string_view hashpath, int groupid, std::string_view sid)
{
    constexpr array_view_const_char fdx_suffix = ".fdx"_av;

    str_append(this->record_prefix, path, '/', sid, fdx_suffix);
    str_append(this->hash_prefix, hashpath, '/', sid, fdx_suffix);
    auto const filename_len = sid.size() - fdx_suffix.size();

    this->pos_end_record_prefix = this->record_prefix.size() - filename_len;
    this->pos_end_hash_prefix = this->hash_prefix.size() - filename_len;
    this->groupid = groupid;

    auto derivator = array_view(this->record_prefix).drop_front(this->pos_end_record_prefix);
    this->out_crypto_transport.open(
        this->record_prefix.c_str(), this->hash_prefix.c_str(), groupid, derivator);
    this->out_crypto_transport.send(Mwrm3::header_compatibility_packet);
}

FdxCapture::TflFile FdxCapture::new_tfl()
{
    auto const suffix_filename = this->tfl_name_generator.next();

    this->record_prefix.erase(this->pos_end_record_prefix);
    this->record_prefix += suffix_filename;

    this->hash_prefix.erase(this->pos_end_hash_prefix);
    this->hash_prefix += suffix_filename;

    auto trans = std::make_unique<OutCryptoTransport>(
        this->cctx, this->rnd, this->fstat, this->report_error);

    auto derivator = array_view{this->record_prefix}.drop_front(this->pos_end_record_prefix);
    trans->open(this->record_prefix.c_str(), this->hash_prefix.c_str(), this->groupid, derivator);

    return TflFile{this->tfl_name_generator.get_current_id(), std::move(trans)};
}

void FdxCapture::write_tfl(
    FileId id, std::string_view tfl_filename, FileSize file_size,
    Mwrm3::QuickHash qhash, Mwrm3::FullHash fhash, std::string_view original_filename)
{
    char buf[~uint16_t{} * 2 + 128];
    OutStream out{buf};

    auto write_in_buf = [&](Mwrm3::Type /*type*/, auto... data) {
        (out.out_copy_bytes(data), ...);
    };

    // truncate filename if too long
    original_filename = std::string_view(original_filename.data(),
        std::min(original_filename.size(), std::string_view::size_type(~uint16_t())));

    Mwrm3::serialize_tfl_new(id, original_filename, tfl_filename, write_in_buf);
    Mwrm3::serialize_tfl_stat(id, file_size, qhash, fhash, write_in_buf);

    this->out_crypto_transport.send(out.get_bytes());
}
