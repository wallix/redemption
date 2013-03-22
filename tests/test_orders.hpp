/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2011
   Author(s): Christophe Grosjean

   Shared test functions for Unit test of RDP Orders coder/decoder
   - Using lib boost functions for testing
*/

#include"RDP/orders/RDPOrdersCommon.hpp"


void check_datas(size_t lg_data, uint8_t * data,
                 size_t lg_result, uint8_t * expected_result,
                 const char * message)
{
    size_t lg_min = std::min(lg_result,lg_data);
    if (lg_result != lg_data){
        printf("Got      %u:", (unsigned)lg_data);
        size_t j = 0;
        for (j = 0; j < lg_data; j++){
            printf(" %.2x,", data[j]);
        }
        printf("\n");
        printf("Expected %u:", (unsigned)lg_result);
        for (j = 0; j < lg_result; j++){
            printf(" %.2x,", expected_result[j]);
        }
        printf("\n");
        size_t i = 0;
        for (i = 0; i < lg_min; i++){
            if (data[i] != expected_result[i]){
                break;
            }
        }
        BOOST_CHECK_MESSAGE(lg_result == lg_data,
            "test " << message << ": length mismatch\n"
            "Expected " << lg_result << "\n"
            "Got " << lg_data << "\n"
            "Data differs at index " << i << "\n"
            "Expected " << (int)expected_result[i] << "\n"
            "Got " << (int)data[i] << "\n");
        return;
    }

    for (size_t i = 0; i < lg_result; i++){
        if (expected_result[i] != data[i]){
            printf("Got      %u:", (unsigned)lg_data);
            for (size_t j = 0; j < lg_data; j++){
                printf(" %.2x,", data[j]);
            }
            printf("\n");
            printf("Expected %u:", (unsigned)lg_result);
            for (size_t j = 0; j < lg_result; j++){
                printf(" %.2x,", expected_result[j]);
            }
            printf("\n");
        }
        BOOST_CHECK_MESSAGE(expected_result[i] == data[i],
            "test "         << message << " :"
            << " expected " << (int)expected_result[i]
            << " got "      << (int)data[i]
            << " at index " << i
            << "\n");
    }
}

template <class RDPOrderType>
void check(const RDPOrderCommon & common,
           const RDPOrderType & cmd,
           const RDPOrderCommon & expected_common,
           const RDPOrderType & expected_cmd,
           const char * message)
{
    char buffer[1024];
    size_t idx = 0;
    idx += snprintf(buffer + idx,   1024 - idx, "%s:\n", message);
    idx += snprintf(buffer + idx,   1024 - idx, "Expected ");
    idx += expected_cmd.str(buffer + idx, 1024 - idx, expected_common);
    idx += snprintf(buffer + idx,   1024 - idx, "\nGot ");
    idx += cmd.str(buffer + idx, 1024 - idx, common);
    buffer[1023] = 0;

    BOOST_CHECK_MESSAGE((expected_common == common) && (expected_cmd == cmd), buffer);
}

template <class RDPOrderType>
void check(const RDPOrderType & cmd, const RDPOrderType & expected_cmd,
           const char * message)
{
    char buffer[2048];
    size_t idx = 0;
    idx += snprintf(buffer + idx,   1024 - idx, "%s:\n", message);
    idx += snprintf(buffer + idx,   1024 - idx, "Expected ");
    idx += expected_cmd.str(buffer + idx, 1024 - idx);
    idx += snprintf(buffer + idx,   1024 - idx, "\nGot ");
    idx += cmd.str(buffer + idx, 1024 - idx);
    buffer[1023] = 0;

    BOOST_CHECK_MESSAGE(expected_cmd == cmd, buffer);
}
