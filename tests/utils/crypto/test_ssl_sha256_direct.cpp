#define RED_TEST_MODULE TestSha256

#include "utils/crypto/ssl_sha256_direct.hpp"

using SslSha256 = SslSha256_direct;
using SslHMAC_Sha256 = SslHMAC_Sha256_direct;
using SslHMAC_Sha256_Delayed = SslHMAC_Sha256_Delayed_direct;

#include "../../system/common/test_ssl_sha256.hpp"
