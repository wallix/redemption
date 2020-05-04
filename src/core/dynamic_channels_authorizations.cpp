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
        array_view_const_char list,
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
            else {
                names.emplace_back(trimmed.begin(), trimmed.end());
            }
        }
        return false;
    };

    this->all_allow = extract(allow, this->allow_names);
    this->all_deny  = extract(deny, this->deny_names);

    if (this->allow_names.empty() && !this->all_allow) {
        this->all_deny = true;

        this->deny_names.clear();
    }

    if (this->deny_names.empty() && !this->all_deny) {
        this->all_allow = true;

        this->allow_names.clear();
    }

    if (!this->allow_names.empty() && !this->deny_names.empty()) {
        this->all_deny = true;

        this->deny_names.clear();
    }

    if (this->all_allow && this->all_deny) {
        this->all_allow = false;
    }

    LOG(LOG_INFO, "all_allow=%s", (this->all_allow ? "Yes" : "No"));
    LOG(LOG_INFO, "all_deny=%s",  (this->all_deny ? "Yes" : "No"));

    assert((this->all_allow && !this->all_deny) ||
           (!this->all_allow && this->all_deny));

    assert((this->all_allow && this->allow_names.empty()) ||
           !this->all_allow);

    assert((this->all_deny && this->deny_names.empty()) ||
           !this->all_deny);
}

bool DynamicChannelsAuthorizations::is_authorized(const char * name) const noexcept
{
    if (this->all_allow) {
        return (std::find(this->deny_names.begin(), this->deny_names.end(), name) == this->deny_names.end());
    }

    assert(this->all_deny);

    return (std::find(this->allow_names.begin(), this->allow_names.end(), name) != this->allow_names.end());
}
