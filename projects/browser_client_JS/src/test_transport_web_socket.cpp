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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean
*/

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "transport/transport_web_socket.hpp"
#include "utils/parse.hpp"


TransportWebSocket trans(nullptr);


//  bjam -a test_transport_web_socket |& grep error || iceweasel file:///home/cmoroldo/Bureau/redemption/projects/browser_client_JS/sandbox/test_transport_web_socket.html



extern "C" void run_main() {

    uint8_t data[64] = { 0 };

    int len(64);

    if (data[0 ] == 0) {EM_ASM_({ console.log('init true'); }, 0);} else {EM_ASM_({ console.log('init false'); }, 0);}
    if (data[10] == 0) {EM_ASM_({ console.log('init true'); }, 0);} else {EM_ASM_({ console.log('init false'); }, 0);}
    if (data[20] == 0) {EM_ASM_({ console.log('init true'); }, 0);} else {EM_ASM_({ console.log('init false'); }, 0);}
    if (data[30] == 0) {EM_ASM_({ console.log('init true'); }, 0);} else {EM_ASM_({ console.log('init false'); }, 0);}
    if (data[40] == 0) {EM_ASM_({ console.log('init true'); }, 0);} else {EM_ASM_({ console.log('init false'); }, 0);}

    uint8_t ** pData = reinterpret_cast<uint8_t **>(&data);

    EM_ASM_({ getDataOctet(); }, 0);

    trans.recv(pData, len);

    if ((*pData)[0 ] == 03) {EM_ASM_({ console.log('recv true'); }, 0);} else {EM_ASM_({ console.log('recv false ' + $0); }, (*pData)[0 ]);}
    if ((*pData)[10] == 42) {EM_ASM_({ console.log('recv true'); }, 0);} else {EM_ASM_({ console.log('recv false ' + $0); }, (*pData)[10]);}
    if ((*pData)[20] == 42) {EM_ASM_({ console.log('recv true'); }, 0);} else {EM_ASM_({ console.log('recv false ' + $0); }, (*pData)[20]);}
    if ((*pData)[30] == 42) {EM_ASM_({ console.log('recv true'); }, 0);} else {EM_ASM_({ console.log('recv false ' + $0); }, (*pData)[30]);}
    if ((*pData)[40] == 42) {EM_ASM_({ console.log('recv true'); }, 0);} else {EM_ASM_({ console.log('recv false ' + $0); }, (*pData)[40]);}


    Parse parse(*pData);
    int x = parse.in_uint8();
    if (x == 03) {EM_ASM_({ console.log('Parse true'); }, 0);} else {EM_ASM_({ console.log('Parse false ' + $0); }, x);}
    x = parse.in_uint8();
    if (x == 00) {EM_ASM_({ console.log('Parse true'); }, 0);} else {EM_ASM_({ console.log('Parse false ' + $0); }, x);}
    x = parse.in_uint8();
    if (x == 00) {EM_ASM_({ console.log('Parse true'); }, 0);} else {EM_ASM_({ console.log('Parse false ' + $0); }, x);}
    x = parse.in_uint8();
    if (x == 64) {EM_ASM_({ console.log('Parse true'); }, 0);} else {EM_ASM_({ console.log('Parse false ' + $0); }, x);}
    x = parse.in_uint8();
    if (x == 42) {EM_ASM_({ console.log('Parse true'); }, 0);} else {EM_ASM_({ console.log('Parse false ' + $0); }, x);}

}




extern "C" void recv_value(int value) {
    trans.setBufferValue(value);
}