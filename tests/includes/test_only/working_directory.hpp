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

#include <iosfwd>


#define RED_CHECK_WORKSPACE(wd) RED_CHECK_EQUAL(wd.unmached_files(), "")

#define RED_AUTO_TEST_CASE_WD(name, wd)            \
    struct WD_TU_ ## name {                        \
        static void test(WorkingDirectory& wd); }; \
    RED_AUTO_TEST_CASE(name) {                     \
        WorkingDirectory wd;                       \
        WD_TU_ ## name ::test(wd);                 \
        RED_CHECK_WORKSPACE(wd);                   \
    }                                              \
    void WD_TU_ ## name ::test(WorkingDirectory& wd)

#define RED_AUTO_TEST_CASE_WF(name, wf)       \
    struct WF_TU_ ## name {                   \
        static void test(WorkingFile& wf); }; \
    RED_AUTO_TEST_CASE(name) {                \
        WorkingFile wf(#name);                \
        WF_TU_ ## name ::test(wf);            \
    }                                         \
    void WF_TU_ ## name ::test(WorkingFile& wf)


struct [[nodiscard]] WorkingFileBase
{
    WorkingFileBase(std::string name) noexcept : filename_(std::move(name)) {}
    char const* c_str() const noexcept { return this->filename_.c_str(); }
    std::string const& string() const noexcept { return this->filename_; }
    operator std::string const& () const noexcept { return this->filename_; }
    operator char const* () const noexcept { return this->c_str(); }
    std::size_t size() const noexcept { return this->filename_.size(); }
    ssize_t ssize() const noexcept { return ssize_t(this->filename_.size()); }

protected:
    std::string filename_;
};

std::ostream& operator<<(std::ostream& out, WorkingFileBase const& wf);

struct [[nodiscard]] WorkingFile : WorkingFileBase
{
    WorkingFile(std::string_view name);
    ~WorkingFile();
    void set_removed(bool x = true) noexcept { this->is_removed = x; }

private:
    unsigned start_error_count;
    bool is_removed = false;
};

struct [[nodiscard]] WorkingDirectory
{
    struct SubDirectory
    {
        [[nodiscard]] WorkingFileBase add_file(std::string_view file);
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

    WorkingDirectory(std::string_view name = {});

    WorkingDirectory(WorkingDirectory const&) = delete;

    SubDirectory create_subdirectory(std::string_view dirname);

    /*
     * filename with '/' at back is a directory
     */
    /// @{
    [[nodiscard]] WorkingFileBase add_file(std::string file);
    [[nodiscard]] WorkingDirectory& add_files(std::initializer_list<std::string_view> files);
    void remove_file(std::string file);
    [[nodiscard]] WorkingDirectory& remove_files(std::initializer_list<std::string_view> files);
    /// @}

    std::string path_of(std::string_view path) const;

    WorkingFileBase const& dirname() const noexcept { return this->dirname_; }

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

    std::unordered_set<Path, HashPath> paths_;
    WorkingFileBase dirname_;
    bool has_error_ = false;
    bool is_checked_ = true;
    int counter_id_ = 0;
    unsigned start_error_count_;
};
