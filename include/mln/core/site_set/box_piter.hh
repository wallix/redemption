// Copyright (C) 2007, 2008, 2009 EPITA Research and Development Laboratory (LRDE)
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

#ifndef MLN_CORE_SITE_SET_BOX_PITER_HH
# define MLN_CORE_SITE_SET_BOX_PITER_HH

/// \file
///
/// Definition of iterators on points of boxes.

# include <mln/core/site_set/box.hh>


namespace mln
{

  /// \brief A generic forward iterator on points of boxes.
  ///
  /// The parameter \c P is the type of points.
  ///
  /// \see mln::box
  ///
  class box2d_fwd_piter_
  {
  public:
    // Make definitions from super class available.
    enum { dim = point2d::dim };

    /// Constructor without argument.
    box2d_fwd_piter_()
    {
    }

    /// Constructor.
    ///
    /// \param[in] b A box.
    ///
    box2d_fwd_piter_(const mln::box2d& b)
    : s_(b)
    {
    }

    /// Test the iterator validity.
    bool is_valid() const
    {
        mln_precondition(s_.is_valid());
        return p_[0] != s_.pmax()[0] + 1;
    }

    /// Start an iteration.
    void start()
    {
        mln_precondition(s_.is_valid());
        p_ = s_.pmin();
    }

    /// Go to the next point.
    void next()
    {
        mln_precondition(is_valid());
        if (p_[1] != s_.pmax()[1])
        {
            ++p_[1];
            return;
        }
        p_[1] = s_.pmin()[1];

        if (p_[0] != s_.pmax()[0])
        {
            ++p_[0];
            return;
        }
        p_[0] = static_cast<def::coord>(s_.pmax()[0] + 1);
    }

    operator point2d const &() const
    { return this->p_; }

    point2d::coord row() const
    { return this->p_.row(); }

    point2d::coord col() const
    { return this->p_.col(); }

  protected:
    point2d p_;
    box2d s_;
  };


} // end of namespace mln


#endif // ! MLN_CORE_SITE_SET_BOX_PITER_HH
