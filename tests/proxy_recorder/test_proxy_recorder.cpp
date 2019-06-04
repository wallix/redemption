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
   Copyright (C) Wallix 2019
   Author(s): Christophe Grosjean

*/

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/working_directory.hpp"

#include "test_only/transport/test_transport.hpp"
#include "proxy_recorder/proxy_recorder.hpp"
#include "proxy_recorder/nla_tee_transport.hpp"

struct xxhexdump
{
    const_bytes_view sig;

    std::size_t size() const
    {
        return this->sig.size();
    }

    uint8_t const * data() const
    {
        return this->sig.data();
    }
};

std::ostream & operator<<(std::ostream & out, xxhexdump const & x);
std::ostream & operator<<(std::ostream & out, xxhexdump const & x)
{
    char buffer[2048];
    for (size_t j = 0 ; j < x.size(); j += 16){
        char * line = buffer;
        line += std::sprintf(line, "/* %.4x */ \"", static_cast<unsigned>(j));
        size_t i = 0;
        for (i = 0; i < 16; i++){
            if (j+i >= x.size()){ break; }
            line += std::sprintf(line, "\\x%.2x", static_cast<unsigned>(x.data()[j+i]));
        }
        line += std::sprintf(line, "\"");
        if (i < 16){
            line += std::sprintf(line, "%s", &
                "                "
                "                "
                "                "
                "                "
                [i * 4u]);
        }
        line += std::sprintf(line, " // ");
        for (i = 0; i < 16; i++){
            if (j+i >= x.size()){ break; }
            unsigned char tmp = x.data()[j+i];
            if ((tmp < ' ') || (tmp > '~') || (tmp == '\\')){
                tmp = '.';
            }
            line += std::sprintf(line, "%c", tmp);
        }

        if (line != buffer){
            line[0] = 0;
            out << buffer << "\n";
            buffer[0]=0;
        }
    }
    return out;
}


struct ReplayBackTransport : public Transport
{

uint8_t data0[0x2E] = {
/* 0000 */ 0x03, 0x00, 0x00, 0x2e, 0x29, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x6f, 0x6f, 0x6b, 0x69,  // ....)......Cooki
/* 0010 */ 0x65, 0x3a, 0x20, 0x6d, 0x73, 0x74, 0x73, 0x68, 0x61, 0x73, 0x68, 0x3d, 0x71, 0x61, 0x61, 0x31,  // e: mstshash=qaa1
/* 0020 */ 0x36, 0x33, 0x38, 0x39, 0x0d, 0x0a, 0x01, 0x00, 0x08, 0x00, 0x02, 0x00, 0x00, 0x00,              // 6389..........
};

    size_t index = 0;
    struct Data {
        int type;
        size_t len;
        uint8_t * data;
    } datas[1] = {{1, sizeof(data0), data0}};

    ReplayBackTransport(){}
    ~ReplayBackTransport(){}

private:
    Read do_atomic_read(uint8_t * buffer, size_t len) override {
        LOG(LOG_INFO, "asked to back for reading %zu bytes into buffer", len);
        if (datas[index].type ==0 && len == datas[index].len) {
            memcpy(buffer, datas[index].data, datas[index].len);
            return Read::Ok;
        }
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }

    size_t do_partial_read(uint8_t* buffer, size_t len) override {
        LOG(LOG_INFO, "asked to back for reading %zu bytes into buffer", len);
        if (datas[index].type == 0 && len >= datas[index].len) {
        LOG(LOG_INFO, "asked to back for reading %zu bytes into buffer, got %zu", len, datas[index].len);
            memcpy(buffer, datas[index].data, datas[index].len);
            return datas[index++].len;
        }
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }

    void do_send(const uint8_t * const data, size_t len) override
    {
        LOG(LOG_INFO, "sending to back %zu bytes", len);
        if (this->datas[this->index].type == 1 && len == this->datas[this->index].len) {
            if (0 != memcmp(data, this->datas[this->index].data, this->datas[this->index].len)){
                std::cout << xxhexdump{{data, len}};
                std::cout << xxhexdump{{datas[index].data, len}};
            }
            else {
                std::cout << xxhexdump{{data, len}};
                this->index++;
                return;
            }
        }
        else {
            LOG(LOG_INFO, "sending to back %zu bytes, length mismatch %zu bytes", len, datas[index].len);
        }
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }
};

