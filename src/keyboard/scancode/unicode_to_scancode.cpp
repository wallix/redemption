#include "autogen/all_LCIDs_scancodes.hpp"

#include "unicode_to_scancode.hpp"

namespace scancode
{    
    namespace
    {
        enum Mods : std::uint8_t
        {
           NO_MOD = 0x00,
           SHIFT_MOD = 0x01,
           ALTGR_MOD = 0x02,
           CAPSLOCK_MOD = 0x04,
           CTRL_MOD = 0x08
        };

        enum ModifierKeyScancodes : std::uint16_t
        {
           SHIFT = 0x002A,
           ALTGR = 0xE038,
           CAPSLOCK = 0x003A,
           CTRL = 0x001D
        };

    
        inline void push_scancode(ScancodeSeq& scancode_seq,
                                  Scancode8bits scancode,
                                  std::uint8_t mod_flags = 0x00)
        {
            if (mod_flags & Mods::SHIFT_MOD)
            {
                scancode_seq.push(ModifierKeyScancodes::SHIFT);
            }
            if (mod_flags & Mods::ALTGR_MOD)
            {
                scancode_seq.push(ModifierKeyScancodes::ALTGR);
            }
            if (mod_flags & Mods::CAPSLOCK_MOD)
            {
                scancode_seq.push(ModifierKeyScancodes::CAPSLOCK);
            }
            if (mod_flags & Mods::CTRL_MOD)
            {
                scancode_seq.push(ModifierKeyScancodes::CTRL);
            }
            scancode_seq.push(scancode);
        }
    }
    
    
    void ScancodeSeq::push(Scancode16bits scancode)
    {
        assert(idx_ < MAX_SCANCODES);
        scancodes_[idx_++] = scancode;
    }

    array_view<Scancode16bits> ScancodeSeq::scancodes() const noexcept
    {
        return {scancodes_.data(), idx_};
    }

   
    ScancodeSeq unicode_to_scancode(int LCID, std::uint16_t unicode)
    {
        const auto *scancodes_table_ptr =
            get_scancodes_table_ptr(LCID);
        const auto *extended_scancodes_table_ptr =
            get_extended_scancodes_table_ptr(LCID);

        if (!scancodes_table_ptr || !extended_scancodes_table_ptr)
        {
            return { };
        }

        const auto& scancodes_table = *scancodes_table_ptr;
        ScancodeSeq scancode_seq;
        auto scancode_simple_pusher =
        [&scancode_seq, &scancodes_table](std::uint16_t unicode) -> bool
        {
            auto scancodes_table_it = scancodes_table.find(unicode);

            if (scancodes_table_it != scancodes_table.cend())
            {
                push_scancode(scancode_seq,
                              scancodes_table_it->second.first,
                              scancodes_table_it->second.second);
                return true;
            }
            return false;
        };

        // try to find scancode and push sequence
        if (scancode_simple_pusher(unicode))
        {
            return scancode_seq;
        }

    
        const auto& extended_scancodes_table = *extended_scancodes_table_ptr;
    
        // try to find extended scancode and push sequence
        auto extended_scancodes_table_it =
            extended_scancodes_table.find(unicode);

        if (extended_scancodes_table_it != extended_scancodes_table.cend())
        {
            // push extended scancode
            push_scancode(scancode_seq,
                          extended_scancodes_table_it->second.first);

            /* retry to find scancode from unicode previously retrieved 
               in extended scancode table info, and push sequence */
            if (scancode_simple_pusher
                (extended_scancodes_table_it->second.second))
            {
                return scancode_seq;
            }
        }

        return { };
    }
}
