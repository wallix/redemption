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
    Copyright (C) Wallix 2019
    Author(s): Christophe Grosjean
*/

#pragma once



struct ClipboardVirtualChannelParams {
// Default Values: everything transmitted, everything is logged
    bool clipboard_down_authorized = true;
    bool clipboard_up_authorized   = true;
    bool clipboard_file_authorized = true;
    bool dont_log_data_into_syslog = false;
    bool dont_log_data_into_wrm    = false;
    bool log_only_relevant_clipboard_activities = false;

// Used for Tests only    
    friend ::std::ostream& operator<<(::std::ostream& ostr, ClipboardVirtualChannelParams const& x)
    {
        auto const & [a, b, c, d, e, f] = x;
        return ostr << " down=" << a << " up=" << b << " file=" << c 
                    << " no syslog=" << d << " no wrm=" << e << " no verbose=" << f;
    }
    
    bool operator==(const ClipboardVirtualChannelParams & other) const {
        auto const & [a, b, c, d, e, f] = *this;
        auto const & [a1, b1, c1, d1, e1, f1] = other;
        return (a == a1 && b == b1 && c == c1 && d == d1 && e == e1 && f == f1);
    }
// End Used for Tests only    
    
};

