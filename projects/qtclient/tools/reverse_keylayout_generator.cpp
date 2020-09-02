#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <map>
#include <iomanip>
#include <cerrno>
#include <cctype>
#include <cstring>
#include "keyboard/keylayouts.hpp"



static std::reference_wrapper<const Keylayout> keylayouts[] = {
    keylayout_x00000405, keylayout_x00000406, keylayout_x00000407, keylayout_x00000408,
    keylayout_x00000409, keylayout_x0000040a, keylayout_x0000040b, keylayout_x0000040c,
    keylayout_x0000040f, keylayout_x00000410, keylayout_x00000413, keylayout_x00000414,
    keylayout_x00000415, keylayout_x00000416, keylayout_x00000418, keylayout_x00000419,
    keylayout_x0000041a, keylayout_x0000041b, keylayout_x0000041d, keylayout_x0000041f,
    keylayout_x00000422, keylayout_x00000424, keylayout_x00000425, keylayout_x00000426,
    keylayout_x00000427, keylayout_x0000042f, keylayout_x00000438, keylayout_x0000043a,
    keylayout_x0000043b, keylayout_x0000043f, keylayout_x00000440, keylayout_x00000444,
    keylayout_x00000450, keylayout_x00000452, keylayout_x0000046e, keylayout_x00000481,
    keylayout_x00000807, keylayout_x00000809, keylayout_x0000080a, keylayout_x0000080c,
    keylayout_x00000813, keylayout_x00000816, keylayout_x0000081a, keylayout_x0000083b,
    keylayout_x00000843, keylayout_x0000085d, keylayout_x00000c0c, keylayout_x00000c1a,
    keylayout_x00001009, keylayout_x0000100c, keylayout_x0000201a, keylayout_x00010402,
    keylayout_x00010405, keylayout_x00001809, keylayout_x00010407, keylayout_x00010408,
    keylayout_x0001040a, keylayout_x0001040e, keylayout_x00010409, keylayout_x00010410,
    keylayout_x00010415, keylayout_x00010416, keylayout_x00010419, keylayout_x0001041b,
    keylayout_x0001041f, keylayout_x00010426, keylayout_x00010427, keylayout_x0001043a,
    keylayout_x0001043b, keylayout_x0001080c, keylayout_x0001083b, keylayout_x00011009,
    keylayout_x00011809, keylayout_x00020405, keylayout_x00020408, keylayout_x00020409,
    keylayout_x0002083b, keylayout_x00030402, keylayout_x00030408, keylayout_x00030409,
    keylayout_x00040408, keylayout_x00040409, keylayout_x00050408, keylayout_x00060408
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

void tabToReversedMap(
    const Keylayout::KeyLayout_t & read,
    std::ostream & fichier,
    std::string_view lcid_str,
    std::string_view name);

void tabToReversedDeadMap(
    ArrayView<Keylayout::dkey_t> dkeys,
    std::ostream & fichier,
    std::string_view lcid_str);

int main(int ac, char** av)
{
    if (ac != 2) {
        std::cerr << av[0] << " reversed_keymaps_directory\n";
        return 1;
    }

    std::string predixdir = av[1];
    predixdir += "/keylayout_x";

    int i = 0;
    for(Keylayout const& layout : keylayouts)
    {
        unsigned LCIDreverse = layout.LCID + 0x80000000u;
        std::stringstream ss;
        ss << std::hex << LCIDreverse;
        std::string LCIDreverse_str = ss.str();
        std::string locale_name = layout.locale_name;

        for (char& c : locale_name) {
            if (c == '.' || c == '-') {
                c = '_';
            }
            else {
                c = std::toupper(c);
            }
        }

        std::cout
          << locale_name << std::setw(29 - locale_name.size()) << " = 0x"
          << LCIDreverse_str << (!(++i % 3) ? ",\n" : ",    ");

        auto outfilename = predixdir + LCIDreverse_str + ".hpp";
        std::ofstream fichier(outfilename, std::ios::trunc);

        if (!fichier)
        {
            std::cerr << "Error: " << outfilename << ": " << strerror(errno) << std::endl;
            return 2;
        }

        fichier <<
            "#pragma once\n\n"
            "#include \"keylayout_r.hpp\"\n"
            << (layout.nbDeadkeys ? "\n" : "#include \"no_dead_key_array.hpp\"\n\n") <<
            "namespace\n{\n\n"
            "constexpr static int x" << LCIDreverse_str << "_LCID = 0x"
                << std::hex << layout.LCID << std::dec << ";\n\n"
            "constexpr static char const * x" << LCIDreverse_str << "_locale_name = \""
                << layout.locale_name << "\";\n\n";

        tabToReversedMap(layout.noMod,               fichier, LCIDreverse_str, "noMod");
        tabToReversedMap(layout.shift,               fichier, LCIDreverse_str, "shift");
        tabToReversedMap(layout.altGr,               fichier, LCIDreverse_str, "altGr");
        tabToReversedMap(layout.shiftAltGr,          fichier, LCIDreverse_str, "shiftAltGr");
        tabToReversedMap(layout.capslock_noMod,      fichier, LCIDreverse_str, "capslock_noMod");
        tabToReversedMap(layout.capslock_shift,      fichier, LCIDreverse_str, "capslock_shift");
        tabToReversedMap(layout.capslock_altGr,      fichier, LCIDreverse_str, "capslock_altGr");
        tabToReversedMap(layout.capslock_shiftAltGr, fichier, LCIDreverse_str, "capslock_shiftAltGr");
        tabToReversedMap(layout.ctrl,                fichier, LCIDreverse_str, "ctrl");

        if (layout.nbDeadkeys) {
            tabToReversedDeadMap(view(layout.deadkeys, layout.nbDeadkeys), fichier, LCIDreverse_str);
        }
        else {
            fichier
                << "constexpr Keylayout_r::DeadKeyLayoutMap_t x"
                << LCIDreverse_str << "_dead{no_dead_key_array, nullptr, nullptr};\n\n"
            ;
        }

        fichier <<
            "static const Keylayout_r keylayout_x"<< LCIDreverse_str <<"{\n"
            "    x" << LCIDreverse_str <<"_LCID,\n"
            "    x" << LCIDreverse_str <<"_locale_name,\n"
            "    x" << LCIDreverse_str <<"_noMod,\n"
            "    x" << LCIDreverse_str <<"_shift,\n"
            "    x" << LCIDreverse_str <<"_altGr,\n"
            "    x" << LCIDreverse_str <<"_shiftAltGr,\n"
            "    x" << LCIDreverse_str <<"_ctrl,\n"
            "    x" << LCIDreverse_str <<"_capslock_noMod,\n"
            "    x" << LCIDreverse_str <<"_capslock_shift,\n"
            "    x" << LCIDreverse_str <<"_capslock_altGr,\n"
            "    x" << LCIDreverse_str <<"_capslock_shiftAltGr,\n"
            "    x" << LCIDreverse_str <<"_dead,\n"
            "};\n\n"
            "}\n\n";
    }

    return 0;
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

    friend std::ostream& operator<<(std::ostream& out, Hex16 const& h)
    {
        out << "0x";
        Hex8{uint8_t(h.x >> 8)}.push(out);
        Hex8{uint8_t(h.x)}.push(out);
        return out;
    }
};

struct Hex32
{
    uint32_t x;

    friend std::ostream& operator<<(std::ostream& out, Hex32 const& h)
    {
        out << "0x";
        Hex8{uint8_t(h.x >> 24)}.push(out);
        Hex8{uint8_t(h.x >> 16)}.push(out);
        Hex8{uint8_t(h.x >> 8)}.push(out);
        Hex8{uint8_t(h.x)}.push(out);
        return out;
    }
};

struct Hex8C
{
    int i;
    uint8_t x;

    friend std::ostream& operator<<(std::ostream& out, Hex8C const& h)
    {
        if (h.i % 8) {
            out << " ";
        }
        else {
            out << "/* ";
            Hex8{uint8_t(h.i)}.push(out);
            out << " - ";
            Hex8{uint8_t(h.i+7)}.push(out);
            out << " */ ";
        }

        out << Hex8{h.x};

        if (' ' <= h.i && h.i <= '~') {
            out << " /*" << char(h.i) << "*/";
        }
        else {
            out << "      ";
        }

        return out << (((h.i+1) % 8) ? "," : ",\n");
    }
};

void tabToReversedMap(
    const Keylayout::KeyLayout_t & read,
    std::ostream & fichier,
    std::string_view lcid_str,
    std::string_view name)
{
    static_assert(std::is_same_v<uint16_t const&, decltype(read[0])>);

    using Scancodes = std::array<uint8_t, 256>;
    using Prefix = uint8_t;

    std::map<Prefix, Scancodes> map;
    map[0]; /* create empty table */

    {
        uint8_t i = 0;
        for (uint16_t uchar : read) {
            if (uchar) {
                map[uchar >> 8][uchar & 0xffu] = i;
            }
            ++i;
        }
    }

    for (auto&& p : map) {
        fichier
            << "constexpr Keylayout_r::KeyLayoutMap_t::scancode_type x"
            << lcid_str << "_scancodes_" << Hex8{p.first} << "_" << name << "[] {\n"
        ;
        int i = 0;
        for (uint8_t idx : p.second) {
            fichier << Hex8C{i, idx};
            ++i;
        }
        fichier << "};\n\n";
    }

    fichier
        << "constexpr Keylayout_r::KeyLayoutMap_t::data_type x"
        << lcid_str << "_data_" << name << "[] {\n";
    for (auto&& p : map) {
        fichier
            << "    { " << Hex8{p.first} << ", x"
            << lcid_str << "_scancodes_" << Hex8{p.first} << "_" << name << " },\n"
        ;
    }
    fichier
        << "};\n\n"
        << "constexpr Keylayout_r::KeyLayoutMap_t x" << lcid_str
        << "_" << name << "{ array_view{x" << lcid_str << "_data_" << name << "} };\n\n";
}

void tabToReversedDeadMap(
    ArrayView<Keylayout::dkey_t> dkeys,
    std::ostream & fichier,
    std::string_view lcid_str)
{
    using Scancodes = std::array<uint8_t, 256>;
    using Prefix = uint8_t;
    struct KeyLayoutMap : std::map<Prefix, Scancodes>
    {
        uint8_t idx;
    };
    using DeadScancodes = std::map<uint8_t, KeyLayoutMap>;

    std::array<DeadScancodes, 256> dead_map;

    for (Keylayout::dkey_t const& dkey : dkeys) {
        KeyLayoutMap& map = dead_map[dkey.uchar >> 8][dkey.uchar & 0xffu];
        for (Keylayout::dkey_key_t const& dkeykey : view(dkey.secondKeys, dkey.nbSecondKeys)) {
            map[dkeykey.modifiedKey >> 8][dkeykey.modifiedKey & 0xffu] = dkeykey.secondKey;
        }
    }

    uint8_t i = 0;
    for (auto&& dead_scancodes : dead_map) {
        if (dead_scancodes.empty()) {
            continue;
        }

        for (auto&& maps : dead_scancodes) {
            for (auto&& p : maps.second) {
                fichier
                    << "constexpr Keylayout_r::KeyLayoutMap_t::scancode_type x"
                    << lcid_str << "_dead_scancodes_" << Hex8{i}
                    << "_" << Hex8{maps.first} << "_" << Hex8{p.first} << "[] {\n"
                ;
                int i = 0;
                for (uint8_t idx : p.second) {
                    fichier << Hex8C{i, idx};
                    ++i;
                }
                fichier << "};\n\n";
            }


            fichier
                << "constexpr Keylayout_r::KeyLayoutMap_t::data_type x"
                << lcid_str << "_dead_data_" << Hex8{i}
                << "_" << Hex8{maps.first} << "[] {\n"
            ;
            for (auto&& p : maps.second) {
                fichier
                    << "    { " << Hex8{p.first} << ", x"
                    << lcid_str << "_dead_scancodes_" << Hex8{i}
                    << "_" << Hex8{maps.first} << "_" << Hex8{p.first} << " },\n"
                ;
            }
            fichier << "};\n\n";
        }

        ++i;
    }


    fichier
        << "constexpr Keylayout_r::KeyLayoutMap_t x"
        << lcid_str << "_dead_layouts[] {\n"
    ;
    i = 0;
    uint8_t id = 0;
    for (auto&& dead_scancodes : dead_map) {
        if (dead_scancodes.empty()) {
            continue;
        }

        for (auto&& maps : dead_scancodes) {
            fichier
                << "    {x" << lcid_str << "_dead_data_" << Hex8{i}
                << "_" << Hex8{maps.first} << "},\n"
            ;
            maps.second.idx = id++;
        }

        ++i;
    }
    fichier << "};\n\n";


    i = 0;
    for (auto&& dead_scancodes : dead_map) {
        if (dead_scancodes.empty()) {
            continue;
        }

        fichier << "constexpr uint8_t x" << lcid_str << "_dead_ids_" << Hex8{i} << "[] {\n";
        for (int idx = 0; idx < 256; ++idx) {
            auto p = dead_scancodes.find(uint8_t(unsigned(idx)));
            uint8_t real_id = (p == dead_scancodes.end()) ? 0 : p->second.idx + 1u;
            fichier << Hex8C{idx, real_id};
        }
        fichier << "};\n\n";

        ++i;
    }


    fichier << "constexpr uint8_t const* x" << lcid_str << "_dead_ids2[] {\n";
    i = 0;
    for (auto&& dead_scancodes : dead_map) {
        if (dead_scancodes.empty()) {
            continue;
        }

        fichier << "    x" << lcid_str << "_dead_ids_" << Hex8{i} << ",\n";

        ++i;
    }
    fichier << "};\n\n";


    fichier << "constexpr uint8_t x" << lcid_str << "_dead_id1[] {\n";
    i = 0;
    id = 0;
    for (auto&& dead_scancodes : dead_map) {
        uint8_t real_id = dead_scancodes.empty() ? 0 : ++id;
        fichier << Hex8C{i, real_id};
        ++i;
    }
    fichier << "};\n\n";


    fichier
        << "constexpr Keylayout_r::DeadKeyLayoutMap_t x"
        << lcid_str << "_dead{\n    x" << lcid_str << "_dead_id1,\n    x"
        << lcid_str << "_dead_ids2,\n    x" << lcid_str << "_dead_layouts,\n};\n\n";
}