struct ReplayFrontTransport : public Transport
{

// Sent by RDP client
uint8_t data0[0x2E] = {
/* 0000 */ 0x03, 0x00, 0x00, 0x2e, 0x29, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43, 0x6f, 0x6f, 0x6b, 0x69,  // ....)......Cooki
/* 0010 */ 0x65, 0x3a, 0x20, 0x6d, 0x73, 0x74, 0x73, 0x68, 0x61, 0x73, 0x68, 0x3d, 0x71, 0x61, 0x61, 0x31,  // e: mstshash=qaa1
/* 0020 */ 0x36, 0x33, 0x38, 0x39, 0x0d, 0x0a, 0x01, 0x00, 0x08, 0x00, 0x0b, 0x00, 0x00, 0x00,
};

// Reply to front from recorder
uint8_t data0_reply[0x13] = {
/* 0000 */ 0x03, 0x00, 0x00, 0x13, 0x0e, 0xd0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x08, 0x00, 0x02,  // ................
/* 0010 */ 0x00, 0x00, 0x00,
};

// Sent by RDP Client NLA Nego
uint8_t data1[0x39] = {
/* 0000 */ 0x30, 0x37, 0xa0, 0x03, 0x02, 0x01, 0x06, 0xa1, 0x30, 0x30, 0x2e, 0x30, 0x2c, 0xa0, 0x2a, 0x04,  // 07......00.0,.*.
/* 0010 */ 0x28, 0x4e, 0x54, 0x4c, 0x4d, 0x53, 0x53, 0x50, 0x00, 0x01, 0x00, 0x00, 0x00, 0xb7, 0x82, 0x08,  // (NTLMSSP........
/* 0020 */ 0xe2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ................
/* 0030 */ 0x00, 0x0a, 0x00, 0xee, 0x42, 0x00, 0x00, 0x00, 0x0f,
};

uint8_t data1_reply[0x8B] = {
/* 0000 */ 0x30, 0x81, 0x88, 0xa0, 0x03, 0x02, 0x01, 0x06, 0xa1, 0x81, 0x80, 0x30, 0x7e, 0x30, 0x7c, 0xa0,  // 0..........0~0|.
/* 0010 */ 0x7a, 0x04, 0x78, 0x4e, 0x54, 0x4c, 0x4d, 0x53, 0x53, 0x50, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,  // z.xNTLMSSP......
/* 0020 */ 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0xb7, 0x82, 0x08, 0xe2, 0xff, 0xff, 0xff, 0xff, 0xff,  // ...8............
/* 0030 */ 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x40, 0x00, 0x38,  // ...........@.@.8
/* 0040 */ 0x00, 0x00, 0x00, 0x06, 0x01, 0xb1, 0x1d, 0x00, 0x00, 0x00, 0x0f, 0x01, 0x00, 0x08, 0x00, 0x57,  // ...............W
/* 0050 */ 0x00, 0x49, 0x00, 0x4e, 0x00, 0x37, 0x00, 0x02, 0x00, 0x08, 0x00, 0x57, 0x00, 0x49, 0x00, 0x4e,  // .I.N.7.....W.I.N
/* 0060 */ 0x00, 0x37, 0x00, 0x03, 0x00, 0x08, 0x00, 0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00, 0x04,  // .7.....w.i.n.7..
/* 0070 */ 0x00, 0x08, 0x00, 0x77, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x37, 0x00, 0x07, 0x00, 0x08, 0x00, 0x67,  // ...w.i.n.7.....g
/* 0080 */ 0x95, 0x0e, 0x5a, 0x4e, 0x56, 0x76, 0xd6, 0x00, 0x00, 0x00, 0x00,                                // ..ZNVv.....
};

uint8_t data2[0x144] = {
/* 0000 */ 0x30, 0x82, 0x01, 0x40, 0xa0, 0x03, 0x02, 0x01, 0x06, 0xa1, 0x81, 0xe0, 0x30, 0x81, 0xdd, 0x30,  // 0..@........0..0
/* 0010 */ 0x81, 0xda, 0xa0, 0x81, 0xd7, 0x04, 0x81, 0xd4, 0x4e, 0x54, 0x4c, 0x4d, 0x53, 0x53, 0x50, 0x00,  // ........NTLMSSP.
/* 0020 */ 0x03, 0x00, 0x00, 0x00, 0x18, 0x00, 0x18, 0x00, 0x94, 0x00, 0x00, 0x00, 0x18, 0x00, 0x18, 0x00,  // ................
/* 0030 */ 0xac, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x58, 0x00, 0x00, 0x00, 0x28, 0x00, 0x28, 0x00,  // ........X...(.(.
/* 0040 */ 0x58, 0x00, 0x00, 0x00, 0x14, 0x00, 0x14, 0x00, 0x80, 0x00, 0x00, 0x00, 0x10, 0x00, 0x10, 0x00,  // X...............
/* 0050 */ 0xc4, 0x00, 0x00, 0x00, 0x35, 0x82, 0x08, 0xe2, 0x0a, 0x00, 0xee, 0x42, 0x00, 0x00, 0x00, 0x0f,  // ....5......B....
/* 0060 */ 0x49, 0x2f, 0x0b, 0x18, 0x03, 0x8f, 0xa3, 0x8d, 0xbb, 0xa6, 0x79, 0xc4, 0xd9, 0x8e, 0xa1, 0x89,  // I/........y.....
/* 0070 */ 0x71, 0x00, 0x61, 0x00, 0x61, 0x00, 0x31, 0x00, 0x36, 0x00, 0x33, 0x00, 0x38, 0x00, 0x39, 0x00,  // q.a.a.1.6.3.8.9.
/* 0080 */ 0x40, 0x00, 0x6e, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x68, 0x00, 0x6d, 0x00, 0x6c, 0x00, 0x2e, 0x00,  // @.n.1...h.m.l...
/* 0090 */ 0x63, 0x00, 0x6e, 0x00, 0x61, 0x00, 0x76, 0x00, 0x5a, 0x00, 0x50, 0x00, 0x50, 0x00, 0x57, 0x00,  // c.n.a.v.Z.P.P.W.
/* 00a0 */ 0x31, 0x00, 0x32, 0x00, 0x30, 0x00, 0x37, 0x00, 0x32, 0x00, 0x38, 0x00, 0x8d, 0x27, 0x53, 0xcf,  // 1.2.0.7.2.8..'S.
/* 00b0 */ 0xa7, 0x26, 0x00, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // .&..............
/* 00c0 */ 0x00, 0x00, 0x00, 0x00, 0x3e, 0x44, 0xc8, 0x1d, 0x6b, 0xf7, 0x62, 0x77, 0xcf, 0x02, 0x38, 0xa4,  // ....>D..k.bw..8.
/* 00d0 */ 0x66, 0x69, 0x98, 0x4d, 0x88, 0x73, 0xaa, 0xf4, 0x51, 0xd3, 0x3a, 0x64, 0x9e, 0x3f, 0x8a, 0xe2,  // fi.M.s..Q.:d.?..
/* 00e0 */ 0xa0, 0x74, 0xee, 0x85, 0xcb, 0xd2, 0x4d, 0x2a, 0xe5, 0x39, 0xad, 0x60, 0xa3, 0x32, 0x04, 0x30,  // .t....M*.9.`.2.0
/* 00f0 */ 0x01, 0x00, 0x00, 0x00, 0x58, 0x33, 0xb2, 0xe5, 0x42, 0x26, 0x9d, 0x2d, 0x00, 0x00, 0x00, 0x00,  // ....X3..B&.-....
/* 0100 */ 0xa8, 0x35, 0x1b, 0x32, 0x58, 0x06, 0x79, 0x15, 0xf1, 0x1a, 0xda, 0xb6, 0x74, 0x68, 0xd2, 0x32,  // .5.2X.y.....th.2
/* 0110 */ 0x99, 0xe8, 0x3d, 0x50, 0xb3, 0xab, 0x1d, 0x56, 0x0b, 0x19, 0x16, 0x17, 0xf5, 0xa3, 0x12, 0x24,  // ..=P...V.......$
/* 0120 */ 0xa5, 0x22, 0x04, 0x20, 0x98, 0xd8, 0x2a, 0xd2, 0x4b, 0x95, 0x58, 0x4a, 0x67, 0x9e, 0x98, 0x2d,  // .". ..*.K.XJg..-
/* 0130 */ 0x19, 0x30, 0xdc, 0x56, 0x07, 0xd1, 0x0e, 0x73, 0xba, 0x55, 0x9b, 0x1a, 0x39, 0x5e, 0x23, 0xe6,  // .0.V...s.U..9^#.
/* 0140 */ 0xab, 0x85, 0x2f, 0x00,                                                                          // ../.
};

uint8_t data2_reply[1] = {0};

