/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "utils/sugar/bytes_view.hpp"
#include "utils/sugar/noncopyable.hpp"


class Random : noncopyable
{
public:
             Random() = default;
    virtual ~Random() = default;

    virtual void random(writable_bytes_view buf) = 0;

    uint32_t rand32()
    {
        uint32_t ret;
        this->random(writable_bytes_view::from_raw_object(ret));
        return ret;
    }
};
