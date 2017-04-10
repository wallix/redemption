#define RED_TEST_MODULE TestMd5

#include "utils/crypto/ssl_md5_direct.hpp"
#include "utils/crypto/ssl_md4_direct.hpp"
#include "utils/crypto/ssl_rc4_direct.hpp"

using SslMd5 = SslMd5_direct;
using SslHMAC_Md5 = SslHMAC_Md5_direct;

using SslMd4 = SslMd4_direct;
using SslHMAC_Md4 = SslHMAC_Md4_direct;

using SslRC4 = SslRC4_direct;

#include "system/common/test_ssl_md5.cpp"
