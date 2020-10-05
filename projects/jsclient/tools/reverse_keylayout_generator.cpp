#include <iostream>
#include <bitset>
#include <limits>
#include <algorithm>
#include <vector>
#include <array>
#include <cctype>
#include <cassert>
#include "keyboard/keymap2.hpp"


namespace
{

template<class T>
struct ArrayView
{
    T const* ptr;
    ptrdiff_t n;

    T const* begin() const { return ptr; }
    T const* end() const { return ptr + n; }
};

template<class T>
ArrayView<T> view(T const* p, ptrdiff_t n)
{
    return {p, n};
}

struct Hex8
{
    uint8_t x;

    void push(std::ostream& out) const
    {
        char const* hex = "0123456789abcdef";
        out << hex[x >> 4] << hex[x & 0xf];
    }

    friend std::ostream& operator<<(std::ostream& out, Hex8 const& h)
    {
        out << "0x";
        h.push(out);
        return out;
    }
};

struct Hex16
{
    uint16_t x;

    void push(std::ostream& out) const
    {
        Hex8{uint8_t(x >> 8)}.push(out);
        Hex8{uint8_t(x)}.push(out);
    }

    friend std::ostream& operator<<(std::ostream& out, Hex16 const& h)
    {
        out << "0x";
        h.push(out);
        return out;
    }
};

struct Hex32
{
    uint32_t x;

    void push(std::ostream& out) const
    {
        Hex16{uint16_t(x >> 16)}.push(out);
        Hex16{uint16_t(x)}.push(out);
    }

    friend std::ostream& operator<<(std::ostream& out, Hex32 const& h)
    {
        out << "0x";
        h.push(out);
        return out;
    }
};

struct UnicodeC
{
    uint16_t unicode;

    friend std::ostream& operator<<(std::ostream& out, UnicodeC const& u)
    {
        if (' ' <= u.unicode && u.unicode <= '~') {
            out << " ";
            out << uint8_t(u.unicode);
        }
        else if ('\a' <= u.unicode && u.unicode <= '\r') {
            out << "\\" << "abtnvfr"[u.unicode - '\a'];
        }
        else if (u.unicode & 0xF800u) {
            out << " ";
            out << uint8_t(0xE0 | ((u.unicode >> 12) & 0x0Fu));
            out << uint8_t(0x80 | ((u.unicode & 0x0F00u) >> 6) | ((u.unicode & 0xFFu) >> 6));
            out << uint8_t(0x80 | (u.unicode & 0x3Fu));
        }
        else if (u.unicode & 0xFF80u) {
            out << " ";
            out << uint8_t(0xC0 | ((u.unicode >> 6) & 0x1Cu) | ((u.unicode >> 6) & 0x3u));
            out << uint8_t(0x80 | (u.unicode & 0x3Fu));
        }
        else {
            out << "  ";
        }

        return out;
    }
};

struct UnicodeCJs
{
    uint16_t unicode;

    friend std::ostream& operator<<(std::ostream& out, UnicodeCJs const& u)
    {
        out << "/* " << UnicodeC{u.unicode} << " */ '\\u";
        Hex16{u.unicode}.push(out);
        return out << "'";
    }
};


enum class Mods : int8_t
{
    NO_MOD       = 0x00,
    SHIFT_MOD    = 0x01,
    ALTGR_MOD    = 0x02,
    CAPSLOCK_MOD = 0x04,
    CTRL_MOD     = 0x08,
};

inline constexpr std::size_t max_regular_mods = 9;

constexpr Mods operator|(Mods a, Mods b)
{
    return Mods(int(a) | int(b));
}

inline constexpr const char * modnames[]{
    "   |",
    " N |",
    "   |",
    " S |",
    "   |",
    " A |",
    "     |",
    " S+A |",
    "   |",
    " L |",
    "     |",
    " L+S |",
    "     |",
    " L+A |",
    "       |",
    " L+S+A |",
    "   ",
    " C ",
};

struct ScancodeMods
{
    uint8_t scancode;
    uint16_t mod_flags;

    friend std::ostream& operator<<(std::ostream& out, ScancodeMods const& x)
    {
        out << "0x";
        Hex16{x.mod_flags}.push(out);
        Hex16{x.scancode}.push(out);
        out << " /*";
        for (std::size_t i = 0; i < max_regular_mods; ++i) {
            out << modnames[i*2 + ((x.mod_flags >> i) & 1)];
        }
        return out << "*/";
    }
};

}

