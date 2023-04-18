/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "headlessclient/headless_path.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/tm_to_chars.hpp"
#include "utils/strutils.hpp"


enum class HeadlessPath::ComponentType : char
{
    Text,
    IncrementedCounter,
    IncrementedGlobalCounter,
    DateTime,
    Time,
    Extension,
    AddExtension,
    Home,
    Suffix,
};

void HeadlessPath::compile(chars_view path)
{
    if (path.empty()) {
        components_ = {};
        return;
    }

    bool has_home = false;

    if (path[0] == '~') {
        path = path.drop_front(1);
        while (!path.empty() && path.front() == '/') {
            path = path.drop_front(1);
        }

        if (path.empty()) {
            static constexpr auto home = ComponentType::Home;
            components_ = {&home, 1};
            return;
        }

        has_home = true;
    }

    std::size_t component_len = has_home;
    std::size_t text_component_len = 1; // reserve 1 text
    std::size_t previous_text_index = 0;

    for (std::size_t i = 1; i < path.size(); ++i) {
        if (path[i-1] == '%') {
            switch (path[i]) {
                case '%':
                    ++i;
                    break;

                case 'i':
                case 'I':
                case 'd':
                case 'h':
                case 'e':
                case 'E':
                case 's':
                    if (previous_text_index != i-1) {
                        ++text_component_len;
                    }
                    previous_text_index = i+1;
                    ++component_len;
                    ++i;
                    break;
            }
        }
    }

    component_len += text_component_len;

    std::size_t data_cap = text_component_len + (path.size() + text_component_len + component_len) / sizeof(chars_view) + 1;
    if (data_cap > data_len_) {
        data_.reset(new chars_view[data_cap]);
        data_len_ = data_cap;
    }

    auto* text_ptr = data_.get();
    auto* component_ptr = reinterpret_cast<ComponentType*>(text_ptr + text_component_len); /* NOLINT */
    auto* str_ptr = reinterpret_cast<char*>(component_ptr + text_component_len + component_len); /* NOLINT */

    auto* text_ptr_start = text_ptr;
    auto* component_ptr_start = component_ptr;
    auto* str_ptr_start = str_ptr;

    if (has_home) {
        *component_ptr++ = ComponentType::Home;
    }

    bool add_ext = false;

    for (std::size_t i = 0; i < path.size(); ++i) {
        if (path[i] == '%' && i + 1 < path.size()) {
            switch (path[i+1]) {
                case '%':
                    ++i;
                    break;

                case 'E':
                    ++i;
                    add_ext = true;
                    continue;

                case 'i':
                case 'I':
                case 'd':
                case 'h':
                case 'e':
                case 's':
                    if (str_ptr != str_ptr_start) {
                        *component_ptr++ = ComponentType::Text;
                        *text_ptr++ = {str_ptr_start, str_ptr};
                        str_ptr_start = str_ptr;
                    }

                    switch (path[i+1]) {
                        case 'i': *component_ptr++ = ComponentType::IncrementedCounter; break;
                        case 'I': *component_ptr++ = ComponentType::IncrementedGlobalCounter; break;
                        case 'd': *component_ptr++ = ComponentType::DateTime; break;
                        case 'h': *component_ptr++ = ComponentType::Time; break;
                        case 'e': *component_ptr++ = ComponentType::Extension; break;
                        case 's': *component_ptr++ = ComponentType::Suffix; break;
                    }

                    ++i;
                    continue;
            }
        }

        *str_ptr++ = path[i];
    }

    if (str_ptr != str_ptr_start) {
        *component_ptr++ = ComponentType::Text;
        *text_ptr++ = {str_ptr_start, str_ptr};
    }

    if (add_ext) {
        *component_ptr++ = ComponentType::AddExtension;
    }

    components_ = {component_ptr_start, component_ptr};
    texts_ = {text_ptr_start, text_ptr};

    output_buffer_.clear();

    // pre-compute first text (general case)
    if (!components_.empty() && components_[0] == ComponentType::Text) {
        output_buffer_.reserve(texts_[0].size() + 64);
        output_buffer_.assign(texts_[0].begin(), texts_[0].end());
    }
}

HeadlessPath::ComputedPath HeadlessPath::compute_path(Context ctx)
{
    if (components_.empty()
     || (ctx.filename.size() >= 3 && ctx.filename[0] == '.' && ctx.filename[1] == '.' && ctx.filename[1] == '/')
     || (ctx.filename.size() >= 2 && ctx.filename[0] == '.' && ctx.filename[1] == '/')
     || (ctx.filename.size() >= 1 && ctx.filename[0] == '/')
    ) {
        return {ctx.filename, true};
    }

    auto components = components_;
    auto* text_it = texts_.data();

    // ignore pre-computed first text (general case)
    if (components[0] == ComponentType::Text) {
        output_buffer_.resize(texts_[0].size());
        components = components.drop_front(1);
        ++text_it;
    }
    else {
        output_buffer_.clear();
    }

    auto insert_text = [this](chars_view av) {
        output_buffer_.insert(output_buffer_.end(), av.begin(), av.end());
    };

    auto insert_int = [&](unsigned num) {
        auto s = int_to_decimal_chars(num);
        constexpr std::size_t zero_pad = 6;
        if (s.size() < zero_pad) {
            auto* zero = "000000";
            output_buffer_.insert(output_buffer_.end(), zero, zero + zero_pad - s.size());
        }
        insert_text(s);
    };

    auto insert_date = [&](auto date_format) {
        char buffer[date_format.output_length];
        tm res;
        auto duration = ctx.real_time.time_since_epoch();
        time_t sec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        localtime_r(&sec, &res);
        insert_text(chars_view(buffer, date_format.to_chars(buffer, res)));
    };

    bool add_ext = false;
    bool is_regular = true;

    for (auto component : components) {
        switch (component) {
            case ComponentType::Text:
                insert_text(*text_it);
                ++text_it;
                break;

            case ComponentType::IncrementedCounter:
                insert_int(ctx.counter);
                is_regular = false;
                break;

            case ComponentType::IncrementedGlobalCounter:
                insert_int(ctx.global_counter);
                is_regular = false;
                break;

            case ComponentType::DateTime:
                insert_date(dateformats::YYYY_mm_dd());
                is_regular = false;
                break;

            case ComponentType::Time:
                insert_date(dateformats::HH_MM_SS());
                is_regular = false;
                break;

            case ComponentType::Extension:
                insert_text((!ctx.extension.empty() && ctx.extension.front() == '.')
                    ? ctx.extension.drop_front(1)
                    : ctx.extension
                );
                break;

            case ComponentType::Home:
                insert_text(ctx.home);
                break;

            case ComponentType::Suffix:
                insert_text(ctx.suffix);
                break;

            case ComponentType::AddExtension:
                add_ext = true;
                break;
        }
    }

    insert_text(ctx.filename);

    if (add_ext && !utils::ends_with(output_buffer_, ctx.extension)) {
        insert_text(ctx.extension);
    }

    output_buffer_.push_back('\0');
    return {zstring_view::from_null_terminated(chars_view(output_buffer_).drop_back(1)), is_regular};
}
