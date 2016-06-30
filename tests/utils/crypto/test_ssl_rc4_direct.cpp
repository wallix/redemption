#define BOOST_TEST_MODULE TestRc4

#include "utils/crypto/ssl_rc4_direct.hpp"

using SslRC4 = SslRC4_direct;

#include "system/common/test_ssl_rc4.cpp"
