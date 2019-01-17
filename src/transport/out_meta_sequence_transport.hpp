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

#include "utils/log.hpp"

#include "utils/genfstat.hpp"
#include "utils/strutils.hpp"
#include "core/error.hpp"

#include "transport/crypto_transport.hpp"
#include "transport/mwrm_reader.hpp"

#include <cstdio>


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
        const char * const extension)
    {
        if (!utils::strbcpy(this->path, prefix)
         || !utils::strbcpy(this->hash_path, hash_prefix)
         || !utils::strbcpy(this->filename, filename)
         || !utils::strbcpy(this->extension, extension)) {
            LOG(LOG_ERR, "Filename too long");
            throw Error(ERR_TRANSPORT);
        }

        this->filename_gen[0] = 0;
    }

    const char * get_filename(unsigned count) const
    {
        std::snprintf(
            this->filename_gen, sizeof(this->filename_gen), "%s%s-%06u%s",
            this->path, this->filename, count, this->extension);
        return this->filename_gen;
    }

    const char * get_hash_filename(unsigned count) const
    {
        std::snprintf(
            this->hash_filename_gen, sizeof(this->hash_filename_gen), "%s%s-%06u%s",
            this->hash_path, this->filename, count, this->extension);
        return this->hash_filename_gen;
    }

};


struct MetaFilename
{
    char filename[2048];
    MetaFilename(const char * path, const char * basename)
    {
        int res = snprintf(this->filename, sizeof(this->filename)-1, "%s%s.mwrm", path, basename);
        if (res > int(sizeof(this->filename) - 6) || res < 0) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED);
        }
    }
};


class MetaSeqBuf
{
    OutCryptoTransport meta_buf_encrypt_transport;
    OutCryptoTransport wrm_filter_encrypt_transport;

    Fstat & fstat;

    char current_filename_[1024];
    WrmFGen filegen_;
    unsigned num_file_;
    int groupid_;

    MetaFilename mf_;
    MetaFilename hf_;
    time_t start_sec_;
    time_t stop_sec_;

    CryptoContext & cctx;

public:
    explicit MetaSeqBuf(
        CryptoContext & cctx,
        Random & rnd,
        Fstat & fstat,
        ReportError report_error,
        time_t start_sec,
        const char * const hash_prefix,
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid
    )
    : meta_buf_encrypt_transport(cctx, rnd, fstat, report_error)
    , wrm_filter_encrypt_transport(cctx, rnd, fstat, report_error)
    , fstat(fstat)
    , current_filename_{}
    , filegen_(prefix, hash_prefix, filename, extension)
    , num_file_(0)
    , groupid_(groupid)
    , mf_(prefix, filename)
    , hf_(hash_prefix, filename)
    , start_sec_(start_sec)
    , stop_sec_(start_sec)
    , cctx(cctx)
    {
        //LOG(LOG_INFO, "hash_prefix=%s prefix=%s", hash_prefix, prefix);
    }

    void open(uint16_t width, uint16_t height)
    {
        this->meta_buf_encrypt_transport.open(
            this->mf_.filename,
            this->hf_.filename,
            S_IRUSR | S_IRGRP | S_IWUSR);

        MwrmWriterBuf mwrm_file_buf;
        mwrm_file_buf.write_header(width, height, this->cctx.get_with_checksum());
        this->meta_buf_encrypt_transport.send(mwrm_file_buf.buffer());
    }

    ~MetaSeqBuf()
    {
        this->close();
    }

    void write(const uint8_t * data, size_t len)
    {
        if (!this->wrm_filter_encrypt_transport.is_open()) {
            const char * filename = this->filegen_.get_filename(this->num_file_);
            const char * hash_filename = this->filegen_.get_hash_filename(this->num_file_);
            this->wrm_filter_encrypt_transport.open(filename, hash_filename, this->groupid_);
        }
        this->wrm_filter_encrypt_transport.send(data, len);
    }

    bool next()
    {
        if (this->wrm_filter_encrypt_transport.is_open()) {
            this->next_meta_file();
            return true;
        }
        return false;
    }

    int close()
    {
        if (this->wrm_filter_encrypt_transport.is_open()) {
            this->next_meta_file();
        }

        uint8_t qhash[MD_HASH::DIGEST_LENGTH];
        uint8_t fhash[MD_HASH::DIGEST_LENGTH];

        this->meta_buf_encrypt_transport.close(qhash, fhash);
        return 0;
    }

    void update_sec(time_t sec)
    { this->stop_sec_ = sec; }

private:
    void next_meta_file()
    {
        uint8_t qhash[MD_HASH::DIGEST_LENGTH];
        uint8_t fhash[MD_HASH::DIGEST_LENGTH];

        this->wrm_filter_encrypt_transport.close(qhash, fhash);

        const char * filename = this->filegen_.get_filename(this->num_file_);
        this->current_filename_[0] = 0;

        this->num_file_ ++;

        struct stat stat;
        if (fstat.stat(filename, stat)){
            throw Error(ERR_TRANSPORT_WRITE_FAILED);
        }

        MwrmWriterBuf mwrm_file_buf;
        mwrm_file_buf.write_line(
            filename, stat,
            this->start_sec_, this->stop_sec_ + 1,
            this->cctx.get_with_checksum(), qhash, fhash);
        this->meta_buf_encrypt_transport.send(mwrm_file_buf.buffer());

        this->start_sec_ = this->stop_sec_+1;
    }
};


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
        ReportMessageApi * report_message)
    : buf(
        cctx, rnd, fstat,
        report_error_from_reporter(report_message),
        now.tv_sec, hash_path, path, basename, ".wrm", groupid)
    {
        this->buf.open(width, height);
    }

    void timestamp(timeval now) override
    {
        this->buf.update_sec(now.tv_sec);
    }

    bool next() override
    {
        if (!this->buf.next()) {
            throw Error(ERR_TRANSPORT_NO_MORE_DATA);
        }
        ++this->seqno;
        return true;
    }

    bool disconnect() override
    {
        return !this->buf.close();
    }

private:
    void do_send(const uint8_t * data, size_t len) override
    {
        this->buf.write(data, len);
    }

    MetaSeqBuf buf;
};
