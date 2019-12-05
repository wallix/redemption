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

#include "capture/fdx_capture.hpp"
#include "std17/charconv.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/genfstat.hpp"
#include "utils/fileutils.hpp"

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

    str_append(this->record_path, path, '/', sid, '/', sid, fdx_suffix);
    str_append(this->hash_path, hashpath, array_view(this->record_path).drop_front(path.size()));

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


namespace
{
    void open_crypto_transport(
        OutCryptoTransport& crypto_trans,
        FdxNameGenerator const& gen,
        int groupid)
    {
        auto derivator = gen.get_current_filename();
        crypto_trans.open(
            gen.get_current_record_path().c_str(),
            gen.get_current_hash_path().c_str(),
            groupid, derivator);
    }
}


FdxCapture::FdxCapture(
    std::string_view record_path, std::string_view hash_path, std::string_view sid, int groupid,
    CryptoContext& cctx, Random& rnd, Fstat& fstat, ReportError report_error)
: name_generator(record_path, hash_path, sid)
, cctx(cctx)
, rnd(rnd)
, fstat(fstat)
, report_error(report_error)
, groupid(groupid)
, out_crypto_transport(cctx, rnd, fstat, std::move(report_error))
{
    auto const filename_len = this->name_generator.get_current_filename().size();

    for (std::string const* path : {
        &this->name_generator.get_current_record_path(),
        &this->name_generator.get_current_hash_path()
    }) {
        char dirname[4096];
        auto const dirname_len = path->size() - filename_len - 1u;

        if (dirname_len + 1u >= std::size(dirname)) {
            LOG(LOG_ERR, "FdxCapture: Failed to create directory: \"%.*s\": name too long",
                int(dirname_len), path->data());
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }

        memcpy(dirname, path->data(), dirname_len);
        dirname[dirname_len] = '\0';

        if (recursive_create_directory(dirname, S_IRWXU | S_IRGRP | S_IXGRP, groupid) != 0) {
            auto err = errno;
            LOG(LOG_ERR, "FdxCapture: Failed to create directory: \"%s\": %s",
                path, strerror(err));
            throw Error(ERR_TRANSPORT_OPEN_FAILED, err);
        }
    }

    open_crypto_transport(this->out_crypto_transport, this->name_generator, groupid);
    this->out_crypto_transport.send(Mwrm3::header_compatibility_packet);
}

FdxCapture::TflFile::TflFile(FdxCapture const& fdx)
: file_id(fdx.name_generator.get_current_id())
, trans(fdx.cctx, fdx.rnd, fdx.fstat, fdx.report_error)
{
    open_crypto_transport(this->trans, fdx.name_generator, fdx.groupid);
}

FdxCapture::TflFile FdxCapture::new_tfl()
{
    this->name_generator.next_tfl();

    return TflFile{*this};
}

void FdxCapture::cancel_tfl(FdxCapture::TflFile& tfl)
{
    tfl.trans.cancel();
}

void FdxCapture::close_tfl(FdxCapture::TflFile& tfl, std::string_view original_filename)
{
    OutCryptoTransport::HashArray qhash;
    OutCryptoTransport::HashArray fhash;
    tfl.trans.close(qhash, fhash);

    char buf[~uint16_t{} * 2 + 256];
    OutStream out{buf};

    auto write_in_buf = [&](Mwrm3::Type /*type*/, auto... data) {
        (out.out_copy_bytes(data), ...);
    };

    // truncate filename if too long
    original_filename = std::string_view(original_filename.data(),
        std::min(original_filename.size(), std::string_view::size_type(~uint16_t())));

    char const* filename = tfl.trans.get_finalname();
    struct stat stat;
    this->fstat.stat(filename, stat);

    bool const with_checksum = this->cctx.get_with_checksum();

    auto const dirname_len = this->name_generator.get_current_record_path().size()
        - this->name_generator.get_current_filename().size();

    Mwrm3::serialize_tfl_new(
        tfl.file_id, original_filename, std::string_view(filename + dirname_len), write_in_buf);
    Mwrm3::serialize_tfl_stat(
        tfl.file_id, Mwrm3::FileSize(stat.st_size),
        Mwrm3::QuickHash{with_checksum ? make_array_view(qhash) : bytes_view{"", 0}},
        Mwrm3::FullHash{with_checksum ? make_array_view(fhash) : bytes_view{"", 0}},
        write_in_buf);

    this->out_crypto_transport.send(out.get_bytes());
}

void FdxCapture::close(OutCryptoTransport::HashArray& qhash, OutCryptoTransport::HashArray& fhash)
{
    this->out_crypto_transport.close(qhash, fhash);
}
