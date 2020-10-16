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

#include "utils/translation.hpp"
#include "utils/sugar/zstring_view.hpp"

#define EACH_ERROR(f, fv)                                    \
    fv(NO_ERROR, 0)                                          \
                                                             \
    f(ERR_STREAM_MEMORY_TOO_SMALL)                           \
    fv(ERR_BUFFER_TOO_SMALL, 6)                              \
                                                             \
    fv(ERR_UNEXPECTED, 8)                                    \
    f(ERR_DISCONNECT_BY_USER)                                \
    f(ERR_AUTOMATIC_RECONNECTION_REQUIRED)                   \
                                                             \
    fv(ERR_MEMORY_ALLOCATION_FAILED, 200)                    \
                                                             \
    fv(ERR_SOCKET_CONNECT_FAILED, 1000)                      \
                                                             \
    fv(ERR_TRANSPORT, 1500)                                  \
    f(ERR_TRANSPORT_NO_MORE_DATA)                            \
    f(ERR_TRANSPORT_WRITE_FAILED)                            \
    f(ERR_TRANSPORT_CLOSED)                                  \
    f(ERR_TRANSPORT_READ_FAILED)                             \
    f(ERR_TRANSPORT_INPUT_ONLY_USED_FOR_RECV)                \
    f(ERR_TRANSPORT_OUTPUT_ONLY_USED_FOR_SEND)               \
    f(ERR_TRANSPORT_DIFFERS)                                 \
    f(ERR_TRANSPORT_OPEN_FAILED)                             \
    f(ERR_TRANSPORT_SEEK_NOT_AVAILABLE)                      \
    f(ERR_TRANSPORT_SEEK_FAILED)                             \
                                                             \
    fv(ERR_TRANSPORT_WRITE_NO_ROOM, 1512)                    \
                                                             \
    fv(ERR_TRANSPORT_TLS_CERTIFICATE_CHANGED, 1601)          \
    f(ERR_TRANSPORT_TLS_CERTIFICATE_MISSED)                  \
    f(ERR_TRANSPORT_TLS_CERTIFICATE_CORRUPTED)               \
    f(ERR_TRANSPORT_TLS_CERTIFICATE_INACCESSIBLE)            \
    f(ERR_TRANSPORT_TLS_SERVER)                              \
    f(ERR_TRANSPORT_TLS_CERTIFICATE_INVALID)                 \
                                                             \
    fv(ERR_ACL_UNEXPECTED_IN_ITEM_OUT, 1700)                 \
    f(ERR_ACL_MESSAGE_TOO_BIG)                               \
                                                             \
    fv(ERR_X224, 3000)                                       \
                                                             \
    fv(ERR_X224_EXPECTED_DATA_PDU, 3002)                     \
                                                             \
    /*Disconnect Request - Transport Protocol Data Unit*/    \
    fv(ERR_X224_RECV_ID_IS_RD_TPDU, 3004)                    \
                                                             \
    fv(ERR_MCS, 5000)                                        \
    fv(ERR_MCS_APPID_IS_MCS_DPUM, 5003)                      \
    fv(ERR_MCS_DATA_SHORT_HEADER, 5026)                      \
    f(ERR_MCS_BAD_USERID)                                    \
    fv(ERR_MCS_PDU_TRUNCATED, 5029)                          \
    f(ERR_MCS_INFOPACKET_TRUNCATED)                          \
    f(ERR_MCS_PDU_TRAILINGDATA)                              \
    f(ERR_MCS_SYSTEM_TIME_TRUNCATED)                         \
                                                             \
    fv(ERR_GCC, 5500)                                        \
                                                             \
    fv(ERR_SEC, 6000)                                        \
    fv(ERR_SEC_PARSE_CRYPT_INFO_MOD_SIZE_NOT_OK, 6009)       \
    fv(ERR_SEC_PROCESS_LOGON_UNKNOWN_FLAGS, 6013)            \
    fv(ERR_SEC_EXPECTED_LOGON_INFO, 6018)                    \
    fv(ERR_SEC_EXPECTING_512_BITS_CLIENT_RANDOM, 6024)       \
    f(ERR_SEC_TRAILINGDATA)                                  \
                                                             \
    fv(ERR_LIC, 6500)                                        \
                                                             \
    fv(ERR_CHANNEL_UNKNOWN_CHANNEL, 7000)                    \
    fv(ERR_CHANNEL_OUT_OF_RANGE, 7003)                       \
                                                             \
    fv(ERR_RDP_PROCESS_POINTER_CACHE_NOT_OK, 8010)           \
    fv(ERR_RDP_PROCESS_NEW_POINTER_LEN_NOT_OK, 8012)         \
    fv(ERR_RDP_EXPECTING_CONFIRMACTIVEPDU, 8016)             \
    fv(ERR_RDP_RESIZE_NOT_AVAILABLE, 8019)                   \
    f(ERR_RDP_SLOWPATH)                                      \
    f(ERR_RDP_FASTPATH)                                      \
    f(ERR_RDP_DATA_TRUNCATED)                                \
    f(ERR_RDP_DATA_CHANNEL_FRAGMENTATION)                    \
    f(ERR_RDP_LOGON_USER_CHANGED)                            \
    fv(ERR_RDP_UNSUPPORTED, 8026)                            \
    f(ERR_RDP_PROTOCOL)                                      \
    f(ERR_RDP_SERVER_REDIR)                                  \
    f(ERR_RDP_OPEN_SESSION_TIMEOUT)                          \
    f(ERR_RDP_HANDSHAKE_TIMEOUT)                             \
    f(ERR_RDP_UNSUPPORTED_MONITOR_LAYOUT)                    \
    f(ERR_RDP_INTERNAL)                                      \
                                                             \
    fv(ERR_VNC, 10000)                                       \
    f(ERR_VNC_UNEXPECTED_ENCODING_IN_LIB_FRAME_BUFFER)       \
    fv(ERR_VNC_CONNECTION_ERROR, 10005)                      \
    fv(ERR_VNC_OLDER_RDP_CLIENT_CANT_RESIZE, 10007)          \
    f(ERR_VNC_ZLIB_INITIALIZATION)                           \
    fv(ERR_VNC_ZRLE_PROTOCOL, 10010)                         \
    f(ERR_VNC_HEXTILE_PROTOCOL)                              \
                                                             \
    fv(ERR_SESSION_UNKNOWN_BACKEND, 13000)                   \
                                                             \
    fv(ERR_RECORDER_FAILED_TO_OPEN_TARGET_FILE, 13006)       \
                                                             \
    fv(ERR_RECORDER_FAILED_TO_FOUND_PATH, 13013)             \
    f(ERR_RECORDER_ALLOCATION_FAILED)                        \
    fv(ERR_RECORDER_EMPTY_IMAGE, 13016)                      \
    fv(ERR_RECORDER_SNAPSHOT_FAILED, 13020)                  \
                                                             \
    fv(ERR_VIDEO_RECORDER, 16000)                            \
                                                             \
    fv(ERR_BITMAP_CACHE_TOO_BIG, 18001)                      \
    fv(ERR_BITMAP_DECOMPRESSED_DATA_TOO_LARGE, 18003)        \
    fv(ERR_BITMAP_CACHE_PERSISTENT, 18005)                   \
                                                             \
    fv(ERR_NEGO_SSL_REQUIRED_BY_SERVER, 19000)               \
    f(ERR_NEGO_SSL_NOT_ALLOWED_BY_SERVER)                    \
    f(ERR_NEGO_SSL_CERT_NOT_ON_SERVER)                       \
    f(ERR_NEGO_INCONSISTENT_FLAGS)                           \
    f(ERR_NEGO_HYBRID_REQUIRED_BY_SERVER)                    \
    f(ERR_NEGO_NLA_REQUIRED_BY_RESTRICTED_ADMIN_MODE)        \
                                                             \
    fv(ERR_WRM, 20000)                                       \
    fv(ERR_WRM_INVALID_INIT_CRYPT, 20012)                    \
                                                             \
    fv(ERR_WIDGET, 21000)                                    \
                                                             \
    fv(ERR_RDP61_DECOMPRESS, 22000)                          \
    f(ERR_RDP61_DECOMPRESS_DATA_TRUNCATED)                   \
                                                             \
    fv(ERR_RDP45_COMPRESS_BUFFER_OVERFLOW, 22100)            \
                                                             \
    fv(ERR_RAIL_PDU_TRUNCATED, 22200)                        \
    f(ERR_RAIL_LOGON_FAILED_OR_WARNING)                      \
    f(ERR_RAIL_NOT_ENABLED)                                  \
    f(ERR_RAIL_CLIENT_EXECUTE)                               \
    f(ERR_RAIL_STARTING_PROGRAM)                             \
    f(ERR_RAIL_UNAUTHORIZED_PROGRAM)                         \
    f(ERR_RAIL_NO_SUCH_WINDOW_EXIST)                         \
                                                             \
    fv(ERR_RDPDR_PDU_TRUNCATED, 22300)                       \
                                                             \
    fv(ERR_FSCC_DATA_TRUNCATED, 22400)                       \
                                                             \
    fv(ERR_NLA_AUTHENTICATION_FAILED, 23000)                 \
    fv(ERR_CREDSSP_KERBEROS_INIT_FAILED, 23001)              \
    f(ERR_CREDSSP_TS_REQUEST)                                \
                                                             \
    /* Persistent Disk Bitmap Cache. */                      \
    fv(ERR_PDBC_LOAD, 24100)                                 \
    f(ERR_PDBC_SAVE)                                         \
                                                             \
    fv(ERR_SESSION_PROBE_LAUNCH, 24200)                      \
    fv(ERR_SESSION_PROBE_DISCONNECTION_RECONNECTION, 24201)  \
    f(ERR_SESSION_PROBE_ASBL_FSVC_UNAVAILABLE)               \
    f(ERR_SESSION_PROBE_ASBL_MAYBE_SOMETHING_BLOCKS)         \
    f(ERR_SESSION_PROBE_ASBL_UNKNOWN_REASON)                 \
    f(ERR_SESSION_PROBE_CBBL_FSVC_UNAVAILABLE)               \
    f(ERR_SESSION_PROBE_CBBL_CBVC_UNAVAILABLE)               \
    f(ERR_SESSION_PROBE_CBBL_DRIVE_NOT_READY_YET)            \
    f(ERR_SESSION_PROBE_CBBL_MAYBE_SOMETHING_BLOCKS)         \
    f(ERR_SESSION_PROBE_CBBL_LAUNCH_CYCLE_INTERRUPTED)       \
    f(ERR_SESSION_PROBE_CBBL_UNKNOWN_REASON_REFER_TO_SYSLOG) \
    f(ERR_SESSION_PROBE_RP_LAUNCH_REFER_TO_SYSLOG)           \
                                                             \
    fv(ERR_SESSION_CLOSE_USER_INACTIVITY, 24500)             \
    f(ERR_SESSION_CLOSE_REJECTED_BY_ACL_MESSAGE)             \
    f(ERR_SESSION_CLOSE_ACL_KEEPALIVE_MISSED)                \
    f(ERR_SESSION_CLOSE_ENDDATE_REACHED)                     \
    f(ERR_SESSION_CLOSE_MODULE_NEXT)                         \
                                                             \
    fv(ERR_SSL_CALL_FAILED, 25000)                           \
    f(ERR_SSL_CALL_HMAC_INIT_FAILED)                         \
    f(ERR_SSL_CALL_HMAC_UPDATE_FAILED)                       \
    f(ERR_SSL_CALL_HMAC_FINAL_FAILED)                        \
    f(ERR_SSL_CALL_SHA1_INIT_FAILED)                         \
    f(ERR_SSL_CALL_SHA1_UPDATE_FAILED)                       \
    f(ERR_SSL_CALL_SHA1_FINAL_FAILED)                        \
    f(ERR_SSL_CALL_SHA256_INIT_FAILED)                       \
    f(ERR_SSL_CALL_SHA256_UPDATE_FAILED)                     \
    f(ERR_SSL_CALL_SHA256_FINAL_FAILED)                      \
                                                             \
    fv(ERR_CRYPTO_BUFFER_TOO_SMALL, 25500)                   \
    f(ERR_CRYPTO_SNAPPY_BUFFER_TOO_SMALL)                    \
    f(ERR_CRYPTO_SNAPPY_COMPRESSION_INVALID_INPUT)           \
                                                             \
    fv(ERR_BACK_EVENT_NEXT, 30000)


