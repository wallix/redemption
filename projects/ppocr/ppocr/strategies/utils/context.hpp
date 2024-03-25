/*
SPDX-FileCopyrightText: 2024 Wallix Proxies Team
SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <type_traits>


namespace ppocr {

class Image;

struct no_context
{
    void reset()
    {}
};

template<class Ctx>
struct cache_context : Ctx
{
    decltype(auto) compute(Image const & img)
    {
        Ctx& ctx = *this;
        if (!is_loaded) {
            is_loaded = true;
            ctx.compute(img);
        }
        return ctx.value();
    }

    void reset()
    {
        is_loaded = false;
    }

private:
    bool is_loaded = false;
};

namespace details_
{
    template<class... Ctx>
    struct contexts_t : Ctx...
    {
        void reset()
        {
            (..., static_cast<Ctx&>(*this).reset());
        }
    };

    template<class StackCtx, class... Ctx>
    struct unique_contexts_impl
    {
        using type = StackCtx;
    };

    template<class... StackCtx, class Ctx, class... OtherCtx>
    struct unique_contexts_impl<contexts_t<StackCtx...>, Ctx, OtherCtx...>
        : unique_contexts_impl<
            std::conditional_t<
                std::is_base_of_v<Ctx, contexts_t<StackCtx...>>,
                contexts_t<StackCtx...>,
                contexts_t<StackCtx..., Ctx>
            >,
            OtherCtx...
        >
    {};
}

template<class... Ctx>
using unique_contexts_t = typename details_::unique_contexts_impl<
    details_::contexts_t<>, Ctx...>::type;

}
