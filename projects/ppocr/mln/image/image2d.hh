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

#include <mln/core/point.hh>
#include <mln/core/def/coord.hh>
#include <mln/core/site_set/box.hh>
#include <mln/core/site_set/box_piter.hh>

#include <cassert>

namespace mln {

    template <typename T>
    class image2d
    {
        struct data_type
        {
            static const unsigned bdr_ = 1;
            T*  buffer_ = nullptr;
            std::size_t real_size = 0;
            unsigned row_ = 0;
            unsigned col_ = 0;

            data_type() = default;

            data_type(unsigned nrows, unsigned ncols)
            : row_(nrows + this->bdr_ * 2)
            , col_(ncols + this->bdr_ * 2)
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
                this->row_ = row + this->bdr_ * 2;
                this->col_ = col + this->bdr_ * 2;

                const std::size_t sz = this->row_ * this->col_;

                if (sz > this->real_size) {
                    delete [] this->buffer_;
                    this->buffer_ = nullptr;
                    this->real_size = sz;
                    this->buffer_ = new T[this->real_size];
                }
            }

            int index(def::coord row, def::coord col) const
            {
                return (row + this->bdr_) * this->col_ + col + this->bdr_;
            }
        };

        // Internal data, sharable by several images.
        data_type data_;

    public:
        /// Value associated type.
        typedef T         value;

        /// Site_Set associated type.
        typedef box2d domain_t;

        /// Site associated type.
        typedef box2d::site site;


        /// Forward Site_Iterator associated type.
        typedef box2d::fwd_piter fwd_piter;

        /// Site_Iterator associated type; default definition is
        /// fwd_piter.
        typedef fwd_piter        piter;

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
            assert(! this->is_valid());
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

        /// Test if \p p is valid.
        bool has(const point2d& p) const
        {
            assert(this->is_valid());
            return box2d(this->data_.row_ - this->data_.bdr_*2, this->data_.col_ - this->data_.bdr_*2).has(p);
        }

        /// Give the definition domain.
        box2d domain() const
        {
            assert(this->is_valid());
            return box2d(this->nrows(), this->ncols());
        }

        /// Read-only access to the image value located at point \p p.
        const T& operator()(const point2d& p) const
        {
            assert(this->has_with_border(p));
            return this->data_.buffer_[this->data_.index(p.row(), p.col())];
        }

        /// Read-write access to the image value located at point \p p.
        T& operator()(const point2d& p)
        {
            assert(this->has_with_border(p));
            return this->data_.buffer_[this->data_.index(p.row(), p.col())];
        }

        // Specific methods:
        // -----------------

        /// Read-only access to the image value located at (\p row, \p col).
        const T& at(def::coord row, def::coord col) const
        {
            assert(this->has_with_border(point2d(row, col)));
            return this->data_.buffer_[this->data_.index(row, col)];
        }

        /// Read-write access to the image value located at (\p row, \p col).
        T& at(def::coord row, def::coord col)
        {
            assert(this->has_with_border(point2d(row, col)));
            return this->data_.buffer_[this->data_.index(row, col)];
        }

        /// Give the number of rows.
        unsigned nrows() const
        {
            assert(this->is_valid());
            return this->data_.row_ - this->data_.bdr_ * 2;
        }

        /// Give the number of columns.
        unsigned ncols() const
        {
            assert(this->is_valid());
            return this->data_.col_ - this->data_.bdr_ * 2;
        }

        /// Give the border thickness.
        unsigned border() const
        {
            assert(this->is_valid());
            return this->data_.bdr_;
        }

        void fill_border(value x)
        {
            if (this->data_.col_ && this->data_.row_) {
                value * eptr = this->data_.buffer_ + this->data_.col_ * this->data_.bdr_ - this->data_.bdr_;
                std::fill(this->data_.buffer_, eptr, x);
                const unsigned nr = this->nrows();
                for (unsigned r = 0; r < nr; ++r) {
                    eptr[0] = x;
                    eptr[1] = x;
                    eptr += this->data_.col_;
                }
                std::fill(eptr, eptr + this->data_.col_ * this->data_.bdr_ + this->data_.bdr_, x);
            }
        }

    private:
        bool has_with_border(const point2d& p) const
        {
            assert(this->is_valid());
            return box2d(this->data_.row_, this->data_.col_).has(mln::point2d(p.row()+1, p.col()+1));
        }
    };
}

#endif
