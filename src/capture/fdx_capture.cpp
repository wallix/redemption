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
#include "utils/sugar/array_view.hpp"
#include "utils/strutils.hpp"
#include "utils/fileutils.hpp"

#include <array>
#include <limits>
#include <iterator>
#include <charconv>


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

    std::to_chars_result const chars_result = std::to_chars(p, std::end(path), i);
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
    str_append(this->record_path, record_path, '/', sid, '/', sid);
    str_append(this->hash_path, hash_path,
        array_view(this->record_path).drop_front(record_path.size()));

    this->pos_end_record_suffix = checked_int(this->record_path.size());
    this->pos_end_hash_suffix = checked_int(this->hash_path.size());
    this->pos_start_basename = checked_int(this->pos_end_record_suffix - sid.size());
    this->pos_start_relative_path = checked_int(this->pos_start_basename - sid.size() - 1);
}

void FdxNameGenerator::next_tfl()
{
    auto const suffix_filename = this->tfl_suffix_generator.next();

    this->record_path.erase(this->pos_end_record_suffix);
    this->record_path += suffix_filename;

    this->hash_path.erase(this->pos_end_hash_suffix);
    this->hash_path += suffix_filename;
}

std::string_view FdxNameGenerator::get_current_relative_path() const noexcept
{
    std::string_view sv = this->record_path;
    sv.remove_prefix(this->pos_start_relative_path);
    return sv;
}

std::string_view FdxNameGenerator::get_current_basename() const noexcept
{
    std::string_view sv = this->record_path;
    sv.remove_prefix(this->pos_start_basename);
    return sv;
}

namespace
{
    std::string_view remove_end_slash(std::string_view& s)
    {
        assert(not s.empty());
        if (s.back() == '/') {
            s.remove_suffix(1);
        }
        return s;
    }
}

FdxCapture::FdxCapture(
    std::string_view record_path, std::string_view hash_path,
    std::string fdx_filebase, std::string_view sid,
    int groupid, FilePermissions file_permissions,
    CryptoContext& cctx, Random& rnd,
    std::function<void(const Error & error)> notify_error)
: name_generator(
    record_path = remove_end_slash(record_path),
    hash_path = remove_end_slash(hash_path),
    sid)
, cctx(cctx)
, rnd(rnd)
, notify_error(notify_error)
, groupid(groupid)
, file_permissions(file_permissions)
, out_crypto_transport(this->cctx, this->rnd, this->notify_error)
{
    // create directory
    std::string directory;

    for (std::string_view path : {record_path, hash_path}) {
        str_assign(directory, path, '/', sid);
        if (recursive_create_directory(directory.c_str(), S_IRWXU | S_IRGRP | S_IXGRP, this->groupid) != 0) {
            auto err = errno;
            LOG(LOG_ERR, "FdxCapture: Failed to create directory: \"%s\": %s",
                directory, strerror(err));
            throw Error(ERR_TRANSPORT_OPEN_FAILED, err);
        }
    }

    fdx_filebase += ".fdx";
    this->out_crypto_transport.open(
        str_concat(record_path, '/', fdx_filebase).c_str(),
        str_concat(hash_path, '/', fdx_filebase).c_str(),
        this->groupid,
        this->file_permissions,
        /*derivator=*/fdx_filebase);

    this->out_crypto_transport.send(Mwrm3::header_compatibility_packet);
}

FdxCapture::TflFile::TflFile(FdxCapture const& fdx, Mwrm3::Direction direction)
: file_id(fdx.name_generator.get_current_id())
, trans(fdx.cctx, fdx.rnd, fdx.notify_error)
, direction(direction)
{
    auto derivator = fdx.name_generator.get_current_basename();
    this->trans.open(
        fdx.name_generator.get_current_record_path().c_str(),
        fdx.name_generator.get_current_hash_path().c_str(),
        fdx.groupid,
        fdx.file_permissions,
        derivator);
}

FdxCapture::TflFile FdxCapture::new_tfl(Mwrm3::Direction direction)
{
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
    OutCryptoTransport::HashArray qhash;
    OutCryptoTransport::HashArray fhash;
    tfl.trans.close(qhash, fhash);

    char buf[1024 * 16 * 2 + 256];
    OutStream out{buf};

    auto write_in_buf = [&](Mwrm3::Type /*type*/, auto... data) {
        (out.out_copy_bytes(data), ...);
    };

    // truncate filename if too long
    original_filename = std::string_view(original_filename.data(),
        std::min(original_filename.size(), std::string_view::size_type(1024 * 16)));

    char const* filename = tfl.trans.get_finalname();
    struct stat stat;
    ::stat(filename, &stat);

    bool const with_checksum = this->cctx.get_with_checksum();

    auto const dirname_len = this->name_generator.get_current_record_path().size()
        - this->name_generator.get_current_relative_path().size();

    Mwrm3::tfl_new.serialize(
        tfl.file_id, Mwrm3::FileSize(stat.st_size), tfl.direction, transfered_status,
        Mwrm3::Filename{original_filename},
        Mwrm3::TflRelativeFilename{std::string_view(filename + dirname_len)},
        Mwrm3::QuickHash{with_checksum ? make_array_view(qhash) : bytes_view{"", 0}},
        Mwrm3::FullHash{with_checksum ? make_array_view(fhash) : bytes_view{"", 0}},
        sig,
        write_in_buf);

    this->out_crypto_transport.send(out.get_produced_bytes());
}

void FdxCapture::close(OutCryptoTransport::HashArray& qhash, OutCryptoTransport::HashArray& fhash)
{
    this->out_crypto_transport.close(qhash, fhash);
}

bool FdxCapture::is_open() const
{
    return this->out_crypto_transport.is_open();
}
