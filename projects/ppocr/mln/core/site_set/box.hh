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

# include <mln/core/point.hh>
# include <mln/core/contract.hh>

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
    /// Dimension.
    enum { dim = point2d::dim };

    /// Site associated type.
    typedef point2d site;

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

    /// Constructor without argument.
    box2d()
    : pmin_(point2d::plus_infty())
    , pmax_(point2d::minus_infty())
    {
        // FIXME: The code above can be slow; think about removing it...
    }

    /// Constructor of a box2d going from \p pmin to \p pmax.
    box2d(const site& point_min, const site& point_max)
    : pmin_(point_min)
    , pmax_(point_max)
    {
        mln_precondition(is_valid());
    }

    box2d(point2d::coord nbrows, point2d::coord nbcols)
    : pmin_(0, 0)
    , pmax_(--nbrows, --nbcols)
    {
        mln_precondition(nbrows != 0 && nbcols != 0);
        mln_postcondition(is_valid());
    }

    /*! \brief Test if \p p belongs to the box2d.
     *
     * \param[in] p A point site.
     */
    bool has(const point2d& p) const
    {
        mln_precondition(is_valid());
        return !(p[0] < pmin_[0] || p[0] > pmax_[0] || p[1] < pmin_[1] || p[1] > pmax_[1]);
    }

    /// Test that the box2d owns valid data, i.e., is initialized and
    /// with pmin being 'less-than' pmax.
    bool is_valid() const
    {
        // Validity is: for all i, pmin_[i] <= pmax_[i].
        // Nota bene: a one-point box2d is valid.
        return pmin_[0] <= pmax_[0] && pmin_[1] <= pmax_[1];
    }

    unsigned nrows() const
    {
        return this->len(0);
    }

    point2d::coord min_row() const
    {
        return this->pmin()[0];
    }

    point2d::coord max_row() const
    {
        return this->pmax()[0];
    }

    unsigned ncols() const
    {
        return this->len(1);
    }

    point2d::coord min_col() const
    {
        return this->pmin()[1];
    }

    point2d::coord max_col() const
    {
        return this->pmax()[1];
    }

    unsigned len(unsigned i) const
    {
        return this->is_valid()
        ? 1 + this->pmax()[i] - this->pmin()[i]
        : 0u;
    }

    unsigned nsites() const
    {
        if (! this->is_valid())
            return 0;
        return this->len(0) * this->len(1);
    }

  protected:

    point2d pmin_, pmax_;
  };


  inline bool operator==(const box2d& a, const box2d& b)
  {
    return a.min_row() == b.min_row()
        && a.min_col() == b.min_col()
        && a.max_row() == b.max_row()
        && a.max_col() == b.max_col();
  }


  /*! \brief point2drint a generic box2d \p b into the output stream \p ostr.
   *
   * \param[in,out] ostr An output stream.
   * \param[in] b A generic box2d.
   *
   * \return The modified output stream \p ostr.
   *
   * \relates mln::box2d
   */
  inline
  std::ostream& operator<<(std::ostream& ostr, const box2d& b)
  {
      return ostr << "[" << b.pmin() << ".." << b.pmax() << ']';
  }

} // end of namespace mln


#endif // ! MLN_CORE_SITE_SET_BOX_HH
