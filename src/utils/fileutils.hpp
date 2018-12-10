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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

   File related utility functions
*/


#pragma once

#include <string>
#include <sys/types.h> // mode_t

#ifdef __EMSCRIPTEN__
char* basename(char* path);
char const* basename(char const* path);
#endif

// two flavors of basename_len to make it const agnostic
const char * basename_len(const char * path, size_t & len);

char * basename_len(char * path, size_t & len);

int filesize(const char * path);
int filesize(std::string const& path);

bool file_exist(const char * path);
bool file_exist(std::string const& path);

bool dir_exist(const char * path);
bool dir_exist(std::string const& path);

bool file_equals(char const* filename1, char const* filename2);

void ParsePath(const char * fullpath, std::string & directory,
               std::string & filename, std::string & extension);

void MakePath(std::string & fullpath, const char * directory,
              const char * filename, const char * extension);

bool canonical_path(const char * fullpath, char * path, size_t path_len,
                    char * basename, size_t basename_len, char * extension,
                    size_t extension_len);

int recursive_create_directory(const char * directory, mode_t mode, int groupid);

int recursive_delete_directory(const char * directory_path);
