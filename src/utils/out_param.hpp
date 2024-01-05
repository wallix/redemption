/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

template<class T>
struct OutParam
{
    explicit OutParam(T& out_value) noexcept
    : out_value(out_value)
    {}

    OutParam(OutParam const&) = default;
    OutParam& operator==(OutParam const&) = delete;

    T& out_value;
};
