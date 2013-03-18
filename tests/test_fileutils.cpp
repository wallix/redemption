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

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestFileUtils
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"
#include <fcntl.h>
#include "fileutils.hpp"


BOOST_AUTO_TEST_CASE(TestClearTargetFiles)
{
    {
        char tmpdirname[128];
        sprintf(tmpdirname, "/tmp/test_dir_XXXXXX");
        BOOST_CHECK(NULL != mkdtemp(tmpdirname));
        
//        int fd = ::mkostemp(tmpdirname, O_WRONLY|O_CREAT);
        
        char toto_mwrm[512]; sprintf(toto_mwrm, "%s/%s", tmpdirname, "toto.mwrm");
        { int fd = ::creat(toto_mwrm, 0777); BOOST_CHECK_EQUAL(10, write(fd, "toto_mwrm", sizeof("toto_mwrm"))); close(fd); }

        char toto_0_wrm[512]; sprintf(toto_0_wrm, "%s/%s", tmpdirname, "toto_0.mwrm");
        { int fd = ::creat(toto_0_wrm, 0777); BOOST_CHECK_EQUAL(11, write(fd, "toto_0_wrm", sizeof("toto_0_wrm"))); close(fd); }

        char toto_1_wrm[512]; sprintf(toto_1_wrm, "%s/%s", tmpdirname, "toto_1.wrm");
        { int fd = ::creat(toto_1_wrm, 0777); BOOST_CHECK_EQUAL(11, write(fd, "toto_1_wrm", sizeof("toto_1_wrm"))); close(fd); }

        char toto_0_flv[512]; sprintf(toto_0_flv, "%s/%s", tmpdirname, "toto_0.flv");
        { int fd = ::creat(toto_0_flv, 0777); BOOST_CHECK_EQUAL(11, write(fd, "toto_0_flv", sizeof("toto_0_flv"))); close(fd); }

        char toto_1_flv[512]; sprintf(toto_1_flv, "%s/%s", tmpdirname, "toto_1.flv");
        { int fd = ::creat(toto_1_flv, 0777); BOOST_CHECK_EQUAL(11, write(fd, "toto_1_flv", sizeof("toto_1_flv"))); close(fd); }

        char toto_meta[512]; sprintf(toto_meta, "%s/%s", tmpdirname, "toto.meta");
        { int fd = ::creat(toto_meta, 0777); BOOST_CHECK_EQUAL(10, write(fd, "toto_meta", sizeof("toto_meta"))); close(fd); }

        char toto_0_png[512]; sprintf(toto_0_png, "%s/%s", tmpdirname, "toto_0.png");
        { int fd = ::creat(toto_0_png, 0777); BOOST_CHECK_EQUAL(11, write(fd, "toto_0_png", sizeof("toto_0_png"))); close(fd); }

        char toto_1_png[512]; sprintf(toto_1_png, "%s/%s", tmpdirname, "toto_1.png");
        { int fd = ::creat(toto_1_png, 0777); BOOST_CHECK_EQUAL(11, write(fd, "toto_1_png", sizeof("toto_1_png"))); close(fd); }

        char tititi_mwrm[512]; sprintf(tititi_mwrm, "%s/%s", tmpdirname, "tititi.mwrm");
        { int fd = ::creat(tititi_mwrm, 0777); BOOST_CHECK_EQUAL(12, write(fd, "tititi_mwrm", sizeof("tititi_mwrm"))); close(fd); }

        char tititi_0_wrm[512]; sprintf(tititi_0_wrm, "%s/%s", tmpdirname, "tititi_0.mwrm");
        { int fd = ::creat(tititi_0_wrm, 0777); BOOST_CHECK_EQUAL(13, write(fd, "tititi_0_wrm", sizeof("tititi_0_wrm"))); close(fd); }

        char tititi_1_wrm[512]; sprintf(tititi_1_wrm, "%s/%s", tmpdirname, "tititi_1.wrm");
        { int fd = ::creat(tititi_1_wrm, 0777); BOOST_CHECK_EQUAL(13, write(fd, "tititi_1_wrm", sizeof("tititi_1_wrm"))); close(fd); }

        char tititi_0_flv[512]; sprintf(tititi_0_flv, "%s/%s", tmpdirname, "tititi_0.flv");
        { int fd = ::creat(tititi_0_flv, 0777); BOOST_CHECK_EQUAL(13, write(fd, "tititi_0_flv", sizeof("tititi_0_flv"))); close(fd); }

        char tititi_1_flv[512]; sprintf(tititi_1_flv, "%s/%s", tmpdirname, "tititi_1.flv");
        { int fd = ::creat(tititi_1_flv, 0777); BOOST_CHECK_EQUAL(13, write(fd, "tititi_1_flv", sizeof("tititi_1_flv"))); close(fd); }

        char tititi_meta[512]; sprintf(tititi_meta, "%s/%s", tmpdirname, "tititi.meta");
        { int fd = ::creat(tititi_meta, 0777); BOOST_CHECK_EQUAL(12, write(fd, "tititi_meta", sizeof("tititi_meta"))); close(fd); }

        char tititi_0_png[512]; sprintf(tititi_0_png, "%s/%s", tmpdirname, "tititi_0.png");
        { int fd = ::creat(tititi_0_png, 0777); BOOST_CHECK_EQUAL(13, write(fd, "tititi_0_png", sizeof("tititi_0_png"))); close(fd); }

        char tititi_1_png[512]; sprintf(tititi_1_png, "%s/%s", tmpdirname, "tititi_1.png");
        { int fd = ::creat(tititi_1_png, 0777); BOOST_CHECK_EQUAL(13, write(fd, "tititi_1_png", sizeof("tititi_1_png"))); close(fd); }

        BOOST_CHECK_EQUAL(10, filesize(toto_mwrm));       
        BOOST_CHECK_EQUAL(11, filesize(toto_0_wrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_wrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_0_flv));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_flv));
        BOOST_CHECK_EQUAL(10, filesize(toto_meta));
        BOOST_CHECK_EQUAL(11, filesize(toto_0_png));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_png));
        BOOST_CHECK_EQUAL(12, filesize(tititi_mwrm));       
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_flv));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_flv));
        BOOST_CHECK_EQUAL(12, filesize(tititi_meta));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_png));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_png));

        clear_files_flv_meta_png(tmpdirname, "ddd");

        BOOST_CHECK_EQUAL(10, filesize(toto_mwrm));       
        BOOST_CHECK_EQUAL(11, filesize(toto_0_wrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_wrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_0_flv));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_flv));
        BOOST_CHECK_EQUAL(10, filesize(toto_meta));
        BOOST_CHECK_EQUAL(11, filesize(toto_0_png));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_png));
        BOOST_CHECK_EQUAL(12, filesize(tititi_mwrm));       
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_flv));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_flv));
        BOOST_CHECK_EQUAL(12, filesize(tititi_meta));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_png));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_png));

        clear_files_flv_meta_png(tmpdirname, "toto");

        BOOST_CHECK_EQUAL(10, filesize(toto_mwrm));       
        BOOST_CHECK_EQUAL(11, filesize(toto_0_wrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_wrm));
        BOOST_CHECK_EQUAL(-1, filesize(toto_0_flv));
        BOOST_CHECK_EQUAL(-1, filesize(toto_1_flv));
        BOOST_CHECK_EQUAL(-1, filesize(toto_meta));
        BOOST_CHECK_EQUAL(-1, filesize(toto_0_png));
        BOOST_CHECK_EQUAL(-1, filesize(toto_1_png));
        BOOST_CHECK_EQUAL(12, filesize(tititi_mwrm));       
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_flv));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_flv));
        BOOST_CHECK_EQUAL(12, filesize(tititi_meta));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_png));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_png));

        clear_files_flv_meta_png(tmpdirname, "titititi");

        BOOST_CHECK_EQUAL(10, filesize(toto_mwrm));       
        BOOST_CHECK_EQUAL(11, filesize(toto_0_wrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_wrm));
        BOOST_CHECK_EQUAL(-1, filesize(toto_0_flv));
        BOOST_CHECK_EQUAL(-1, filesize(toto_1_flv));
        BOOST_CHECK_EQUAL(-1, filesize(toto_meta));
        BOOST_CHECK_EQUAL(-1, filesize(toto_0_png));
        BOOST_CHECK_EQUAL(-1, filesize(toto_1_png));
        BOOST_CHECK_EQUAL(12, filesize(tititi_mwrm));       
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_flv));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_flv));
        BOOST_CHECK_EQUAL(12, filesize(tititi_meta));
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_png));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_png));

        clear_files_flv_meta_png(tmpdirname, "tititi");

        BOOST_CHECK_EQUAL(10, filesize(toto_mwrm));       
        BOOST_CHECK_EQUAL(11, filesize(toto_0_wrm));
        BOOST_CHECK_EQUAL(11, filesize(toto_1_wrm));
        BOOST_CHECK_EQUAL(-1, filesize(toto_0_flv));
        BOOST_CHECK_EQUAL(-1, filesize(toto_1_flv));
        BOOST_CHECK_EQUAL(-1, filesize(toto_meta));
        BOOST_CHECK_EQUAL(-1, filesize(toto_0_png));
        BOOST_CHECK_EQUAL(-1, filesize(toto_1_png));
        BOOST_CHECK_EQUAL(12, filesize(tititi_mwrm));       
        BOOST_CHECK_EQUAL(13, filesize(tititi_0_wrm));
        BOOST_CHECK_EQUAL(13, filesize(tititi_1_wrm));
        BOOST_CHECK_EQUAL(-1, filesize(tititi_0_flv));
        BOOST_CHECK_EQUAL(-1, filesize(tititi_1_flv));
        BOOST_CHECK_EQUAL(-1, filesize(tititi_meta));
        BOOST_CHECK_EQUAL(-1, filesize(tititi_0_png));
        BOOST_CHECK_EQUAL(-1, filesize(tititi_1_png));

        ::unlink(toto_mwrm);
        ::unlink(toto_0_wrm);
        ::unlink(toto_1_wrm);
        ::unlink(tititi_mwrm);
        ::unlink(tititi_0_wrm);
        ::unlink(tititi_1_wrm);

        ::rmdir(tmpdirname);
    }
}

