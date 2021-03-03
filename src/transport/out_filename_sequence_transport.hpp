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
*   Copyright (C) Wallix 2013-2017
*   Author(s): Christophe Grosjean, Jonatan Poelen, Raphael Zhou, Meng Tan
*/

#pragma once

#include "transport/file_transport.hpp"


// TODO in PngCapture
class OutFilenameSequenceTransport : public Transport
{
public:
    OutFilenameSequenceTransport(
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid,
        std::function<void(const Error & error)> notify_error);

    ~OutFilenameSequenceTransport();

    char const* seqgen(unsigned i) const noexcept;

    bool next() override;

    [[nodiscard]] uint32_t get_seqno() const { return this->seqno_; }

    bool disconnect() override;

private:
    struct FilenameGenerator
    {
    private:
        char         extension[12];
        mutable char filename_gen[2070];
        std::size_t  filename_suffix_pos;

        const char * last_filename;
        unsigned     last_num;

    public:
        FilenameGenerator(FilenameGenerator const &) = delete;
        FilenameGenerator& operator=(FilenameGenerator const &) = delete;

        FilenameGenerator(
            const char * const prefix,
            const char * const filename,
            const char * const extension);

        const char * get(unsigned count) const;

        void set_last_filename(unsigned num, const char * name);
    };

    void do_send(const uint8_t * data, size_t len) override;

    /// \return 0 if success
    int do_next();
    void open_filename(const char * filename);

    const char * rename_filename();

    char current_filename_[1024];
    FilenameGenerator filegen_;
    OutFileTransport buf_;
    unsigned num_file_ = 0;
    int groupid_;
    uint32_t seqno_ = 0;
    bool status = true;
};
