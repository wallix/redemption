#include <cstdint>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>

#include "utils/sugar/zstring_view.hpp"
#include "utils/strutils.hpp"
#include "keyboard/keymap2.hpp"

namespace
{
    template <typename T,
              int STREAM_WIDTH_FIELD = 8>
    std::string number_to_hex_string(T number,
                                     const char *hex_prefix_fmt = "0x")
    {
        std::ostringstream oss;

        oss << hex_prefix_fmt
            << std::setfill('0')
            << std::setw(STREAM_WIDTH_FIELD)
            << std::hex
            << number;
        return oss.str();
    }

    struct Hex8
    {
        std::uint8_t x;

        void push(std::ostream& out) const
        {
            constexpr const char *HEX = "0123456789abcdef";

            out << HEX[x >> 4] << HEX[x & 0xF];
        }

        friend std::ostream& operator<<(std::ostream& out, const Hex8& h)
        {
            out << "0x";
            h.push(out);
            return out;
        }
    };

    struct Hex16
    {
        std::uint16_t x;

        void push(std::ostream& out) const
        {
            Hex8{static_cast<std::uint8_t>(x >> 8)}.push(out);
            Hex8{static_cast<std::uint8_t>(x)}.push(out);
        }

        friend std::ostream& operator<<(std::ostream& out, const Hex16& h)
        {
            out << "0x";
            h.push(out);
            return out;
        }
    };


    enum class Mods : std::uint8_t
    {
       NO_MOD = 0x00,
       SHIFT_MOD = 0x01,
       ALTGR_MOD = 0x02,
       CAPSLOCK_MOD = 0x04,
       CTRL_MOD = 0x08
    };

    inline constexpr Mods operator|(Mods a, Mods b) noexcept
    {
        return Mods(int(a) | int(b));
    }

    struct Layout
    {
        const Keylayout::KeyLayout_t& klayout;
        Mods mods;
    };

    using LCIDs_t = std::vector<std::string>;

    constexpr auto GEN_DIR_PATH = "src/keyboard/scancode/autogen/"_zv;
    constexpr auto FINAL_HEADER_FILENAME = "all_LCIDs_scancodes.hpp"_zv;
    constexpr auto FINAL_SOURCE_FILENAME = "all_LCIDs_scancodes.cpp"_zv;


    inline void write_body_all_LCIDs_scancodes_source_file(std::ofstream& ofs,
                                                           const LCIDs_t& lcids)
    {
        ofs << "namespace scancode\n"
            << "{\n";


        // Write function for get unicode to scancode conversion table
        ofs << "    const std::unordered_map<std::uint16_t, std::pair<std::uint8_t, std::uint8_t>> *get_scancodes_table_ptr(int LCID) noexcept\n"
            << "    {\n"
            << "        switch (LCID)\n"
            << "        {\n";

        for (const auto& lcid : lcids)
        {
            ofs << "           case 0" << lcid << " :\n"
                << "               return &" << lcid << "::SCANCODES_TABLE;\n";
        }

        ofs << "           default :\n"
            << "               return nullptr;\n"
            << "        }\n"
            << "    }\n"



            << "\n";



        // Write function for get extend unicode to scancode conversion table
        ofs << "    const std::unordered_map<std::uint16_t, std::pair<std::uint8_t, std::uint16_t>> *get_extended_scancodes_table_ptr(int LCID) noexcept\n"
            << "    {\n"
            << "        switch (LCID)\n"
            << "        {\n";

        for (const auto& lcid : lcids)
        {
            ofs << "           case 0" << lcid << " :\n"
                << "               return &" << lcid << "::EXTENDED_SCANCODES_TABLE;\n";
        }

        ofs << "           default :\n"
            << "               return nullptr;\n"
            << "        }\n"
            << "    }\n"



            << "}\n";
    }

    inline void write_header_all_LCIDs_scancodes_source_file(std::ofstream& ofs,
                                                             const LCIDs_t& lcids)
    {
        for (const auto& lcid : lcids)
        {
            ofs << "#include \"scancodes_" << lcid << ".hpp\"\n";
        }

        ofs << '\n'
            << "#include \"" << FINAL_HEADER_FILENAME << "\"\n\n";
    }