int main()
{
    auto& fichier = std::cout;

    fichier << "const layouts = [";

    for(Keylayout const* layout_ptr : Keymap2::keylayouts()) {
        auto& layout = *layout_ptr;

        fichier
            << "\n{ name: \"" << layout.locale_name
            << "\", lcid: " << Hex32{uint32_t(layout.LCID)}
            << ", keymap: {\n"
        ;

        auto is_numpad = [](uint8_t scancode){
            return 0x47 <= scancode && scancode <= 0x53;
        };

        auto is_ascii = [](uint16_t ucs){
            return 0x20 <= ucs && ucs <= 0x7E;
        };

        auto is_standard_scancode = [](uint8_t scancode){
            return scancode <= 0x60;
        };

        struct KeyInfo
        {
            uint8_t scancode;
            Mods mods;
        };

        using Keys = std::vector<KeyInfo>;

        // index is a unicode character
        std::array<Keys, std::numeric_limits<uint16_t>::max()> keycodes {};

        struct Layout
        {
            const Keylayout::KeyLayout_t & klayout;
            Mods mods;
        };
        for (auto layout : {
            Layout{layout.noMod, Mods::NO_MOD},
            Layout{layout.shift, Mods::SHIFT_MOD},
            Layout{layout.altGr, Mods::ALTGR_MOD},
            Layout{layout.ctrl, Mods::CTRL_MOD},
            Layout{layout.shiftAltGr, Mods::SHIFT_MOD | Mods::ALTGR_MOD},
            Layout{layout.capslock_noMod, Mods::CAPSLOCK_MOD},
            Layout{layout.capslock_shift, Mods::CAPSLOCK_MOD | Mods::SHIFT_MOD},
            Layout{layout.capslock_altGr, Mods::CAPSLOCK_MOD | Mods::ALTGR_MOD},
            Layout{layout.capslock_shiftAltGr, Mods::CAPSLOCK_MOD | Mods::SHIFT_MOD | Mods::ALTGR_MOD},
        }) {
            uint8_t scancode = 0;
            for (uint16_t ucs : layout.klayout) {
                if (ucs
                 && !is_numpad(scancode)
                 // filter stange values
                 && (!is_ascii(ucs) || is_standard_scancode(scancode))
                ) {
                    keycodes[ucs].push_back(KeyInfo{scancode, layout.mods});
                }
                ++scancode;
            }
        }

        auto for_each_key = [](Keys const& keys, auto f){
            assert(!keys.empty());
            auto first = keys.begin();
            auto last = keys.end();
            uint8_t scancode = first->scancode;
            unsigned mod_flags = 1u << int(first->mods);
            while (++first != last) {
                if (first->scancode != scancode) {
                    f(ScancodeMods{scancode, uint16_t(mod_flags)});
                    scancode = first->scancode;
                    mod_flags = 0;
                }
                mod_flags |= 1u << int(first->mods);
            }
            f(ScancodeMods{scancode, uint16_t(mod_flags)});
        };

        uint16_t ucs = 0;
        fichier <<
            "    // scancode = x & 0xff\n"
            "    // modMask = x & (0x10000 << mod)\n"
            "    //            (N)oMod, (S)hift, (C)trl, (A)ltGr, Caps(L)ock\n"
            "    //           mod: 0 | 1 | 2 | 3 |  4  | 5 |  6  |  7  |   8\n"
            ;
        for (Keys& keys : keycodes) {
            if (!keys.empty()) {
                std::sort(keys.begin(), keys.end(), [](KeyInfo& a, KeyInfo& b){
                    return a.scancode < b.scancode;
                });

                fichier << "    " << UnicodeCJs{ucs} << ": [\n";
                for_each_key(keys, [&](ScancodeMods scancode_mods){
                    fichier << "        " << scancode_mods << ",\n";
                });
                fichier << "    ],\n";
            }
            ++ucs;
        }

        fichier << "}, deadmap: {";

        for (auto const& dkey : view(layout.deadkeys, layout.nbDeadkeys)) {
            fichier << "\n    //                   " << UnicodeC{dkey.uchar};
            for (auto const& dkk : view(dkey.secondKeys, dkey.nbSecondKeys)) {
                fichier
                    << "\n    " << UnicodeCJs{dkk.modifiedKey} << ": ["
                    << Hex8{dkey.extendedKeyCode} << ", [\n        // "
                    << UnicodeC{dkk.secondKey} << "\n"
                ;
                for_each_key(keycodes[dkk.secondKey], [&](ScancodeMods scancode_mods){
                    fichier << "        " << scancode_mods << ",\n";
                });
                fichier << "    ]],\n";
            }
        }

        fichier << "\n}, deadmaps: {\n";

        for (auto const& dkey : view(layout.deadkeys, layout.nbDeadkeys)) {
            fichier
                << "    " << UnicodeCJs{dkey.uchar}
                << ": {\n        scancode: " << Hex8{dkey.extendedKeyCode}
                << ","
            ;
            for (auto const& dkk : view(dkey.secondKeys, dkey.nbSecondKeys)) {
                fichier
                    << "\n        " << UnicodeCJs{dkk.secondKey} << ": [ // "
                    << UnicodeC{dkk.modifiedKey} << "  " << Hex16{dkk.modifiedKey}
                    << "\n"
                ;
                for_each_key(keycodes[dkk.secondKey], [&](ScancodeMods scancode_mods){
                    fichier << "            " << scancode_mods << ",\n";
                });
                fichier << "        ],";
            }
            fichier << "\n    },\n";
        }

        fichier << "} },\n";
    }

    fichier << "];\n";
}
