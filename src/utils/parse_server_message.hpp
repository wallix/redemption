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
   Author(s): Christophe Grosjean, Raphael ZHOU
*/

#pragma once

#include <string>
#include <vector>

inline bool parse_server_message(const char * svr_msg, std::string & order_ref, std::vector<std::string> & parameters_ref) {
    order_ref.clear();
    parameters_ref.clear();

    const char * separator = strchr(svr_msg, '=');

    if (separator) {
        order_ref.assign(svr_msg, separator - svr_msg);

        const char * params = (separator + 1);

        /** TODO
         * for (r : get_split(separator, this->server_message.c_str() + this->server_message.size(), '\ x01')) {
         *     parameters.push_back({r.begin(), r.end()});
         * }
         */
        while ((separator = ::strchr(params, '\x01')) != nullptr) {
            parameters_ref.emplace_back(params, separator - params);

            params = (separator + 1);
        }
        parameters_ref.emplace_back(params);
    }
    else {
        order_ref.assign(svr_msg);
    }

    return order_ref.length();
}
