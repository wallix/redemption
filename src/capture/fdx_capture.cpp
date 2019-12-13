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
    std::string_view record_path, std::string_view hash_path, std::string_view sid)
{
    constexpr array_view_const_char fdx_suffix = ".fdx"_av;

    str_append(this->record_path, record_path, '/', sid, '/', sid, fdx_suffix);
    str_append(this->hash_path, hash_path,
        array_view(this->record_path).drop_front(record_path.size()));

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
, report_error(std::move(report_error))
, groupid(groupid)
, fdx_basename_len(this->name_generator.get_current_filename().size())
, record_fdx_path(this->name_generator.get_current_record_path())
, hash_fdx_path(this->name_generator.get_current_hash_path())
{}

void FdxCapture::_create_dir()
{
    if (this->fdx_basename_len < 0) {
        return;
    }

    std::string directory;

    for (std::string* path : {&this->record_fdx_path, &this->hash_fdx_path}) {
        directory.assign(path->begin(), path->begin() + (path->size() - this->fdx_basename_len));

        if (recursive_create_directory(directory.data(), S_IRWXU | S_IRGRP | S_IXGRP, this->groupid) != 0) {
            auto err = errno;
            LOG(LOG_ERR, "FdxCapture: Failed to create directory: \"%s\": %s",
                directory, strerror(err));
            throw Error(ERR_TRANSPORT_OPEN_FAILED, err);
        }
    }

    // disable _create_dir
    this->fdx_basename_len = -this->fdx_basename_len;
}

bool FdxCapture::is_open() const noexcept
{
    return bool(this->out_crypto_transport);
}

void FdxCapture::_open_fdx()
{
    if (this->is_open()) {
        return;
    }

    this->out_crypto_transport.emplace(this->cctx, this->rnd, this->fstat, this->report_error);

    assert(this->fdx_basename_len < 0);
    bytes_view derivator = array_view(this->record_fdx_path)
        .last(-this->fdx_basename_len);

    this->out_crypto_transport->open(
        this->record_fdx_path.c_str(),
        this->hash_fdx_path.c_str(),
        this->groupid,
        derivator);

    this->out_crypto_transport->send(Mwrm3::header_compatibility_packet);
}

FdxCapture::TflFile::TflFile(FdxCapture const& fdx, Mwrm3::Direction direction)
: file_id(fdx.name_generator.get_current_id())
, trans(fdx.cctx, fdx.rnd, fdx.fstat, fdx.report_error)
, direction(direction)
{
    open_crypto_transport(this->trans, fdx.name_generator, fdx.groupid);
}

FdxCapture::TflFile FdxCapture::new_tfl(Mwrm3::Direction direction)
{
    this->_create_dir();
    this->name_generator.next_tfl();

    return TflFile{*this, direction};
}

void FdxCapture::cancel_tfl(FdxCapture::TflFile& tfl)
{
    tfl.trans.cancel();
}

void FdxCapture::close_tfl(
    FdxCapture::TflFile& tfl, std::string_view original_filename,
    Mwrm3::TransferedStatus transfered_status, Mwrm3::Sha256Signature sig)
{
    this->_open_fdx();

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

    Mwrm3::tfl_new.serialize(
        tfl.file_id, tfl.direction, original_filename,
        std::string_view(filename + dirname_len), write_in_buf);
    Mwrm3::tfl_info.serialize(
        tfl.file_id, Mwrm3::FileSize(stat.st_size), transfered_status,
        Mwrm3::QuickHash{with_checksum ? make_array_view(qhash) : bytes_view{"", 0}},
        Mwrm3::FullHash{with_checksum ? make_array_view(fhash) : bytes_view{"", 0}},
        sig, write_in_buf);

    this->out_crypto_transport->send(out.get_bytes());
}

void FdxCapture::close(OutCryptoTransport::HashArray& qhash, OutCryptoTransport::HashArray& fhash)
{
    if (this->out_crypto_transport) {
        this->out_crypto_transport->close(qhash, fhash);
        this->out_crypto_transport.reset();
    }
}
