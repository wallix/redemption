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
   Copyright (C) Wallix 2010-2013
   Author(s): Christophe Grosjean, Javier Caverni, Raphael Zhou,
              Jonathan Poelen

   Error exception object
*/


#pragma once

#include <cstdio>

#define EACH_ERROR(f, fv)                                        \
    fv(NO_ERROR, 0)                                              \
                                                                 \
    f(ERR_STREAM_MEMORY_TOO_SMALL)                               \
    f(ERR_STREAM_MEMORY_ALLOCATION_ERROR)                        \
    f(ERR_STREAM_VALUE_TOO_LARGE_FOR_OUT_BER_LEN_UINT7)          \
    f(ERR_BITMAP_CACHE_ITEM_MEMORY_ALLOCATION_FAILED)            \
    f(ERR_PATH_TOO_LONG)                                         \
    f(ERR_BUFFER_TOO_SMALL)                                      \
    f(ERR_NOT_YET_SUPPORTED)                                     \
    f(ERR_UNEXPECTED)                                            \
    f(ERR_DISCONNECT_BY_USER)                                    \
    f(ERR_AUTOMATIC_RECONNECTION_REQUIRED)                       \
                                                                 \
    fv(ERR_MEMORY_ALLOCATION_FAILED, 200)                        \
                                                                 \
    fv(ERR_SOCKET_CONNECT_FAILED, 1000)                          \
    f(ERR_SOCKET_GETHOSTBYNAME_FAILED)                           \
    f(ERR_SOCKET_ALLREADY_CLOSED)                                \
    f(ERR_SOCKET_CLOSED)                                         \
    f(ERR_SOCKET_ERROR)                                          \
                                                                 \
    fv(ERR_TRANSPORT, 1500)                                      \
    f(ERR_TRANSPORT_NO_MORE_DATA)                                \
    f(ERR_TRANSPORT_WRITE_FAILED)                                \
    f(ERR_TRANSPORT_CLOSED)                                      \
    f(ERR_TRANSPORT_READ_FAILED)                                 \
    f(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV)                    \
    f(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND)                   \
    f(ERR_TRANSPORT_DIFFERS)                                     \
    f(ERR_TRANSPORT_OPEN_FAILED)                                 \
    f(ERR_TRANSPORT_SEEK_NOT_AVAILABLE)                          \
    f(ERR_TRANSPORT_SEEK_FAILED)                                 \
    f(ERR_TRANSPORT_LOAD_CRYPTO_KEY)                             \
    f(ERR_TRANSPORT_WRITE_NO_ROOM)                               \
                                                                 \
    fv(ERR_TRANSPORT_TLS_CONNECT_FAILED, 1600)                   \
    f(ERR_TRANSPORT_TLS_CERTIFICATE_CHANGED)                     \
    f(ERR_TRANSPORT_TLS_CERTIFICATE_MISSED)                      \
    f(ERR_TRANSPORT_TLS_CERTIFICATE_CORRUPTED)                   \
    f(ERR_TRANSPORT_TLS_CERTIFICATE_INACCESSIBLE)                \
                                                                 \
    fv(ERR_ACL_UNEXPECTED_IN_ITEM_OUT, 1700)                     \
    f(ERR_ACL_MESSAGE_TOO_BIG)                                   \
                                                                 \
    fv(ERR_SESSION_TERMINATED, 2000)                             \
                                                                 \
    f(ERR_T123_EXPECTED_TPKT_VERSION_3)                          \
                                                                 \
    fv(ERR_X224, 3000)                                           \
    f(ERR_X224_SENDING_UNKNOWN_PDU_TYPE)                         \
    f(ERR_X224_EXPECTED_DATA_PDU)                                \
    f(ERR_X224_EXPECTED_CONNECTION_CONFIRM)                      \
    /*Disconnect Request - Transport Protocol Data Unit*/        \
    f(ERR_X224_RECV_ID_IS_RD_TPDU)                               \
                                                                 \
    fv(ERR_ISO_INCOMING_CODE_NOT_PDU_CR, 4000)                   \
    f(ERR_ISO_INCOMING_BAD_PDU_CR_LENGTH)                        \
    f(ERR_ISO_RECV_MSG_VER_NOT_3)                                \
    f(ERR_ISO_RECV_CODE_NOT_PDU_DT)                              \
    f(ERR_ISO_CONNECT_CODE_NOT_PDU_CC)                           \
                                                                 \
    fv(ERR_MCS, 5000)                                            \
    f(ERR_MCS_RECV_VER_NOT_3)                                    \
    f(ERR_MCS_RECV_ID_NOT_MCS_SDIN)                              \
    f(ERR_MCS_APPID_IS_MCS_DPUM)                                 \
    f(ERR_MCS_APPID_NOT_MCS_SDRQ)                                \
    f(ERR_MCS_BER_PARSE_HEADER_VAL_NOT_MATCH)                    \
    f(ERR_MCS_BER_PARSE_HEADER_ERROR_CHECKING_STREAM)            \
    f(ERR_MCS_BER_HEADER_UNEXPECTED_TAG)                         \
    f(ERR_MCS_BER_HEADER_TRUNCATED)                              \
    f(ERR_MCS_PARSE_DOMAIN_PARAMS_ERROR_CHECKING_STREAM)         \
    f(ERR_MCS_RECV_AUCF_OPCODE_NOT_OK)                           \
    f(ERR_MCS_RECV_AUCF_RES_NOT_0)                               \
    f(ERR_MCS_RECV_AUCF_ERROR_CHECKING_STREAM)                   \
    f(ERR_MCS_RECV_CJCF_OPCODE_NOT_CJCF)                         \
    f(ERR_MCS_RECV_CJCF_EMPTY)                                   \
    f(ERR_MCS_RECV_CJCF_ERROR_CHECKING_STREAM)                   \
    f(ERR_MCS_RECV_CONNECTION_REP_RES_NOT_0)                     \
    f(ERR_MCS_RECV_CONNECTION_REP_ERROR_CHECKING_STREAM)         \
    f(ERR_MCS_RECV_CONNECT_INITIAL_TRUNCATED)                    \
    f(ERR_MCS_RECV_EDQR_APPID_NOT_EDRQ)                          \
    f(ERR_MCS_RECV_EDQR_TRUNCATED)                               \
    f(ERR_MCS_RECV_AURQ_APPID_NOT_AURQ)                          \
    f(ERR_MCS_RECV_AURQ_TRUNCATED)                               \
    f(ERR_MCS_RECV_CJRQ_APPID_NOT_CJRQ)                          \
    f(ERR_MCS_RECV_CJRQ_TRUNCATED)                               \
    f(ERR_MCS_CHANNEL_NOT_FOUND)                                 \
    f(ERR_MCS_DATA_SHORT_HEADER)                                 \
    f(ERR_MCS_BAD_USERID)                                        \
    f(ERR_MCS_BAD_CHANID)                                        \
    f(ERR_MCS_PDU_TRUNCATED)                                     \
    f(ERR_MCS_INFOPACKET_TRUNCATED)                              \
    f(ERR_MCS_PDU_TRAILINGDATA)                                  \
    f(ERR_MCS_SYSTEM_TIME_TRUNCATED)                             \
                                                                 \
    fv(ERR_GCC, 5500)                                            \
                                                                 \
    fv(ERR_SEC, 6000)                                            \
    f(ERR_SEC_PARSE_PUB_KEY_MAGIC_NOT_OK)                        \
    f(ERR_SEC_PARSE_PUB_KEY_MODUL_NOT_OK)                        \
    f(ERR_SEC_PARSE_PUB_KEY_ERROR_CHECKING_STREAM)               \
    f(ERR_SEC_PARSE_CRYPT_INFO_CERT_NOK)                         \
    f(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NULL)                      \
    f(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NOT_LOADED)                \
    f(ERR_SEC_PARSE_CRYPT_INFO_CACERT_NOT_MATCH)                 \
    f(ERR_SEC_PARSE_CRYPT_INFO_X509_NOT_PARSED)                  \
    f(ERR_SEC_PARSE_CRYPT_INFO_MOD_SIZE_NOT_OK)                  \
    f(ERR_SEC_PARSE_CRYPT_INFO_RSA_EXP_NOT_OK)                   \
    f(ERR_SEC_PARSE_CRYPT_INFO_ERROR_CHECKING_STREAM)            \
    f(ERR_SEC_PARSE_PUB_SIG_LEN_NOT_OK)                          \
    f(ERR_SEC_PROCESS_LOGON_UNKNOWN_FLAGS)                       \
    f(ERR_SEC_READING_RSAKEYS_INI_FAILED)                        \
    f(ERR_SEC_UNEXPECTED_LICENSE_NEGOTIATION_PDU)                \
    f(ERR_SEC_EXPECTED_LICENSE_NEGOTIATION_PDU)                  \
    f(ERR_SEC_EXPECTING_CLIENT_RANDOM)                           \
    f(ERR_SEC_EXPECTED_LOGON_INFO)                               \
    f(ERR_SEC_EXPECTED_LICENSE_NEG)                              \
    f(ERR_SEC_PARSE_CRYPT_INFO_ENCRYPTION_REQUIRED)              \
    f(ERR_SEC_PARSE_CRYPT_INFO_BAD_RANDOM_LEN)                   \
    f(ERR_SEC_PARSE_CRYPT_INFO_BAD_RSA_LEN)                      \
    f(ERR_SEC_PARSE_CRYPT_INFO_UNIMPLEMENTED_TAG)                \
    f(ERR_SEC_PARSE_BAD_TAG_SIG_LEN)                             \
    f(ERR_SEC_EXPECTING_512_BITS_CLIENT_RANDOM)                  \
    f(ERR_SEC_TRAILINGDATA)                                      \
                                                                 \
    fv(ERR_LIC, 6500)                                            \
                                                                 \
    fv(ERR_CHANNEL_UNKNOWN_CHANNEL, 7000)                        \
    f(ERR_CHANNEL_SESSION_CALLBACK_NOT_INITIALIZED)              \
    f(ERR_CHANNEL_SESSION_CALLBACK_FAILED)                       \
    f(ERR_CHANNEL_OUT_OF_RANGE)                                  \
                                                                 \
    fv(ERR_RDP_RDP_INIT, 8000)                                   \
    f(ERR_RDP_CHAN_INIT)                                         \
    f(ERR_RDP_SEND_LOGIN_SEC_INIT_NOK)                           \
    f(ERR_RDP_SEND_INPUT_INIT_DATA_NOK)                          \
    f(ERR_RDP_SEND_INVALIDATE_INIT_DATA_NOK)                     \
    f(ERR_RDP_CHECK_FILE_INDEX_NOK)                              \
    f(ERR_RDP_SEND_REDIR_CHANNEL_NOT_INIT)                       \
    f(ERR_RDP_PROCESS_COLOR_POINTER_CACHE_NOT_OK)                \
    f(ERR_RDP_PROCESS_COLOR_POINTER_LEN_NOT_OK)                  \
    f(ERR_DEPRECATED1) /*ERR_RDP_PROCESS_POINTER_CACHE_LESS_0*/  \
    f(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK)                      \
    f(ERR_RDP_PROCESS_NEW_POINTER_CACHE_NOT_OK)                  \
    f(ERR_RDP_PROCESS_NEW_POINTER_LEN_NOT_OK)                    \
    f(ERR_RDP_SEND_CONFIRM_ACT_SEC_INIT_NOK)                     \
    f(ERR_RDP_REC_WRITE_ITEM_NOT_FD)                             \
    f(ERR_RDP_EXPECTING_DEMANDACTIVEPDU)                         \
    f(ERR_RDP_EXPECTING_CONFIRMACTIVEPDU)                        \
    f(ERR_RDP_UNEXPECTED_DEMANDACTIVEPDU)                        \
    f(ERR_RDP_UNEXPECTED_VIRTUAL_CHANNEL)                        \
    f(ERR_RDP_RESIZE_NOT_AVAILABLE)                              \
    f(ERR_RDP_SLOWPATH)                                          \
    f(ERR_RDP_FASTPATH)                                          \
    f(ERR_RDP_DATA_TRUNCATED)                                    \
    f(ERR_RDP_LOGON_USER_CHANGED)                                \
    f(ERR_RDP_LOGON_TIMER_EXPIRED)                               \
    f(ERR_RDP_UNSUPPORTED)                                       \
    f(ERR_RDP_PROTOCOL)                                          \
    f(ERR_RDP_SERVER_REDIR)                                      \
    f(ERR_RDP_OPEN_SESSION_TIMEOUT)                              \
    f(ERR_RDP_HANDSHAKE_TIMEOUT)                                 \
    f(ERR_RDP_UNSUPPORTED_MONITOR_LAYOUT)                        \
    f(ERR_RDP_INTERNAL)                                          \
                                                                 \
    fv(ERR_WM_PASSWORD, 9000)                                    \
    f(ERR_WM_USERNAME)                                           \
                                                                 \
    fv(ERR_VNC, 10000)                                           \
    f(ERR_VNC_UNEXPECTED_ENCODING_IN_LIB_FRAME_BUFFER)           \
    f(ERR_VNC_MOD_SIGNAL_UNKNOWN_TYPE)                           \
    f(ERR_VNC_BAD_BPP)                                           \
    f(ERR_VNC_NO_IP_SET)                                         \
    f(ERR_VNC_CONNECTION_ERROR)                                  \
    f(ERR_VNC_MEMORY_ALLOCATION_FAILED)                          \
    f(ERR_VNC_OLDER_RDP_CLIENT_CANT_RESIZE)                      \
    f(ERR_VNC_ZLIB_INITIALIZATION)                               \
    f(ERR_VNC_ZLIB_INFLATE)                                      \
    f(ERR_VNC_ZRLE_DATA_TRUNCATED)                               \
    f(ERR_VNC_ZRLE_PROTOCOL)                                     \
    f(ERR_VNC_NEED_MORE_DATA)                                    \
                                                                 \
    fv(ERR_XUP_BAD_BPP, 11000)                                   \
                                                                 \
    fv(ERR_SERVER_BEGIN_UPDATE, 12000)                           \
    f(ERR_SERVER_PALETTE)                                        \
    f(ERR_SERVER_PAINT_RECT)                                     \
    f(ERR_SERVER_SCREEN_BLT)                                     \
    f(ERR_SERVER_SET_POINTER)                                    \
    f(ERR_SERVER_SEND_TO_CHANNEL)                                \
    f(ERR_SERVER_END_UPDATE)                                     \
    f(ERR_SERVER_RDP_RECV_ERR)                                   \
                                                                 \
    fv(ERR_SESSION_UNKNOWN_BACKEND, 13000)                       \
                                                                 \
    fv(ERR_WAIT_OBJ_SOCKET, 14000)                               \
                                                                 \
    fv(ERR_ORDERS_FORCE_SEND_FAILED, 15000)                      \
                                                                 \
    fv(ERR_RECORDER_NO_OUTPUT_CODEC, 16000)                      \
    f(ERR_RECORDER_FAILED_ALLOCATING_OUTPUT_MEDIA_CONTEXT)       \
    f(ERR_RECORDER_FAILED_TO_OPEN_CODEC)                         \
    f(ERR_RECORDER_CODEC_NOT_FOUND)                              \
    f(ERR_RECORDER_FAILED_TO_ALLOCATE_PICTURE)                   \
    f(ERR_RECORDER_FAILED_TO_ALLOCATE_YUV420P_TEMPORARY_PICTURE) \
    f(ERR_RECORDER_INVALID_OUTPUT_FORMAT_PARAMETER)              \
    f(ERR_RECORDER_FAILED_TO_OPEN_TARGET_FILE)                   \
    f(ERR_RECORDER_FAILED_TO_ALLOC_STREAM)                       \
    f(ERR_RECORDER_FAILED_TO_ALLOCATE_PICTURE_BUF)               \
    f(ERR_RECORDER_FAILED_TO_ALLOCATE_FRAME)                     \
    f(ERR_RECORDER_FAILED_TO_ALLOCATE_FRAME_BUFFER)              \
    f(ERR_RECORDER_FAILED_TO_INITIALIZE_CONVERSION_CONTEXT)      \
    f(ERR_RECORDER_FAILED_TO_WRITE_RAW_FRAME)                    \
    f(ERR_RECORDER_FAILED_TO_WRITE_ENCODED_FRAME)                \
    f(ERR_RECORDER_FAILED_TO_FOUND_PATH)                         \
    f(ERR_RECORDER_ALLOCATION_FAILED)                            \
    f(ERR_RECORDER_FRAME_ALLOCATION_FAILED)                      \
    f(ERR_RECORDER_EMPTY_IMAGE)                                  \
    f(ERR_RECORDER_IMAGE_ALLOCATION_FAILED)                      \
    f(ERR_RECORDER_INIT_FAILED)                                  \
    f(ERR_RECORDER_FILE_CRYPTED)                                 \
                                                                 \
    f(ERR_RECORDER_SNAPSHOT_FAILED)                              \
    f(ERR_RECORDER_FAILED_TO_ENCODE_FRAME)                       \
                                                                 \
    fv(ERR_BITMAP_LOAD_FAILED, 17000)                            \
    f(ERR_BITMAP_LOAD_UNKNOWN_TYPE_FILE)                         \
    f(ERR_BITMAP_PNG_LOAD_FAILED)                                \
                                                                 \
    fv(ERR_BITMAP_CACHE, 18000)                                  \
    f(ERR_BITMAP_CACHE_TOO_BIG)                                  \
    f(ERR_BITMAP_UNSUPPORTED_COLOR_DEPTH)                        \
    f(ERR_BITMAP_DECOMPRESSED_DATA_TOO_LARGE)                    \
    f(ERR_BITMAP_8BIT_COLOR_DEPTH_MISSING_PALETTE)               \
    f(ERR_BITMAP_CACHE_PERSISTENT)                               \
                                                                 \
    fv(ERR_NEGO_SSL_REQUIRED_BY_SERVER, 19000)                   \
    f(ERR_NEGO_SSL_NOT_ALLOWED_BY_SERVER)                        \
    f(ERR_NEGO_SSL_CERT_NOT_ON_SERVER)                           \
    f(ERR_NEGO_INCONSISTENT_FLAGS)                               \
    f(ERR_NEGO_HYBRID_REQUIRED_BY_SERVER)                        \
                                                                 \
    fv(ERR_WRM, 20000)                                           \
    f(ERR_WRM_INVALID_FILE)                                      \
    f(ERR_NATIVE_CAPTURE_OPEN_FAILED)                            \
    f(ERR_NATIVE_CAPTURE_ZIP_COMPRESS)                           \
                                                                 \
    f(ERR_CIPHER_START)                                          \
    f(ERR_CIPHER_UPDATE)                                         \
                                                                 \
    f(ERR_RECORDER_META_REFERENCE_WRM)                           \
                                                                 \
    f(ERR_WRM_RECORDER_OPEN_FAILED)                              \
    f(ERR_WRM_RECORDER_ZIP_UNCOMPRESS)                           \
    f(ERR_WRM_CHUNK_META_NOT_FOUND)                              \
    f(ERR_WRM_INVALID_META_CHUNK)                                \
    f(ERR_WRM_IDX_NOT_FOUND)                                     \
    f(ERR_WRM_INVALID_INIT_CRYPT)                                \
    f(ERR_WRM_FAILED_OPENING_META_FILENAME)                      \
                                                                 \
    fv(ERR_WIDGET, 21000)                                        \
    f(ERR_WIDGET_INVALID_COMPOSITE_DESTROY)                      \
                                                                 \
    fv(ERR_RDP61_DECOMPRESS, 22000)                              \
    f(ERR_RDP61_DECOMPRESS_DATA_TRUNCATED)                       \
    f(ERR_RDP61_DECOMPRESS_LEVEL_2)                              \
                                                                 \
    fv(ERR_RDP45_COMPRESS_BUFFER_OVERFLOW, 22100)                \
                                                                 \
    fv(ERR_RAIL_PDU_TRUNCATED, 22200)                            \
    f(ERR_RAIL_LOGON_FAILED_OR_WARNING)                          \
    f(ERR_RAIL_NOT_ENABLED)                                      \
    f(ERR_RAIL_CLIENT_EXECUTE)                                   \
    f(ERR_RAIL_STARTING_PROGRAM)                                 \
    f(ERR_RAIL_UNAUTHORIZED_PROGRAM)                             \
                                                                 \
    fv(ERR_RDPDR_PDU_TRUNCATED, 22300)                           \
    f(ERR_RDPDR_READ_REQUEST)                                    \
                                                                 \
    fv(ERR_FSCC_DATA_TRUNCATED, 22400)                           \
                                                                 \
    fv(ERR_NLA_AUTHENTICATION_FAILED, 23000)                     \
                                                                 \
    fv(ERR_TRM_UNKNOWN_CHUNK_TYPE, 24000)                        \
    f(ERR_TRM_CHANNEL_NAME_TOO_LONG)                             \
                                                                 \
    /* Persistent Disk Bitmap Cache. */                          \
    fv(ERR_PDBC_LOAD, 24100)                                     \
    f(ERR_PDBC_SAVE)                                             \
                                                                 \
    fv(ERR_SESSION_PROBE_LAUNCH, 24200)                          \
    f(ERR_SESSION_PROBE_ENDING_IN_PROGRESS)                      \
    f(ERR_SESSION_PROBE_DISCONNECTION_RECONNECTION)              \
                                                                 \
    fv(ERR_CROPPER_INVALID_PARAMETER, 24500)                     \
                                                                 \
    fv(ERR_SSL_CALL_FAILED, 25000)                               \
    f(ERR_SSL_CALL_HMAC_INIT_FAILED)                             \
    f(ERR_SSL_CALL_HMAC_UPDATE_FAILED)                           \
    f(ERR_SSL_CALL_HMAC_FINAL_FAILED)                            \
    f(ERR_SSL_CALL_SHA1_INIT_FAILED)                             \
    f(ERR_SSL_CALL_SHA1_UPDATE_FAILED)                           \
    f(ERR_SSL_CALL_SHA1_FINAL_FAILED)                            \
    f(ERR_SSL_CALL_SHA256_INIT_FAILED)                           \
    f(ERR_SSL_CALL_SHA256_UPDATE_FAILED)                         \
    f(ERR_SSL_CALL_SHA256_FINAL_FAILED)                          \
                                                                 \
    fv(ERR_CRYPTO_BUFFER_TOO_SMALL, 25500)                       \
    f(ERR_CRYPTO_SNAPPY_BUFFER_TOO_SMALL)                        \
    f(ERR_CRYPTO_SNAPPY_COMPRESSION_INVALID_INPUT)               \
                                                                 \
    f(ERR_SSH_PARSE_PRIVATE_DSA_KEY)                             \
    f(ERR_SSH_PARSE_PRIVATE_RSA_KEY)


