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
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include "utils/sugar/noncopyable.hpp"

#include <iosfwd>


struct ConfigurationHolder : private noncopyable
{
    virtual void set_value(const char * section, const char * key, const char * value) = 0;

    virtual ~ConfigurationHolder() = default;
};

bool configuration_load(ConfigurationHolder & configuration_holder, std::istream & inifile_stream);
bool configuration_load(ConfigurationHolder & configuration_holder, const char * filename);
bool configuration_load(ConfigurationHolder & configuration_holder, std::string const & filename);

inline
bool configuration_load(ConfigurationHolder && configuration_holder, std::istream & inifile_stream)
{
    return configuration_load(configuration_holder, inifile_stream);
}

inline
bool configuration_load(ConfigurationHolder && configuration_holder, const char * filename)
{
    return configuration_load(configuration_holder, filename);
}

inline
bool configuration_load(ConfigurationHolder && configuration_holder, std::string const & filename)
{
    return configuration_load(configuration_holder, filename);
}
