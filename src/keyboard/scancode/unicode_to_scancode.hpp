#pragma once

# include <cstdint>
# include <array>

# include "utils/sugar/array_view.hpp"

namespace scancode
{
    using Scancode8bits = std::uint8_t;
    
    // scancode in 16 bits because of extended scancode like ALTGR scancode...
    using Scancode16bits = std::uint16_t;

    class ScancodeSeq
    {
    public :
        void push(Scancode16bits scancode);
        array_view<Scancode16bits> scancodes() const noexcept;

    private :
        enum { MAX_SCANCODES = 8 };

        std::array<Scancode16bits, MAX_SCANCODES> scancodes_;
        std::uint8_t idx_ = 0;
    };

    
    [[nodiscard]]
    ScancodeSeq unicode_to_scancode(int LCID, std::uint16_t unicode);
}
