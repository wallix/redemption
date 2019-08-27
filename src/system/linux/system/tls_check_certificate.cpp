/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan

   Transport layer abstraction, socket implementation with TLS support
*/

#include "core/error.hpp"
#include "system/tls_check_certificate.hpp"
#include "core/server_notifier_api.hpp"
#include "utils/file.hpp"
#include "utils/fileutils.hpp"
#include "utils/log.hpp"
#include "utils/sugar/algostring.hpp"

#include <string>
#include <memory>
#include <cstring>
#include <cstdlib>

#include <fcntl.h>

#include "cxx/diagnostic.hpp"

#include <openssl/ssl.h>
#include <openssl/x509.h>


REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")
REDEMPTION_DIAGNOSTIC_GCC_ONLY_IGNORE("-Wzero-as-null-pointer-constant")
#if REDEMPTION_COMP_CLANG_VERSION >= REDEMPTION_COMP_VERSION_NUMBER(5, 0, 0)
    REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wzero-as-null-pointer-constant")
#endif

namespace
{

std::unique_ptr<char[]> crypto_print_name(X509_NAME* name)
{
    std::unique_ptr<char[]> buffer;
    BIO* outBIO = BIO_new(BIO_s_mem());

    if (X509_NAME_print_ex(outBIO, name, 0, XN_FLAG_ONELINE) > 0)
    {
        unsigned long size = BIO_number_written(outBIO);
        buffer = std::make_unique<char[]>(size + 1);
        memset(buffer.get(), 0, size + 1);
        BIO_read(outBIO, buffer.get(), size);
    }
    BIO_free(outBIO);
    return buffer;
}

// TODO we should be able to simplify that to just put expected value in a provided buffer
inline std::unique_ptr<char[]> crypto_cert_fingerprint(X509 const* xcert)
{
    uint32_t fp_len;
    uint8_t fp[EVP_MAX_MD_SIZE];

    X509_digest(xcert, EVP_sha1(), fp, &fp_len);

    auto fp_buffer = std::make_unique<char[]>(3 * fp_len);
    memset(fp_buffer.get(), 0, 3 * fp_len);

    char * p = fp_buffer.get();

    int i = 0;
    for (i = 0; i < int(fp_len - 1); i++)
    {
        sprintf(p, "%02x:", unsigned(fp[i]));
        p = &fp_buffer[(i + 1) * 3];
    }
    sprintf(p, "%02x", unsigned(fp[i]));

    return fp_buffer;
}

} // anonymous namespace

