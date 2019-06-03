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
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean, Jonatan Poelen
*/

#pragma once

#include "transport/crypto_transport.hpp"


struct OutMetaSequenceTransport : Transport
{
    OutMetaSequenceTransport(
        CryptoContext & cctx,
        Random & rnd,
        Fstat & fstat,
        const char * path,
        const char * hash_path,
        const char * basename,
        timeval now,
        uint16_t width,
        uint16_t height,
        const int groupid,
        ReportMessageApi * report_message);

    ~OutMetaSequenceTransport();

    void timestamp(timeval now) override;

    bool next() override;

    bool disconnect() override;

private:
    void do_send(const uint8_t * data, size_t len) override;

    void do_close();

    void next_meta_file();

    class WrmFGen
    {
        char         path[1024];
        char         hash_path[1024];
        char         filename[1012];
        char         extension[12];
        mutable char filename_gen[2070];
        mutable char hash_filename_gen[2070];

    public:
        WrmFGen(
            const char * const prefix,
            const char * const hash_prefix,
            const char * const filename,
            const char * const extension);

        const char * get_filename(unsigned count) const noexcept;

        const char * get_hash_filename(unsigned count) const noexcept;

    };

    struct MetaFilename
    {
        char filename[2048];
        MetaFilename(const char * path, const char * basename);
    };

    OutCryptoTransport meta_buf_encrypt_transport;
    OutCryptoTransport wrm_filter_encrypt_transport;

    Fstat & fstat;

    char current_filename_[1024] {};
    WrmFGen filegen_;
    unsigned num_file_ = 0;
    int groupid_;

    MetaFilename mf_;
    MetaFilename hf_;
    time_t start_sec_;
    time_t stop_sec_;

    CryptoContext & cctx;
};
