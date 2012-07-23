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

/**
 * MetaFileReadOnly
 * MetaFileWriteOnly
 * DataMetaFile
 */

struct DataMetaFile
{
    typedef std::pair<std::string, std::string> WrmInfo;
    std::vector<WrmInfo> files;

    uint16_t version;
    uint16_t width;
    uint16_t height;

    bool loaded;

    DataMetaFile()
    : files()
    , version(0)
    , width(0)
    , height(0)
    , loaded(false)
    {}
};

inline std::istream& operator>>(std::istream& is, DataMetaFile& data)
{
    is >> data.width >> data.height;
    std::string line;
    while (std::getline(is, line) && line != "--")
    {
        if (!line.empty()){
            std::size_t pos = line.find(',', 1);
            std::size_t posend = 0;
            if (std::string::npos != pos){
                posend = pos;
            }
            posend = line.find(' ', posend + 1);
            if (std::string::npos == pos && std::string::npos == posend)
            {
                data.files.push_back(DataMetaFile::WrmInfo(line, ""));
            }
            else
            {
                if (std::string::npos == pos){
                    pos = posend;
                }
                data.files.push_back(DataMetaFile::WrmInfo(
                    line.substr(0, pos),
                    line.substr(pos + 1, posend - pos - 1)
                ));
            }
        }
    }
    data.loaded = true;
    return is;
}

/**
 width height

 wrm_filename[,png_filename][ ignore-text]
 ...
 [--
 [ignore-text]]
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
    os << data.width << ' ' << data.height << '\n';
    for (std::size_t i = 0, last = data.files.size(); i < last; ++i)
    {
        os << data.files[i].first;
        if (!data.files[i].second.empty()){
            os << ',' << data.files[i].second;
        }
        os << '\n';
    }
    return os << "--\n";
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