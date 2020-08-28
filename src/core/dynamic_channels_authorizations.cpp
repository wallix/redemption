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
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#include "core/dynamic_channels_authorizations.hpp"
#include "utils/log.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/array_view.hpp"
#include "utils/sugar/splitter.hpp"

#include <algorithm>
#include <cassert>

DynamicChannelsAuthorizations::DynamicChannelsAuthorizations(
    std::string const & allow, std::string const & deny)
{
    auto extract = [](
        chars_view list,
        std::vector<std::string> & names)
    {
        for (auto && r : get_split(list.begin(), list.end(), ',')) {
            auto trimmed = trim(begin(r), end(r));
            if (trimmed.empty()) {
                continue;
            }

            if ((trimmed[0] == '*') && (trimmed.size() == 1)) {
                names.clear();
                return true;
            }

            names.emplace_back(trimmed.begin(), trimmed.end());
        }

        return false;
    };

    this->all_allowed = extract(allow, this->allowed_names);
    this->all_denied  = extract(deny, this->denied_names);

    if (this->allowed_names.empty() && !this->all_allowed) {
        this->all_denied = true;

        this->denied_names.clear();
    }

    if (this->denied_names.empty() && !this->all_denied) {
        this->all_allowed = true;

        this->allowed_names.clear();
    }

    if (!this->allowed_names.empty() && !this->denied_names.empty()) {
        this->all_denied = true;

        this->denied_names.clear();
    }

    if (this->all_allowed && this->all_denied) {
        this->all_allowed = false;
    }

    LOG(LOG_INFO, "all_allowed=%s", (this->all_allowed ? "Yes" : "No"));
    LOG(LOG_INFO, "all_denied=%s",  (this->all_denied ? "Yes" : "No"));

    assert((this->all_allowed && !this->all_denied) ||
           (!this->all_allowed && this->all_denied));

    assert((this->all_allowed && this->allowed_names.empty()) ||
           !this->all_allowed);

    assert((this->all_denied && this->denied_names.empty()) ||
           !this->all_denied);
}

bool DynamicChannelsAuthorizations::is_authorized(const char * name) const noexcept
{
    if (this->all_allowed) {
        return (std::find(this->denied_names.begin(), this->denied_names.end(), name) == this->denied_names.end());
    }

    assert(this->all_denied);

    return (std::find(this->allowed_names.begin(), this->allowed_names.end(), name) != this->allowed_names.end());
}