[[nodiscard]] bool tls_check_certificate(
    X509& x509,
    bool server_cert_store,
    bool ensure_server_certificate_match,
    bool ensure_server_certificate_exists,
    ServerNotifier& server_notifier,
    const char* certif_path,
    std::string* error_message,
    const char* ip_address,
    int port)
{
    X509* px509 = &x509;

    // TODO("Before to have default value certificate doesn't exists")
    bool bad_certificate_path = false;
    error_type checking_exception = NO_ERROR;

    // ensures the certificate directory exists
        LOG(LOG_INFO, "certificate directory is: '%s'", certif_path);
        if (recursive_create_directory(certif_path, S_IRWXU|S_IRWXG, -1) != 0) {
        LOG(LOG_WARNING, "Failed to create certificate directory: %s ", certif_path);
        if (error_message) {
            str_assign(*error_message, "Failed to create certificate directory: \"", certif_path, "\"\n");
        }
        bad_certificate_path = true;

        server_notifier.server_cert_status(ServerNotifier::Status::CertError, strerror(errno));
        checking_exception = ERR_TRANSPORT_TLS_CERTIFICATE_INACCESSIBLE;
    }

    char filename[1024];

    // generates the name of certificate file associated with RDP target
    snprintf(filename, sizeof(filename) - 1, "%s/rdp,%s,%d,X509.pem",
        certif_path, ip_address, port);
    filename[sizeof(filename) - 1] = '\0';
    LOG(LOG_INFO, "certificate file is: '%s'", filename);

    bool certificate_exists  = false;
    bool certificate_matches = false;

    if (!bad_certificate_path) {
        File fp(filename, "r");
        if (!fp) {
            switch (errno) {
            default: {
                // failed to open stored certificate file
                LOG(LOG_WARNING, "Failed to open stored certificate: \"%s\"", filename);
                if (error_message) {
                    str_assign(*error_message, "Failed to open stored certificate: \"", filename, "\"\n");
                }
                server_notifier.server_cert_status(ServerNotifier::Status::CertError, strerror(errno));
                checking_exception = ERR_TRANSPORT_TLS_CERTIFICATE_INACCESSIBLE;
            }
            break;
            case ENOENT:
            {
                LOG(LOG_WARNING, "There's no stored certificate: \"%s\"", filename);
                if (error_message) {
                    str_assign(*error_message, "There's no stored certificate: \"", filename, "\"\n");
                }

                if (ensure_server_certificate_exists) {
                    server_notifier.server_cert_status(ServerNotifier::Status::CertFailure);
                    checking_exception = ERR_TRANSPORT_TLS_CERTIFICATE_MISSED;
                }
            }
            break;
            }
        } // fp
        else {
            certificate_exists  = true;
            X509 *px509Existing = PEM_read_X509(fp.get(), nullptr, nullptr, nullptr);
            fp.close();
            if (!px509Existing) {
                // failed to read stored certificate file
                LOG(LOG_WARNING, "Failed to read stored certificate: \"%s\"", filename);
                if (error_message) {
                    str_assign(*error_message, "Failed to read stored certificate: \"", filename, "\"\n");
                }
                certificate_matches = false;

                server_notifier.server_cert_status(ServerNotifier::Status::CertError, strerror(errno));
                checking_exception = ERR_TRANSPORT_TLS_CERTIFICATE_CORRUPTED;
            }
            else  {
                char tmpfilename[1024];
                // temp file for certificate binary check
                snprintf(tmpfilename, sizeof(tmpfilename) - 1, "/tmp/rdp,%s,%d,X509,XXXXXX", ip_address, port);
                tmpfilename[sizeof(tmpfilename) - 1] = 0;
                int tmpfd = ::mkostemp(tmpfilename, O_RDWR|O_CREAT);
                PEM_write_X509(File(::fdopen(tmpfd, "w+")).get(), px509);

                certificate_matches = file_equals(filename, tmpfilename);
                ::unlink(tmpfilename);

                const std::unique_ptr<char[]> issuer_existing      = crypto_print_name(X509_get_issuer_name(px509Existing));
                const std::unique_ptr<char[]> subject_existing     = crypto_print_name(X509_get_subject_name(px509Existing));
                const std::unique_ptr<char[]> fingerprint_existing = crypto_cert_fingerprint(px509Existing);

                LOG(LOG_INFO, "TLS::X509 existing::issuer=%s", issuer_existing.get());
                LOG(LOG_INFO, "TLS::X509 existing::subject=%s", subject_existing.get());
                LOG(LOG_INFO, "TLS::X509 existing::fingerprint=%s", fingerprint_existing.get());

                const std::unique_ptr<char[]> issuer               = crypto_print_name(X509_get_issuer_name(px509));
                const std::unique_ptr<char[]> subject              = crypto_print_name(X509_get_subject_name(px509));
                const std::unique_ptr<char[]> fingerprint          = crypto_cert_fingerprint(px509);

                if (!certificate_matches
                    // Read certificate fields to ensure change is not irrelevant
                    // Relevant changes are either:
                    // - issuer changed
                    // - subject changed (or appears, or disappears)
                    // - fingerprint changed
                    // other changes are ignored (expiration date for instance,
                    //  and revocation list is not checked)
                    && ((0 != strcmp(issuer_existing.get(), issuer.get()))
                    // Only one of subject_existing and subject is null
                    || ((!subject_existing || !subject) && (subject_existing != subject))
                    // All of subject_existing and subject are not null
                    || (subject && (0 != strcmp(subject_existing.get(), subject.get())))
                    || (0 != strcmp(fingerprint_existing.get(), fingerprint.get())))) {
                    if (error_message) {
                        char buff[256];
                        snprintf(buff, sizeof(buff), "The certificate for host %s:%d has changed!",
                                    ip_address, port);
                        *error_message = buff;
                    }
                    LOG(LOG_WARNING, "The certificate for host %s:%d has changed Previous=\"%s\" \"%s\" \"%s\", New=\"%s\" \"%s\" \"%s\"",
                        ip_address, port,
                        issuer_existing.get(), subject_existing.get(),
                        fingerprint_existing.get(), issuer.get(),
                        subject.get(), fingerprint.get());
                    if (error_message) {
                        str_assign(*error_message, "The certificate has changed: \"", filename, "\"\n");
                    }
                    certificate_exists  = true;
                    certificate_matches = false;

                    if (ensure_server_certificate_match) {
                        server_notifier.server_cert_status(ServerNotifier::Status::CertFailure);
                        checking_exception = ERR_TRANSPORT_TLS_CERTIFICATE_CHANGED;
                    }
                }
                else {
                    server_notifier.server_cert_status(ServerNotifier::Status::CertSuccess);
                }

                X509_free(px509Existing);
            }
        }
    }

    if ((certificate_exists || !ensure_server_certificate_exists)
    && (!certificate_exists || certificate_matches || !ensure_server_certificate_match))
    {
        if ((!certificate_exists || !certificate_matches) && server_cert_store) {
            ::unlink(filename);

            LOG(LOG_INFO, "Dumping X509 peer certificate: \"%s\"", filename);
            if (File fp{filename, "w+"}) {
                PEM_write_X509(fp.get(), px509);
                fp.close();
                LOG(LOG_INFO, "Dumped X509 peer certificate");
                server_notifier.server_cert_status(ServerNotifier::Status::CertCreate);
            }
            else {
                LOG(LOG_WARNING, "Failed to dump X509 peer certificate");
            }
        }

        server_notifier.server_cert_status(ServerNotifier::Status::AccessAllowed);

        // SSL_get_verify_result();

        // SSL_get_verify_result - get result of peer certificate verification
        // -------------------------------------------------------------------
        // SSL_get_verify_result() returns the result of the verification of the X509 certificate
        // presented by the peer, if any.

        // SSL_get_verify_result() can only return one error code while the verification of
        // a certificate can fail because of many reasons at the same time. Only the last
        // verification error that occurred during the processing is available from
        // SSL_get_verify_result().

        // The verification result is part of the established session and is restored when
        // a session is reused.

        // bug: If no peer certificate was presented, the returned result code is X509_V_OK.
        // This is because no verification error occurred, it does however not indicate
        // success. SSL_get_verify_result() is only useful in connection with SSL_get_peer_certificate(3).

        // RETURN VALUES The following return values can currently occur:

        // X509_V_OK: The verification succeeded or no peer certificate was presented.
        // Any other value: Documented in verify(1).


        //  A X.509 certificate is a structured grouping of information about an individual,
        // a device, or anything one can imagine. A X.509 CRL (certificate revocation list)
        // is a tool to help determine if a certificate is still valid. The exact definition
        // of those can be found in the X.509 document from ITU-T, or in RFC3280 from PKIX.
        // In OpenSSL, the type X509 is used to express such a certificate, and the type
        // X509_CRL is used to express a CRL.

        // A related structure is a certificate request, defined in PKCS#10 from RSA Security,
        // Inc, also reflected in RFC2896. In OpenSSL, the type X509_REQ is used to express
        // such a certificate request.

        // To handle some complex parts of a certificate, there are the types X509_NAME
        // (to express a certificate name), X509_ATTRIBUTE (to express a certificate attributes),
        // X509_EXTENSION (to express a certificate extension) and a few more.

        // Finally, there's the supertype X509_INFO, which can contain a CRL, a certificate
        // and a corresponding private key.

        X509* xcert = px509;
        X509_STORE* cert_ctx = X509_STORE_new();

        // OpenSSL_add_all_algorithms(3SSL)
        // --------------------------------

        // OpenSSL keeps an internal table of digest algorithms and ciphers. It uses this table
        // to lookup ciphers via functions such as EVP_get_cipher_byname().

        // OpenSSL_add_all_digests() adds all digest algorithms to the table.

        // OpenSSL_add_all_algorithms() adds all algorithms to the table (digests and ciphers).

        // OpenSSL_add_all_ciphers() adds all encryption algorithms to the table including password
        // based encryption algorithms.

        // EVP_cleanup() removes all ciphers and digests from the table.

        OpenSSL_add_all_algorithms();

        X509_LOOKUP* lookup = X509_STORE_add_lookup(cert_ctx, X509_LOOKUP_file());
        lookup = X509_STORE_add_lookup(cert_ctx, X509_LOOKUP_hash_dir());

        X509_LOOKUP_add_dir(lookup, nullptr, X509_FILETYPE_DEFAULT);
        //X509_LOOKUP_add_dir(lookup, certificate_store_path, X509_FILETYPE_ASN1);

        X509_STORE_CTX* csc = X509_STORE_CTX_new();
        X509_STORE_set_flags(cert_ctx, 0);
        X509_STORE_CTX_init(csc, cert_ctx, xcert, nullptr);
        X509_verify_cert(csc);
        X509_STORE_CTX_free(csc);

        X509_STORE_free(cert_ctx);

        // int index = X509_NAME_get_index_by_NID(subject_name, NID_commonName, -1);
        // X509_NAME_ENTRY *entry = X509_NAME_get_entry(subject_name, index);
        // ASN1_STRING * entry_data = X509_NAME_ENTRY_get_data(entry);
        // void * subject_alt_names = X509_get_ext_d2i(xcert, NID_subject_alt_name, 0, 0);

        X509_NAME * issuer_name = X509_get_issuer_name(xcert);
        LOG(LOG_INFO, "TLS::X509::issuer=%s", crypto_print_name(issuer_name).get());

        X509_NAME * subject_name = X509_get_subject_name(xcert);
        LOG(LOG_INFO, "TLS::X509::subject=%s", crypto_print_name(subject_name).get());

        LOG(LOG_INFO, "TLS::X509::fingerprint=%s", crypto_cert_fingerprint(xcert).get());
    }
    else {
        throw Error(checking_exception);
    }

    if (error_message) {
        error_message->clear();
    }

    return true;
}

REDEMPTION_DIAGNOSTIC_POP
