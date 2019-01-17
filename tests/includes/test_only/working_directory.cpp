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
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "working_directory.hpp"
#include "utils/fileutils.hpp"
#include "utils/pp.hpp"
#include "utils/sugar/algostring.hpp"
#include "utils/sugar/scope_exit.hpp"
#include "cxx/compiler_version.hpp"

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
    std::string const& tempbase()
    {
        static const std::string base = []{
            std::string dirname;
            char const* s = std::getenv("TMPDIR");
            if (s) {
                if (*s) {
                    dirname = s;
                    if (dirname.back() != '/') {
                        dirname += '/';
                    }
                }
            }
            else {
                dirname = "/tmp/";
            }
            return dirname;
        }();
        return base;
    }

    constexpr std::string_view suffix_by_compiler()
    {
        return
            "-red_" RED_PP_STRINGIFY(REDEMPTION_COMP_NAME) "-"
            REDEMPTION_COMP_STRING_VERSION "/"
        ;
    }

#define WD_ERROR_S(ostream_expr) RED_ERROR("WorkingDirectory: " ostream_expr)
#define WD_ERROR(ostream_expr) RED_ERROR("WorkingDirectory: " << ostream_expr)
}


WorkingDirectory::SubDirectory::SubDirectory(
    WorkingDirectory& wd, std::string fullpath, std::size_t dirname_pos)
: wd_(wd)
, fullpath(fullpath)
, dirname_pos(dirname_pos)
{}

std::string WorkingDirectory::SubDirectory::add_file(std::string_view file)
{
    return this->wd_.add_file(str_concat(this->dirname(), file));
}

WorkingDirectory::SubDirectory& WorkingDirectory::SubDirectory::add_files(
    std::initializer_list<std::string_view> files)
{
    for (auto sv : files) {
        (void)this->wd_.add_file_(str_concat(this->dirname(), sv));
    }
    return *this;
}

WorkingDirectory::SubDirectory& WorkingDirectory::SubDirectory::remove_files(
    std::initializer_list<std::string_view> files)
{
    for (auto sv : files) {
        (void)this->wd_.remove_files({str_concat(this->dirname(), sv)});
    }
    return *this;
}

std::string_view WorkingDirectory::SubDirectory::dirname() const
{
    return std::string_view(this->fullpath).substr(this->dirname_pos);
}

std::string WorkingDirectory::SubDirectory::path_of(std::string_view path) const
{
    return this->wd_.path_of(str_concat(this->dirname(), path));
}


WorkingDirectory::Path::Path() noexcept = default;

WorkingDirectory::Path::Path(std::string name, int counter_id) noexcept
: name(std::move(name))
, type(this->name.back() == '/' ? Type::Directory : Type::File)
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
{
    if (dirname.empty() || dirname.find_first_of("/.") != std::string::npos) {
        WD_ERROR_S("invalid dirname");
    }

    this->directory = str_concat(tempbase(), dirname, suffix_by_compiler());

    recursive_delete_directory(this->directory.c_str());
    if (-1 == mkdir(this->directory.c_str(), 0755) && errno != EEXIST) {
        WD_ERROR(strerror(errno) << ": " << this->directory);
    }
}

WorkingDirectory::SubDirectory WorkingDirectory::create_subdirectory(std::string_view dirname)
{
    auto pos = dirname.find_first_not_of('/');
    if (pos) {
        dirname = dirname.substr(pos+1);
    }

    if (dirname.empty()) {
        WD_ERROR_S("empty dirname");
    }

    if (dirname.back() == '/') {
        dirname = dirname.substr(0, dirname.size()-1);
    }

    auto path = this->add_file(str_concat(dirname, '/'));
    recursive_create_directory(path.c_str(), 0755, -1);
    return SubDirectory(*this, std::move(path), this->directory.size());
}

std::string const& WorkingDirectory::add_file_(std::string file)
{
    auto [it, b] = this->paths.emplace(std::move(file), this->counter_id);
    if (!b) {
        this->has_error = true;
        WD_ERROR(it->name << " already exists");
    }
    this->is_checked = false;
    return it->name;
}

void WorkingDirectory::remove_file_(std::string file)
{
    // transparent compare to C++20
    Path path(std::move(file), 0);
    if (!this->paths.erase(path)) {
        this->has_error = true;
        WD_ERROR_S("unknown file '" << path.name << '\'');
    }
}

std::string WorkingDirectory::add_file(std::string file)
{
    return this->path_of(this->add_file_(std::move(file)));
}

WorkingDirectory& WorkingDirectory::add_files(std::initializer_list<std::string_view> files)
{
    for (auto const& sv : files) {
        this->add_file_(str_concat(sv));
    }
    return *this;
}

void WorkingDirectory::remove_file(std::string file)
{
    this->remove_file_(std::move(file));
    this->is_checked = false;
}

WorkingDirectory& WorkingDirectory::remove_files(std::initializer_list<std::string_view> files)
{
    for (auto const& sv : files) {
        this->remove_file_(str_concat(sv));
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

    auto unmached_files_impl = [&](auto recursive) -> bool
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

            auto it = this->paths.find(filename);
            if (it == this->paths.end()) {
                if (type == Type::Directory) {
                    if(!recursive(recursive)) {
                        str_append(err, path.name, " unknown\n");
                    }
                }
                else {
                    str_append(err, path.name, " unknown\n");
                }
            }
            else {
                it->counter_id = this->counter_id;
                if (it->type != type) {
                    str_append(err, path.name, " unmatching file type\n");
                }
                else if (type == Type::Directory){
                    recursive(recursive);
                }
            }

            path.name.resize(original_path_len);
            filename.name.resize(original_filename_len);
        }

        return has_entry;
    };

    unmached_files_impl(unmached_files_impl);

    for (auto const& p : this->paths) {
        if (p.counter_id != this->counter_id) {
            str_append(err, p.name, ' ', this->path_of(p.name), " not found\n");
        }
    }

    this->has_error = this->has_error || !err.empty();

    return err;
}

WorkingDirectory::~WorkingDirectory() noexcept(false)
{
    if (!this->has_error) {
        recursive_delete_directory(this->dirname().c_str());
        if (!this->is_checked) {
            WD_ERROR_S("unchecked entries");
        }
    }
}
