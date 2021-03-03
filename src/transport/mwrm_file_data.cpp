/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#include "transport/mwrm_file_data.hpp"

using EncryptionMode = InCryptoTransport::EncryptionMode;

MwrmFileData load_mwrm_file_data(
    char const* mwrm_filename,
    CryptoContext & cctx,
    EncryptionMode encryption_mode)
{
    MwrmFileData mwrm_data;

    std::vector<MetaLine>& wrms = mwrm_data.wrms;
    wrms.reserve(32);

    InCryptoTransport mwrm_file(cctx, encryption_mode);
    MwrmReader mwrm_reader(mwrm_file);

    mwrm_file.open(mwrm_filename);
    mwrm_data.encryption_mode = mwrm_file.is_encrypted() ? EncryptionMode::Encrypted
                                                         : EncryptionMode::NotEncrypted;
    mwrm_reader.read_meta_headers();
    mwrm_data.header = mwrm_reader.get_header();

    while (Transport::Read::Ok == mwrm_reader.read_meta_line(wrms.emplace_back())) {
    }
    wrms.pop_back();

    return mwrm_data;
}