    size_t index = 0;
    struct Data {
        int type;
        size_t len;
        uint8_t * data;
    } datas[6] = {
        {0, sizeof(data0), data0},
        {1, sizeof(data0_reply), data0_reply},
        {0, sizeof(data1), data1},
        {1, sizeof(data1_reply), data1_reply},
        {0, sizeof(data2), data2},
        {1, sizeof(data2_reply), data2_reply}
    };

    ReplayFrontTransport(){}
    ~ReplayFrontTransport(){}

private:
    Read do_atomic_read(uint8_t * buffer, size_t len) override {
        LOG(LOG_INFO, "reading %zu bytes into buffer", len);
        if (datas[index].type ==0 && len == datas[index].len) {
            memcpy(buffer, datas[index].data, datas[index].len);
            return Read::Ok;
        }
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }

    size_t do_partial_read(uint8_t* buffer, size_t len) override {
        LOG(LOG_INFO, "asked to front for reading %zu bytes into buffer", len);
        if (datas[index].type == 0 && len >= datas[index].len) {
        LOG(LOG_INFO, "asked to front for reading %zu bytes into buffer, got %zu", len, datas[index].len);
            memcpy(buffer, datas[index].data, datas[index].len);
            return datas[index++].len;
        }
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }

    void do_send(const uint8_t * const data, size_t len) override
    {
        LOG(LOG_INFO, "sending to front %zu bytes", len);
        if (this->datas[this->index].type == 1 && len == this->datas[this->index].len) {
            if (0 != memcmp(data, this->datas[this->index].data, this->datas[this->index].len)){
                std::cout << xxhexdump{{data, len}};
                std::cout << xxhexdump{{datas[index].data, len}};
            }
            else {
                std::cout << xxhexdump{{data, len}};
                this->index++;
                return;
            }
        }
        else {
            LOG(LOG_INFO, "sending to front %zu bytes, length mismatch %zu bytes", len, datas[index].len);
        }
        throw Error(ERR_TRANSPORT_READ_FAILED);
    }
};

