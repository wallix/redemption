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

#define RED_TEST_MODULE TestFileUtils
#include "system/redemption_unit_tests.hpp"

// #define LOGPRINT
#define LOGNULL
#include "utils/log.hpp"

#include "utils/fileutils.hpp"

RED_AUTO_TEST_CASE(TestBasename)
{
// basename() change behavior depending if <filegen.h> is included
// or not. The POSIX version chnage it's argument, not the glibc one
// we WANT to use the glibc one. This test below will fail if
// <filegen.h> is included

//     Below expected behavior from the unix man pages
//          path        basename
//          "/usr/lib"  "lib"
//          "/usr/"     ""
//          "usr"       "usr"
//          "/"         "/"
//          "."         "."
//          ".."        ".."
    {
        char path[]= "/usr/lib";
        RED_CHECK(0 == strcmp(basename(path), "lib"));
    }
    {
        char path[]= "/usr/lib";
        size_t len = 0;
        char * base = basename_len(path, len);
        RED_CHECK_EQUAL(3, len);
        RED_CHECK(0 == memcmp(base, "lib", len));

    }

    {
        char path[]= "/usr/lib/";
        RED_CHECK(0 == strcmp(basename(path), ""));
    }
    {
        char path[]= "/usr/lib/";
        size_t len = 0;
        char * base = basename_len(path, len);
        RED_CHECK_EQUAL(0, len);
        RED_CHECK(0 == memcmp(base, "", len));

    }
    {
        char path[]= "/usr/";
        RED_CHECK(0 == strcmp(basename(path), ""));
    }
    {
        char path[]= "/usr";
        size_t len = 0;
        char * base = basename_len(path, len);
        RED_CHECK_EQUAL(3, len);
        RED_CHECK(0 == memcmp(base, "usr", len));

    }
    {
        char path[]= "usr";
        RED_CHECK(0 == strcmp(basename(path), "usr"));
    }
    {
        char path[]= "usr";
        size_t len = 0;
        char * base = basename_len(path, len);
        RED_CHECK_EQUAL(3, len);
        RED_CHECK(0 == memcmp(base, "usr", len));

    }
    {
        char path[]= "/";
        RED_CHECK(0 == strcmp(basename(path), ""));
    }
    {
        char path[]= "/";
        size_t len = 0;
        char * base = basename_len(path, len);
        RED_CHECK_EQUAL(0, len);
        RED_CHECK(0 == memcmp(base, "", len));

    }
    {
        char path[]= ".";
        RED_CHECK(0 == strcmp(basename(path), "."));
    }
    {
        char path[]= ".";
        size_t len = 0;
        char * base = basename_len(path, len);
        RED_CHECK_EQUAL(1, len);
        RED_CHECK(0 == memcmp(base, ".", len));

    }
    {
        char path[]= "..";
        RED_CHECK(0 == strcmp(basename(path), ".."));
    }
    {
        const char path[]= "..";
        size_t len = 0;
        const char * base = basename_len(path, len);
        RED_CHECK_EQUAL(2, len);
        RED_CHECK(0 == memcmp(base, "..", len));

    }

}


#include <unistd.h> // for getgid
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



RED_AUTO_TEST_CASE(CanonicalPath)
{
  // check that function that splits a path between canonical parts has expected behavior
  // Parts are:
  // - path : full path absolute or relative to directory containing file
  // - basename : the filename without extension
  // - extension : the extension = part following the last dot, removed from basename
  //  if initial fullpath does not has any dot in it nothing is removed

  char path[4096];
  char basename[4096];
  char extension[128];
  strcpy(path, "no path");
  strcpy(basename, "no basename");
  strcpy(extension, "no extension");

  canonical_path("./titi/result.tmp", path, 4096, basename, 4096, extension, 128);
  RED_CHECK_EQUAL("./titi/", path);
  RED_CHECK_EQUAL("result", basename);
  RED_CHECK_EQUAL(".tmp", extension);


  strcpy(path, "no path");
  strcpy(basename, "no basename");
  strcpy(extension, "no extension");
  canonical_path("result", path, 4096, basename, 4096, extension, 128);
  RED_CHECK_EQUAL("no path", path);
  RED_CHECK_EQUAL("result", basename);
  RED_CHECK_EQUAL("no extension", extension);

  strcpy(path, "no path");
  strcpy(basename, "no basename");
  strcpy(extension, "no extension");
  canonical_path("result/", path, 4096, basename, 4096, extension, 128);
  RED_CHECK_EQUAL("result/", path);
  RED_CHECK_EQUAL("no basename", basename);
  RED_CHECK_EQUAL("no extension", extension);

  strcpy(path, "no path");
  strcpy(basename, "no basename");
  strcpy(extension, "no extension");
  canonical_path("result.tmp", path, 4096, basename, 4096, extension, 128);
  RED_CHECK_EQUAL("no path", path);
  RED_CHECK_EQUAL("result", basename);
  RED_CHECK_EQUAL(".tmp", extension);

  strcpy(path, "no extension");
  strcpy(basename, "no basename");
  strcpy(extension, "no extension");
  canonical_path("tmp/.tmp", path, 4096, basename, 4096, extension, 128);
  RED_CHECK_EQUAL("tmp/", path);
  RED_CHECK_EQUAL("no basename", basename);
  RED_CHECK_EQUAL(".tmp", extension);

  strcpy(path, "no path");
  strcpy(basename, "no basename");
  strcpy(extension, "no extension");
  canonical_path(".tmp", path, 4096, basename, 4096, extension, 128);
  RED_CHECK_EQUAL("no path", path);
  RED_CHECK_EQUAL("no basename", basename);
  RED_CHECK_EQUAL(".tmp", extension);

  strcpy(path, "no path");
  strcpy(basename, "no basename");
  strcpy(extension, "no extension");
  canonical_path("", path, 4096, basename, 4096, extension, 128);
  RED_CHECK_EQUAL("no path", path);
  RED_CHECK_EQUAL("no basename", basename);
  RED_CHECK_EQUAL("no extension", extension);
}

