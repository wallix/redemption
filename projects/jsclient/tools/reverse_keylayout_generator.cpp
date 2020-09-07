#include <iostream>
#include <limits>
#include <array>
#include <cctype>
#include "../src/keyboard/keylayouts.hpp"

namespace
{

static const Keylayout* keylayouts[] = {
    &keylayout_x00000405, &keylayout_x00000406, &keylayout_x00000407, &keylayout_x00000408,
    &keylayout_x00000409, &keylayout_x0000040a, &keylayout_x0000040b, &keylayout_x0000040c,
    &keylayout_x0000040f, &keylayout_x00000410, &keylayout_x00000413, &keylayout_x00000414,
    &keylayout_x00000415, &keylayout_x00000416, &keylayout_x00000418, &keylayout_x00000419,
    &keylayout_x0000041a, &keylayout_x0000041b, &keylayout_x0000041d, &keylayout_x0000041f,
    &keylayout_x00000422, &keylayout_x00000424, &keylayout_x00000425, &keylayout_x00000426,
    &keylayout_x00000427, &keylayout_x0000042f, &keylayout_x00000438, &keylayout_x0000043a,
    &keylayout_x0000043b, &keylayout_x0000043f, &keylayout_x00000440, &keylayout_x00000444,
    &keylayout_x00000450, &keylayout_x00000452, &keylayout_x0000046e, &keylayout_x00000481,
    &keylayout_x00000807, &keylayout_x00000809, &keylayout_x0000080a, &keylayout_x0000080c,
    &keylayout_x00000813, &keylayout_x00000816, &keylayout_x0000081a, &keylayout_x0000083b,
    &keylayout_x00000843, &keylayout_x0000085d, &keylayout_x00000c0c, &keylayout_x00000c1a,
    &keylayout_x00001009, &keylayout_x0000100c, &keylayout_x0000201a, &keylayout_x00010402,
    &keylayout_x00010405, &keylayout_x00001809, &keylayout_x00010407, &keylayout_x00010408,
    &keylayout_x0001040a, &keylayout_x0001040e, &keylayout_x00010409, &keylayout_x00010410,
    &keylayout_x00010415, &keylayout_x00010416, &keylayout_x00010419, &keylayout_x0001041b,
    &keylayout_x0001041f, &keylayout_x00010426, &keylayout_x00010427, &keylayout_x0001043a,
    &keylayout_x0001043b, &keylayout_x0001080c, &keylayout_x0001083b, &keylayout_x00011009,
    &keylayout_x00011809, &keylayout_x00020405, &keylayout_x00020408, &keylayout_x00020409,
    &keylayout_x0002083b, &keylayout_x00030402, &keylayout_x00030408, &keylayout_x00030409,
    &keylayout_x00040408, &keylayout_x00040409, &keylayout_x00050408, &keylayout_x00060408
};

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

Mods operator|(Mods a, Mods b)
{
    return Mods(int(a) | int(b));
}

}

int main()
{
    auto& fichier = std::cout;

    fichier << "const layouts = [";

    for(Keylayout const* layout_ptr : keylayouts) {
        auto& layout = *layout_ptr;

        fichier
            << "\n{ name: \"" << layout.locale_name
            << "\", lcid: " << Hex32{uint32_t(layout.LCID)}
            << ", keymap: {\n"
        ;

        struct KeyCode
        {
            uint8_t scancode;
            uint16_t mod_flags;
        };
        // index is a unicode character
        std::array<KeyCode, std::numeric_limits<uint16_t>::max()> keycodes {};

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
                if (ucs) {
                    keycodes[ucs].scancode = scancode;
                    keycodes[ucs].mod_flags |= (1u << int(layout.mods));
                }
                ++scancode;
            }
        }

        const char * modnames[]{
            "       |",
            " NoMod |",
            "   |",
            " S |",
            "   |",
            " A |",
            "   |",
            " C |",
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
        };
        uint16_t ucs = 0;
        fichier <<
            "    // scancode = x & 0xff\n"
            "    // modMask = x & (0x10000 << mod)\n"
            "    //                  "
            "                      (S)hift, (C)trl, (A)ltGr, Caps(L)ock\n"
            "    //                  "
            "           mod:  0  | 1 | 2 | 3 |  4  | 5 |  6  |  7  |   8   |\n"
            ;
        for (KeyCode k : keycodes) {
            if (k.scancode) {
                fichier << "    " << UnicodeCJs{ucs} << ": 0x";
                Hex16{k.mod_flags}.push(fichier);
                Hex16{k.scancode}.push(fichier);
                fichier << ", //";
                for (int i = 0; i < 9; ++i) {
                    fichier << modnames[i*2 + ((k.mod_flags >> i) & 1)];
                }
                fichier << "\n";
            }
            ++ucs;
        }

        fichier << "}, deadmap: {";

        for (auto const& dkey : view(layout.deadkeys, layout.nbDeadkeys)) {
            fichier << "\n    //                   " << UnicodeC{dkey.uchar};
            for (auto const& dkk : view(dkey.secondKeys, dkey.nbSecondKeys)) {
                fichier
                    << "\n    " << UnicodeCJs{dkk.modifiedKey} << ": ["
                    << Hex8{dkey.extendedKeyCode} << ", "
                    << Hex8{keycodes[dkk.secondKey].scancode}
                    << "], //" << UnicodeC{dkk.secondKey}
                ;
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
                    << "\n        " << UnicodeCJs{dkk.secondKey} << ": "
                    << Hex8{keycodes[dkk.secondKey].scancode} << ", // "
                    << UnicodeC{dkk.modifiedKey} << "  " << Hex16{dkk.modifiedKey}
                ;
            }
            fichier << "\n    },\n";
        }

        fichier << "} },\n";
    }

    fichier << "];\n";
}
