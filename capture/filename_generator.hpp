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

#if !defined(__CAPTURE_FILENAME_GENERATOR__)
#define __CAPTURE_FILENAME_GENERATOR__

#include <string>
#include <limits>
#include <boost/lexical_cast.hpp>

#include <sys/types.h>
#include <unistd.h>

class FilenameGenerator
{
protected:
    std::string _filename;

public:
    FilenameGenerator(const std::string filename)
    : _filename(filename)
    {}

    virtual ~FilenameGenerator()
    {}

    virtual const std::string& operator()()
    { return this->_filename; }

    const std::string& next()
    { return (*this)(); }
};


class FilenameIncrementalGenerator
: public FilenameGenerator
{
    std::size_t _counter;
    std::string _extension;
    std::size_t _counter_position;

public:
    FilenameIncrementalGenerator(const std::string& filename, std::size_t first = 0)
    : FilenameGenerator(filename)
    , _counter(first)
    , _extension()
    , _counter_position(filename.length())
    {
        std::size_t p = filename.find_last_of('.');
        if (p != std::string::npos)
        {
            this->_extension = filename.substr(p);
            this->_filename[p] = '-';
            this->_filename.erase(p+1);
            this->_counter_position = p+1;
        }
    }

    FilenameIncrementalGenerator(const std::string& filename, const std::string& extension, std::size_t first = 0)
    : FilenameGenerator(filename)
    , _counter(first)
    , _extension(extension)
    , _counter_position(filename.length())
    {}

    virtual const std::string& operator()()
    {
        this->_filename.erase(this->_counter_position);
        this->_filename += boost::lexical_cast<std::string>(this->_counter);
        this->_filename += this->_extension;
        ++this->_counter;
        return this->_filename;
    }
};

inline std::string filename_to_pid_filename(const std::string& filename)
{
    std::size_t p = filename.find_last_of('.');
    std::string pid_filename = filename.substr(0, p);
    pid_filename += '-';
    pid_filename += boost::lexical_cast<std::string>(getpid());
    if (p != std::string::npos)
        pid_filename += filename.substr(p);
    return pid_filename;
}

inline FilenameGenerator make_pid_filename_generator(const std::string& filename)
{
    return FilenameGenerator(filename_to_pid_filename(filename));
}

inline FilenameGenerator make_pid_filename_generator(const std::string& filename, const std::string& extension)
{
    return FilenameGenerator(filename + boost::lexical_cast<std::string>(getpid()) + extension);
}

inline FilenameIncrementalGenerator make_pid_filename_incremental_generator(const std::string& filename, std::size_t first = 0)
{
    return FilenameIncrementalGenerator(filename_to_pid_filename(filename), first);
}

inline FilenameIncrementalGenerator make_pid_filename_incremental_generator(const std::string& filename, const std::string& extension, std::size_t first = 0)
{
    return FilenameIncrementalGenerator(filename + boost::lexical_cast<std::string>(getpid()), extension, first);
}

#endif