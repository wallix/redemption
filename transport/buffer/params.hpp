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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */

#ifndef REDEMPTION_PUBLIC_TRANSPORT_BUFFER_PARAMS_HPP
#define REDEMPTION_PUBLIC_TRANSPORT_BUFFER_PARAMS_HPP

namespace transbuf {
    const struct no_param_t {
        no_param_t(){} /*fix clang-3.2*/
    } no_param;

    template<class BufParams, class OtherParams>
    struct two_params
    {
        //tuple
        BufParams buf_params;
        OtherParams other_params;

        two_params(const BufParams & buf_params, const OtherParams & other_params)
        : buf_params(buf_params)
        , other_params(other_params)
        {}
    };

    template<class BufParams, class OtherParams>
    two_params<BufParams, OtherParams>
    buf_params(const BufParams & buf_params, const OtherParams & open_close_params)
    { return two_params<BufParams, OtherParams>(buf_params, open_close_params); }
}

#endif
