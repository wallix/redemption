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
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#include "working_directory.hpp"
#include "utils/fileutils.hpp"

#include <algorithm>
#include <stdexcept>
#include <string_view>

#include <cerrno>
#include <cassert>
#include <cstring>
#include <cstdio>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

namespace
{
    static inline constexpr std::string_view dirbase = "/tmp/";

    void normalize_directory(std::string& directory)
    {
        directory.insert(0, dirbase);
        if (directory[directory.size()-1] != '/') {
            directory += '/';
        }
    }

    std::string concat(std::string s, std::string_view sv)
    {
        s.append(sv.begin(), sv.end());
        return s;
    }
}


WorkingDirectory::WorkingDirectory(std::string dirname)
    : directory(std::move(dirname))
{
    auto is_invalid_char = [](char c) { return c == '.' || c == '/'; };
    if (directory.empty()
     || directory.end() != std::find_if(directory.begin(), directory.end(), is_invalid_char)
    ) {
        throw std::runtime_error("invalid dirname");
    }
    normalize_directory(this->directory);
    recursive_delete_directory(this->directory.c_str());
    if (-1 == mkdir(this->directory.c_str(), 0755) && errno != EEXIST) {
        throw std::runtime_error(strerror(errno));
    }
}


WorkingDirectory::WorkingDirectory(
    std::string dirname, std::initializer_list<std::string_view> files)
    : WorkingDirectory(std::move(dirname))
{
    this->paths.resize(files.size());
    for (std::size_t i = 0; i < files.size(); ++i) {
        auto sv = files.begin()[i];
        this->paths[i].is_checked = false;
        this->paths[i].name = concat(this->directory, sv);
    }

    for (auto& path : this->paths) {
        unlink(path.name.c_str());
    }
}

WorkingDirectory::FileIndex WorkingDirectory::add_file(std::string_view filename)
{
    this->paths.push_back({concat(this->directory, filename), false});
    unlink(this->paths.back().name.c_str());
    return {this->paths.size() - 1u};
}

WorkingDirectory WorkingDirectory::neested(
    std::string directory, std::initializer_list<std::string_view> files)
{
    directory.insert(0, this->directory.c_str() + dirbase.size());
    return WorkingDirectory(std::move(directory), std::move(files));
}

std::string const& WorkingDirectory::operator[](std::size_t i) noexcept
{
    this->paths[i].is_checked = true;
    return this->paths[i].name;
}

std::string const& WorkingDirectory::operator[](FileIndex file_index) noexcept
{
    return operator[](file_index.index);
}

std::string const& WorkingDirectory::operator[](std::string filename)
{
    filename.insert(0, this->directory);
    for (std::size_t i = 0; i < this->paths.size(); ++i) {
        if (filename == this->paths[i].name) {
            return operator[](i);
        }
    }
    throw std::logic_error("unknown file");
}

std::string const& WorkingDirectory::dirname() const noexcept
{
    return this->directory;
}


std::string WorkingDirectory::check_final_state()
{
    assert(!this->is_checked);

    this->is_checked = true;

    std::string message;

    if (DIR* dir = opendir(this->directory.c_str())) {
        while (auto ent = readdir(dir)) {
            if (0 == strcmp(ent->d_name, ".") 
            ||  0 == strcmp(ent->d_name, "..")) {
                continue;
            }
            std::size_t i = 0;
            for (; i < this->paths.size(); ++i) {
                auto& path = this->paths[i];
                std::string entry(directory);
                entry += ent->d_name;
                if (0 == strcmp(path.name.c_str(), entry.c_str())){
                    break;
                }
            }
            if (i == this->paths.size()){
                if (message.size() == 0){
                    message = "Unexpected Files: \n";
                }
                message += '\"';
                message += directory;
                message += ent->d_name;
                message += "\"";
                message += "\n";
            }
        }
        closedir(dir);
    }
    else {
        message = "Can't access directory ";
        message += '\"';
        message += directory;
        message += "\" ";
    }

    return message;
}


std::string WorkingDirectory::clean_and_get()
{
    assert(!this->is_checked);

    this->is_checked = true;

    std::string message;

    for (std::size_t i = 0; i < this->paths.size(); ++i) {
        auto& path = this->paths[i];
        if (path.is_checked) {
            unlink(path.name.c_str());
        }
        else {
            if (message.empty()) {
                message = "not checked: ";
            }
            char buf[42];
            std::snprintf(buf, sizeof(buf), "[%zu]\"", i);
            message += buf;
            message += path.name;
            message += "\", ";
        }
    }

    if (!message.empty()) {
        message.resize(message.size() - 2);
    }

    auto const len = message.size();

    if (-1 == rmdir(this->directory.c_str()) && errno == ENOTEMPTY) {
        if (DIR* dir = opendir(this->directory.c_str())) {
            while (auto ent = readdir(dir)) {
                if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
                    continue;
                }
                if (len == message.size()) {
                    message += "   files exists: ";

                }
                message += '\"';
                message += directory;
                message += ent->d_name;
                message += "\" ";
            }
            closedir(dir);
        }
    }

    if (len != message.size()) {
        message.resize(message.size() - 2);
    }

    return message;
}

WorkingDirectory::~WorkingDirectory() noexcept(false)
{
    if (!this->is_checked) {
        auto s = this->clean_and_get();
        if (!s.empty()) {
            throw std::runtime_error(s);
        }
    }
}
