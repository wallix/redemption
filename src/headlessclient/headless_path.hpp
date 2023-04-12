/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "utils/sugar/array_view.hpp"
#include "utils/real_clock.hpp"
#include "utils/sugar/zstring_view.hpp"

#include <vector>
#include <memory>


// see help path of headless commande
struct HeadlessPath
{
    HeadlessPath() = default;

    HeadlessPath(HeadlessPath&&) noexcept = default;
    HeadlessPath& operator = (HeadlessPath&&) noexcept = default;

    // possible, but no trivial
    HeadlessPath(HeadlessPath const&) = delete;
    HeadlessPath& operator = (HeadlessPath const&) = delete;

    struct Context
    {
        unsigned counter;
        unsigned global_counter;
        RealTimePoint real_time;
        chars_view extension;
        zstring_view filename;
        chars_view suffix;
        chars_view home; // require '/' terminated when not empty
    };

    struct ComputedPath
    {
        zstring_view path;
        // have no dynamic parameter
        bool is_regular;
    };

    void compile(chars_view path);

    ComputedPath compute_path(Context ctx);

private:
    enum class ComponentType : char;

    array_view<ComponentType> components_;
    array_view<chars_view> texts_;
    std::size_t data_len_ = 0;
    std::unique_ptr<chars_view[]> data_;
    std::vector<char> output_buffer_;
};
