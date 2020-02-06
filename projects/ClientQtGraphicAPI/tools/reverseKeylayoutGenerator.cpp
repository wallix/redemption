// g++ -std=c++17 -I ../../../src reverseKeylayoutGenerator.cpp

#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <functional>
#include <string_view>
#include <iomanip>
#include <cerrno>
#include <cctype>
#include <cstring>
#include "../src/keyboard/keylayouts.hpp"

void tabToReversedMap(const Keylayout::KeyLayout_t & read, std::ofstream & fichier, std::string_view name);

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
        int LCIDreverse = layout.LCID + 0x80000000;
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

        fichier << "#pragma once\n\n";
        fichier << "#include \"keylayout_r.hpp\"\n\n";
        fichier << "namespace x" << LCIDreverse_str << "\n{\n\n";
        fichier << "const static int LCID = 0x" << std::hex << layout.LCID << ";\n\n";
        fichier << "const static char * const locale_name = \"" << layout.locale_name << "\";\n\n";

        tabToReversedMap(layout.noMod,               fichier,  "noMod");
        tabToReversedMap(layout.shift,               fichier,  "shift");
        tabToReversedMap(layout.altGr,               fichier,  "altGr");
        tabToReversedMap(layout.shiftAltGr,          fichier,  "shiftAltGr");
        tabToReversedMap(layout.capslock_noMod,      fichier,  "capslock_noMod");
        tabToReversedMap(layout.capslock_shift,      fichier,  "capslock_shift");
        tabToReversedMap(layout.capslock_altGr,      fichier,  "capslock_altGr");
        tabToReversedMap(layout.capslock_shiftAltGr, fichier,  "capslock_shiftAltGr");
        tabToReversedMap(layout.ctrl,                fichier,  "ctrl");

        fichier << "const Keylayout_r::KeyLayoutMap_t deadkeys {\n";
        // TODO this is wrong
        // for (int i = 0; i < layout.nbDeadkeys; i++) {
        //     unsigned deadCode = layout.deadkeys[i].extendedKeyCode;
        //     fichier << "    { 0x" << std::setw(4) << layout.deadkeys[i].uchar << ", 0x" << std::setw(4) << deadCode << " },\n";
        //     int nbSecondDK(layout.deadkeys[i].nbSecondKeys);
        //     for (int j = 0; j < nbSecondDK; j++) {
        //         fichier << "    { 0x" << std::setw(4) << layout.deadkeys[i].secondKeys[j].secondKey << ", 0x" << std::setw(4) << layout.deadkeys[i].secondKeys[j].modifiedKey << " },\n";
        //     }
        // }
        fichier << std::dec;
        fichier << "};\n";
        fichier << "\n\n";
        fichier << "const static uint8_t nbDeadkeys = " << unsigned(layout.nbDeadkeys) << ";\n\n";
        fichier << "}\n\n";

        fichier << "static const Keylayout_r keylayout_x"<< LCIDreverse_str <<"(\n"
            "    x" << LCIDreverse_str <<"::LCID,\n"
            "    x" << LCIDreverse_str <<"::locale_name,\n"
            "    x" << LCIDreverse_str <<"::noMod,\n"
            "    x" << LCIDreverse_str <<"::shift,\n"
            "    x" << LCIDreverse_str <<"::altGr,\n"
            "    x" << LCIDreverse_str <<"::shiftAltGr,\n"
            "    x" << LCIDreverse_str <<"::ctrl,\n"
            "    x" << LCIDreverse_str <<"::capslock_noMod,\n"
            "    x" << LCIDreverse_str <<"::capslock_shift,\n"
            "    x" << LCIDreverse_str <<"::capslock_altGr,\n"
            "    x" << LCIDreverse_str <<"::capslock_shiftAltGr,\n"
            "    x" << LCIDreverse_str <<"::deadkeys,\n"
            "    x" << LCIDreverse_str <<"::nbDeadkeys\n"
            ");\n\n";
    }

    return 0;
}

void tabToReversedMap(const Keylayout::KeyLayout_t & read, std::ofstream & fichier, std::string_view name)
{
    struct P {
        size_t idx;
        uint32_t uchar;
    };
    std::array<P, std::size(decltype(read){})> layout_r;

    auto last = begin(layout_r);

    for(size_t i = 0; i < std::size(read); ++i) {
        if (read[i]) {
            *last = {i, read[i]};
            ++last;
        }
    }

    std::sort(begin(layout_r), last, [](auto& p1, auto& p2) {
        return p1.uchar < p2.uchar;
    });

    fichier << std::hex << std::setfill('0');
    fichier << "constexpr Keylayout_r::KeyLayoutMap_t::uchar_type uchar_"<< name << "[] {\n";
    int i = 0;
    std::for_each(begin(layout_r), last, [&](P& p){
        if (!(i % 8)) {
            fichier << "    ";
        };
        fichier << "0x" << std::setw(4) << p.uchar << ((++i % 8) ? ", " : ",\n");
    });
    fichier << "\n};\nconstexpr Keylayout_r::KeyLayoutMap_t::scancode_type scancode_"<< name << "[] {\n";
    i = 0;
    fichier << std::dec << std::setfill(' ');
    std::for_each(begin(layout_r), last, [&](P& p){
        if (!(i % 8)) {
            fichier << "    ";
        };
        fichier << std::setw(6) << p.idx << ((++i % 8) ? ", " : ",\n");
    });
    auto layout_r_size = (last - begin(layout_r));
    fichier << "\n};\nconstexpr Keylayout_r::KeyLayoutMap_t "<< name
        << "{uchar_"<< name << ", scancode_" << name << ", " << layout_r_size << "};\n\n";
}