BOOST_AUTO_TEST_CASE(CanonicalPath)
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
  BOOST_CHECK_EQUAL("./titi/", path);
  BOOST_CHECK_EQUAL("result", basename);
  BOOST_CHECK_EQUAL(".tmp", extension);


  strcpy(path, "no path");  
  strcpy(basename, "no basename");  
  strcpy(extension, "no extension");  
  canonical_path("result", path, 4096, basename, 4096, extension, 128);
  BOOST_CHECK_EQUAL("no path", path);
  BOOST_CHECK_EQUAL("result", basename);
  BOOST_CHECK_EQUAL("no extension", extension);

  strcpy(path, "no path");  
  strcpy(basename, "no basename");  
  strcpy(extension, "no extension");  
  canonical_path("result/", path, 4096, basename, 4096, extension, 128);
  BOOST_CHECK_EQUAL("result/", path);
  BOOST_CHECK_EQUAL("no basename", basename);
  BOOST_CHECK_EQUAL("no extension", extension);

  strcpy(path, "no path");  
  strcpy(basename, "no basename");  
  strcpy(extension, "no extension");  
  canonical_path("result.tmp", path, 4096, basename, 4096, extension, 128);
  BOOST_CHECK_EQUAL("no path", path);
  BOOST_CHECK_EQUAL("result", basename);
  BOOST_CHECK_EQUAL(".tmp", extension);

  strcpy(path, "no extension");  
  strcpy(basename, "no basename");  
  strcpy(extension, "no extension");  
  canonical_path("tmp/.tmp", path, 4096, basename, 4096, extension, 128);
  BOOST_CHECK_EQUAL("tmp/", path);
  BOOST_CHECK_EQUAL("no basename", basename);
  BOOST_CHECK_EQUAL(".tmp", extension);

  strcpy(path, "no path");  
  strcpy(basename, "no basename");  
  strcpy(extension, "no extension");  
  canonical_path(".tmp", path, 4096, basename, 4096, extension, 128);
  BOOST_CHECK_EQUAL("no path", path);
  BOOST_CHECK_EQUAL("no basename", basename);
  BOOST_CHECK_EQUAL(".tmp", extension);

  strcpy(path, "no path");  
  strcpy(basename, "no basename");  
  strcpy(extension, "no extension");  
  canonical_path("", path, 4096, basename, 4096, extension, 128);
  BOOST_CHECK_EQUAL("no path", path);
  BOOST_CHECK_EQUAL("no basename", basename);
  BOOST_CHECK_EQUAL("no extension", extension);

}