#define MAKE_ENUM(e) e,
#define MAKE_ENUM_V(e, x) e = x,
enum error_type
{
    EACH_ERROR(MAKE_ENUM, MAKE_ENUM_V)
};
#undef MAKE_ENUM
#undef MAKE_ENUM_V

struct Error {
    error_type id;
    int errnum;

private:
    mutable bool has_msg = false;
    mutable bool with_id = false;
    mutable char msg[64]  = { 0 };

    Error() = delete;

public:
    explicit Error(error_type id, int errnum = 0) noexcept
    : id(id)
    , errnum(errnum)
    {}

    const char * errmsg(bool with_id = true) const noexcept {
        switch(this->id) {
        case NO_ERROR:
            return "No error";
        case ERR_SESSION_UNKNOWN_BACKEND:
            return "Unknown Backend";
        case ERR_NLA_AUTHENTICATION_FAILED:
            return "NLA Authentication Failed";
        case ERR_TRANSPORT_OPEN_FAILED:
            return "Open file failed";
        case ERR_TRANSPORT_TLS_CERTIFICATE_CHANGED:
            return "TLS certificate changed";
        case ERR_TRANSPORT_TLS_CERTIFICATE_MISSED:
            return "TLS certificate missed";
        case ERR_TRANSPORT_TLS_CERTIFICATE_CORRUPTED:
            return "TLS certificate corrupted";
        case ERR_TRANSPORT_TLS_CERTIFICATE_INACCESSIBLE:
            return "TLS certificate is inaccessible";
        case ERR_VNC_CONNECTION_ERROR:
            return "VNC connection error.";
        case ERR_WIDGET_INVALID_COMPOSITE_DESTROY:
            return "Composite Widget Destroyed without child list not empty";

        case ERR_SESSION_PROBE_ENDING_IN_PROGRESS:
            return "Session logoff in progress";

        case ERR_RDP_UNSUPPORTED_MONITOR_LAYOUT:
            return "Unsupported client display monitor layout";

        case ERR_LIC:
            return "An error occurred during the licensing protocol";

        case ERR_RAIL_CLIENT_EXECUTE:
            return "The RemoteApp program did not start on the remote computer";

        case ERR_RAIL_STARTING_PROGRAM:
            return "Cannot start the RemoteApp program";

        case ERR_RAIL_UNAUTHORIZED_PROGRAM:
            return "The RemoteApp program is not in the list of authorized programs";

        case ERR_RDP_OPEN_SESSION_TIMEOUT:
            return "Logon timer expired";

        default:
            if (this->has_msg && this->with_id == with_id) {
                return this->msg;
            }
            char const * c_err = "Exception";
            switch (this->id) {
                #define MAKE_CASE(e) case e: c_err = "Exception " #e; break;
                #define MAKE_CASE_V(e, x) case e: c_err = "Exception " #e; break;
                EACH_ERROR(MAKE_CASE, MAKE_CASE_V)
                #undef MAKE_CASE
                #undef MAKE_CASE_V
            }
            if (with_id) {
                std::snprintf(this->msg, sizeof(this->msg), "%s no : %d", c_err, this->id);
            }
            else {
                std::snprintf(this->msg, sizeof(this->msg), "%s", c_err);
            }
            this->msg[sizeof(this->msg) - 1] = 0;
            this->has_msg = true;
            this->with_id = with_id;
            return this->msg;
        }
    }
};

#undef EACH_ERROR
