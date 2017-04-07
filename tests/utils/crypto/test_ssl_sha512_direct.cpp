#define UNIT_TEST_MODULE TestSha512

#include "utils/crypto/ssl_sha512_direct.hpp"

using SslSha512 = SslSha512_direct;
using SslHMAC_Sha512 = SslHMAC_Sha512_direct;

#include "system/common/test_ssl_sha512.cpp"
