#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestVerifier
#include <boost/test/auto_unit_test.hpp>

#define LOGPRINT

#include <iostream>
#include "proto/proto.hpp"

namespace XXX {
    PROTO_VAR(proto::types::u8, a);
    PROTO_VAR(proto::types::u8, b);
    PROTO_VAR(proto::types::bytes, c);
    PROTO_VAR(proto::types::u16_le, d);
    PROTO_VAR(proto::types::str8_to_str16, e);
    PROTO_VAR(proto::types::u16_encoding, f);
    PROTO_VAR(proto::types::pkt_sz<proto::types::u8>, sz);
    PROTO_VAR(proto::types::pkt_sz_with_self<proto::types::u8>, sz2);

    constexpr auto desc = proto::desc(a, b, c, d, e, f, sz, sz2);
}

#include "core/RDP/sec.hpp"
#include "core/RDP/x224.hpp"

// https://github.com/jonathanpoelen/falcon.parse_number
#include <falcon/literals/integer_constant.hpp>
using namespace falcon::literals::integer_constant_literals;

namespace x224
{
    PROTO_VAR(proto::types::u8, version);
    PROTO_VAR(proto::types::u8, unknown);
    PROTO_VAR(proto::types::pkt_sz_with_self<proto::types::u16_be>, pkt_len);
    PROTO_VAR(proto::types::u8, LI);
    PROTO_VAR(proto::types::enum_u8<decltype(X224::DT_TPDU)>, type);
    PROTO_VAR(proto::types::enum_u8<decltype(X224::EOT_EOT)>, cat);

    constexpr auto dt_tpdu_send = proto::desc(
        version = 3_c,
        unknown = 0_c,
        pkt_len,
        LI = 2_c,
        type = X224::DT_TPDU,
        cat = X224::EOT_EOT
    );
}

namespace sec
{
    struct proto_signature
    {
        proto::types::mutable_bytes av;
        proto::types::value<CryptContext&> crypt;

        using sizeof_ = proto::size_<8>;

        auto static_serialize(uint8_t * p) const
        {
            auto & signature = reinterpret_cast<uint8_t(&)[proto_signature::sizeof_{}]>(*p);
            this->crypt.val.sign(this->av.av.data(), this->av.av.size(), signature);
            this->crypt.val.decrypt(const_cast<uint8_t*>(this->av.av.data()), this->av.av.size());
            return sizeof_{};
        }
    };

    inline std::ostream & operator <<(std::ostream & os, proto_signature const &)
    {
        return os << "proto_signature";
    }

    PROTO_VAR(proto::types::u32_le, flags);
    PROTO_VAR(proto::types::mutable_bytes, data);
    PROTO_VAR(proto::types::value<CryptContext&>, crypt);

    constexpr auto sec_send = proto::desc(
        proto::if_true(flags),
        proto::if_(proto::params[flags] & SEC::SEC_ENCRYPT)
            [proto::creater<proto_signature>(data, crypt)]
    );
}


void test_old();
void test_new();
void other_test();
void test();
void bench();

#include "proto/buffering_policy.hpp"
#include "proto/buffering2_policy.hpp"
#include "proto/buffering3_policy.hpp"

#include "utils/log.hpp" //hexdump_c

struct log_policy : buffering2_policy_base
{
    static void send(iovec_array iovs)
    {
        for (auto iov : iovs) {
            PROTO_TRACE(" [" << iov.iov_base << "] [len: " << iov.iov_len << "]\n");
            hexdump_c(static_cast<uint8_t const*>(iov.iov_base), iov.iov_len);
        }
    }
};

RED_AUTO_TEST_CASE(proto_test)
{
    struct {
        uint8_t a = 1;
        uint8_t b = 2;
        uint16_t d = 3;
        char const c[3] = "ab";
    } pkt;

    auto packet = XXX::desc(
        XXX::a = pkt.a,
        XXX::b = pkt.b,
        XXX::c = /*cstr_*/make_array_view(pkt.c),
        XXX::d = pkt.d,
        XXX::e = /*cstr_*/make_array_view(pkt.c),
        XXX::f = pkt.d/*,
//         XXX::sz,
        XXX::sz2
      , 1*/
    );

    packet.apply_for_each(Printer{});
    std::cout << "\n";
    packet.apply(Buffering{});
    std::cout << "\n";
    proto::apply(Buffering2<log_policy>{}, packet, packet);

    test();
    other_test();
    bench();
}