RED_AUTO_TEST_CASE(TestParsePath)
{
    {
        std::string directory;
        std::string filename ;
        std::string extension;
        ParsePath("/etc/rdpproxy/rdpproxy.ini", directory, filename, extension);
        RED_CHECK_EQUAL("/etc/rdpproxy/", directory);
        RED_CHECK_EQUAL("rdpproxy"      , filename );
        RED_CHECK_EQUAL(".ini"          , extension);
    }

    {
        std::string directory;
        std::string filename ;
        std::string extension;
        ParsePath("/etc/rdpproxy/rdpproxy", directory, filename, extension);
        RED_CHECK_EQUAL("/etc/rdpproxy/", directory);
        RED_CHECK_EQUAL("rdpproxy"      , filename );
        RED_CHECK_EQUAL(""              , extension);
    }

    {
        std::string directory;
        std::string filename ;
        std::string extension;
        ParsePath("/etc/rdpproxy/", directory, filename, extension);
        RED_CHECK_EQUAL("/etc/rdpproxy/", directory);
        RED_CHECK_EQUAL(""              , filename );
        RED_CHECK_EQUAL(""              , extension);
    }

    {
        std::string directory;
        std::string filename ;
        std::string extension;
        ParsePath("rdpproxy.ini", directory, filename, extension);
        RED_CHECK_EQUAL(""        , directory);
        RED_CHECK_EQUAL("rdpproxy", filename );
        RED_CHECK_EQUAL(".ini"    , extension);
    }

    {
        std::string directory;
        std::string filename ;
        std::string extension;
        ParsePath("rdpproxy.", directory, filename, extension);
        RED_CHECK_EQUAL(""        , directory);
        RED_CHECK_EQUAL("rdpproxy", filename );
        RED_CHECK_EQUAL("."       , extension);
    }

    {
        std::string directory;
        std::string filename ;
        std::string extension;
        ParsePath("rdpproxy", directory, filename, extension);
        RED_CHECK_EQUAL(""        , directory);
        RED_CHECK_EQUAL("rdpproxy", filename );
        RED_CHECK_EQUAL(""        , extension);
    }

    {
        std::string directory;
        std::string filename ;
        std::string extension;
        ParsePath(".rdpproxy", directory, filename, extension);
        RED_CHECK_EQUAL(""         , directory);
        RED_CHECK_EQUAL(".rdpproxy", filename );
        RED_CHECK_EQUAL(""         , extension);
    }

    {
        std::string directory = "./"    ;
        std::string filename  = "sesman";
        std::string extension = ".conf" ;
        ParsePath("/etc/rdpproxy/rdpproxy.ini", directory, filename, extension);
        RED_CHECK_EQUAL("/etc/rdpproxy/", directory);
        RED_CHECK_EQUAL("rdpproxy"      , filename );
        RED_CHECK_EQUAL(".ini"          , extension);
    }

    {
        std::string directory = "./"    ;
        std::string filename  = "sesman";
        std::string extension = ".conf" ;
        ParsePath("/etc/rdpproxy/rdpproxy", directory, filename, extension);
        RED_CHECK_EQUAL("/etc/rdpproxy/", directory);
        RED_CHECK_EQUAL("rdpproxy"      , filename );
        RED_CHECK_EQUAL(".conf"         , extension);
    }

    {
        std::string directory           ;
        std::string filename            ;
        std::string extension = ".conf" ;
        ParsePath("/etc/rdpproxy/rdpproxy", directory, filename, extension);
        RED_CHECK_EQUAL("/etc/rdpproxy/", directory);
        RED_CHECK_EQUAL("rdpproxy"      , filename );
        RED_CHECK_EQUAL(".conf"         , extension);
    }

    {
        std::string directory = "./"    ;
        std::string filename  = "sesman";
        std::string extension = ".conf" ;
        ParsePath("rdpproxy.ini", directory, filename, extension);
        RED_CHECK_EQUAL("./"      , directory);
        RED_CHECK_EQUAL("rdpproxy", filename );
        RED_CHECK_EQUAL(".ini"    , extension);
    }

    {
        std::string directory = "./"    ;
        std::string filename  = "sesman";
        std::string extension = ".conf" ;
        ParsePath("rdpproxy", directory, filename, extension);
        RED_CHECK_EQUAL("./"      , directory);
        RED_CHECK_EQUAL("rdpproxy", filename );
        RED_CHECK_EQUAL(".conf"   , extension);
    }

    {
        std::string directory = "./"    ;
        std::string filename  = "sesman";
        std::string extension = ".conf" ;
        ParsePath(".rdpproxy.ini", directory, filename, extension);
        RED_CHECK_EQUAL("./"       , directory);
        RED_CHECK_EQUAL(".rdpproxy", filename );
        RED_CHECK_EQUAL(".ini"     , extension);
    }

    {
        std::string directory = "./"    ;
        std::string filename  = "sesman";
        std::string extension = ".conf" ;
        ParsePath("", directory, filename, extension);
        RED_CHECK_EQUAL("./"    , directory);
        RED_CHECK_EQUAL("sesman", filename );
        RED_CHECK_EQUAL(".conf" , extension);
    }
}

