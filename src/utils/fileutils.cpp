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

#include "utils/fileutils.hpp"
#include "utils/log.hpp"
#include "utils/file.hpp"
#include "utils/strutils.hpp"

#include <cstdio>
#include <cstddef>
#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <alloca.h>

#ifdef __EMSCRIPTEN__
char const* basename(char const* path)
{
    char const* p = path;

    do {
        while (*p != '/' && *p) {
            ++p;
        }

        if (!*p) {
            return path;
        }

        ++p;
        path = p;
    } while (*path);

    return path;
}

char* basename(char* path)
{
    return const_cast<char*>(basename(const_cast<char const*>(path)));
}
#endif

// two flavors of basename_len to make it const agnostic
const char * basename_len(const char * path, size_t & len)
{
    const char * tmp = strrchr(path, '/');
    if (tmp){
        len = strlen(tmp+1);
        return tmp+1;
    }
    len = strlen(path);
    return path;
}

char * basename_len(char * path, size_t & len) /* NOLINT(readability-non-const-parameter) */
{
    char const * const_path = path;
    return const_cast<char*>(basename_len(const_path, len)); /*NOLINT*/
}

int filesize(const char * path)
{
    struct stat sb;
    int status = stat(path, &sb);
    if (status >= 0){
        return sb.st_size;
    }
    //LOG(LOG_INFO, "%s", strerror(errno));
    return -1;
}

bool file_exist(const char * path)
{
    struct stat sb;
    return (stat(path, &sb) == 0);
}

bool dir_exist(const char * path)
{
    struct stat sb;
    int statok = ::stat(path, &sb);
    return (statok == 0) && ((sb.st_mode & S_IFDIR) != 0);
}


int filesize(std::string const& path)
{
    return filesize(path.c_str());
}

bool file_exist(std::string const& path)
{
    return file_exist(path.c_str());
}

bool dir_exist(std::string const& path)
{
    return dir_exist(path.c_str());
}


bool file_equals(char const* filename1, char const* filename2)
{
    File f1(filename1, "r");
    File f2(filename2, "r");

    char buffer1[2048];
    char buffer2[2048];

    for (;;){
        auto const buf1 = f1.read(make_array_view(buffer1));
        auto const buf2 = f2.read(make_array_view(buffer2));
        LOG(LOG_INFO, "nb1=%zu nb2=%zu", buf1.size(), buf2.size());

        if (buf1.size() != buf2.size()
            || 0 != memcmp(buf1.data(), buf2.data(), buf2.size())
        ) {
            return false;
        }

        bool const is_eof1 = f1.is_eof();
        bool const is_eof2 = f2.is_eof();

        if (is_eof1 && is_eof2) {
            return true;
        }

        if (is_eof1 || is_eof2 || f1.has_error() || f2.has_error()) {
            return false;
        }
    }
}

void ParsePath(const char * fullpath, std::string & directory,
               std::string & filename, std::string & extension)
{
    const char * end_of_directory = strrchr(fullpath, '/');
    if (end_of_directory > fullpath) {
        directory.assign(fullpath, end_of_directory - fullpath + 1);
    }

    const char * begin_of_filename =
        (end_of_directory ? end_of_directory + 1 : fullpath);
    const char * end_of_filename   =
        [begin_of_filename] () {
            const char * dot = strrchr(begin_of_filename, '.');
            if (!dot || (dot == begin_of_filename)) {
                return begin_of_filename + strlen(begin_of_filename) - 1;
            }
            return dot - 1;
        } ();

    if (end_of_filename >= begin_of_filename) {
        filename.assign(begin_of_filename,
            end_of_filename - begin_of_filename + 1);
    }

    if (*(end_of_filename + 1)) {
        extension = end_of_filename + 1;
    }
}


void MakePath(std::string & fullpath, const char * directory,
              const char * filename, const char * extension)
{
    fullpath = (directory ? directory : "");
    if (!fullpath.empty() && (fullpath.back() != '/')) { fullpath += '/'; }
    if (filename) { fullpath += filename; }
    if (extension && *extension) {
        if (*extension != '.') { fullpath += '.'; }
        fullpath += extension;
    }
}