    inline void generate_all_LCIDs_scancodes_source_file(const LCIDs_t& lcids)
    {
        std::string file_path = str_concat(GEN_DIR_PATH,
                                           FINAL_SOURCE_FILENAME);
        std::ofstream ofs;

        ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        try
        {
            ofs.open(file_path);
            write_header_all_LCIDs_scancodes_source_file(ofs, lcids);
            write_body_all_LCIDs_scancodes_source_file(ofs, lcids);
            ofs.close();
        }
        catch (const std::ofstream::failure&)
        {
            std::cerr << "something wrong with \""
                      << FINAL_SOURCE_FILENAME
                      << " file\n";
            throw;
        }
    }



    inline void write_source_all_LCIDs_scancodes_header_file(std::ofstream& ofs)
    {
        ofs << "namespace scancode\n"
            << "{\n"
            << "    const std::unordered_map<std::uint16_t, std::pair<std::uint8_t, std::uint8_t>> *get_scancodes_table_ptr(int LCID) noexcept;\n"
            << "    const std::unordered_map<std::uint16_t, std::pair<std::uint8_t, std::uint16_t>> *get_extended_scancodes_table_ptr(int LCID) noexcept;\n"
            << "}\n";
    }

    inline void write_header_all_LCIDs_scancodes_header_file(std::ofstream& ofs)
    {
        ofs << "#pragma once\n\n"
            << "# include <cstdint>\n"
            << "# include <unordered_map>\n\n";
    }

    inline void generate_all_LCIDs_scancodes_header_file()
    {
        std::string file_path = str_concat(GEN_DIR_PATH,
                                           FINAL_HEADER_FILENAME);
        std::ofstream ofs;

        ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        try
        {
            ofs.open(file_path);
            write_header_all_LCIDs_scancodes_header_file(ofs);
            write_source_all_LCIDs_scancodes_header_file(ofs);
            ofs.close();
        }
        catch (const std::ofstream::failure&)
        {
            std::cerr << "something wrong with \""
                      << FINAL_HEADER_FILENAME
                      << " file\n";
            throw;
        }
    }



    void write_body_scancodes_LCID_file(std::ofstream& ofs,
                                        const Keylayout& layout,
                                        const std::string& LCID)
    {
        ofs << "// \"" << layout.locale_name << "\" locale \n"
            << "namespace " << LCID << '\n'
            << "{\n";


        // Write unicode to scancode + mod flags conversion table
        ofs << "  // key : unicode, value : scancode + mod flags [8 + 8 bits]\n"
            << "  inline const std::unordered_map<std::uint16_t, std::pair<std::uint8_t, std::uint8_t>> SCANCODES_TABLE\n"
            << "  {\n";

        std::unordered_set<std::uint16_t> unicode_occurences;

        for (const auto& wrapped_layout :
            {
                Layout{layout.noMod, Mods::NO_MOD},
                Layout{layout.shift, Mods::SHIFT_MOD},
                Layout{layout.altGr, Mods::ALTGR_MOD},
                Layout{layout.shiftAltGr, Mods::SHIFT_MOD | Mods::ALTGR_MOD},
                Layout{layout.ctrl, Mods::CTRL_MOD},
                Layout{layout.capslock_noMod, Mods::CAPSLOCK_MOD},
                Layout{layout.capslock_shift,
                       Mods::CAPSLOCK_MOD | Mods::SHIFT_MOD},
                Layout{layout.capslock_altGr,
                       Mods::CAPSLOCK_MOD | Mods::ALTGR_MOD},
                Layout{layout.capslock_shiftAltGr,
                       Mods::CAPSLOCK_MOD | Mods::SHIFT_MOD | Mods::ALTGR_MOD}
            })
        {
            /* don't generate with numpad scancodes because
               there are already ones */
            auto is_numpad = [](std::uint8_t scancode)
            {
                return scancode >= 0x47 && scancode <= 0x53;
            };
            std::uint8_t scancode = 0;

            for (std::uint16_t unicode : wrapped_layout.klayout)
            {
                if (unicode_occurences.find(unicode)
                    == unicode_occurences.cend())
                {
                    if (!is_numpad(scancode))
                    {
                        ofs << "    {"
                            << Hex16{unicode}
                            << ", {"
                            << Hex8{scancode}
                            << ", "
                            << Hex8{std::uint8_t(wrapped_layout.mods)}
                            << "}},\n";

                        unicode_occurences.emplace(unicode);
                    }
                }
                ++scancode;
            }
        }

        ofs << "  };\n\n";



        /* Write extended unicode to
           extended scancode + unicode conversion table */
        ofs << "  // key : extended unicode, value : extended scancode + unicode [8 + 16 bits]\n"
            << "  inline const std::unordered_map<std::uint16_t, std::pair<std::uint8_t, std::uint16_t>> EXTENDED_SCANCODES_TABLE\n"
            << "  {\n";

        std::unordered_set<std::uint16_t> extended_unicode_occurences;

        for (std::uint8_t i = 0; i < layout.nbDeadkeys; ++i)
        {
            for (std::uint8_t j = 0; j < layout.deadkeys[i].nbSecondKeys; ++j)
            {
                std::uint16_t extended_unicode =
                    layout.deadkeys[i].secondKeys[j].modifiedKey;

                if (extended_unicode_occurences.find(extended_unicode)
                    == extended_unicode_occurences.cend())
                {
                    ofs << "    {"
                        << Hex16{extended_unicode}
                        << ", {";

                    std::uint8_t extended_scancode =
                        layout.deadkeys[i].extendedKeyCode;
                    std::uint16_t unicode =
                        layout.deadkeys[i].secondKeys[j].secondKey;

                    ofs << Hex8{extended_scancode}
                        << ", "
                        << Hex16{unicode}
                        << "}},\n";

                    extended_unicode_occurences.emplace(extended_unicode);
                }
            }
        }

        ofs << "  };\n"

            << "}\n";
    }

