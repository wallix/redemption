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

#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>


#define RED_CHECK_WORKSPACE(wd) RED_CHECK_EQUAL(wd.unmached_files(), "")

struct [[nodiscard]] WorkingDirectory
{
    struct SubDirectory
    {
        [[nodiscard]] std::string add_file(std::string_view file);
        [[nodiscard]] SubDirectory& add_files(std::initializer_list<std::string_view> files);
        [[nodiscard]] SubDirectory& remove_files(std::initializer_list<std::string_view> files);

        std::string path_of(std::string_view path) const;
        std::string_view dirname() const;

        WorkingDirectory& wd() const noexcept
        {
            return this->wd_;
        }

    private:
        WorkingDirectory& wd_;
        std::string fullpath;
        std::size_t dirname_pos;

        friend class WorkingDirectory;

        SubDirectory(WorkingDirectory& wd, std::string fullpath, std::size_t dirname_pos);
    };

    WorkingDirectory(WorkingDirectory const&) = delete;

    WorkingDirectory(std::string_view dirname);

    SubDirectory create_subdirectory(std::string_view dirname);

    /*
     * filename with '/' at back is a directory
     */
    /// @{
    [[nodiscard]] std::string add_file(std::string file);
    [[nodiscard]] WorkingDirectory& add_files(std::initializer_list<std::string_view> files);
    void remove_file(std::string file);
    [[nodiscard]] WorkingDirectory& remove_files(std::initializer_list<std::string_view> files);
    /// @}

    std::string path_of(std::string_view path) const;

    std::string const& dirname() const noexcept;

    [[nodiscard]] std::string unmached_files();

    ~WorkingDirectory() noexcept(false);

private:
    std::string const& add_file_(std::string file);
    void remove_file_(std::string file);

    enum class Type : bool
    {
        File, Directory
    };

    struct Path
    {
        std::string name;
        Type type;
        mutable int counter_id;

        Path() noexcept;

        Path(std::string name, int counter_id) noexcept;

        Path(std::unique_ptr<WorkingDirectory>&& child, int counter_id) noexcept;

        bool operator == (Path const& other) const;
    };

    struct HashPath
    {
        std::size_t operator()(Path const& path) const;
    };

    std::unordered_set<Path, HashPath> paths;
    std::string directory;
    bool has_error = false;
    bool is_checked = true;
    int counter_id = 0;
};
