// Copyright (C) 2008, 2009, 2010 EPITA Research and Development
// Laboratory (LRDE)
//
// This file is part of Olena.
//
// Olena is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation, version 2 of the License.
//
// Olena is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Olena.  If not, see <http://www.gnu.org/licenses/>.
//
// As a special exception, you may use this file as part of a free
// software project without restriction.  Specifically, if other files
// instantiate templates or use macros or inline functions from this
// file, or you compile this file and link it with other files to produce
// an executable, this file does not by itself cause the resulting
// executable to be covered by the GNU General Public License.  This
// exception does not however invalidate any other reasons why the
// executable file might be covered by the GNU General Public License.

#ifndef MLN_IMAGE_IMAGE2D_HH
# define MLN_IMAGE_IMAGE2D_HH

#include <ppocr/image/coordinate.hpp>
#include <mln/core/site_set/box.hh>

#include <cassert>

namespace mln {

    template <typename T>
    class image2d
    {
        struct data_type
        {
            T*  buffer_ = nullptr;
            std::size_t real_size = 0;
            unsigned row_ = 0;
            unsigned col_ = 0;

            data_type() = default;

            data_type(unsigned nrows, unsigned ncols)
            : row_(nrows)
            , col_(ncols)
            {
                this->allocate_();
            }

            ~data_type()
            {
                delete [] this->buffer_;
            }

            operator bool () const
            {
                return this->buffer_;
            }

            void allocate_()
            {
                this->real_size = this->row_ * this->col_;
                this->buffer_ = new T[this->real_size];
            }

            void reallocate_(unsigned row, unsigned col)
            {
                this->row_ = row;
                this->col_ = col;

                const std::size_t sz = this->row_ * this->col_;

                if (sz > this->real_size) {
                    delete [] this->buffer_;
                    this->buffer_ = nullptr;
                    this->real_size = sz;
                    this->buffer_ = new T[this->real_size];
                }
            }

            int index(ppocr::Index p) const
            {
                return p.y() * this->col_ + p.x();
            }
        };

        // Internal data, sharable by several images.
        data_type data_;

    public:
        /// Value associated type.
        typedef T         value;

    private:
        image2d(const image2d&) /* = delete*/;
        image2d& operator=(const image2d&) /* = delete*/;

    public:
        /// Constructor without argument.
        image2d()
        : data_()
        {}

        /// Constructor with the numbers of rows and columns and the
        /// border thickness.
        image2d(unsigned nbrows, unsigned nbcols)
        : data_(nbrows, nbcols)
        {}

        /// Initialize an empty image.
        void init_(unsigned nbrows, unsigned nbcols)
        {
            assert(!this->is_valid());
            new (&this->data_) data_type(nbrows, nbcols);
        }

        void realloc(unsigned nbrows, unsigned nbcols)
        {
            if (!this->data_) {
                new (&this->data_) data_type(nbrows, nbcols);
            }
            else {
                this->data_.reallocate_(nbrows, nbcols);
            }
        }

        /// Test if this image has been initialized; default impl.
        bool is_valid() const
        {
            return this->data_;
        }

        /// Read-only access to the image value located at point \p p.
        const T& operator[](ppocr::Index p) const
        {
            assert(this->check_index(p));
            return this->data_.buffer_[this->data_.index(p)];
        }

        /// Read-write access to the image value located at point \p p.
        T& operator[](ppocr::Index p)
        {
            assert(this->check_index(p));
            return this->data_.buffer_[this->data_.index(p)];
        }

        /// Give the number of rows.
        unsigned nrows() const
        {
            assert(this->is_valid());
            return this->data_.row_;
        }

        /// Give the number of columns.
        unsigned ncols() const
        {
            assert(this->is_valid());
            return this->data_.col_;
        }

    private:
#ifndef NDEBUG
        bool check_index(ppocr::Index p) const
        {
            assert(is_valid());
            return 0 <= p.y() && p.y() < nrows()
                && 0 <= p.x() && p.x() < ncols();
        }
#endif
    };
}

#endif
