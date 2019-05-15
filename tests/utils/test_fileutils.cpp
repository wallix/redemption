/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"

#include "utils/fileutils.hpp"

#include <functional>
#include <numeric>

#include <cstring>
#include <unistd.h> // for getgid


RED_AUTO_TEST_CASE(TestBasename)
{
    // basename() change behavior depending if <filegen.h> is included
    // or not. The POSIX version change it's argument, not the glibc one
    // we WANT to use the glibc one. This test below will fail if
    // <filegen.h> is included

    struct Data
    {
        char const* filename;
        array_view_const_char basename;
    };
    RED_TEST_CONTEXT_DATA(Data const& data, data.filename, {
        //  Below expected behavior from the unix man pages
        Data{"/usr/lib",  "lib"_av},
        Data{"/usr/lib/", ""_av},
        Data{"/usr/",     ""_av},
        Data{"/usr",      "usr"_av},
        Data{"usr",       "usr"_av},
        Data{"/",         ""_av},
        Data{".",         "."_av},
        Data{"..",        ".."_av}
    })
    {
        RED_CHECK_EQ(basename(data.filename), data.basename.data());
        size_t len = 0;
        char const * base = basename_len(data.filename, len);
        RED_CHECK_SMEM(make_array_view(base, len), data.basename);
    }
}


RED_AUTO_TEST_CASE(CanonicalPath)
{
    struct Data
    {
        char const* filename;
        char const* path;
        char const* basename;
        char const* extension;
    };
    char path[4096];
    char basename[4096];
    char extension[128];
    // check that function that splits a path between canonical parts has expected behavior
    // Parts are:
    // - path : full path absolute or relative to directory containing file
    // - basename : the filename without extension
    // - extension : the extension = part following the last dot, removed from basename
    //  if initial fullpath does not has any dot in it nothing is removed
    RED_TEST_CONTEXT_DATA(Data const& data, data.filename, {
        Data{"./titi/result.tmp",   "./titi/",  "result",       ".tmp"},
        Data{"./titi/result.tmp",   "./titi/",  "result",       ".tmp"},
        Data{"./titi/result.tmp",   "./titi/",  "result",       ".tmp"},
        Data{"result",              "no path",  "result",       "no extension"},
        Data{"result/",             "result/",  "no basename",  "no extension"},
        Data{"result.tmp",          "no path",  "result",       ".tmp"},
        Data{"tmp/.tmp",            "tmp/",     "no basename",  ".tmp"},
        Data{".tmp",                "no path",  "no basename",  ".tmp"},
        Data{"",                    "no path",  "no basename",  "no extension"}
    })
    {
        strcpy(path, "no path");
        strcpy(basename, "no basename");
        strcpy(extension, "no extension");

        canonical_path(data.filename, path, 4096, basename, 4096, extension, 128);
        RED_CHECK_EQUAL(data.path, path);
        RED_CHECK_EQUAL(data.basename, basename);
        RED_CHECK_EQUAL(data.extension, extension);
    }
}

