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

#pragma once

#include "core/report_error.hpp"
#include "capture/mwrm3.hpp"

#include <vector>
#include <string>
#include <string_view>

#include <cstdint>



struct TflSuffixGenerator
{
    std::string next()
    {
        ++this->idx;
        return name_at(this->idx);
    }

    Mwrm3::FileId get_current_id() const noexcept
    {
        return Mwrm3::FileId(this->idx);
    }

    static std::string name_at(uint64_t i);

private:
    uint64_t idx = 0;
};


#include "transport/crypto_transport.hpp"
#include <utility>
#include <memory>

struct FdxNameGenerator
{
    FdxNameGenerator(std::string_view path, std::string_view hashpath, std::string_view sid);

    // before next_tfl(): is a fdx file
    // after next_tfl(): is a tfl file
    //@{
    std::string_view get_current_filename() const noexcept;
    std::string const& get_current_record_path() const noexcept { return this->record_path; }
    std::string const& get_current_hash_path() const noexcept { return this->hash_path; }
    //@}

    void next_tfl();

private:
    std::string record_path;
    std::string hash_path;
    uint16_t pos_start_filename;
    uint16_t pos_end_record_suffix;
    uint16_t pos_end_hash_suffix;
    TflSuffixGenerator tfl_suffix_generator;
};

struct FdxCapture
{
    using FileId = Mwrm3::FileId;
    using FileSize = Mwrm3::FileSize;

    struct TflFile
    {
        FileId id;
        std::unique_ptr<OutCryptoTransport> trans;

        std::string_view get_final_filename() const;
    };

    explicit FdxCapture(
        CryptoContext& cctx, Random& rnd, Fstat& fstat,
        ReportError report_error = ReportError());

    void open(std::string_view path, std::string_view hashpath, int groupid, std::string_view sid);

    TflFile new_tfl();

    void write_tfl(
        FileId id, std::string_view tfl_filename, FileSize file_size,
        Mwrm3::QuickHash qhash, Mwrm3::FullHash fhash, std::string_view original_filename);

private:
    std::string record_prefix;
    std::string hash_prefix;
    std::size_t pos_end_record_prefix;
    std::size_t pos_end_hash_prefix;
    TflSuffixGenerator tfl_name_generator;

    CryptoContext& cctx;
    Random& rnd;
    Fstat& fstat;
    ReportError report_error;
    int groupid;

    OutCryptoTransport out_crypto_transport;
};