RED_AUTO_TEST_CASE(TestMakePath)
{
    {
        std::string fullpath;
        MakePath(fullpath, nullptr, nullptr, nullptr);
        RED_CHECK_EQUAL("", fullpath);
    }

    {
        std::string fullpath;
        MakePath(fullpath, "", "", "");
        RED_CHECK_EQUAL("", fullpath);
    }

    {
        std::string fullpath;
        MakePath(fullpath, "/etc/rdpproxy/", "rdpproxy", ".ini");
        RED_CHECK_EQUAL("/etc/rdpproxy/rdpproxy.ini", fullpath);
    }

    {
        std::string fullpath;
        MakePath(fullpath, "/etc/rdpproxy", "rdpproxy", "ini");
        RED_CHECK_EQUAL("/etc/rdpproxy/rdpproxy.ini", fullpath);
    }
}

RED_AUTO_TEST_CASE(TestRecursiveCreateDirectory)
{
    char tmpdirname[128];
    sprintf(tmpdirname, "/tmp/test_dir_XXXXXX");
    RED_CHECK(nullptr != mkdtemp(tmpdirname));
    RED_CHECK_EQUAL(true, file_exist(tmpdirname));

    recursive_delete_directory(tmpdirname);

    RED_CHECK_EQUAL(false, file_exist(tmpdirname));

    recursive_create_directory(tmpdirname, 0777, getgid());

    RED_CHECK_EQUAL(true, file_exist(tmpdirname));

    char tmpfilename[128];
    strcpy(tmpfilename, tmpdirname);
    strcat(tmpfilename, "/test_file_XXXXXX");
    close(mkstemp(tmpfilename));

    recursive_delete_directory(tmpdirname);
    RED_CHECK_EQUAL(false, file_exist(tmpdirname));

}

RED_AUTO_TEST_CASE(TestRecursiveCreateDirectoryTrailingSlash)
{
    char tmpdirname[128];
    sprintf(tmpdirname, "/tmp/test_dir_XXXXXX");
    RED_CHECK(nullptr != mkdtemp(tmpdirname));
    RED_CHECK_EQUAL(true, file_exist(tmpdirname));

    // Add a trailing slash to tmpdirname
    strcat(tmpdirname, "/");
    recursive_delete_directory(tmpdirname);

    RED_CHECK_EQUAL(false, file_exist(tmpdirname));

    recursive_create_directory(tmpdirname, 0777, getgid());

    RED_CHECK_EQUAL(true, file_exist(tmpdirname));

    char tmpfilename[128];
    strcpy(tmpfilename, tmpdirname);
    strcat(tmpfilename, "/test_file_XXXXXX");
    close(mkstemp(tmpfilename));

    recursive_delete_directory(tmpdirname);
    RED_CHECK_EQUAL(false, file_exist(tmpdirname));
}