void test()
{
    std::cout << "\n\n======== old ========\n\n";
    test_old();
    std::cout << "\n\n======== new ========\n\n";
    test_new();
}


void test_old() {
    uint8_t data[10];
    CryptContext crypt;

    uint8_t buf[256];
    OutStream out_stream(buf + 126, 126);
    StaticOutStream<128> hstream;
    SEC::Sec_Send(out_stream, data, 10, ~SEC::SEC_ENCRYPT, crypt, 0);
    X224::DT_TPDU_Send(hstream, out_stream.get_offset());
    RED_REQUIRE_EQUAL(4, out_stream.get_offset());
    RED_REQUIRE_EQUAL(7, hstream.get_offset());
    auto p = out_stream.get_data() - hstream.get_offset();
    RED_REQUIRE_EQUAL(11, out_stream.get_current() - p);
    memcpy(p, hstream.get_data(), hstream.get_offset());
    out_stream = OutStream(p, out_stream.get_current() - p);
    out_stream.out_skip_bytes(out_stream.get_capacity());
    hexdump_c(out_stream.get_data(), out_stream.get_offset());
}

#include "utils/sugar/bytes_t.hpp"
inline bool check_range(const_bytes_array p, const_bytes_array mem, char * message)
{
    if (p.size() != mem.size() || memcmp(p.data(), mem.data(), p.size())) {
        if (auto len = p.size()) {
            auto sig = p.data();
            message += std::sprintf(message, "Expected signature: \"\\x%.2x", unsigned(*sig));
            while (--len) {
                message += std::sprintf(message, "\\x%.2x", unsigned(*++sig));
            }
            message[0] = '"';
            message[1] = 0;
        }
        message[0] = 0;
        return false;
    }
    return true;
}

#define CHECK_RANGE(p, mem)                      \
    {                                            \
        char message[1024*64];                   \
        if (!check_range(p, mem, message)) {     \
            RED_CHECK_MESSAGE(false, message); \
        }                                        \
    }


void test_new()
{
    auto packet1 = x224::dt_tpdu_send();

    uint8_t data[10];
    CryptContext crypt;
    auto packet2 = sec::sec_send(
        sec::flags = uint32_t(~SEC::SEC_ENCRYPT),
        sec::crypt = crypt,
        sec::data = data
    );

    struct Policy : log_policy {
        void send(iovec_array iovs) const {
            RED_CHECK_EQUAL(iovs.size(), 1);
            CHECK_RANGE(
                make_array_view(reinterpret_cast<uint8_t const *>(iovs[0].iov_base), iovs[0].iov_len),
                cstr_array_view("\x03\x00\x00\x0b\x02\xf0\x80\xf7\xff\xff\xff")
            );
            log_policy::send(iovs);
            this->used = true;
        }

        Policy(bool & used) : used(used) {}
        bool & used;
    };

    bool used = false;
    proto::apply(Buffering2<Policy>{used}, packet1, packet2);
    RED_CHECK(used);


    struct Policy2 : log_policy {
        void send(array_view_u8 av) const {
            CHECK_RANGE(av, cstr_array_view("\x03\x00\x00\x0b\x02\xf0\x80\xf7\xff\xff\xff"));
            iovec iov{av.data(), av.size()};
            log_policy::send(iovec_array{&iov, 1u});
            this->used = true;
        }
        Policy2(bool & used) : used(used) {}
        bool & used;
    };

    uint8_t buf[1024];
    used = false;
    proto::apply(Buffering3<Policy2>{{used}, {buf}}, packet1, packet2);
    RED_CHECK(used);
}

void other_test()
{
    PROTO_VAR(proto::types::u8, a);
    PROTO_VAR(proto::types::u8, b);
    constexpr auto bl = proto::desc(
        proto::if_(proto::params[a])
            [proto::composer(a, b)]
    );
    proto::apply(Buffering2<log_policy>{}, bl(a = 1_c, b = 1_c));
}

