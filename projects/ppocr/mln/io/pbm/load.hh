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

#ifndef MLN_IO_PBM_LOAD_HH
#define MLN_IO_PBM_LOAD_HH

/// \file
///
/// Define a function which loads an image of kind pbm with
/// given path.


#include <iostream>
#include <fstream>
#include <string>

#include <mln/image/image2d.hh>
#include <mln/io/pnm/load_header.hh>


namespace mln
{

  namespace io
  {

    namespace pbm
    {

      /// Load a pbm image in a Milena image.
      ///
      /// \param[out] ima A reference to the image2d<bool> which will receive
      /// data.
      /// \param[in] filename The source.
      ///
      bool load(image2d<bool>& ima, const std::string& filename);


# ifndef MLN_INCLUDE_ONLY

      namespace internal
      {

    /// load_ascii.
    template <typename I>
    inline
    void load_ascii(std::ifstream& file, image2d<I>& ima)
    {
      typename image2d<I>::fwd_piter p(ima.domain());
      for(p .start(); p .is_valid(); p .next())
        {
          unsigned char value;
          file >> value;

          mln_assertion(value == '0' || value == '1');
          ima(p) = (value == '0'); // In pbm, '0' means 'white' so 'object', thus 'true'!
        }
    }


    /// load_raw_2d.
    template <typename I>
    inline
    void load_raw_2d(std::ifstream& file, image2d<I>& ima)
    {
      typedef typename image2d<I>::site::coord coord;
      const coord min_row = 0;
      const coord min_col = 0;
      const coord max_row = ima.nrows();
      const coord max_col = ima.ncols();
      typename image2d<I>::site p = point2d(0, min_col);

      char c = 0;
      int i;
      for (p.row() = min_row; p.row() < max_row; ++p.row())
      {
        i = 0;
        for (p.col() = min_col; p.col() < max_col; ++p.col())
          {
        if (i % 8 == 0)
          file.read(&c, 1);
        ima(p) = !(c & 128);
        c = static_cast<char>(c * 2);
        ++i;
          }
      }
    }


      } // end of namespace mln::io::internal


      inline
      bool load(image2d<bool>& ima, const std::string& filename)
      {
    std::ifstream file(filename.c_str());
    if (! file)
    {
      return false;
    }
    char type = 0;
    int nrows, ncols;
    if (!io::pnm::read_header('1', '4', file, type, nrows, ncols)) {
        return false;
    }

    ima.init_(nrows, ncols);
    if (type == '4')
      internal::load_raw_2d(file, ima);
    else if (type == '1')
        internal::load_ascii(file, ima);
    return file.eof() == true || file.rdbuf()->sgetc() == std::ifstream::traits_type::eof();
      }

# endif // ! MLN_INCLUDE_ONLY

    } // end of namespace mln::io::pbm

  } // end of namespace mln::io

} // end of namespace mln


#endif // ! MLN_IO_PBM_LOAD_HH
