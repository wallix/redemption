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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

   Adapted from parts of the SSH Library
   Copyright (c) 2003-2009 by Aris Adamantiadis
 */

#include "utils/invalid_socket.hpp"

#pragma once

#if __GNUC__ >= 4 && !defined(__OS2__)
  #define LIBSSH_API __attribute__((visibility("default")))
#else
  #define LIBSSH_API
#endif

#include <inttypes.h>
#include <sys/stat.h> // for mode_t
#include <unistd.h>

#include <memory>
#include <string.h>
#include <memory.h>
#include <stdarg.h>

#include "utils/log.hpp"
#include "utils/invalid_socket.hpp"
#include "sashimi/string.hpp"

/* Error return codes */
enum {
    SSH_OK = 0,     /* No error */
    SSH_ERROR = -1,
    SSH_AGAIN = -2, /* The nonblocking call must be repeated */
    SSH_EOF = -127 /* We have already a eof */
};


enum ssh_error_types_e {
	SSH_NO_ERROR = 0,
	SSH_REQUEST_DENIED,
	SSH_FATAL,
	SSH_EINTR
};

/* error handling structure */
struct error_struct {
    int eid;
    int error_code;
    char error_buffer[1024];
};

extern "C"
{
    LIBSSH_API const char *ssh_get_error(struct error_struct * error);
    LIBSSH_API int ssh_get_error_code(struct error_struct * error);
    LIBSSH_API struct error_struct * ssh_new_error();
    LIBSSH_API void ssh_free_error(struct error_struct*);
}

#ifdef __GNUC__
__attribute__((format(printf, 3, 4)))
#endif
static inline void ssh_set_error(error_struct & error, int code, const char *descr, ...)
{
    va_list va;

    va_start(va, descr);
    vsnprintf(error.error_buffer, sizeof(error.error_buffer), descr, va);
    va_end(va);

    error.error_code = code;
    LOG(LOG_ERR, "%s", error.error_buffer);
}

#include "core/error.hpp"

#undef des_cbc_encrypt
#include "sashimi/libcrypto.hpp"


