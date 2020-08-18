/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2012
   Author(s): Christophe Grosjean

   Unit test to conversion of RDP drawing orders to PNG images
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"
#include "test_only/log_buffered.hpp"

#include "utils/cfgloader.hpp"
#include "utils/sugar/algostring.hpp"

#include <fstream>

namespace
{

struct SectionOrKeyValue
{
    enum Type { Section, KeyValue, Unknown, };
    Type type;
    zstring_view str1;
    zstring_view str2;

    SectionOrKeyValue()
    : type(Unknown)
    {}

    SectionOrKeyValue(zstring_view section_name)
    : type(Section)
    , str1(section_name)
    {}

    SectionOrKeyValue(zstring_view key_name, zstring_view value_name)
    : type(KeyValue)
    , str1(key_name)
    , str2(value_name)
    {}

    friend std::ostream& operator<<(std::ostream& out, SectionOrKeyValue const& other)
    {
        switch (other.type)
        {
            case Section:
                out << "set_section(\"" << other.str1 << "\")";
                break;
            case KeyValue:
                out << "set_value(\"" << other.str1 << "\", \"" << other.str2 << "\")";
                break;
            case Unknown:
                out << "not value";
                break;
        }

        return out;
    }

    bool operator == (SectionOrKeyValue const& other) const
    {
        return this->type == other.type
            && this->str1.to_sv() == other.str1.to_sv()
        && (this->type == Section || this->str2.to_sv() == other.str2.to_sv());
    }
};

struct Cfg : ConfigurationHolder
{
    using func_t = void(*)(SectionOrKeyValue const&);

    Cfg(array_view<func_t> funcs, func_t funknown)
    : funcs(funcs)
    , f(funcs.data())
    , funknown(funknown)
    {}

    void set_section(zstring_view section) override
    {
        if (this->f == this->funcs.end()) {
            funknown(SectionOrKeyValue(section));
            return ;
        }

        (*this->f)(SectionOrKeyValue(section));
        ++this->f;
    }

    void set_value(zstring_view key, zstring_view value) override
    {
        if (this->f == this->funcs.end()) {
            funknown(SectionOrKeyValue(key, value));
            return ;
        }

        (*this->f)(SectionOrKeyValue(key, value));
        ++this->f;
    }

    array_view<func_t> funcs;
    func_t const * f;
    func_t funknown;
};

}

RED_AUTO_TEST_CASE_WF(TestCfgloader, wf)
{
    std::ofstream file(wf.c_str());

    #define PUSH_F(v) [](SectionOrKeyValue const& expected){ RED_CHECK(v == expected); }
    #define PUSH(s, v) (void(file << s), PUSH_F(v))

    Cfg::func_t funcs[]{
        PUSH("[abc]\n", SectionOrKeyValue("abc"_zv)),
        PUSH("abc=abc\n", SectionOrKeyValue("abc"_zv, "abc"_zv)),
        PUSH("\n\n   \n #blah blah\nvv=  plop\n", SectionOrKeyValue("vv"_zv, "plop"_zv)),
        PUSH("#[xx]\n[s]\n", SectionOrKeyValue("s"_zv)),
        PUSH("aaa=bbb   \n", SectionOrKeyValue("aaa"_zv, "bbb"_zv)),
        PUSH("[    ss  ]\n", SectionOrKeyValue("ss"_zv)),
        PUSH("  bbb=aaa\n", SectionOrKeyValue("bbb"_zv, "aaa"_zv)),
        PUSH("val=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n",
            SectionOrKeyValue("val"_zv, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"_zv)),
        PUSH("val2   =1\n", SectionOrKeyValue("val2"_zv, "1"_zv)),
        PUSH("[ a  a ]\n", SectionOrKeyValue("a  a"_zv)),
        PUSH("  v =  1 \n", SectionOrKeyValue("v"_zv, "1"_zv)),
        PUSH("  mac =  2 \n\r", SectionOrKeyValue("mac"_zv, "2"_zv)),
        PUSH("  mac =  3 \n\r", SectionOrKeyValue("mac"_zv, "3"_zv)),
        PUSH("  window =  4 \r\n", SectionOrKeyValue("window"_zv, "4"_zv)),
        PUSH("  window =  5 \r\n", SectionOrKeyValue("window"_zv, "5"_zv)),
        PUSH("  [bbb]", SectionOrKeyValue("bbb"_zv)),
        PUSH("v = 1\n", SectionOrKeyValue("v"_zv, "1"_zv)),
        PUSH("  [b bb]\n", SectionOrKeyValue("b bb"_zv)),
        PUSH("  [aa]  \n", SectionOrKeyValue("aa"_zv)),
        PUSH("v = [d] a\n", SectionOrKeyValue("v"_zv, "[d] a"_zv)),
        PUSH("v3 = x", SectionOrKeyValue("v3"_zv, "x"_zv)),
    };

    Cfg cfg(funcs, PUSH_F(SectionOrKeyValue()));
    file.close();
    RED_CHECK(configuration_load(cfg, wf.c_str()));
    RED_CHECK(0 == cfg.funcs.end() - cfg.f);
}

RED_AUTO_TEST_CASE_WF(TestCfgloaderError, wf)
{
    std::ofstream file(wf.c_str());

    #define PUSH_F(v) [](SectionOrKeyValue const& expected){ RED_CHECK(v == expected); }
    #define PUSH(s, v) (void(file << s), PUSH_F(v))

    Cfg::func_t funcs[]{
        PUSH("[bad\n", SectionOrKeyValue("bad"_zv)),
        PUSH("abc\n=dds\nx=y\n", SectionOrKeyValue("x"_zv, "y"_zv)),
    };

    Cfg cfg(funcs, PUSH_F(SectionOrKeyValue()));
    file.close();
    tu::log_buffered log;
    RED_CHECK(!configuration_load(cfg, wf.c_str()));
    RED_CHECK(0 == cfg.funcs.end() - cfg.f);
    RED_CHECK(array_view(log.buf()) == str_concat(
        "ERR -- "_zv, wf.string(), ":1:4: ']' not found, assume new section\n"
        "ERR -- "_zv, wf.string(), ":2:4: invalid syntax, expected '=' ; this line is ignored\n"
        "ERR -- "_zv, wf.string(), ":3:5: invalid syntax, expected '=' ; this line is ignored\n"
        ""_zv
    ));
}