RED_AUTO_TEST_CASE_WF(TestNLAOnSiteCapture, wf)
{
    std::string nla_username("qaa16389@n1.hml.cnav");
    std::string nla_password("ZPPW120728");
    bool enable_kerberos = false;
    uint64_t verbosity = 4096;

    FrozenTime timeobj;
    RecorderFile outFile(timeobj, wf.c_str());

    ReplayBackTransport backConn;
    ReplayFrontTransport frontConn;
    NlaTeeTransport front_nla_tee_trans(frontConn, outFile, NlaTeeTransport::Type::Server);
    NlaTeeTransport back_nla_tee_trans(backConn, outFile, NlaTeeTransport::Type::Client);

    ProxyRecorder conn(back_nla_tee_trans, outFile, timeobj, "0.0.0.0", nla_username, nla_password, enable_kerberos, verbosity);

    uint8_t front_public_key[1024] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF};
    array_view_u8 front_public_key_av = {front_public_key, 16};

    // Receiving data from Client : x224
    conn.frontBuffer.load_data(frontConn);

    RED_REQUIRE(conn.frontBuffer.next(TpduBuffer::PDU));
    conn.front_step1(frontConn);
    hexdump(front_public_key, 16);
    // front public key ready from here
    conn.back_step1(front_public_key_av, backConn);

    // Receiving data from Client: CredSSP
    conn.frontBuffer.load_data(frontConn);
    conn.front_nla(frontConn);

    // // Receiving data from Client: CredSSP
    // conn.frontBuffer.load_data(frontConn);
    // conn.front_nla(frontConn);
}

// RED_AUTO_TEST_CASE(TestNLARedemptionToWindows2012DC)
// {
// }
//
// RED_AUTO_TEST_CASE(TestNLAWindows7ClientToRedemption)
// {
// }
