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


struct FilenameGenerator
{
    enum Format {
        PATH_FILE_PID_COUNT_EXTENSION,
        PATH_FILE_COUNT_EXTENSION,
        PATH_FILE_PID_EXTENSION,
        PATH_FILE_EXTENSION
    };

private:
    char         path[1024];
    char         filename[1012];
    char         extension[12];
    Format       format;
    unsigned     pid;
    mutable char filename_gen[2070];

    const char * last_filename;
    unsigned     last_num;

public:
    FilenameGenerator(FilenameGenerator const &) = delete;
    FilenameGenerator& operator=(FilenameGenerator const &) = delete;

    FilenameGenerator(
        Format format,
        const char * const prefix,
        const char * const filename,
        const char * const extension);

    const char * get(unsigned count) const;

    void set_last_filename(unsigned num, const char * name);
};


// TODO in PngCapture
class OutFilenameSequenceTransport : public Transport
{
public:
    OutFilenameSequenceTransport(
        FilenameGenerator::Format format,
        const char * const prefix,
        const char * const filename,
        const char * const extension,
        const int groupid,
        ReportError report_error);

    ~OutFilenameSequenceTransport();

    const FilenameGenerator * seqgen() const noexcept;

    bool next() override;

    bool disconnect() override;

private:
    void do_send(const uint8_t * data, size_t len) override;
/// \return 0 if success
    int do_next();
    void open_filename(const char * filename);

    const char * rename_filename();

    const char * get_filename_generate();

    char current_filename_[1024];
    FilenameGenerator filegen_;
    OutFileTransport buf_;
    unsigned num_file_ = 0;
    int groupid_;
    bool status = true;
};
