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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/


#pragma once

#include <vector>
#include <functional>

namespace gdi {
    class GraphicApi;
    class CaptureApi;
    class CaptureProbeApi;
    class KbdInputApi;
    class ExternalCaptureApi;
    class UpdateConfigCaptureApi;
}

struct ApisRegister
{
    std::vector<std::reference_wrapper<gdi::GraphicApi>> * graphic_list;
    std::vector<std::reference_wrapper<gdi::CaptureApi>> * graphic_snapshot_list;
    std::vector<std::reference_wrapper<gdi::CaptureApi>> & capture_list;
    std::vector<std::reference_wrapper<gdi::KbdInputApi>> & kbd_input_list;
    std::vector<std::reference_wrapper<gdi::CaptureProbeApi>> & capture_probe_list;
    std::vector<std::reference_wrapper<gdi::ExternalCaptureApi>> & external_capture_list;
    std::vector<std::reference_wrapper<gdi::UpdateConfigCaptureApi>> & update_config_capture_list;
};


template<class T>
struct ApiRegisterElement
{
    using list_type = std::vector<std::reference_wrapper<T>>;

    ApiRegisterElement() = default;

    ApiRegisterElement(list_type & l, T & x)
    : l(&l)
    , i(l.size())
    {
        l.push_back(x);
    }

    ApiRegisterElement & operator = (ApiRegisterElement const &) = default;
    ApiRegisterElement & operator = (T & x) { (*this->l)[this->i] = x; return *this; }

    bool operator == (T const & x) const { return &this->get() == &x; }
    bool operator != (T const & x) const { return !(this == x); }

    T & get() { return (*this->l)[this->i]; }
    T const & get() const { return (*this->l)[this->i]; }

private:
    list_type * l = nullptr;
    std::size_t i = ~std::size_t{};
};

