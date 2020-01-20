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
#include "transport/crypto_transport.hpp"

#include <vector>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <memory>

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


struct FdxNameGenerator
{
    FdxNameGenerator(std::string_view record_path, std::string_view hash_path, std::string_view sid);

    // before next_tfl(): is a fdx file
    // after next_tfl(): is a tfl file
    //@{
    std::string_view get_current_filename() const noexcept;
    std::string const& get_current_record_path() const noexcept { return this->record_path; }
    std::string const& get_current_hash_path() const noexcept { return this->hash_path; }
    //@}

    Mwrm3::FileId get_current_id() const noexcept { return this->tfl_suffix_generator.get_current_id(); }

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

    class TflFile
    {
        friend FdxCapture;
        TflFile(FdxCapture const& fdx, Mwrm3::Direction direction);

    public:
        FileId file_id;
        OutCryptoTransport trans;
        Mwrm3::Direction direction;
    };


    explicit FdxCapture(
        std::string_view record_path, std::string_view hash_path,
        std::string_view fdx_basename, std::string_view sid, int groupid,
        CryptoContext& cctx, Random& rnd, Fstat& fstat,
        ReportError report_error);

    TflFile new_tfl(Mwrm3::Direction direction);

    void cancel_tfl(TflFile& tfl);
    void close_tfl(
        TflFile& tfl, std::string_view original_filename,
        Mwrm3::TransferedStatus transfered_status, Mwrm3::Sha256Signature sig);

    void close(OutCryptoTransport::HashArray & qhash, OutCryptoTransport::HashArray & fhash);

private:
    friend TflFile;

    FdxNameGenerator name_generator;

    CryptoContext& cctx;
    Random& rnd;
    Fstat& fstat;
    ReportError report_error;
    int groupid;

    OutCryptoTransport out_crypto_transport;
};