extern "C" {

/* some constants */
#ifndef MAX_PACKET_LEN
#define MAX_PACKET_LEN 262144
#endif
#ifndef KBDINT_MAX_PROMPT
#define KBDINT_MAX_PROMPT 256 /* more than openssh's :) */
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

 
/* Socket type */
#ifndef socket_t
typedef int socket_t;
#endif

typedef struct ssh_channel_struct * ssh_channel;

/**
 * @brief SSH authentication callback.
 *
 * @param prompt        Prompt to be displayed.
 * @param buf           Buffer to save the password. You should null-terminate it.
 * @param len           Length of the buffer.
 * @param echo          Enable or disable the echo of what you type.
 * @param verify        Should the password be verified?
 * @param userdata      Userdata to be passed to the callback function. Useful
 *                      for GUI applications.
 *
 * @return              0 on success, < 0 on error.
 */

/* Socket type */
#ifndef socket_t
typedef int socket_t;
#endif

typedef struct ssh_channel_struct * ssh_channel;

typedef int (*ssh_auth_callback) (const char *prompt, char *buf, size_t len, int echo, int verify, void *userdata);

typedef int (*ssh_event_callback)(socket_t fd, int revents, void *userdata);

/**
 * @brief SSH global request callback. All global request will go through this
 * callback.
 * @param session Current session handler
 * @param type type of global request
 * @param want_reply does it need a replay from other end ?
 * @param bind_address target address
 * @param bind_port target port
 * @param userdata Userdata to be passed to the callback function.
 */
typedef void (*ssh_global_request_callback) (struct ssh_session_struct * session,
            int type, int want_reply,
            char * bind_address, uint32_t bind_port,
            void *userdata);

/**
 * @brief Handles an SSH new channel open X11 request. This happens when the server
 * sends back an X11 connection attempt. This is a client-side API
 * @warning The channel pointer returned by this callback must be closed by the application.
 */
typedef ssh_channel (*ssh_channel_open_request_x11_callback) (ssh_session_struct * session,
      const char * originator_address, int originator_port, uint32_t sender, uint32_t window, uint32_t packet_size, void *userdata);

typedef ssh_channel (*ssh_channel_open_request_auth_agent_callback) (ssh_session_struct * session,
      void *userdata);


/** These are callbacks used specifically in SSH servers.
 */

/**
 * @brief SSH authentication callback.
 * @param session Current session handler
 * @param user User that wants to authenticate
 * @param password Password used for authentication
 * @param userdata Userdata to be passed to the callback function.
 * @returns SSH_AUTH_SUCCESS Authentication is accepted.
 * @returns SSH_AUTH_PARTIAL Partial authentication, more authentication means are needed.
 * @returns SSH_AUTH_DENIED Authentication failed.
 */
typedef int (*ssh_auth_password_callback) (ssh_session_struct * session, const char *user, const char *password, void *userdata);

/**
 * @brief SSH authentication callback. Tries to authenticates user with the "none" method
 * which is anonymous or passwordless.
 * @param user User that wants to authenticate
 * @param userdata Userdata to be passed to the callback function.
 * @returns SSH_AUTH_SUCCESS Authentication is accepted.
 * @returns SSH_AUTH_PARTIAL Partial authentication, more authentication means are needed.
 * @returns SSH_AUTH_DENIED Authentication failed.
 */
typedef int (*ssh_auth_none_callback) (const char *user, void *userdata);


/**
 * @brief SSH authentication callback. Tries to authenticates user with the "interactive" method
 * @param user User that wants to authenticate
 * @param userdata Userdata to be passed to the callback function.
 * @returns SSH_AUTH_SUCCESS Authentication is accepted.
 * @returns SSH_AUTH_PARTIAL Partial authentication, more authentication means are needed.
 * @returns SSH_AUTH_DENIED Authentication failed.
 */
    typedef int (*ssh_auth_interactive_callback) (const char *user, const char kbdint_response, void *userdata);


/**
 * @brief SSH authentication callback. Tries to authenticates user with the "gssapi-with-mic" method
 * @param user Username of the user (can be spoofed)
 * @param principal Authenticated principal of the user, including realm.
 * @param userdata Userdata to be passed to the callback function.
 * @returns SSH_AUTH_SUCCESS Authentication is accepted.
 * @returns SSH_AUTH_PARTIAL Partial authentication, more authentication means are needed.
 * @returns SSH_AUTH_DENIED Authentication failed.
 * @warning Implementations should verify that parameter user matches in some way the principal.
 * user and principal can be different. Only the latter is guaranteed to be safe.
 */
typedef int (*ssh_auth_gssapi_mic_callback) (const char *user, const char *principal, void *userdata);

/**
 * @brief SSH authentication callback.
 * @param user User that wants to authenticate
 * @param pubkey public key used for authentication
 * @param signature_state SSH_PUBLICKEY_STATE_NONE if the key is not signed (simple public key probe),
*                          SSH_PUBLICKEY_STATE_VALID if the signature is valid. Others values should be
 *                         replied with a SSH_AUTH_DENIED.
 * @param userdata Userdata to be passed to the callback function.
 * @returns SSH_AUTH_SUCCESS Authentication is accepted.
 * @returns SSH_AUTH_PARTIAL Partial authentication, more authentication means are needed.
 * @returns SSH_AUTH_DENIED Authentication failed.
 */
typedef int (*ssh_auth_pubkey_callback) (const char *user, struct ssh_key_struct *pubkey, char signature_state, void *userdata);


/**
 * @brief Handles an SSH service request
 * @param service name of the service (e.g. "ssh-userauth") requested
 * @param userdata Userdata to be passed to the callback function.
 * @returns 0  if the request is to be allowed
 *          -1 if the request should not be allowed
 */
typedef int (*ssh_service_request_callback) (const char *service, void *userdata);

/**
 * @brief Handles an SSH new channel open session request
 * @param userdata Userdata to be passed to the callback function.
 * @returns a valid ssh_channel handle if the request is to be allowed
 * @returns NULL if the request should not be allowed
 * @warning The channel pointer returned by this callback must be closed by the application.
 */
typedef ssh_channel (*ssh_channel_open_request_session_callback) (void *userdata);

/*
 * @brief Handles an SSH new channel open direct TCPIP request
 */
typedef ssh_channel (*ssh_channel_open_request_direct_tcpip_callback) (
            const char *destination, uint16_t destination_port,
            const char *originator, uint16_t originator_port,
            void *userdata);

/*
 * @brief Handles an SSH new channel open forwarded TCPIP request
 */

typedef ssh_channel (*ssh_channel_open_request_forwarded_tcpip_callback) (ssh_session_struct * session,
        const char *destination, uint16_t destination_port, const char *originator,
        uint16_t originator_port, void *userdata);

/**
 * This structure can be used to implement a libssh server, with appropriate callbacks.
 */

struct ssh_server_callbacks_struct {
  size_t size;
  /**
   * User-provided data. User is free to set anything he wants here
   */
  void *userdata;

  /**
   * This function gets called during connection time to indicate the
   * percentage of connection steps completed.
   */
  void (*connect_status_server_cb)(void *userdata, float status);
  /**
   * This function will be called each time a global request is received.
   */
  ssh_global_request_callback global_request_server_cb;

  /** This function gets called when a client tries to authenticate through
   * password method.
   */
  ssh_auth_password_callback auth_password_server_cb;

  /** This function gets called when a client tries to authenticate through
   * none method.
   */
  ssh_auth_none_callback auth_none_server_cb;

  /** This function gets called when a client tries to authenticate through
   * keyboard-interactive method.
   */
  ssh_auth_interactive_callback auth_interactive_server_cb;

  /** This function gets called when a client tries to authenticate through
   * gssapi-mic method.
   */
  ssh_auth_gssapi_mic_callback auth_gssapi_mic_server_cb;

  /** this function gets called when a client tries to authenticate or offer
   * a public key.
   */
  ssh_auth_pubkey_callback auth_pubkey_server_cb;

  /** This function gets called when a service request is issued by the
   * client
   */
  ssh_service_request_callback service_request_server_cb;

  /** This function gets called when a new channel request is issued by
   * the client
   */
  ssh_channel_open_request_session_callback channel_open_request_session_server_cb;

  /** This function gets called when a new direct tcpip channel request is issued by
   * the client
   */
  ssh_channel_open_request_direct_tcpip_callback channel_open_request_direct_tcpip_server_cb;

  /** This function gets called when a new forwarded tcpip channel request is issued by
   * the client
   */
  ssh_channel_open_request_forwarded_tcpip_callback channel_open_request_forwarded_tcpip_function;
};
typedef struct ssh_server_callbacks_struct *ssh_server_callbacks;


typedef int (*ssh_auth_none_reply_callback)(int status, void *userdata);

/**
 * The structure to replace libssh functions with appropriate callbacks.
 */
struct ssh_client_callbacks_struct {
  size_t size;
  /**
   * User-provided data. User is free to set anything he wants here
   */
  void *userdata;

  /**
   * This functions will be called when we'll get the reply
   from auth none request
   TODO: we will probably use the same reply
   */
  ssh_auth_none_reply_callback ssh_auth_none_reply_client_cb;

  /**
   * This functions will be called if e.g. a keyphrase is needed.
   */
  ssh_auth_callback auth_function;
  /**
   * This function gets called during connection time to indicate the
   * percentage of connection steps completed.
   */
  void (*connect_status_function)(void *userdata, float status);
  /**
   * This function will be called each time a global request is received.
   */
  ssh_global_request_callback global_request_function;
  /** This function will be called when an incoming X11 request is received.
   */
  ssh_channel_open_request_x11_callback channel_open_request_x11_function;

  /** This function will be called when an incoming auth_agent request is received.
   */
  ssh_channel_open_request_auth_agent_callback channel_open_request_auth_agent_function;

};
typedef struct ssh_client_callbacks_struct *ssh_client_callbacks;



struct ssh_kex_struct;

int ssh_get_key_params(ssh_session_struct * session, ssh_key_struct **privkey);

/* in base64.c */
struct ssh_buffer_struct * base64_to_bin(const char *source);

// SSH-TRANS constants
enum {
    SSH_MSG_DISCONNECT      = 1,
    SSH_MSG_IGNORE          = 2,
    SSH_MSG_UNIMPLEMENTED   = 3,
    SSH_MSG_DEBUG           = 4,
    SSH_MSG_SERVICE_REQUEST = 5,
    SSH_MSG_SERVICE_ACCEPT  = 6,

    SSH_MSG_KEXINIT         = 20,
    SSH_MSG_NEWKEYS         = 21,

    SSH_MSG_KEXDH_INIT      = 30,
    SSH_MSG_KEXDH_REPLY     = 31,
};

enum {
  SSH_MSG_KEX_ECDH_INIT  = 30,
  SSH_MSG_KEX_ECDH_REPLY = 31,
};

enum {
  SSH_MSG_ECMQV_INIT  = 30,
  SSH_MSG_ECMQV_REPLY = 31,
};


// secsh fh group exchange
enum {
    SSH_MSG_KEX_DH_GEX_REQUEST_OLD = 30,
    SSH_MSG_KEX_DH_GEX_REQUEST     = 34,
    SSH_MSG_KEX_DH_GEX_GROUP       = 31,
    SSH_MSG_KEX_DH_GEX_INIT        = 32,
    SSH_MSG_KEX_DH_GEX_REPLY       = 33,
};

// secsh-gsskeyex
enum {
    SSH_MSG_KEXGSS_INIT                       = 30,
    SSH_MSG_KEXGSS_CONTINUE                   = 31,
    SSH_MSG_KEXGSS_COMPLETE                   = 32,
    SSH_MSG_KEXGSS_HOSTKEY                    = 33,
    SSH_MSG_KEXGSS_ERROR                      = 34,
    SSH_MSG_KEXGSS_GROUPREQ                   = 40,
    SSH_MSG_KEXGSS_GROUP                      = 41,
    SSH_MSG_USERAUTH_GSSAPI_RESPONSE          = 60,
    SSH_MSG_USERAUTH_GSSAPI_TOKEN             = 61,
    SSH_MSG_USERAUTH_GSSAPI_EXCHANGE_COMPLETE = 63,
    SSH_MSG_USERAUTH_GSSAPI_ERROR             = 64,
    SSH_MSG_USERAUTH_GSSAPI_ERRTOK            = 65,
    SSH_MSG_USERAUTH_GSSAPI_MIC               = 66,
};


// rfc4256.txt
enum {
    SSH_MSG_USERAUTH_INFO_REQUEST          = 60,
    SSH_MSG_USERAUTH_INFO_RESPONSE         = 61,
};

enum {
  SSH_MSG_USERAUTH_PASSWD_CHANGEREQ   = 60,
};

enum {
  SSH_MSG_USERAUTH_PK_OK   = 60,
};

// SSH-USERAUTH constants
enum {
  SSH_MSG_USERAUTH_REQUEST = 50,
  SSH_MSG_USERAUTH_FAILURE = 51,
  SSH_MSG_USERAUTH_SUCCESS = 52,
  SSH_MSG_USERAUTH_BANNER  = 53,
};

// SSH-CONNECT constants
enum {
  SSH_MSG_GLOBAL_REQUEST               =  80,
  SSH_MSG_REQUEST_SUCCESS              =  81,
  SSH_MSG_REQUEST_FAILURE              =  82,
  SSH_MSG_CHANNEL_OPEN                 =  90,
  SSH_MSG_CHANNEL_OPEN_CONFIRMATION    =  91,
  SSH_MSG_CHANNEL_OPEN_FAILURE         =  92,
  SSH_MSG_CHANNEL_WINDOW_ADJUST        =  93,
  SSH_MSG_CHANNEL_DATA                 =  94,
  SSH_MSG_CHANNEL_EXTENDED_DATA        =  95,
  SSH_MSG_CHANNEL_EOF                  =  96,
  SSH_MSG_CHANNEL_CLOSE                =  97,
  SSH_MSG_CHANNEL_REQUEST              =  98,
  SSH_MSG_CHANNEL_SUCCESS              =  99,
  SSH_MSG_CHANNEL_FAILURE              = 100,
};

#define SSH2_DISCONNECT_HOST_NOT_ALLOWED_TO_CONNECT 1
#define SSH2_DISCONNECT_PROTOCOL_ERROR 2
#define SSH2_DISCONNECT_KEY_EXCHANGE_FAILED 3
#define SSH2_DISCONNECT_HOST_AUTHENTICATION_FAILED 4
#define SSH2_DISCONNECT_RESERVED	 4
#define SSH2_DISCONNECT_MAC_ERROR 5
#define SSH2_DISCONNECT_COMPRESSION_ERROR 6
#define SSH2_DISCONNECT_SERVICE_NOT_AVAILABLE 7
#define SSH2_DISCONNECT_PROTOCOL_VERSION_NOT_SUPPORTED 8
#define SSH2_DISCONNECT_HOST_KEY_NOT_VERIFIABLE 9
#define SSH2_DISCONNECT_CONNECTION_LOST 10
#define SSH2_DISCONNECT_BY_APPLICATION 11
#define SSH2_DISCONNECT_TOO_MANY_CONNECTIONS 12
#define SSH2_DISCONNECT_AUTH_CANCELLED_BY_USER 13
#define SSH2_DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE 14
#define SSH2_DISCONNECT_ILLEGAL_USER_NAME 15

#define SSH2_OPEN_ADMINISTRATIVELY_PROHIBITED		1
#define SSH2_OPEN_CONNECT_FAILED			2
#define SSH2_OPEN_UNKNOWN_CHANNEL_TYPE			3
#define SSH2_OPEN_RESOURCE_SHORTAGE			4

#define SSH2_EXTENDED_DATA_STDERR			1

enum key_type {
    RSAKEY = 0,
    DSAKEY = 1,
    ECDSAKEY = 2
};

typedef struct ssh_message_struct* ssh_message;
typedef struct ssh_pcap_file_struct* ssh_pcap_file;
//typedef struct ssh_session_struct* ssh_session;
typedef void* ssh_gssapi_creds;

#define SSH_SOCKET_FLOW_WRITEWILLBLOCK 1
#define SSH_SOCKET_FLOW_WRITEWONTBLOCK 2

#define SSH_SOCKET_EXCEPTION_EOF 	     1
#define SSH_SOCKET_EXCEPTION_ERROR     2

#define SSH_SOCKET_CONNECTED_OK 			1
#define SSH_SOCKET_CONNECTED_ERROR 		2
#define SSH_SOCKET_CONNECTED_TIMEOUT 	3

/** @brief Prototype for a packet callback, to be called when a new packet arrives
 * @param session The current session of the packet
 * @param type packet type (see ssh2.h)
 * @param packet buffer containing the packet, excluding size, type and padding fields
 * @param user user argument to the callback
 * and are called each time a packet shows up
 * @returns SSH_PACKET_USED Packet was parsed and used
 * @returns SSH_PACKET_NOT_USED Packet was not used or understood, processing must continue
 */
//typedef int (*ssh_packet_callback) (ssh_session_struct * session, uint8_t type, ssh_buffer_struct* packet, void *user);

/** return values for a ssh_packet_callback */
/** Packet was used and should not be parsed by another callback */
#define SSH_PACKET_USED 1
/** Packet was not used and should be passed to any other callback
 * available */
#define SSH_PACKET_NOT_USED 2


//struct ssh_packet_callbacks_struct {
//	/** Index of the first packet type being handled */
//	uint8_t start;
//	/** Number of packets being handled by this callback struct */
//	uint8_t n_callbacks;
//	/** A pointer to n_callbacks packet callbacks */
//	ssh_packet_callback *callbacks;
//  /**
//   * User-provided data. User is free to set anything he wants here
//   */
//	void *user;
//};

//typedef struct ssh_packet_callbacks_struct *ssh_packet_callbacks;

/**
 * @brief SSH channel data callback. Called when data is available on a channel
 * @param session Current session handler
 * @param channel the actual channel
 * @param data the data that has been read on the channel
 * @param len the length of the data
 * @param is_stderr is 0 for stdout or 1 for stderr
 * @param userdata Userdata to be passed to the callback function.
 * @returns number of bytes processed by the callee. The remaining bytes will
 * be sent in the next callback message, when more data is available.
 */
typedef int (*ssh_channel_data_callback) (ssh_session_struct * session,
                                           ssh_channel channel,
                                           void *data,
                                           uint32_t len,
                                           int is_stderr,
                                           void *userdata);

/**
 * @brief SSH channel eof callback. Called when a channel receives EOF
 * @param session Current session handler
 * @param channel the actual channel
 * @param userdata Userdata to be passed to the callback function.
 */
typedef void (*ssh_channel_eof_callback) (ssh_session_struct * session,
                                           ssh_channel channel,
                                           void *userdata);

/**
 * @brief SSH channel close callback. Called when a channel is closed by remote peer
 * @param session Current session handler
 * @param channel the actual channel
 * @param userdata Userdata to be passed to the callback function.
 */
typedef void (*ssh_channel_close_callback) (ssh_session_struct * session,
                                            ssh_channel channel,
                                            void *userdata);

/**
 * @brief SSH channel signal callback. Called when a channel has received a signal
 * @param session Current session handler
 * @param channel the actual channel
 * @param signal the signal name (without the SIG prefix)
 * @param userdata Userdata to be passed to the callback function.
 */
typedef void (*ssh_channel_signal_callback) (ssh_session_struct * session,
                                            ssh_channel channel,
                                            const char *signal,
                                            void *userdata);

/**
 * @brief SSH channel exit status callback. Called when a channel has received an exit status
 * @param session Current session handler
 * @param channel the actual channel
 * @param userdata Userdata to be passed to the callback function.
 */
typedef void (*ssh_channel_exit_status_callback) (ssh_session_struct * session,
                                            ssh_channel channel,
                                            int exit_status,
                                            void *userdata);

/**
 * @brief SSH channel exit signal callback. Called when a channel has received an exit signal
 * @param session Current session handler
 * @param channel the actual channel
 * @param signal the signal name (without the SIG prefix)
 * @param core a boolean telling wether a core has been dumped or not
 * @param errmsg the description of the exception
 * @param lang the language of the description (format: RFC 3066)
 * @param userdata Userdata to be passed to the callback function.
 */
typedef void (*ssh_channel_exit_signal_callback) (ssh_session_struct * session,
                                            ssh_channel channel,
                                            const char *signal,
                                            int core,
                                            const char *errmsg,
                                            const char *lang,
                                            void *userdata);

/**
 * @brief SSH channel PTY request from a client.
 * @param channel the channel
 * @param term The type of terminal emulation
 * @param width width of the terminal, in characters
 * @param height height of the terminal, in characters
 * @param pxwidth width of the terminal, in pixels
 * @param pxheight height of the terminal, in pixels
 * @param userdata Userdata to be passed to the callback function.
 * @returns 0 if the pty request is accepted
 * @returns -1 if the request is denied
 */
typedef int (*ssh_channel_pty_request_callback) (ssh_session_struct * session,
                                            ssh_channel channel,
                                            const char *term,
                                            int width, int height,
                                            int pxwidth, int pxheight,
                                            void *userdata);

/**
 * @brief SSH channel Shell request from a client.
 * @param channel the channel
 * @param userdata Userdata to be passed to the callback function.
 * @returns 0 if the shell request is accepted
 * @returns 1 if the request is denied
 */
typedef int (*ssh_channel_shell_request_callback) (ssh_session_struct * session,
                                            ssh_channel channel,
                                            void *userdata);
/**
 * @brief SSH auth-agent-request from the client. This request is
 * sent by a client when agent forwarding is available.
 * Server is free to ignore this callback, no answer is expected.
 * @param channel the channel
 * @param userdata Userdata to be passed to the callback function.
 */
typedef void (*ssh_channel_auth_agent_req_callback) (ssh_session_struct * session,
                                            ssh_channel channel,
                                            void *userdata);

/**
 * @brief SSH X11 request from the client. This request is
 * sent by a client when X11 forwarding is requested(and available).
 * Server is free to ignore this callback, no answer is expected.
 * @param channel the channel
 * @param userdata Userdata to be passed to the callback function.
 */
typedef void (*ssh_channel_x11_req_callback) (ssh_session_struct * session,
                                            ssh_channel channel,
                                            int single_connection,
                                            const char *auth_protocol,
                                            const char *auth_cookie,
                                            uint32_t screen_number,
                                            void *userdata);
/**
 * @brief SSH channel PTY windows change (terminal size) from a client.
 * @param channel the channel
 * @param width width of the terminal, in characters
 * @param height height of the terminal, in characters
 * @param pxwidth width of the terminal, in pixels
 * @param pxheight height of the terminal, in pixels
 * @param userdata Userdata to be passed to the callback function.
 * @returns 0 if the pty request is accepted
 * @returns -1 if the request is denied
 */
typedef int (*ssh_channel_pty_window_change_callback) (ssh_session_struct * session,
                                            ssh_channel channel,
                                            int width, int height,
                                            int pxwidth, int pxheight,
                                            void *userdata);

/**
 * @brief SSH channel Exec request from a client.
 * @param channel the channel
 * @param command the shell command to be executed
 * @param userdata Userdata to be passed to the callback function.
 * @returns 0 if the exec request is accepted
 * @returns 1 if the request is denied
 */
typedef int (*ssh_channel_exec_request_callback) (ssh_session_struct * session,
                                            ssh_channel channel,
                                            const char *command,
                                            void *userdata);

/**
 * @brief SSH channel environment request from a client.
 * @param channel the channel
 * @param env_name name of the environment value to be set
 * @param env_value value of the environment value to be set
 * @param userdata Userdata to be passed to the callback function.
 * @returns 0 if the env request is accepted
 * @returns 1 if the request is denied
 * @warning some environment variables can be dangerous if changed (e.g.
 * 			LD_PRELOAD) and should not be fulfilled.
 */
typedef int (*ssh_channel_env_request_callback) (ssh_session_struct * session,
                                            ssh_channel channel,
                                            const char *env_name,
                                            const char *env_value,
                                            void *userdata);
/**
 * @brief SSH channel subsystem request from a client.
 * @param channel the channel
 * @param subsystem the subsystem required
 * @param userdata Userdata to be passed to the callback function.
 * @returns 0 if the subsystem request is accepted
 * @returns 1 if the request is denied
 */
typedef int (*ssh_channel_subsystem_request_callback) (ssh_session_struct * session,
                                            ssh_channel channel,
                                            char * subsystem,
                                            void *userdata);


struct ssh_channel_callbacks_struct {
  size_t size;
  /**
   * User-provided data. User is free to set anything he wants here
   */
  void *userdata;
  /**
   * This functions will be called when there is data available.
   */
  ssh_channel_data_callback channel_data_function;
  /**
   * This functions will be called when the channel has received an EOF.
   */
  ssh_channel_eof_callback channel_eof_function;
  /**
   * This functions will be called when the channel has been closed by remote
   */
  ssh_channel_close_callback channel_close_function;
  /**
   * This functions will be called when a signal has been received
   */
  ssh_channel_signal_callback channel_signal_function;
  /**
   * This functions will be called when an exit status has been received
   */
  ssh_channel_exit_status_callback channel_exit_status_function;
  /**
   * This functions will be called when an exit signal has been received
   */
  ssh_channel_exit_signal_callback channel_exit_signal_function;
  /**
   * This function will be called when a client requests a PTY
   */
  ssh_channel_pty_request_callback channel_pty_request_function;
  /**
   * This function will be called when a client requests a shell
   */
  ssh_channel_shell_request_callback channel_shell_request_function;
  /** This function will be called when a client requests agent
   * authentication forwarding.
   */
  ssh_channel_auth_agent_req_callback channel_auth_agent_req_function;
  /** This function will be called when a client requests X11
   * forwarding.
   */
  ssh_channel_x11_req_callback channel_x11_req_function;
  /** This function will be called when a client requests a
   * window change.
   */
  ssh_channel_pty_window_change_callback channel_pty_window_change_function;
  /** This function will be called when a client requests a
   * command execution.
   */
  ssh_channel_exec_request_callback channel_exec_request_function;
  
  /** This function will be called when a client requests an environment
   * variable to be set.
   */
  ssh_channel_env_request_callback channel_env_request_function;

  /** This function will be called when a client requests a subsystem
   * (like sftp).
   * brief: SSH channel subsystem request from a client.
   * param: session the current session
   * param: channel the channel
   * param: subsystem the subsystem required
   * param: userdata Userdata to be passed to the callback function.
   * returns 0 if the subsystem request is accepted
   *          or 1 if the request is denied
  */
  int (*channel_subsystem_request_function)(ssh_session_struct * session,
                                            ssh_channel channel,
                                            char * subsystem,
                                            void *userdata);

  /** This function will be called when SSH client confirms opening of
   * a new X11 channel (or opening is rejected / has failed)
   */
  void (*channel_open_x11_server_status_function)(ssh_session_struct * session,
                                            ssh_channel channel,
                                            int status,
                                            void *userdata);
};

typedef struct ssh_channel_callbacks_struct *ssh_channel_callbacks;

/**
 * @brief Set the channel callback functions.
 *
 * This functions sets the callback structure to use your own callback
 * functions for channel data and exceptions
 *
 * @code
 * struct ssh_channel_callbacks_struct cb = {
 *   .userdata = data,
 *   .channel_data = my_channel_data_function
 * };
 * ssh_set_channel_callbacks(channel, &cb);
 * @endcode
 *
 * @param  channel      The channel to set the callback structure.
 *
 * @param  cb           The callback structure itself.
 *
 * @return SSH_OK on success, SSH_ERROR on error.
 */
LIBSSH_API int ssh_set_channel_callbacks(ssh_channel channel, ssh_channel_callbacks cb);



enum ssh_auth_e {
	SSH_AUTH_AGAIN=-2,
	SSH_AUTH_ERROR=-1,
	SSH_AUTH_SUCCESS=0,
	SSH_AUTH_DENIED,
	SSH_AUTH_PARTIAL,
	SSH_AUTH_INFO,
};

/* auth flags */
#define SSH_AUTH_METHOD_UNKNOWN 0
#define SSH_AUTH_METHOD_NONE 0x0001
#define SSH_AUTH_METHOD_PASSWORD 0x0002
#define SSH_AUTH_METHOD_PUBLICKEY 0x0004
#define SSH_AUTH_METHOD_HOSTBASED 0x0008
#define SSH_AUTH_METHOD_INTERACTIVE 0x0010
#define SSH_AUTH_METHOD_GSSAPI_MIC 0x0020

/* messages */
enum ssh_requests_e {
	SSH_REQUEST_AUTH=1,
	SSH_REQUEST_CHANNEL_OPEN,
	SSH_REQUEST_CHANNEL,
	SSH_REQUEST_SERVICE,
	SSH_REQUEST_GLOBAL
};

enum ssh_channel_type_e {
	SSH_CHANNEL_UNKNOWN=0,
	SSH_CHANNEL_SESSION,
	SSH_CHANNEL_DIRECT_TCPIP,
	SSH_CHANNEL_FORWARDED_TCPIP,
	SSH_CHANNEL_X11
};

enum ssh_channel_requests_e {
	SSH_CHANNEL_REQUEST_UNKNOWN=0,
	SSH_CHANNEL_REQUEST_PTY,
	SSH_CHANNEL_REQUEST_EXEC,
	SSH_CHANNEL_REQUEST_SHELL,
	SSH_CHANNEL_REQUEST_ENV,
	SSH_CHANNEL_REQUEST_SUBSYSTEM,
	SSH_CHANNEL_REQUEST_WINDOW_CHANGE,
	SSH_CHANNEL_REQUEST_X11
};

enum ssh_global_requests_e {
	SSH_GLOBAL_REQUEST_UNKNOWN=0,
	SSH_GLOBAL_REQUEST_TCPIP_FORWARD,
	SSH_GLOBAL_REQUEST_CANCEL_TCPIP_FORWARD,
};

enum ssh_publickey_state_e {
	SSH_PUBLICKEY_STATE_ERROR=-1,
	SSH_PUBLICKEY_STATE_NONE=0,
	SSH_PUBLICKEY_STATE_VALID=1,
	SSH_PUBLICKEY_STATE_WRONG=2
};

/* Status flags */
/** Socket is closed */
#define SSH_CLOSED 0x01
/** Reading to socket won't block */
#define SSH_READ_PENDING 0x02
/** Session was closed due to an error */
#define SSH_CLOSED_ERROR 0x04
/** Output buffer not empty */
#define SSH_WRITE_PENDING 0x08

enum ssh_server_known_e {
	SSH_SERVER_ERROR=-1,
	SSH_SERVER_NOT_KNOWN=0,
	SSH_SERVER_KNOWN_OK,
	SSH_SERVER_KNOWN_CHANGED,
	SSH_SERVER_FOUND_OTHER,
	SSH_SERVER_FILE_NOT_FOUND
};

/* some types for keys */
enum ssh_keytypes_e{
  SSH_KEYTYPE_UNKNOWN=0,
  SSH_KEYTYPE_DSS=1,
  SSH_KEYTYPE_RSA,
  SSH_KEYTYPE_RSA1,
  SSH_KEYTYPE_ECDSA
};

enum ssh_keycmp_e {
  SSH_KEY_CMP_PUBLIC = 0,
  SSH_KEY_CMP_PRIVATE
};



enum ssh_options_e {
  SSH_OPTIONS_HOST,
  SSH_OPTIONS_PORT,
  SSH_OPTIONS_PORT_STR,
  SSH_OPTIONS_FD,
  SSH_OPTIONS_USER,
  SSH_OPTIONS_SSH_DIR,
  SSH_OPTIONS_IDENTITY,
  SSH_OPTIONS_ADD_IDENTITY,
  SSH_OPTIONS_KNOWNHOSTS,
  SSH_OPTIONS_TIMEOUT,
  SSH_OPTIONS_TIMEOUT_USEC,
  SSH_OPTIONS_SSH1,
  SSH_OPTIONS_SSH2,
  SSH_OPTIONS_CIPHERS_C_S,
  SSH_OPTIONS_CIPHERS_S_C,
  SSH_OPTIONS_COMPRESSION_C_S,
  SSH_OPTIONS_COMPRESSION_S_C,
  SSH_OPTIONS_PROXYCOMMAND,
  SSH_OPTIONS_BINDADDR,
  SSH_OPTIONS_STRICTHOSTKEYCHECK,
  SSH_OPTIONS_COMPRESSION,
  SSH_OPTIONS_COMPRESSION_LEVEL,
  SSH_OPTIONS_KEY_EXCHANGE,
  SSH_OPTIONS_HOSTKEYS,
  SSH_OPTIONS_GSSAPI_SERVER_IDENTITY,
  SSH_OPTIONS_GSSAPI_CLIENT_IDENTITY,
  SSH_OPTIONS_GSSAPI_DELEGATE_CREDENTIALS,
};

// Functions called by ssh client code
// ===================================

LIBSSH_API ssh_session_struct * ssh_new_client_session(ssh_client_callbacks cb, struct ssh_poll_ctx_struct * ctx, char * host, char * port, char * user, char * hostkeys, char * verbosity, error_struct * error);

LIBSSH_API int ssh_channel_close_client(ssh_session_struct * session, ssh_channel channel);
LIBSSH_API void ssh_channel_free_client(ssh_session_struct * session, ssh_channel channel);
LIBSSH_API int ssh_channel_open_forward_client(ssh_session_struct * session, ssh_channel channel, const char *remotehost, int remoteport, const char *sourcehost, int localport);
LIBSSH_API int ssh_sessionchannel_open_client(ssh_session_struct * session, ssh_channel channel);
LIBSSH_API int ssh_channel_send_eof_client(ssh_session_struct * session, ssh_channel channel);
LIBSSH_API void ssh_disconnect_client(ssh_session_struct * session);
/* USERAUTH */
LIBSSH_API int ssh_userauth_none_client(ssh_session_struct * session, const char *username, error_struct * error);
LIBSSH_API int ssh_userauth_list_client(ssh_session_struct * session, const char *username, error_struct * error);
LIBSSH_API int ssh_userauth_agent_client(ssh_session_struct * session, ssh_session_struct * front_session, const char *username, error_struct * error);
LIBSSH_API int ssh_userauth_password_client(ssh_session_struct * session,
                                     const char *username,
                                     const char *password, error_struct * error);
LIBSSH_API int ssh_userauth_kbdint_client(ssh_session_struct * session, const char *user, const char *submethods, error_struct * error);
LIBSSH_API int ssh_userauth_kbdint_getnprompts_client(ssh_session_struct * session);
LIBSSH_API const char *ssh_userauth_kbdint_getprompt_client(ssh_session_struct * session, unsigned int i, char *echo, error_struct * error);
LIBSSH_API int ssh_userauth_kbdint_setanswer_client(ssh_session_struct * session, unsigned int i,
    const char *answer, error_struct * error);
LIBSSH_API int ssh_userauth_gssapi_client(ssh_session_struct * session, error_struct * error);
LIBSSH_API int ssh_event_set_fd_client(ssh_poll_ctx_struct * ctx, socket_t fd, ssh_event_callback cb, void *userdata);
LIBSSH_API void ssh_event_set_session_client(ssh_poll_ctx_struct * ctx, ssh_session_struct * session);
LIBSSH_API int ssh_channel_change_pty_size_client(ssh_session_struct * client_session, ssh_channel channel, int cols, int rows);
LIBSSH_API void ssh_event_x11_requested_channel_client(ssh_session_struct * client_session, ssh_channel channel, uint32_t sender, uint32_t window, uint32_t packet_size);
LIBSSH_API void ssh_event_x11_requested_channel_failure_client(ssh_session_struct * client_session, uint32_t sender);
LIBSSH_API int ssh_channel_request_env_client(ssh_session_struct * session, ssh_channel channel, const char *name, const char *value);
LIBSSH_API int ssh_channel_request_send_signal_client(ssh_session_struct * session, ssh_channel channel, const char *signum);
LIBSSH_API int ssh_channel_request_x11_client(ssh_session_struct * client_session, ssh_channel channel, int single_connection, const char *protocol, const char *cookie, int screen_number);

enum ssh_publickey_hash_type {
    SSH_PUBLICKEY_HASH_SHA1,
    SSH_PUBLICKEY_HASH_MD5
};

LIBSSH_API int ssh_get_server_publickey_hash_value_client(const ssh_session_struct * session,
                                                   enum ssh_publickey_hash_type type,
                                                   unsigned char *buf,
                                                   size_t *hlen, error_struct * error);
LIBSSH_API void ssh_gssapi_set_creds_client(ssh_session_struct * session, const ssh_gssapi_creds creds);
LIBSSH_API int ssh_set_agent_channel_client(ssh_session_struct * session, ssh_channel channel);

LIBSSH_API int ssh_channel_write_client(ssh_session_struct * session, ssh_channel channel, const uint8_t *data, uint32_t len);

LIBSSH_API int ssh_channel_write_stderr_client(ssh_session_struct * session, ssh_channel channel, const uint8_t *data, uint32_t len);

LIBSSH_API int ssh_channel_request_exec_client(ssh_session_struct * session, ssh_channel channel, const char *cmd);

LIBSSH_API int ssh_channel_request_subsystem_client(ssh_session_struct * session, ssh_channel channel, const char *subsystem);

LIBSSH_API int ssh_channel_request_pty_size_client(ssh_session_struct * session, ssh_channel channel, const char *term, int cols, int rows);

LIBSSH_API int ssh_channel_request_shell_client(ssh_session_struct * session, ssh_channel channel);
LIBSSH_API int ssh_channel_is_eof_client(ssh_session_struct * session, ssh_channel channel);

LIBSSH_API int ssh_channel_request_auth_agent_client(ssh_session_struct * session, ssh_channel channel);

LIBSSH_API int ssh_channel_is_closed_client(ssh_session_struct * session, ssh_channel channel);

LIBSSH_API int ssh_channel_is_open_client(ssh_session_struct * session, ssh_channel channel);



// Functions called by ssh server code
// ===================================

LIBSSH_API ssh_session_struct * ssh_start_new_server_session(ssh_server_callbacks cb_server,
                                            struct ssh_poll_ctx_struct * ctx,
                                            int fd,
                                            const char * filename, int authmethods);
LIBSSH_API int ssh_channel_close_server(ssh_session_struct * session, ssh_channel channel);
LIBSSH_API void ssh_channel_free_server(ssh_session_struct * session, ssh_channel channel);
LIBSSH_API int ssh_channel_send_eof_server(ssh_session_struct * session, ssh_channel channel);
LIBSSH_API void ssh_disconnect_server(ssh_session_struct * session);
LIBSSH_API int ssh_userauth_kbdint_getnanswers_server(ssh_session_struct * session, error_struct * error);
LIBSSH_API const char *ssh_userauth_kbdint_getanswer_server(ssh_session_struct * session, unsigned int i, error_struct * error);

LIBSSH_API int ssh_userauth_kbdint_settmpprompts_server(ssh_session_struct * session, const char * name,
                                                const char * instruction,
                                                unsigned int num_prompt,
                                                const char ** prompts,
                                                unsigned char * echo);
LIBSSH_API int ssh_userauth_kbdint_settmpprompt_server(ssh_session_struct * session, const char * name,
                                               const char * instruction,
                                               const char * prompt, unsigned char echo, error_struct * error);
LIBSSH_API void ssh_event_set_session_server(ssh_poll_ctx_struct * ctx, ssh_session_struct * session);
LIBSSH_API void ssh_channel_open_x11_server(ssh_session_struct * session, ssh_channel channel, const char *orig_addr, int orig_port);

LIBSSH_API int ssh_channel_write_server(ssh_session_struct * session, ssh_channel channel, const uint8_t *data, uint32_t len);

LIBSSH_API int ssh_channel_write_stderr_server(ssh_session_struct * session, ssh_channel channel, const uint8_t *data, uint32_t len);

LIBSSH_API int ssh_channel_is_open_server(ssh_session_struct * session, ssh_channel channel);
LIBSSH_API int ssh_forward_cancel_server(ssh_session_struct * session, const char *address, int port);
LIBSSH_API int ssh_forward_listen_server(ssh_session_struct * session, const char *address, int port, int *bound_port);
LIBSSH_API int ssh_channel_is_closed_server(ssh_session_struct * session, ssh_channel channel);
LIBSSH_API int ssh_channel_is_eof_server(ssh_session_struct * session, ssh_channel channel);
LIBSSH_API int ssh_channel_open_reverse_forward_server(ssh_session_struct * session, ssh_channel channel, const char *remotehost, int remoteport, const char *sourcehost, int localport);

LIBSSH_API int ssh_channel_open_auth_agent_server(ssh_session_struct * session, ssh_channel channel);
LIBSSH_API int ssh_channel_request_send_exit_status_server(ssh_session_struct * session, ssh_channel channel, int exit_status);
LIBSSH_API int ssh_channel_request_send_exit_signal_server(ssh_session_struct * session, ssh_channel channel, const char *signum, int core, const char *errmsg, const char *lang);
LIBSSH_API int ssh_set_auth_methods_server(ssh_session_struct *, int authmethods);


// Unsorted functions
// ==================


LIBSSH_API ssh_channel ssh_channel_accept_forward(ssh_session_struct * session, int timeout_ms, int *destination_port);
LIBSSH_API void ssh_free(ssh_session_struct * session);

LIBSSH_API int ssh_is_blocking(ssh_session_struct * session);
LIBSSH_API int ssh_is_server_known(ssh_session_struct * session);

LIBSSH_API struct ssh_poll_ctx_struct * ssh_new_poll_ctx();

LIBSSH_API ssh_channel ssh_new_channel(ssh_session_struct * session, ssh_channel_callbacks cb);

LIBSSH_API int ssh_options_set(ssh_session_struct * session, enum ssh_options_e type,
    const void *value);

LIBSSH_API void ssh_key_free (ssh_key_struct * key);
LIBSSH_API const char *ssh_key_type_to_char(enum ssh_keytypes_e type);

LIBSSH_API int ssh_pki_export_pubkey_base64_p(const ssh_key_struct * key, char *b64, int b64_len);

LIBSSH_API void ssh_set_blocking(ssh_session_struct * session, int blocking);

LIBSSH_API int ssh_event_dopoll(ssh_poll_ctx_struct * ctx, int timeout);

LIBSSH_API ssh_gssapi_creds ssh_gssapi_get_creds_server(ssh_session_struct * session);



/* SSH-1 Flags */

#define SSH_MSG_NONE				0	/* no message */
#define SSH_MSG_DISCONNECT			1	/* cause (string) */
#define SSH_SMSG_PUBLIC_KEY			2	/* ck,msk,srvk,hostk */
#define SSH_CMSG_SESSION_KEY			3	/* key (BIGNUM) */
#define SSH_CMSG_USER				4	/* user (string) */
#define SSH_CMSG_AUTH_RHOSTS			5	/* user (string) */
#define SSH_CMSG_AUTH_RSA			6	/* modulus (BIGNUM) */
#define SSH_SMSG_AUTH_RSA_CHALLENGE		7	/* int (BIGNUM) */
#define SSH_CMSG_AUTH_RSA_RESPONSE		8	/* int (BIGNUM) */
#define SSH_CMSG_AUTH_PASSWORD			9	/* pass (string) */
#define SSH_CMSG_REQUEST_PTY			10	/* TERM, tty modes */
#define SSH_CMSG_WINDOW_SIZE			11	/* row,col,xpix,ypix */
#define SSH_CMSG_EXEC_SHELL			12	/* */
#define SSH_CMSG_EXEC_CMD			13	/* cmd (string) */
#define SSH_SMSG_SUCCESS			14	/* */
#define SSH_SMSG_FAILURE			15	/* */
#define SSH_CMSG_STDIN_DATA			16	/* data (string) */
#define SSH_SMSG_STDOUT_DATA			17	/* data (string) */
#define SSH_SMSG_STDERR_DATA			18	/* data (string) */
#define SSH_CMSG_EOF				19	/* */
#define SSH_SMSG_EXITSTATUS			20	/* status (int) */

//#define SSH_MSG_CHANNEL_OPEN_CONFIRMATION	21	/* channel (int) */
//#define SSH_MSG_CHANNEL_OPEN_FAILURE		22	/* channel (int) */
//#define SSH_MSG_CHANNEL_DATA			23	/* ch,data (int,str) */
//#define SSH_MSG_CHANNEL_CLOSE			24	/* channel (int) */
//#define SSH_MSG_CHANNEL_CLOSE_CONFIRMATION	25	/* channel (int) */

/*      SSH_CMSG_X11_REQUEST_FORWARDING		26	   OBSOLETE */

#define SSH_SMSG_X11_OPEN			27	/* channel (int) */
#define SSH_CMSG_PORT_FORWARD_REQUEST		28	/* p,host,hp (i,s,i) */
#define SSH_MSG_PORT_OPEN			29	/* ch,h,p (i,s,i) */
#define SSH_CMSG_AGENT_REQUEST_FORWARDING	30	/* */
#define SSH_SMSG_AGENT_OPEN			31	/* port (int) */
#define SSH_MSG_IGNORE				32	/* string */
#define SSH_CMSG_EXIT_CONFIRMATION		33	/* */
#define SSH_CMSG_X11_REQUEST_FORWARDING		34	/* proto,data (s,s) */
#define SSH_CMSG_AUTH_RHOSTS_RSA		35	/* user,mod (s,mpi) */
#define SSH_MSG_DEBUG				36	/* string */
#define SSH_CMSG_REQUEST_COMPRESSION		37	/* level 1-9 (int) */
#define SSH_CMSG_MAX_PACKET_SIZE		38	/* size 4k-1024k (int) */
#define SSH_CMSG_AUTH_TIS			39	/* we use this for s/key */
#define SSH_SMSG_AUTH_TIS_CHALLENGE		40	/* challenge (string) */
#define SSH_CMSG_AUTH_TIS_RESPONSE		41	/* response (string) */
#define SSH_CMSG_AUTH_KERBEROS			42	/* (KTEXT) */
#define SSH_SMSG_AUTH_KERBEROS_RESPONSE		43	/* (KTEXT) */
#define SSH_CMSG_HAVE_KERBEROS_TGT		44	/* credentials (s) */
#define SSH_CMSG_HAVE_AFS_TOKEN			65	/* token (s) */

/* protocol version 1.5 overloads some version 1.3 message types */
#define SSH_MSG_CHANNEL_INPUT_EOF	SSH_MSG_CHANNEL_CLOSE
#define SSH_MSG_CHANNEL_OUTPUT_CLOSE	SSH_MSG_CHANNEL_CLOSE_CONFIRMATION

/*
 * Authentication methods.  New types can be added, but old types should not
 * be removed for compatibility.  The maximum allowed value is 31.
 */
#define SSH_AUTH_RHOSTS		1
#define SSH_AUTH_RSA		2
#define SSH_AUTH_PASSWORD	3
#define SSH_AUTH_RHOSTS_RSA	4
#define SSH_AUTH_TIS		5
#define SSH_AUTH_KERBEROS	6
#define SSH_PASS_KERBEROS_TGT	7
				/* 8 to 15 are reserved */
#define SSH_PASS_AFS_TOKEN	21

/* Protocol flags.  These are bit masks. */
#define SSH_PROTOFLAG_SCREEN_NUMBER	1	/* X11 forwarding includes screen */
#define SSH_PROTOFLAG_HOST_IN_FWD_OPEN	2	/* forwarding opens contain host */

/* cipher flags. they are bit numbers */
#define SSH_CIPHER_NONE   0      /* No encryption */
#define SSH_CIPHER_IDEA   1      /* IDEA in CFB mode */
#define SSH_CIPHER_DES    2      /* DES in CBC mode */
#define SSH_CIPHER_3DES   3      /* Triple-DES in CBC mode */
#define SSH_CIPHER_RC4    5      /* RC4 */
#define SSH_CIPHER_BLOWFISH     6

}

