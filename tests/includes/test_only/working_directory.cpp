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
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/scope_exit.hpp"

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
    struct TempBase
    {
        TempBase()
        {
            char const* s = std::getenv("TMPDIR");
            if (s) {
                if (*s) {
                    this->dirname = s;
                    if (this->dirname.back() != '/') {
                        this->dirname += '/';
                    }
                }
            }
            else {
                this->dirname = "/tmp/";
            }
        }

        std::string dirname;
    };

    std::string const& tempbase()
    {
        static const TempBase tempbase;
        return tempbase.dirname;
    }
}


WorkingDirectory::Path::Path() noexcept = default;

WorkingDirectory::Path::Path(std::string name, int counter_id) noexcept
: name(std::move(name))
, type(this->name.back() == '/' ? Type::Directory : Type::File)
, counter_id(counter_id)
{}

WorkingDirectory::Path::Path(std::unique_ptr<WorkingDirectory>&& child, int counter_id) noexcept
: name(child->dirname())
, child(std::move(child))
, type(Type::Directory)
, counter_id(counter_id)
{}

bool WorkingDirectory::Path::operator == (Path const& other) const
{
    return this->name == other.name;
}


std::size_t WorkingDirectory::HashPath::operator()(Path const& path) const
{
    return std::hash<std::string>()(path.name);
}


WorkingDirectory::WorkingDirectory(std::string_view dirname)
  : WorkingDirectory(dirname, tempbase())
{}

WorkingDirectory::WorkingDirectory(std::string_view dirname, std::string_view dirbase)
{
    if (dirname.empty() || dirname.find_first_of("/.") != std::string::npos) {
        throw std::runtime_error("invalid dirname");
    }

    this->directory = str_concat(dirbase, dirname, '/');

    recursive_delete_directory(this->directory.c_str());
    if (-1 == mkdir(this->directory.c_str(), 0755) && errno != EEXIST) {
        throw std::runtime_error(strerror(errno));
    }
}

WorkingDirectory& WorkingDirectory::create_subdirectory(std::string_view directory)
{
    auto [it, inserted] = this->paths.emplace(
        std::unique_ptr<WorkingDirectory>(new WorkingDirectory(directory, this->directory)),
        this->counter_id);
    if (!inserted) {
        this->has_error = true;
        throw std::runtime_error(str_concat(directory, " exists"));
    }
    return *it->child;
}

std::string WorkingDirectory::add_file(std::string file)
{
    auto [it, b] = this->paths.emplace(std::move(file), this->counter_id);
    if (!b) {
        this->has_error = true;
        throw std::runtime_error(str_concat("WorkingDirectory: ", it->name, " already exists"));
    }
    this->is_checked = false;
    return this->path_of(it->name);
}

WorkingDirectory& WorkingDirectory::add_files(std::initializer_list<std::string> files)
{
    for (auto const& sv : files) {
        if (!this->paths.emplace(sv, this->counter_id).second) {
            this->has_error = true;
            throw std::runtime_error(str_concat("WorkingDirectory: ", sv, " already exists"));
        }
    }
    this->is_checked = false;
    return *this;
}

WorkingDirectory& WorkingDirectory::remove_files(std::initializer_list<std::string> files)
{
    for (auto const& sv : files) {
        if (this->paths.erase(Path(sv, 0))) {
            this->has_error = true;
            throw std::runtime_error(str_concat("WorkingDirectory: ", sv, " unknown"));
        }
    }
    this->is_checked = false;
    return *this;
}

std::string WorkingDirectory::path_of(std::string_view path) const
{
    return str_concat(this->directory, path);
}

std::string const& WorkingDirectory::dirname() const noexcept
{
    return this->directory;
}

std::string WorkingDirectory::unmached_files()
{
    this->is_checked = true;
    ++this->counter_id;

    Path path(this->dirname(), 0);
    Path filename;
    std::string err;

    auto check_not_found = [&err](WorkingDirectory const& wd){
        for (auto const& p : wd.paths) {
            if (p.counter_id != wd.counter_id) {
                str_append(err, p.name, ' ', wd.path_of(p.name), " not found\n");
            }
        }
    };

    auto unmached_files_impl = [&](auto recursive, WorkingDirectory const& wd) -> bool
    {
        DIR * dir = opendir(path.name.c_str());
        if (!dir) {
            str_append(err, "opendir ", path.name, " error: ", strerror(errno), '\n');
            return false;
        }
        SCOPE_EXIT(closedir(dir));

        auto const original_path_len = path.name.size();
        auto const original_filename_len = filename.name.size();

        bool has_entry = false;

        while (struct dirent* ent = readdir(dir)) {
            if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
                continue;
            }

            has_entry = true;

            path.name += ent->d_name;
            filename.name += ent->d_name;

            struct stat statbuf;
            auto const type = (!stat(path.name.c_str(), &statbuf) && S_ISDIR(statbuf.st_mode))
              ? Type::Directory : Type::File;

            if (type == Type::Directory) {
                path.name += '/';
                filename.name += '/';
            }

            auto it = wd.paths.find(filename);
            if (it == wd.paths.end()) {
                if (type == Type::Directory) {
                    if(!recursive(recursive, wd)) {
                        str_append(err, path.name, " unknown\n");
                    }
                }
                else {
                    str_append(err, path.name, " unknown\n");
                }
            }
            else {
                it->counter_id = wd.counter_id;
                if (it->type != type) {
                    str_append(err, path.name, " unmatching file type\n");
                }
                else if (it->child) {
                    std::string tmp = std::move(filename.name);
                    auto& sub_wd = *it->child;
                    ++sub_wd.counter_id;
                    recursive(recursive, sub_wd);
                    check_not_found(sub_wd);
                    filename.name = std::move(tmp);
                }
                else if (type == Type::Directory){
                    recursive(recursive, wd);
                }
            }

            path.name.resize(original_path_len);
            filename.name.resize(original_filename_len);
        }

        return has_entry;
    };

    unmached_files_impl(unmached_files_impl, *this);
    check_not_found(*this);
    this->has_error = this->has_error || !err.empty();
    return err;
}

WorkingDirectory::~WorkingDirectory() noexcept(false)
{
    if (!this->has_error) {
        recursive_delete_directory(this->dirname().c_str());
        if (!this->is_checked) {
            throw std::runtime_error("WorkingDirectory: unchecked entries");
        }
    }
}
