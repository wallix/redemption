#define RED_TEST_MODULE TestAes

#include "utils/crypto/ssl_aes_direct.hpp"

using SslAes128_CBC = SslAes128_CBC_direct;
using SslAes192_CBC = SslAes192_CBC_direct;
using SslAes256_CBC = SslAes256_CBC_direct;

#include "../../system/common/test_ssl_aes.cpp"
