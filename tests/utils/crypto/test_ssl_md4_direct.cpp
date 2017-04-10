#define RED_TEST_MODULE TestMd4

#include "utils/crypto/ssl_md4_direct.hpp"

using SslMd4 = SslMd4_direct;
using SslHMAC_Md4 = SslHMAC_Md4_direct;

#include "system/common/test_ssl_md4.cpp"
