// Copyright (C) 2007, 2008, 2009, 2010 Epoint2dITA Research and Development
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

#ifndef MLN_CORE_SITE_SET_BOX_HH
# define MLN_CORE_SITE_SET_BOX_HH

/// \file
///
/// This file defines a generic box2d class.
///
/// \todo Test if the safety code in box2d::box2d() is not too slow.

#include <mln/core/point.hh>
#include <cassert>

namespace mln
{
  /// \brief Generic box2d class: site set containing points of a
  /// regular grid.
  ///
  /// Parameter \c point2d is the corresponding type of point.
  ///
  /// \ingroup modsitesetbasic
  //
  struct box2d
  {
    /// Minimum point.
    const point2d &  pmin() const
    {
        return pmin_;
    }

    /// Reference to the minimum point.
    point2d & pmin()
    {
        return pmin_;
    }

    /// Maximum point.
    const point2d & pmax() const
    {
        return pmax_;
    }

    /// Reference to the maximum point.
    point2d & pmax()
    {
        return pmax_;
    }

    box2d()
    : box2d(point2d{}, point2d{})
    {}

    /// Constructor of a box2d going from \p pmin to \p pmax.
    box2d(const point2d& point_min, const point2d& point_max)
    : pmin_(point_min)
    , pmax_(point_max)
    {
        assert(is_valid());
    }

    /*! \brief Test if \p p belongs to the box2d.
     *
     * \param[in] p A point site.
     */
    bool has(const point2d& p) const
    {
        assert(is_valid());
        return !(p.col < pmin_.col || p.col > pmax_.col || p.row < pmin_.row || p.row > pmax_.row);
    }

    /// Test that the box2d owns valid data, i.e., is initialized and
    /// with pmin being 'less-than' pmax.
    bool is_valid() const
    {
        // Validity is: for all i, pmin_[i] <= pmax_[i].
        // Nota bene: a one-point box2d is valid.
        return pmin_.col <= pmax_.col && pmin_.row <= pmax_.row;
    }

    unsigned nrows() const
    {
        return this->is_valid()
        ? 1 + this->pmax().row - this->pmin().row
        : 0u;
    }

    point2d::coord min_row() const
    {
        return this->pmin().row;
    }

    point2d::coord max_row() const
    {
        return this->pmax().row;
    }

    unsigned ncols() const
    {
        return this->is_valid()
        ? 1 + this->pmax().col - this->pmin().col
        : 0u;
    }

    point2d::coord min_col() const
    {
        return this->pmin().col;
    }

    point2d::coord max_col() const
    {
        return this->pmax().col;
    }

  private:
    point2d pmin_, pmax_;
  };

  inline bool operator==(const box2d& a, const box2d& b)
  {
    return a.min_row() == b.min_row()
        && a.min_col() == b.min_col()
        && a.max_row() == b.max_row()
        && a.max_col() == b.max_col();
  }
} // end of namespace mln


#endif // ! MLN_CORE_SITE_SET_BOX_HH
