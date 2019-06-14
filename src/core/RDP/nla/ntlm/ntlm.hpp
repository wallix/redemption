/*
  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the
  Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
  Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  675 Mass Ave, Cambridge, MA 02139, USA.

  Product name: redemption, a FLOSS RDP proxy
  Copyright (C) Wallix 2013
  Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan
*/


#pragma once

#include "core/RDP/nla/sspi.hpp"
#include "core/RDP/nla/ntlm/ntlm_context.hpp"
#include "utils/sugar/byte_ptr.hpp"

#include <memory>
#include <functional>


// TODO: constants below are still globals,
// better to move them in the scope of functions/objects using them
//const char* NTLM_PACKAGE_NAME = "NTLM";
// const char Ntlm_Name[] = "NTLM";
// const char Ntlm_Comment[] = "NTLM Security Package";
// const SecPkgInfo NTLM_SecPkgInfo = {
//     0x00082B37,             // fCapabilities
//     1,                      // wVersion
//     0x000A,                 // wRPCID
//     0x00000B48,             // cbMaxToken
//     Ntlm_Name,              // Name
//     Ntlm_Comment            // Comment
// };
constexpr uint32_t cbMaxSignature = 16;
// SecPkgContext_Sizes ContextSizes;
// ContextSizes.cbMaxToken = 2010;
// ContextSizes.cbMaxSignature = 16;
// ContextSizes.cbBlockSize = 0;
// ContextSizes.cbSecurityTrailer = 16;



