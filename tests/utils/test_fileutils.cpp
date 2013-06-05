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


BOOST_AUTO_TEST_CASE(ParseIpConntrack)
{
    char tmpname[] = "/tmp/test_conntrack_XXXXXX";
    int fd = ::mkostemp(tmpname, O_RDWR|O_CREAT);
    char conntrack[] = 
"unknown  2 580 src=10.10.43.13 dst=224.0.0.251 packets=2 bytes=64 [UNREPLIED] src=224.0.0.251 dst=10.10.43.13 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
"udp      17 28 src=10.10.43.30 dst=255.255.255.255 sport=17500 dport=17500 packets=1102 bytes=154280 [UNREPLIED] src=255.255.255.255 dst=10.10.43.30 sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
"unknown  2 205 src=10.10.47.124 dst=224.0.0.251 packets=2 bytes=64 [UNREPLIED] src=224.0.0.251 dst=10.10.47.124 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
"unknown  2 209 src=10.10.41.13 dst=224.0.0.251 packets=5 bytes=160 [UNREPLIED] src=224.0.0.251 dst=10.10.41.13 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
"udp      17 19 src=10.10.41.14 dst=10.10.47.255 sport=17500 dport=17500 packets=5937 bytes=831180 [UNREPLIED] src=10.10.47.255 dst=10.10.41.14 sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
"udp      17 19 src=10.10.41.14 dst=255.255.255.255 sport=17500 dport=17500 packets=5937 bytes=831180 [UNREPLIED] src=255.255.255.255 dst=10.10.41.14 sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
"udp      17 29 src=10.10.43.33 dst=10.10.47.255 sport=17500 dport=17500 packets=971 bytes=135940 [UNREPLIED] src=10.10.47.255 // dst=10.10.43.33 sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
"udp      17 29 src=10.10.43.33 dst=255.255.255.255 sport=17500 dport=17500 packets=971 bytes=135940 [UNREPLIED] src=255.255.255.255 dst=10.10.43.33 sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
"unknown  2 455 src=10.10.47.170 dst=224.0.0.251 packets=1 bytes=32 [UNREPLIED] src=224.0.0.251 dst=10.10.47.170 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
"unknown  2 580 src=0.0.0.0 dst=224.0.0.1 packets=1422 bytes=45504 [UNREPLIED] src=224.0.0.1 dst=0.0.0.0 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
"tcp      6 431995 ESTABLISHED src=127.0.0.1 dst=127.0.0.1 sport=51040 dport=3450 packets=5 bytes=588 src=127.0.0.1 dst=127.0.0.1 sport=3450 dport=51040 packets=4 bytes=573 [ASSURED] mark=0 secmark=0 use=2\n"
"udp      17 28 src=10.10.43.30 dst=10.10.47.255 sport=17500 dport=17500 packets=1102 bytes=154280 [UNREPLIED] src=10.10.47.255 dst=10.10.43.30 sport=17500 dport=17500 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
"unknown  2 79 src=10.10.45.11 dst=224.0.0.251 packets=1 bytes=32 [UNREPLIED] src=224.0.0.251 dst=10.10.45.11 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
"tcp      6 431979 ESTABLISHED src=10.10.43.13 dst=10.10.46.78 sport=41971 dport=3389 packets=96 bytes=10739 src=10.10.47.93 dst=10.10.43.13 sport=3389 dport=41971 packets=96 bytes=39071 [ASSURED] mark=0 secmark=0 use=2\n"
"udp      17 0 src=10.10.41.14 dst=10.10.47.255 sport=631 dport=631 packets=1 bytes=274 [UNREPLIED] src=10.10.47.255 dst=10.10.41.14 sport=631 dport=631 packets=0 bytes=0 mark=0 secmark=0 use=2\n"
"tcp      6 299 ESTABLISHED src=10.10.43.13 dst=10.10.47.93 sport=36699 dport=22 packets=5256 bytes=437137 src=10.10.47.93 dst=10.10.43.13 sport=22 dport=36699 packets=3523 bytes=572101 [ASSURED] mark=0 secmark=0 use=2\n"
"tcp      6 431997 ESTABLISHED src=10.10.43.13 dst=10.10.47.93 sport=46392 dport=3389 packets=90 bytes=10061 src=10.10.47.93 dst=10.10.43.13 sport=3389 dport=46392 packets=89 bytes=38707 [ASSURED] mark=0 secmark=0 use=2\n"
"udp      17 0 src=10.10.43.31 dst=10.10.47.255 sport=57621 dport=57621 packets=1139 bytes=82008 [UNREPLIED] src=10.10.47.255 dst=10.10.43.31 sport=57621 dport=57621 packets=0 bytes=0 mark=0 secmark=0 use=2\n";
    int res = write(fd, conntrack, sizeof(conntrack)-1);
    BOOST_CHECK_EQUAL(res, sizeof(conntrack)-1);
    
    // ----------------------------------------------------
    BOOST_CHECK_EQUAL(0, lseek(fd, 0, SEEK_SET));
       
    const char first[] = "unknown  2 580 src=10.10.43.13 dst=224.0.0.251 packets=2 bytes=64 [UNREPLIED] src=224.0.0.251 dst=10.10.43.13 packets=0 bytes=0 mark=0 secmark=0 use=2\n";

    // Read first line
    
    LineBuffer line(fd);
    int status = line.readline();
    BOOST_CHECK_EQUAL(status, 1);
    BOOST_CHECK_EQUAL(0, memcmp(first, &line.buffer[line.begin_line], sizeof(first) - 1));
    BOOST_CHECK_EQUAL(line.eol - line.begin_line, sizeof(first) - 1);
    BOOST_CHECK_EQUAL(0, memcmp(first, &line.buffer[line.begin_line], line.eol - line.begin_line));

    // Parse it

    line.eow = line.begin_word = line.begin_line;
    status = line.get_protocol();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(7, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("unknown", &line.buffer[line.begin_word], line.eow - line.begin_word));
    
    line.begin_word = line.eow;
    status = line.get_space();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(2, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("  ", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_protocol_number();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(1, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("2", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_space();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(1, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp(" ", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_ttl_sec();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(3, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("580", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_space();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(1, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp(" ", &line.buffer[line.begin_word], line.eow - line.begin_word));


    int old_eow = line.begin_word = line.eow;
    status = line.get_var();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(4, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("src=", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_ip();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(11, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("10.10.43.13", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = old_eow;
    status = line.get_src_ip();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(15, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("src=10.10.43.13", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_space();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(1, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp(" ", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_dst_ip();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(15, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("dst=224.0.0.251", &line.buffer[line.begin_word], line.eow - line.begin_word));

    // dst=224.0.0.251 packets=2 bytes=64 [UNREPLIED]
    line.begin_word = line.eow;
    status = line.get_space();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(1, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp(" ", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_packets();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(9, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("packets=2", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_space();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(1, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp(" ", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_bytes();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(8, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("bytes=64", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_space();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(1, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp(" ", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_status();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(11, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("[UNREPLIED]", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_space();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(1, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp(" ", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_src_ip();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(15, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("src=224.0.0.251", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_space();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(1, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp(" ", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_dst_ip();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(15, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("dst=10.10.43.13", &line.buffer[line.begin_word], line.eow - line.begin_word));

    // dst=224.0.0.251 packets=2 bytes=64 [UNREPLIED]
    line.begin_word = line.eow;
    status = line.get_space();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(1, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp(" ", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_packets();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(9, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("packets=0", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_space();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(1, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp(" ", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_bytes();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(7, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("bytes=0", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_space();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(1, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp(" ", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_mark();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(6, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("mark=0", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_space();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(1, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp(" ", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_secmark();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(9, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("secmark=0", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_space();
    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(1, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp(" ", &line.buffer[line.begin_word], line.eow - line.begin_word));

    line.begin_word = line.eow;
    status = line.get_use();

    BOOST_CHECK_EQUAL(status, 0);
    BOOST_CHECK_EQUAL(5, line.eow - line.begin_word);
    BOOST_CHECK_EQUAL(0, memcmp("use=2", &line.buffer[line.begin_word], line.eow - line.begin_word));

    // ----------------------------------------------------
    BOOST_CHECK_EQUAL(0, lseek(fd, 0, SEEK_SET));
    char transparent_target[256] = {};
    // "tcp      6 431979 ESTABLISHED src=10.10.43.13 dst=10.10.46.78 sport=41971 dport=3389 packets=96 bytes=10739 src=10.10.47.93 dst=10.10.43.13 sport=3389 dport=41971 packets=96 bytes=39071 [ASSURED] mark=0 secmark=0 use=2\n"

    res = parse_ip_conntrack(fd, "10.10.47.93", "10.10.43.13", 3389, 41971, transparent_target, sizeof(transparent_target));
    BOOST_CHECK_EQUAL(res, 0);
    BOOST_CHECK_EQUAL(0, strcmp(transparent_target, "10.10.46.78"));
    
    BOOST_CHECK_EQUAL(0, lseek(fd, 0, SEEK_SET));
    transparent_target[0] = 0;
    res = parse_ip_conntrack(fd, "10.10.47.21", "10.10.43.13", 3389, 46392, transparent_target, sizeof(transparent_target));
    BOOST_CHECK_EQUAL(res, -1);
    BOOST_CHECK_EQUAL(0, strcmp(transparent_target, ""));

    close(fd);
}

