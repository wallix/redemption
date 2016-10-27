// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 EPITA Research and Development Laboratory (LRDE)
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

#ifndef MLN_IO_PNM_LOAD_HH
#define MLN_IO_PNM_LOAD_HH

/// \file
///
/// Define a function which loads an image of kind PNM 8/16bits with
/// given path.

#include <iostream>
#include <fstream>
#include <string>

#include <mln/image/image2d.hh>
#include <mln/trait_value_.hh>

#include <mln/io/pnm/load_header.hh>
#include <mln/io/pnm/macros.hh>

namespace mln
{

  namespace io
  {

    namespace pnm
    {

      template <typename I>
      bool load_ascii_value(std::ifstream& file, I& ima);

      template <typename I>
      bool load_ascii_builtin(std::ifstream& file, I& ima);

      // used when (sizeof(int_u8) != 1)
      template <typename V>
      inline
      void load_raw_2d_uncontiguous(std::ifstream& file, image2d<V>& ima)
      {
          const def::coord min_row = 0;
          const def::coord min_col = 0;
          const def::coord max_row = ima.nrows();
          const def::coord max_col = ima.ncols();

          point2d p;
          for (p.row() = min_row; p.row() < max_row; ++p.row())
              for (p.col() = min_col; p.col() < max_col; ++p.col())
                  ::mln::trait::value_<V>::read_value(file, ima(p));
      }

      // used in g++ > 2.95
      template <typename V>
      inline
      void load_raw_2d_contiguous(std::ifstream& file, image2d<V>& ima)
      {
          point2d p = point2d(0, ima.ncols() - 1);
          typedef typename image2d<V>::site::coord coord;
          const coord min_row = 0;
          const coord max_row = ima.nrows();

          std::size_t len = ima.ncols() * sizeof(V);
          for (p.row() = min_row; p.row() < max_row; ++p.row())
              file.read(reinterpret_cast<char*>(&ima(p)), len);
      }

      /// load_ascii for Milena value types.
      template <typename V>
      inline
      void load_ascii_value(std::ifstream& file, image2d<V>& ima)
      {
          typename image2d<V>::value::equiv c;
          typename image2d<V>::fwd_piter p(ima.domain());
          for (p .start(); p .is_valid(); p .next())
          {
              file >> c;
              ima(p) = c;
          }
      }

      /// load_ascii for builtin value types.
      template <typename V>
      inline
      void load_ascii_builtin(std::ifstream& file, image2d<V>& ima)
      {
          typename image2d<V>::fwd_piter p(ima.domain());

          // FIXME: May be wrong!
          // Worked out with an image with a max value of 255
          // loaded in an image2d<unsigned char>.
          typename image2d<V>::value n;

          for (p.start(); p.is_valid(); p.next())
          {
              ::mln::trait::value_<V>::read_value(file, n);
              ima(p) = n;
          }
      }

      /// load_raw_2d.
      /// for all pnm 8/16 bits formats
      template <typename V>
      inline
      void load_raw_2d(std::ifstream& file, image2d<V>& ima)
      {
          if (sizeof(V) == 1)
              load_raw_2d_contiguous(file, ima);
          else
              load_raw_2d_uncontiguous(file, ima);
      }

      /// An other way to load pnm files :
      /// the destination is an argument to check if
      /// the type match the file to load.
      template <typename I>
      inline
      bool load(char type_,
              image2d<I>& ima,
              const std::string& filename)
      {
          std::ifstream file(filename.c_str());
          if (! file)
          {
              return false;
          }

          char type = 0;
          int nrows, ncols;
          unsigned int maxval;
          if (!read_header(static_cast<char>(type_ - 3), type_, file, type,
                      nrows, ncols, maxval)) {
              return false;
          }

          if (::mln::trait::value_<I>::max() != maxval)
          {
              std::cerr << "error: file '" << filename
                      << "' cannot be loaded into this type of image\n"
                      << "input image have " << maxval
                      << " as maximum value while the destination's one is "
                      << ::mln::trait::value_<I>::max() << "."
                      << std::endl;
              return false;
          }

          ima.init_(nrows, ncols);
          if (type == type_)
              load_raw_2d(file, ima);
          else if (type == (type_ - 3))
              pnm::load_ascii_builtin(file, ima);
          return file.eof() == true || file.rdbuf()->sgetc() == std::ifstream::traits_type::eof();
      }

    } // end of namespace mln::io::pnm

  } // end of namespace mln::io

} // end of namespace mln


#endif // ! MLN_IO_PNM_LOAD_HH