#include <chrono>

static void escape(void const * p) {
   asm volatile("" : : "g"(p) : "memory");
}

static void clobber() {
   asm volatile("" : : : "memory");
}



inline void test1(uint8_t * p, CryptContext & crypt, uint32_t c) {
    uint8_t data[10];
    auto packet1 = x224::dt_tpdu_send();
    auto packet2 = sec::sec_send(
        sec::flags = c/*uint32_t(~SEC::SEC_ENCRYPT)*/,
        sec::crypt = crypt,
        sec::data = data
    );
    struct Policy : buffering2_policy_base {
        void send(iovec_array iovs) const {
            //escape(data);
            for (auto iovec : iovs) {
                memcpy(p, iovec.iov_base, iovec.iov_len);
                p += iovec.iov_len;
            }
            //clobber();
        }
        Policy(uint8_t * p) : p (p) {}
        mutable uint8_t * p;
    };
    proto::apply(Buffering2<Policy>{p}, packet1, packet2);
}

inline void test2(uint8_t * p, CryptContext & crypt, uint32_t c) {
    uint8_t data[10];
    uint8_t buf[256];
    OutStream out_stream(buf + 126, 126);
    StaticOutStream<128> hstream;
//         escape(out_stream.get_data());
//         escape(hstream.get_data());
    SEC::Sec_Send(out_stream, data, 10, c, crypt, 0);
    X224::DT_TPDU_Send(hstream, out_stream.get_offset());
    auto bufp = out_stream.get_data() - hstream.get_offset();
    memcpy(bufp, hstream.get_data(), hstream.get_offset());
    out_stream = OutStream(bufp, out_stream.get_current() - bufp);
    out_stream.out_skip_bytes(out_stream.get_capacity());
    memcpy(p, out_stream.get_data(), out_stream.get_offset());
//         clobber();
//         clobber();
}

#include "openssl_tls.hpp"

void bench()
{
    SSL_load_error_strings();
    SSL_library_init();

    auto bench = [](auto test) {
        std::vector<long long> v;

        CryptContext crypt;
        crypt.encryptionMethod = 1;
        memcpy(crypt.key, "\xd1\x26\x9e\x63\xec\x51\x65\x1d\x89\x5c\x5a\x2a\x29\xef\x08\x4c", 16);
        memcpy(crypt.update_key, crypt.key, 16);

        crypt.rc4.set_key(crypt.key, (crypt.encryptionMethod==1)?8:16);

        srand(0);

        unsigned imax = 100;
        //unsigned imax = 500;
        for (unsigned i = 0; i < imax; ++i) {
            //alignas(4) uint8_t data[2621];
            alignas(4) uint8_t data[262144];
            //alignas(4) uint8_t data[1048576];
            auto p = data;
            test(p, crypt, 0);
            auto sz = 12;

            using resolution_clock = std::chrono::steady_clock; // std::chrono::high_resolution_clock;

            auto t1 = resolution_clock::now();

            uint32_t r = rand();

            while (static_cast<size_t>(p - data + sz) < sizeof(data)) {
                escape(p);
                test(p, crypt, r);
                clobber();
                p += sz;
            }

            auto t2 = resolution_clock::now();

            v.push_back((t2-t1).count()/imax);
        }
        return v;
    };

     auto v1 = bench(test1);
     auto v2 = bench(test2);

     std::sort(v1.begin(), v1.end());
     std::sort(v2.begin(), v2.end());

     long long sz = v1.size();
     auto pmin = std::max(sz/2-30, 0LL);
     auto pmax = std::min(sz/2+29, sz);
     v1 = decltype(v1)(std::begin(v1) + pmin, std::begin(v1) + pmax);
     v2 = decltype(v2)(std::begin(v2) + pmin, std::begin(v2) + pmax);

     std::cerr << "\n\ntest1\ttest2\n";
     auto it1 = v1.begin();
     for (auto t : v2) {
         std::cerr << *it1 << "\t" << t << "\n";
         ++it1;
     }
}