RED_AUTO_TEST_CASE(TestParsePath)
{
    struct Data
    {
        char const* filename;
        char const* directory;
        char const* basename;
        char const* extension;
    };
    RED_TEST_CONTEXT_DATA(Data const& data, data.filename, {
        Data{"/etc/rdpproxy/rdpproxy.ini",  "/etc/rdpproxy/",   "rdpproxy",     ".ini"},
        Data{"/etc/rdpproxy/rdpproxy",      "/etc/rdpproxy/",   "rdpproxy",     "zzz"},
        Data{"/etc/rdpproxy/",              "/etc/rdpproxy/",   "yyy",          "zzz"},
        Data{"rdpproxy.ini",                "xxx",              "rdpproxy",     ".ini"},
        Data{"rdpproxy.",                   "xxx",              "rdpproxy",     "."},
        Data{"rdpproxy",                    "xxx",              "rdpproxy",     "zzz"},
        Data{".rdpproxy",                   "xxx",              ".rdpproxy",    "zzz"},
        Data{"/etc/rdpproxy/rdpproxy.ini",  "/etc/rdpproxy/",   "rdpproxy",     ".ini"},
        Data{"/etc/rdpproxy/rdpproxy",      "/etc/rdpproxy/",   "rdpproxy",     "zzz"},
        Data{"/etc/rdpproxy/rdpproxy",      "/etc/rdpproxy/",   "rdpproxy",     "zzz"},
        Data{"rdpproxy.ini",                "xxx",              "rdpproxy",     ".ini"},
        Data{"rdpproxy",                    "xxx",              "rdpproxy",     "zzz"},
        Data{".rdpproxy.ini",               "xxx",              ".rdpproxy",    ".ini"},
        Data{"a",                           "xxx",              "a"     ,       "zzz"},
        Data{"",                            "xxx",              "yyy",          "zzz"}
    })
    {
        std::string directory = "xxx";
        std::string basename = "yyy";
        std::string extension = "zzz";
        ParsePath(data.filename, directory, basename, extension);
        RED_CHECK_EQUAL(data.directory, directory);
        RED_CHECK_EQUAL(data.basename,  basename);
        RED_CHECK_EQUAL(data.extension, extension);
    }
}

RED_AUTO_TEST_CASE(TestMakePath)
{
    struct Data
    {
        char const* filename;
        char const* directory;
        char const* basename;
        char const* extension;
    };
    for (Data const& data : {
        Data{"", nullptr, nullptr, nullptr},
        Data{"", "", "", ""},
        Data{"/etc/rdpproxy/rdpproxy.ini", "/etc/rdpproxy/", "rdpproxy", ".ini"},
        Data{"/etc/rdpproxy/rdpproxy.ini", "/etc/rdpproxy", "rdpproxy", "ini"}
    })
    {
        std::string fullpath;
        MakePath(fullpath, data.directory, data.basename, data.extension);
        RED_CHECK_EQUAL(data.filename, fullpath);
    }
}

const auto file_not_exists = std::not_fn<bool(*)(char const*)>(file_exist);

RED_AUTO_TEST_CASE(TestRecursiveCreateDirectory)
{
    RED_TEST_CONTEXT_DATA(char const* subname, subname, {"test_subdir", "test_subdir/"})
    {
        WorkingDirectory wd;
        auto dir = wd.dirname().string() + "test_dir/";
        auto subdir = dir + subname;
        RED_CHECK_MESSAGE(!recursive_create_directory(subdir.c_str(), 0777, getgid()), strerror(errno));

        RED_CHECK_PREDICATE(file_exist, (subdir.c_str()));

        recursive_delete_directory(dir.c_str());

        RED_CHECK_PREDICATE(file_not_exists, (dir.c_str()));
        RED_CHECK_PREDICATE(file_not_exists, (subdir.c_str()));

        RED_CHECK_WORKSPACE(wd);
    }
}

RED_AUTO_TEST_CASE(TestFileEquals)
{
    RED_CHECK(file_equals(__FILE__, __FILE__));
    RED_CHECK(!file_equals(__FILE__, "/dev/zero"));
}

RED_AUTO_TEST_CASE(TestAppendFileContents)
{
    std::string buf;
    RED_TEST(append_file_contents("/unknown_path/unknown_file", buf) != FileContentsError::None);
    RED_TEST(buf.empty());
    RED_TEST(append_file_contents(FIXTURES_PATH "/test_infile.txt", buf) == FileContentsError::None);
    RED_TEST(buf == "We read what we provide!");
    buf.clear();
    RED_TEST(append_file_contents(FIXTURES_PATH "/sample1.wrm", buf) == FileContentsError::None);
    unsigned accu = std::accumulate(buf.begin(), buf.end(), 0u, [](unsigned u, char c){
        return u + uint8_t(c);
    });
    RED_TEST(accu == 47734549u);
}