bool canonical_path(const char * fullpath, char * path, size_t path_len,
                    char * basename, size_t basename_len, char * extension,
                    size_t extension_len)
{
    const char * end_of_path = strrchr(fullpath, '/');
    if (end_of_path){
        if (static_cast<size_t>(end_of_path + 1 - fullpath) <= path_len) {
            memcpy(path, fullpath, end_of_path + 1 - fullpath);
            path[end_of_path + 1 - fullpath] = 0;
        }
        else {
            LOG(LOG_ERR, "canonical_path : Path too long for the buffer\n");
            return false;
        }
        const char * start_of_extension = strrchr(end_of_path + 1, '.');
        if (start_of_extension){
            utils::strlcpy(extension, start_of_extension, extension_len);
            if (start_of_extension > end_of_path + 1){
                if (static_cast<size_t>(start_of_extension - end_of_path - 1) <= basename_len) {
                    memcpy(basename, end_of_path + 1, start_of_extension - end_of_path - 1);
                    basename[start_of_extension - end_of_path - 1] = 0;
                }
                else {
                    LOG(LOG_ERR, "canonical_path : basename too long for the buffer\n");
                    return false;
                }
            }
            // else no basename : leave output buffer for name untouched
        }
        else {
            if (end_of_path[1]){
                utils::strlcpy(basename, end_of_path + 1, basename_len);
                // default extension : leave whatever is in extension output buffer
            }
            else {
                // default name : leave whatever is in name output buffer
                // default extension : leave whatever is in extension output buffer
            }
        }
    }
    else {
        // default path : leave whatever is in path output buffer
        const char * start_of_extension = strrchr(fullpath, '.');
        if (start_of_extension){
            utils::strlcpy(extension, start_of_extension, extension_len);
            if (start_of_extension > fullpath){
                if (static_cast<size_t>(start_of_extension - fullpath) <= basename_len) {
                    memcpy(basename, fullpath, start_of_extension - fullpath);
                    basename[start_of_extension - fullpath] = 0;
                }
                else {
                    LOG(LOG_ERR, "canonical_path : basename too long for the buffer\n");
                    return false;
                }
            }
            // else no basename : leave output buffer for name untouched
        }
        else {
            if (fullpath[0]){
                utils::strlcpy(basename, fullpath, basename_len);
                // default extension : leave whatever is in extension output buffer
            }
            else {
                // default name : leave whatever is in name output buffer
                // default extension : leave whatever is in extension output buffer
            }
        }
    }
    return true;
}


static int _internal_make_directory(const char *directory, mode_t mode, int groupid)
{
    struct stat st;
    int status = 0;

    if ((directory[0] != 0) && 0 != strcmp(directory, ".") && 0 != strcmp(directory, "..")) {
        if (stat(directory, &st) != 0) {
            /* Directory does not exist. */
            if ((mkdir(directory, mode) != 0) && (errno != EEXIST)) {
                status = -1;
                LOG(LOG_ERR, "failed to create directory %s : %s [%d]", directory, strerror(errno), errno);
            }
            if (groupid >= 0) {
                #ifdef __EMSCRIPTEN__
                    groupid = (groupid == -1) ? getpid() : groupid;
                    const uid_t userid = getuid();
                #else
                    const uid_t userid = -1;
                #endif
                if (chown(directory, userid, groupid) < 0){
                    LOG(LOG_ERR, "can't set directory %s group to %d : %s [%d]", directory, groupid, strerror(errno), errno);
                }
            }
        }
        else if (!S_ISDIR(st.st_mode)) {
            errno = ENOTDIR;
            LOG(LOG_ERR, "expecting directory name, got filename, for %s", directory);
            status = -1;
        }
    }
    return status;
}


int recursive_create_directory(const char * directory, mode_t mode, const int groupid)
{
    if (!directory) {
        LOG(LOG_ERR, "Call to recursive create directory without directory path (null)");
        return -1;
    }

    int status = 0;
    std::string copy_directory = directory;
    char * pSearch;

    for (char * pTemp = &copy_directory[0]
        ; (status == 0) && ((pSearch = strchr(pTemp, '/')) != nullptr)
        ; pTemp = pSearch + 1) {
        if (pSearch == pTemp) {
            continue;
        }

        pSearch[0] = 0;
        status = _internal_make_directory(copy_directory.data(), mode, groupid);
        *pSearch = '/';
    }

    // creation of last directory in chain or nothing if path ending with slash
    if (status == 0 && *directory != 0) {
        status = _internal_make_directory(directory, mode, groupid);
    }

    return status;
}

int recursive_delete_directory(const char * directory_path)
{
    DIR * dir = opendir(directory_path);

    int return_value = 0;

    if (dir) {
        struct dirent * ent;
        size_t const directory_path_len = strlen(directory_path);

        while (!return_value && (ent = readdir(dir)))
        {
            /* Skip the names "." and ".." as we don't want to recurse on them. */
            if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
                continue;
            }

            size_t entry_path_length = directory_path_len + strlen(ent->d_name) + 2;
            // TODO don't use alloca !!!
            char * entry_path = static_cast<char*>(alloca(entry_path_length));

            if (entry_path) {
                struct stat statbuf;

                snprintf(entry_path, entry_path_length, "%s/%s", directory_path, ent->d_name);

                if (!stat(entry_path, &statbuf)) {
                    if (S_ISDIR(statbuf.st_mode)) {
                        return_value = recursive_delete_directory(entry_path);
                    }
                    else {
                        return_value = unlink(entry_path);
                    }
                }
            }
        }

        closedir(dir);
    }

    if (!return_value) {
        return_value = rmdir(directory_path);
    }

    return return_value;
}
