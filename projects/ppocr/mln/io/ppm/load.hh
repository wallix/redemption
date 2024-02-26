// Copyright (C) 2009 EPITA Research and Development Laboratory (LRDE)
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

#ifndef MLN_IO_PPM_LOAD_HH
#define MLN_IO_PPM_LOAD_HH

/// \file
///
/// Define a function which loads an image of kind ppm with
/// given path.

#include <string>

#include <mln/image/image2d.hh>
#include <mln/io/pnm/load.hh>


namespace mln
{

  namespace io
  {

    namespace ppm
    {

      /// Load a ppm image in a Milena image.
      ///
      /// \param[out] ima A reference to the image which will receive
      /// data.
      /// \param[in] filename The source.
      template <typename I>
      bool load(image2d<I>& ima, const std::string& filename)
      {
          return io::pnm::load<I>(PPM, ima, filename);
      }

    } // end of namespace mln::io::ppm

  } // end of namespace mln::io

} // end of namespace mln


#endif // ! MLN_IO_PPM_LOAD_HH