    void write_header_scancodes_LCID_file(std::ofstream& ofs)
    {
        ofs << "#pragma once\n\n"
            << "# include <cstdint>\n"
            << "# include <unordered_map>\n\n";
    }

    void generate_scancodes_LCID_file(const Keylayout& layout,
                                      LCIDs_t& lcids)
    {
        constexpr auto FILENAME_PREFIX = "scancodes_"_zv;
        constexpr auto FILENAME_SUFFIX = ".hpp"_zv;

        std::string LCID = number_to_hex_string(layout.LCID, "x");
        std::string filename = str_concat(FILENAME_PREFIX,
                                          LCID,
                                          FILENAME_SUFFIX);
        std::string file_path = str_concat(GEN_DIR_PATH, filename);
        std::ofstream ofs;

        ofs.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        try
        {
            ofs.open(file_path);
            write_header_scancodes_LCID_file(ofs);
            write_body_scancodes_LCID_file(ofs, layout, LCID);
            ofs.close();

            lcids.emplace_back(std::move(LCID));
        }
        catch (const std::ofstream::failure&)
        {
            std::cerr << "something wrong with \""
                      << filename
                      << " file\n";
            throw;
        }
    }

    inline bool is_started_with_help_option(int ac, char **av) noexcept
    {
        using namespace std::string_view_literals;

        for (int i = 1; i < ac; ++i)
        {
            const char *option = av[i];

            if (option == "--help"sv || option == "-h"sv)
            {
                std::cout << "program for generate unicode to scancode "
                          << "conversion tables from keylayout_*.hpp files "
                          << "for each LCID file "
                          << "(to start at \"redemption\" project root)\n";
                return true;
            }
        }

        return false;
    }
}


int main(int ac, char **av)
{
    if (is_started_with_help_option(ac, av))
    {
        return EXIT_SUCCESS;
    }

    try
    {
        namespace fs = std::filesystem;

        if (fs::create_directories(GEN_DIR_PATH.c_str()))
        {
            std::cout << "create \"" << GEN_DIR_PATH << "\" directory\n";
        }


        std::cout << "generate scancodes from unicodes conversion tables files into \""
                  << GEN_DIR_PATH
                  << "\" directory...\n";

        LCIDs_t lcids;
        array_view<const Keylayout *> keylayouts = Keymap2::keylayouts();

        lcids.reserve(keylayouts.size());
        for (const Keylayout *layout_ptr : keylayouts)
        {
            generate_scancodes_LCID_file(*layout_ptr, lcids);
        }


        generate_all_LCIDs_scancodes_header_file();
        generate_all_LCIDs_scancodes_source_file(lcids);
    }
    catch (...)
    {
        std::cerr << "cannot end files generation because of serious error\n";
        return EXIT_FAILURE;
    }

    std::cout << "conversion tables files generation is ended\n";
    return EXIT_SUCCESS;
}
