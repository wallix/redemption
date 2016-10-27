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

#ifndef MLN_IO_PNM_LOAD_HEADER_HH
#define MLN_IO_PNM_LOAD_HEADER_HH

/// \file
///
/// Define a function which loads header for PNM image.

#include <iostream>
#include <fstream>
#include <string>


namespace mln
{

  namespace io
  {

      namespace pnm
      {

# ifndef MLN_INCLUDE_ONLY

    inline
    bool read_header(std::ifstream& istr,
             char& type,
             int& nrows, int& ncols,
             unsigned int& maxval,
             bool test = false)
    {
      // check magic
      if (istr.get() != 'P' )
        goto err;
      type = static_cast<char>(istr.get());

      if (type < '1' || type > '6')
        goto err;
      if (istr.get() != '\n')
        goto err;

      // skip comments
      while (istr.peek() == '#')
      {
        std::string line;
        std::getline(istr, line);
      }

      // get size
      istr >> ncols >> nrows;
      if (nrows <= 0 || ncols <= 0)
        goto err;

      // get maxvalue
      if (istr.get() != '\n')
        goto err;
      if (type != '1' && type != '4')
      {
        istr >> maxval;
        if (istr.get() != '\n')
          goto err;
      }
      return true;

    err:
      if (! test)
      {
        std::cerr << "error: badly formed header!";
      }
      return false;
    }

    inline
    bool read_header(char ascii, char raw,
             std::ifstream& istr,
             char& type,
             int& nrows, int& ncols,
             unsigned int& maxval)
    {
      if (!read_header(istr, type, nrows, ncols, maxval)
       || ! (type == ascii || type == raw))
      {
        std::cerr << "error: bad pnm type; "
              << "expected P" << ascii
              << " or P" << raw
              << ", get P" << type << "!" << std::endl;
        return false;
      }
      return true;
    }

    inline
    bool read_header(char ascii, char raw,
             std::ifstream& istr,
             char& type,
             int& nrows, int& ncols)
    {
      unsigned int maxval;
      return read_header(ascii, raw, istr, type,
              nrows, ncols, maxval);
    }

# endif // ! MLN_INCLUDE_ONLY

      } // end of namespace mln::io::pnm

  } // end of namespace mln::io

} // end of namespace mln


#endif // ! MLN_IO_PNM_LOAD_HEADER_HH
