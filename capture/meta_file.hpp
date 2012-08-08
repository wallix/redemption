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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(__CAPTURE_META_FILE_HPP__)
#define __CAPTURE_META_FILE_HPP__

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <sys/time.h>
#include <sys/types.h>
#include <stdint.h>

struct DataFile
{
    std::string wrm_filename;
    std::string png_filename;
    time_t start_sec;
    suseconds_t start_usec;

    DataFile(const std::string& __wrm_filename = std::string(),
            const std::string& __png_filename = std::string(),
            time_t __start_sec = 0,
            suseconds_t __start_usec = 0
           )
    : wrm_filename(__wrm_filename)
    , png_filename(__png_filename)
    , start_sec(__start_sec)
    , start_usec(__start_usec)
    {}
};

inline bool operator==(const DataFile& a, const DataFile& b)
{
    return a.start_sec == b.start_sec
    && a.start_usec == b.start_usec
    && a.png_filename == b.png_filename
    && a.wrm_filename == b.wrm_filename;
}

inline bool operator!=(const DataFile& a, const DataFile& b)
{
    return !(a == b);
}

struct DataMetaFile
{
    std::vector<DataFile> files;

    uint16_t version;
    uint16_t width;
    uint16_t height;
    unsigned  cipher_mode;

    bool loaded;

    DataMetaFile()
    : files()
    , version(0)
    , width(0)
    , height(0)
    , cipher_mode(0)
    , loaded(false)
    {}
};

inline std::istream& operator>>(std::istream& is, DataMetaFile& data)
{
    data.files.clear();
    is >> data.width >> data.height >> data.cipher_mode;

    std::string line;
    while (std::getline(is, line))
    {
        if (!line.empty()){
            std::size_t p1 = line.find(',', 1);

            DataFile info(line.substr(0, p1));

            if (std::string::npos != p1){
                std::size_t p2 = line.find(' ', p1 + 1);
                info.png_filename = line.substr(p1 + 1, p2 - p1 - 1);

                if (std::string::npos != p2){
                    std::size_t p3 = line.find(' ', p2 + 1);
                    std::istringstream(line.substr(p2 + 1, p3 - p2 - 1)) >> info.start_sec;

                    if (std::string::npos != p3){
                        std::istringstream(line.substr(p3 + 1, line.find(' ', p3 + 1) - p3 - 1)) >> info.start_usec;
                    }
                }
            }

            data.files.push_back(info);
        }
    }
    data.loaded = true;
    return is;
}

/**
width height
cipher_mode

wrm_filename,[png_filename] [start_sec [start_usec]]
*/
inline bool read_meta_file(DataMetaFile& data, const char * filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
        return false;
    file >> data;
    return true;
}

inline std::ostream& operator<<(std::ostream& os, DataMetaFile& data)
{
    os << data.width << ' ' << data.height << "\n"
    << data.cipher_mode << "\n\n";
    for (std::size_t i = 0, last = data.files.size(); i < last; ++i)
    {
        DataFile& info = data.files[i];
        os << info.wrm_filename << ',' << info.png_filename
        << ' ' << info.start_sec << ' ' << info.start_usec << '\n';
    }
    return os;
}

inline bool write_meta_file(DataMetaFile& data, const char * filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
        return false;
    file << data;
    return true;
}

#endif