#define MAKE_ENUM(e) e,
#define MAKE_ENUM_V(e, x) e = x,
enum error_type
{
    EACH_ERROR(MAKE_ENUM, MAKE_ENUM_V)
};
#undef MAKE_ENUM
#undef MAKE_ENUM_V

inline const char * error_name(error_type id)
{
#define MAKE_NAME(e) case e: return "" #e "";
#define MAKE_NAME_V(e, x) case e: return "" #e "";
    switch (id){
        EACH_ERROR(MAKE_NAME, MAKE_NAME_V)
    }
#undef MAKE_NAME
#undef MAKE_NAME_V
    return "COMPILER_ERROR";
}


struct Error
{
    error_type id;
    int errnum;

    uintptr_t data;

public:
    Error() = delete;
    explicit Error(error_type id) noexcept;
    explicit Error(error_type id, int errnum) noexcept;
    explicit Error(error_type id, int errnum, uintptr_t data) noexcept;

    [[nodiscard]] zstring_view errmsg(bool with_id = true) const noexcept; /*NOLINT*/
};

zstring_view local_err_msg(const Error& error, Translation::language_t lang, bool with_id = true) noexcept; /*NOLINT*/

#ifndef NOT_UNDEF_EACH_ERROR
# undef EACH_ERROR
#endif
