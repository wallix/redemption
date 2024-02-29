// Copyright (C) 2007, 2008, 2009, 2010 Eppocr::IndexITA Research and Development
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

#include <cassert>

namespace mln
{
  /// \brief Generic box2d class: site set containing points of a
  /// regular grid.
  ///
  /// Parameter \c ppocr::Index is the corresponding type of point.
  ///
  /// \ingroup modsitesetbasic
  //
  struct box2d
  {
    ppocr::Index pmin;
    ppocr::Index pmax;

    box2d()
    : box2d(ppocr::Index{}, ppocr::Index{})
    {}

    /// Constructor of a box2d going from \p pmin to \p pmax.
    box2d(ppocr::Index point_min, ppocr::Index point_max)
    : pmin(point_min)
    , pmax(point_max)
    {
        assert(is_valid());
    }

    /// Test that the box2d owns valid data, i.e., is initialized and
    /// with pmin being 'less-than' pmax.
    bool is_valid() const
    {
        // Validity is: for all i, pmin[i] <= pmax[i].
        // Nota bene: a one-point box2d is valid.
        return pmin.x() <= pmax.x() && pmin.y() <= pmax.y();
    }

    unsigned nrows() const
    {
        return 1 + this->pmax.y() - this->pmin.y();
    }

    unsigned ncols() const
    {
        return 1 + this->pmax.x() - this->pmin.x();
    }

    unsigned min_col() const
    {
        return pmin.x();
    }

    unsigned min_row() const
    {
        return pmin.y();
    }

    unsigned max_col() const
    {
        return pmax.x();
    }

    unsigned max_row() const
    {
        return pmax.y();
    }

    friend bool operator==(const box2d& a, const box2d& b)
    {
        return a.pmin.y() == b.pmin.y()
            && a.pmin.x() == b.pmin.x()
            && a.pmax.y() == b.pmax.y()
            && a.pmax.x() == b.pmax.x();
    }
  };
} // end of namespace mln


#endif // ! MLN_CORE_SITE_SET_BOX_HH
