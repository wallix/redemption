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

#include "core/RDP/orders/RDPOrdersCommon.hpp"
#include "system/redemption_unit_tests.hpp"


inline
void check_datas(size_t lg_data, uint8_t * data,
                 size_t lg_result, uint8_t * expected_result,
                 const char * message)
{
    size_t lg_min = std::min(lg_result,lg_data);
    if (lg_result != lg_data){
        printf("Got      %u:", static_cast<unsigned>(lg_data));
        size_t j = 0;
        for (j = 0; j < lg_data; j++){
            printf(" %.2x,", static_cast<unsigned>(data[j]));
        }
        printf("\n");
        printf("Expected %u:", static_cast<unsigned>(lg_result));
        for (j = 0; j < lg_result; j++){
            printf(" %.2x,", static_cast<unsigned>(expected_result[j]));
        }
        printf("\n");
        size_t i = 0;
        for (i = 0; i < lg_min; i++){
            if (data[i] != expected_result[i]){
                break;
            }
        }

        char buffer[4000];
        sprintf(buffer, "test %s: length mismatch\n"
                        "Expected %u\n"
                        "Got %u\n"
                        "Data differs at index %u\n"
                        "Expected %02x\n"
                        "Got %02x\n", message,
                        static_cast<unsigned>(lg_result),
                        static_cast<unsigned>(lg_data),
                        static_cast<unsigned>(i),
                        static_cast<unsigned>(expected_result[i]),
                        static_cast<unsigned>(data[i]));

        RED_CHECK_MESSAGE(lg_result == lg_data, buffer);
        return;
    }

    for (size_t i = 0; i < lg_result; i++){
        if (expected_result[i] != data[i]){
            printf("Got      %u:", static_cast<unsigned>(lg_data));
            for (size_t j = 0; j < lg_data; j++){
                printf(" %.2x,", static_cast<unsigned>(data[j]));
            }
            printf("\n");
            printf("Expected %u:", static_cast<unsigned>(lg_result));
            for (size_t j = 0; j < lg_result; j++){
                printf(" %.2x,", static_cast<unsigned>(expected_result[j]));
            }
            printf("\n");
        }

        char buffer[4000];
        sprintf(buffer, "test %s :"
                        " expected %d"
                        " got %d"
                        " at index %u"
                        "\n",
                        message,
                        static_cast<int>(expected_result[i]),
                        static_cast<int>(data[i]),
                        static_cast<unsigned>(i));

        RED_CHECK_MESSAGE(expected_result[i] == data[i], buffer);
    }
}

template <class RDPOrderType>
void check(const RDPOrderCommon & common,
           const RDPOrderType & cmd,
           const RDPOrderCommon & expected_common,
           const RDPOrderType & expected_cmd,
           const char * message)
{
    char buffer[2048];
    size_t idx = 0;
    idx += snprintf(buffer + idx,   2048 - idx, "%s:\n", message);
    idx += snprintf(buffer + idx,   2048 - idx, "Expected ");
    idx += expected_cmd.str(buffer + idx, 2048 - idx, expected_common);
    idx += snprintf(buffer + idx,   2048 - idx, "\nGot ");
    idx += cmd.str(buffer + idx, 2048 - idx, common);
    buffer[2047] = 0;

    RED_CHECK_MESSAGE((expected_common == common) && (expected_cmd == cmd), buffer);
}

template <class RDPOrderType>
void check(const RDPOrderType & cmd, const RDPOrderType & expected_cmd,
           const char * message)
{
    char buffer[2048];
    size_t idx = 0;
    idx += snprintf(buffer + idx,   2048 - idx, "%s:\n", message);
    idx += snprintf(buffer + idx,   2048 - idx, "Expected ");
    idx += expected_cmd.str(buffer + idx, 2048 - idx);
    idx += snprintf(buffer + idx,   2048 - idx, "\nGot ");
    idx += cmd.str(buffer + idx, 2048 - idx);
    buffer[2047] = 0;

    RED_CHECK_MESSAGE(expected_cmd == cmd, buffer);
}
