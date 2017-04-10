#define UNIT_TEST_MODULE TestSha1

#include "utils/crypto/ssl_sha1_direct.hpp"

using SslSha1 = SslSha1_direct;
using SslHMAC_Sha1 = SslHMAC_Sha1_direct;

#include "system/common/test_ssl_sha1.cpp"
