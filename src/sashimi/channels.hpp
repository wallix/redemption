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
   Copyright (c) 2014-2016 by Christophe Grosjean, Meng Tan
   Author(s): Christophe Grosjean

   Adapted from parts of the SSH Library
   Copyright (c) 2003-2009 by Aris Adamantiadis
*/


#pragma once

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h> // fd_set
#include <poll.h>

#include "utils/log.hpp"
#include "sashimi/libssh/libssh.h"
#include "sashimi/libcrypto.hpp"
#include "sashimi/pki.hpp"
#include "sashimi/buffer.hpp"

#include "cxx/cxx.hpp"

/*
 * All implementations MUST be able to process packets with an
 * uncompressed payload length of 32768 bytes or less and a total packet
 * size of 35000 bytes or less.
 */
#define CHANNEL_TOTAL_PACKET_SIZE 35000
#define CHANNEL_MAX_PACKET 32768
#define CHANNEL_INITIAL_WINDOW 64000



struct options_struct {
    int nbidentity;
    char *identity[10];
    char *username;
    char *host;
    char *bindaddr; /* bind the client to an ip addr */
    char *wanted_methods[10];
    char *custombanner;
    unsigned long timeout; /* seconds */
    unsigned long timeout_usec;
    unsigned int port;
    socket_t fd;
    int StrictHostKeyChecking;
    int ssh2;
    int ssh1;
    char compressionlevel;
    char *gss_server_identity;
    char *gss_client_identity;
    int gss_delegate_creds;

    options_struct()
    : nbidentity(0)
    , identity{}
    , username(nullptr)
    , host(nullptr)
    , bindaddr(nullptr)
    , wanted_methods{}
    , custombanner(nullptr)
    , timeout(0)
    , timeout_usec(0)
    , port(0)
    , fd(-1)
    , StrictHostKeyChecking(0)
    , ssh2(1)
    , ssh1(0)
    , compressionlevel(0)
    , gss_server_identity(nullptr)
    , gss_client_identity(nullptr)
    , gss_delegate_creds(0)
    {}
};

int options_set(options_struct & opts, enum ssh_options_e type, const void *value, error_struct & error);


/* Messages for the authentication agent connection. */
#define SSH_AGENTC_REQUEST_RSA_IDENTITIES        1
#define SSH_AGENT_RSA_IDENTITIES_ANSWER          2
#define SSH_AGENTC_RSA_CHALLENGE                 3
#define SSH_AGENT_RSA_RESPONSE                   4
#define SSH_AGENT_FAILURE                        5
#define SSH_AGENT_SUCCESS                        6
#define SSH_AGENTC_ADD_RSA_IDENTITY              7
#define SSH_AGENTC_REMOVE_RSA_IDENTITY           8
#define SSH_AGENTC_REMOVE_ALL_RSA_IDENTITIES     9

/* private OpenSSH extensions for SSH2 */
#define SSH2_AGENTC_REQUEST_IDENTITIES           11
#define SSH2_AGENT_IDENTITIES_ANSWER             12
#define SSH2_AGENTC_SIGN_REQUEST                 13
#define SSH2_AGENT_SIGN_RESPONSE                 14
#define SSH2_AGENTC_ADD_IDENTITY                 17
#define SSH2_AGENTC_REMOVE_IDENTITY              18
#define SSH2_AGENTC_REMOVE_ALL_IDENTITIES        19

/* smartcard */
#define SSH_AGENTC_ADD_SMARTCARD_KEY             20
#define SSH_AGENTC_REMOVE_SMARTCARD_KEY          21

/* lock/unlock the agent */
#define SSH_AGENTC_LOCK                          22
#define SSH_AGENTC_UNLOCK                        23

/* add key with constraints */
#define SSH_AGENTC_ADD_RSA_ID_CONSTRAINED        24
#define SSH2_AGENTC_ADD_ID_CONSTRAINED           25
#define SSH_AGENTC_ADD_SMARTCARD_KEY_CONSTRAINED 26

#define SSH_AGENT_CONSTRAIN_LIFETIME             1
#define SSH_AGENT_CONSTRAIN_CONFIRM              2

/* extended failure messages */
#define SSH2_AGENT_FAILURE                       30

/* additional error code for ssh.com's ssh-agent2 */
#define SSH_COM_AGENT2_FAILURE                   102

#define SSH_AGENT_OLD_SIGNATURE                  0x01


/**  @internal
 * Describes the different possible states in a
 * outgoing (client) channel request
 */
enum ssh_channel_request_state_e {
	/** No request has been made */
	SSH_CHANNEL_REQ_STATE_NONE = 0,
	/** A request has been made and answer is pending */
	SSH_CHANNEL_REQ_STATE_PENDING,
	/** A request has been replied and accepted */
	SSH_CHANNEL_REQ_STATE_ACCEPTED,
	/** A request has been replied and refused */
	SSH_CHANNEL_REQ_STATE_DENIED,
	/** A request has been replied and an error happend */
	SSH_CHANNEL_REQ_STATE_ERROR
};


/* Infinite timeout */
#define SSH_TIMEOUT_INFINITE -1
/* Don't block at all */
#define SSH_TIMEOUT_NONBLOCKING 0
// default is 10000 ms
#define TIMEOUT_DEFAULT_MS 10000


/* The channel has been closed by the remote side */
#define SSH_CHANNEL_FLAG_CLOSED_REMOTE 0x1
/* The channel has been freed by the calling program */
#define SSH_CHANNEL_FLAG_FREED_LOCAL 0x2
/* the channel has not yet been bound to a remote one */
#define SSH_CHANNEL_FLAG_NOT_BOUND 0x4

struct ssh_channel_struct {
    enum class ssh_channel_state_e {
      SSH_CHANNEL_STATE_NOT_OPEN = 0,
      SSH_CHANNEL_STATE_OPENING,
      SSH_CHANNEL_STATE_OPEN_DENIED,
      SSH_CHANNEL_STATE_OPEN,
      SSH_CHANNEL_STATE_CLOSED
    };

//    ssh_session session; /* SSH_SESSION pointer */

    uint32_t local_channel;
    uint32_t local_window;
    bool local_eof;
    bool local_eof_to_send;
    bool local_closed;
    bool local_close_to_send;
    uint32_t local_maxpacket;

    uint32_t remote_channel;
    uint32_t remote_window;
    int remote_eof; /* end of file received */
    uint32_t remote_maxpacket;
    enum ssh_channel_state_e state;
    int delayed_close;
    int flags;
    ssh_buffer_struct* server_outbuffer;
    ssh_buffer_struct* stdout_buffer;
    ssh_buffer_struct* stderr_buffer;
    void *userarg;
    int version;
    int exit_status;
    enum ssh_channel_request_state_e request_state;
    ssh_channel_callbacks callbacks;
    char show_buffer[256];

    ssh_channel_struct(ssh_session_struct * session, ssh_channel_callbacks callbacks, int dummy)
    :
//    session(session),
      local_channel(0)
    , local_window(0)
    , local_eof(false)
    , local_eof_to_send(false)
    , local_closed(false)
    , local_close_to_send(false)
    , local_maxpacket(0)
    , remote_channel(0)
    , remote_window(0)
    , remote_eof(0)
    , remote_maxpacket(0)
    , state(ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_NOT_OPEN)
    , delayed_close(0)
    , flags(SSH_CHANNEL_FLAG_NOT_BOUND)
    , server_outbuffer(new ssh_buffer_struct)
    , stdout_buffer(new ssh_buffer_struct)
    , stderr_buffer(new ssh_buffer_struct)
    , userarg(nullptr)
    , version(0)
    , exit_status(-1)
    , request_state(SSH_CHANNEL_REQ_STATE_NONE)
    , callbacks(callbacks)
    {
        (void)session;
        (void)dummy;
    }

    ~ssh_channel_struct()
    {
        delete this->stdout_buffer;
        delete this->stderr_buffer;
    }


    char * show(){
        sprintf(this->show_buffer, "Channel %s %s (%d%s:%d%s)",
           ((this->state == ssh_channel_state_e::SSH_CHANNEL_STATE_NOT_OPEN)   ? "NOT_OPEN"   :
            (this->state == ssh_channel_state_e::SSH_CHANNEL_STATE_OPENING)    ? "OPENING"    :
            (this->state == ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN_DENIED)? "OPEN DENIED":
            (this->state == ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN)       ? "OPEN"       :
            (this->state == ssh_channel_state_e::SSH_CHANNEL_STATE_CLOSED)     ? "CLOSED" : "?STATE"),

           ((this->request_state == ssh_channel_request_state_e::SSH_CHANNEL_REQ_STATE_NONE)    ? "REQ NONE"     :
            (this->request_state == ssh_channel_request_state_e::SSH_CHANNEL_REQ_STATE_PENDING) ? "REQ PENDING"  :
            (this->request_state == ssh_channel_request_state_e::SSH_CHANNEL_REQ_STATE_ACCEPTED)? "REQ ACCEPTED" :
            (this->request_state == ssh_channel_request_state_e::SSH_CHANNEL_REQ_STATE_DENIED)  ? "REQ DENIED"   :
            (this->request_state == ssh_channel_request_state_e::SSH_CHANNEL_REQ_STATE_ERROR)   ? "REQ ERROR" : "?REQ"),

            this->local_channel, (this->local_eof ?"E":""),
            this->remote_channel, (this->remote_eof?"E":""));

            return this->show_buffer;
    }

    /**
     * @internal
     *
     * @brief Open a channel by sending a SSH_OPEN_CHANNEL message and
     *        wait for the reply.
     *
     * @param[in]  channel_type_c   A C string describing the kind of channel (e.g. "exec").
     *
     * @param[in]  window   The receiving window of the channel. The window is the
     *                      maximum size of data that can stay in buffers and
     *                      network.
     *
     * @param[in]  maxpacket The maximum packet size allowed (like MTU).
     */
    // TODO: I'm not sure constructor should be different of open ?
    // Well, maybe if channel is opened from the remote side when receiving a message
    // instead of sending an open request.
    void channel_open(uint32_t id, const char *channel_type_c, uint32_t maxpacket, uint32_t window, ssh_buffer_struct* buffer) {
        LOG(LOG_INFO,
                "Creating channel #%d with %d window and %d max packet",
                id, window, maxpacket);

        this->local_channel = id;
        this->local_maxpacket = maxpacket;
        this->local_window = window;
        // TODO: shouldn't we copy the channel type locally (for debug purpose ?)

        buffer->out_uint8(SSH_MSG_CHANNEL_OPEN);
        buffer->out_length_prefixed_cstr(channel_type_c);
        buffer->out_uint32_be(this->local_channel);
        buffer->out_uint32_be(this->local_window);
        buffer->out_uint32_be(this->local_maxpacket);

        this->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPENING;
    }

    int channel_request(ssh_session_struct * session);
};

struct ssh_poll_handle_struct;

/** different state of packet reading. */
enum ssh_packet_state_e {
  /** Packet not initialized, must read the size of packet */
  PACKET_STATE_INIT,
  /** Size was read, waiting for the rest of data */
  PACKET_STATE_SIZEREAD,
  /** Full packet was read and callbacks are being called. Future packets
   * should wait for the end of the callback. */
  PACKET_STATE_PROCESSING
};

/* PACKET CRYPT */
int packet_decrypt(struct ssh_crypto_struct & crypto, void *data, uint32_t len, error_struct & error);
unsigned char *packet_encrypt(struct ssh_crypto_struct & crypto, uint32_t seq, void *data, uint32_t len, struct error_struct & error);
int packet_hmac_verify(ssh_session_struct * session,ssh_buffer_struct* buffer, unsigned char *mac);

int channel_write_common_server(ssh_session_struct * server_session, ssh_channel channel, const uint8_t *data, uint32_t len);


typedef struct pollfd ssh_pollfd_t;
typedef struct ssh_poll_handle_struct *ssh_poll_handle;

/**
 * @brief SSH poll callback. This callback will be used when an event
 *                      caught on the socket.
 *
 * @param p             Poll object this callback belongs to.
 * @param fd            The raw socket.
 * @param revents       The current poll events on the socket.
 * @param userdata      Userdata to be passed to the callback function.
 *
 * @return              0 on success, < 0 if you removed the poll object from
 *                      its poll context.
 */
typedef int (*ssh_poll_callback)(ssh_poll_handle p, socket_t fd, int revents, ssh_session_struct * session, void *userdata);

enum ssh_socket_states_e {
    SSH_SOCKET_NONE,
    SSH_SOCKET_CONNECTING,
    SSH_SOCKET_CONNECTED,
    SSH_SOCKET_EOF,
    SSH_SOCKET_ERROR,
    SSH_SOCKET_CLOSED
};

struct ssh_poll_handle_struct {
    int lock;
    struct ssh_socket_struct * socket;

    ssh_poll_handle_struct(ssh_socket_struct * socket)
    : lock(0)
    , socket(socket)
    {
        LOG(LOG_INFO, "ssh_poll_handle_struct");
    }

    ~ssh_poll_handle_struct() {
    }

};

struct ssh_poll_handle_fd_struct {
    socket_t x_fd;
    int lock;
    struct ssh_socket_struct * socket;
    ssh_event_callback pw_cb;
    void * pw_userdata;

    ssh_poll_handle_fd_struct(socket_t fd, ssh_event_callback pw_cb, void * pw_userdata)
    : x_fd(fd)
    , lock(0)
    , socket(nullptr)
    , pw_cb(pw_cb)
    , pw_userdata(pw_userdata)
    {
    }

    ~ssh_poll_handle_fd_struct() {
    }

};


struct ssh_poll_ctx_struct {
    struct SshServerSession * front_session;
    struct SshClientSession * target_session;

    ssh_poll_handle_fd_struct * fd_poll;

    ssh_poll_ctx_struct()
    : front_session(nullptr)
    , target_session(nullptr)
    , fd_poll(nullptr   )
    {
        LOG(LOG_INFO, "new poll_ctx_struct at %p", reinterpret_cast<void*>(this));
    }

    ~ssh_poll_ctx_struct() {
    }
};

#define WINDOWBASE 1280000
#define WINDOWLIMIT (WINDOWBASE/2)

int ssh_channel_read_stdout_server(ssh_session_struct * server_session, ssh_channel channel, void *dest, uint32_t count);


struct ssh_socket_struct {
    socket_t fd_in;
    socket_t fd_out;
    int fd_is_socket;
    int last_errno;
    int read_wontblock; // reading now on socket will not block
    int write_wontblock;
    bool data_except;
    enum ssh_socket_states_e state;
    ssh_buffer_struct* out_buffer;
    ssh_buffer_struct* in_buffer;

    ssh_socket_struct()
        : fd_in(INVALID_SOCKET)
        , fd_out(INVALID_SOCKET)
        , fd_is_socket(1)
        , last_errno(-1)
        , read_wontblock(0)
        , write_wontblock(0)
        , data_except(false)
        , state(SSH_SOCKET_NONE)
        , out_buffer(new ssh_buffer_struct)
        , in_buffer(new ssh_buffer_struct)
    {
        LOG(LOG_INFO, "ssh_socket_struct::ssh_socket_struct()");
    }

    ~ssh_socket_struct(){
      this->close();
      delete this->in_buffer;
      delete this->out_buffer;
    }


    void close(){
        LOG(LOG_INFO, "%s ---", __FUNCTION__);
        if (this->fd_in != INVALID_SOCKET) {
            ::close(this->fd_in);
            if(this->fd_out != this->fd_in && this->fd_out != INVALID_SOCKET){
                ::close(this->fd_out);
            }
            this->last_errno = errno;
            this->fd_in = this->fd_out = INVALID_SOCKET;
        }
        this->state = SSH_SOCKET_CLOSED;
    }
};

/*
 * How does the ssh-agent work?
 *
 * a) client sends a request to get a list of all keys
 *    the agent returns the count and all public keys
 * b) iterate over them to check if the server likes one
 * c) the client sends a sign request to the agent
 *    type, pubkey as blob, data to sign, flags
 *    the agent returns the signed data
 */

struct ssh_agent_struct {
  struct ssh_socket_struct *sock;
  ssh_buffer_struct* ident;
  unsigned int count;
  ssh_channel channel;

  ssh_agent_struct(ssh_session_struct *session, ssh_socket_struct * sock)
  : sock(sock)
  , ident(nullptr)
  , count(0)
  , channel(nullptr)
  {
        LOG(LOG_INFO, "ssh_agent_struct::ssh_agent_struct");
        (void)session;
  }

    /* caller has to free comment */
    ssh_key_struct *ssh_agent_get_next_ident(char **comment) {
        LOG(LOG_INFO, "%s ---", __FUNCTION__);
        // TODO: add boundary checks
        uint32_t blob_len = this->ident->in_uint32_be();
        std::vector<uint8_t> blob;
        blob.resize(blob_len);
        this->ident->buffer_get_data(&blob[0], blob.size());
        uint32_t tmp_len = this->ident->in_uint32_be();
        std::vector<uint8_t> tmp;
        tmp.resize(tmp_len);
        this->ident->buffer_get_data(&tmp[0],tmp.size());

        *comment = static_cast<char*>(malloc(tmp.size() + 1));
        memcpy(*comment, &tmp[0], tmp.size());
        (*comment)[tmp.size()] = 0;

        /* get key from blob */
        struct ssh_key_struct *key;
        ssh_buffer_struct buffer;
        buffer.out_blob(&blob[0], blob.size());

        if (SSH_ERROR == ssh_pki_import_pubkey_blob(buffer, &key)){
            return nullptr;
        }
        return key;
    }


  void set_channel(ssh_channel_struct * channel)
  {
    LOG(LOG_INFO, "%s : set channel to channel=%p this->channel=%p agent=%p",
        __FUNCTION__,
        reinterpret_cast<void*>(channel),
        reinterpret_cast<void*>(this->channel),
        reinterpret_cast<void*>(this));
    this->channel = channel;
    LOG(LOG_INFO, "%s : set channel to channel=%p this->channel=%p agent=%p",
        __FUNCTION__,
        reinterpret_cast<void*>(channel),
        reinterpret_cast<void*>(this->channel),
        reinterpret_cast<void*>(this));
  }

  void agent_close() {
    if (getenv("SSH_AUTH_SOCK")) {
      this->sock->close();
    }
  }

  ~ssh_agent_struct()
  {
      if (this->ident) {
        delete this->ident;
      }
      if (this->sock) {
        this->agent_close();
      }
  }

    bool agent_is_running(ssh_session_struct * session) {
        if (this->sock->fd_in != INVALID_SOCKET){
            LOG(LOG_INFO, "%s : true: agent socket open", __FUNCTION__);
            return true;
        }
        if (this->channel){
            LOG(LOG_INFO, "%s : true: agent channel OK %p", __FUNCTION__,
                reinterpret_cast<void*>(this->channel));
            return true;
        }
        LOG(LOG_INFO, "%s : false agent=%p channel=%p session=%p",
            __FUNCTION__,
            reinterpret_cast<void*>(this),
            reinterpret_cast<void*>(this->channel),
            reinterpret_cast<void*>(session));
        return false;
    }

    int agent_talk_channel_server(ssh_session_struct * server_session, struct ssh_buffer_struct *request, struct ssh_buffer_struct *reply, struct error_struct & error)
    {
        LOG(LOG_INFO, "%s ---", __FUNCTION__);

        /* send length and then the request packet */
        size_t len = request->in_remain();
        uint8_t buf[sizeof(uint32_t)];
        Serialize s(buf);
        s.out_uint32_be(len);

        size_t pos = 0;
        /* using an SSH channel */
        while (4 > pos){
            LOG(LOG_INFO, "%s ---", __FUNCTION__);
            ssize_t res = channel_write_common_server(server_session, this->channel, buf + pos, 4 - pos);
            switch (res){
            default:
                pos += res;
                REDEMPTION_CXX_FALLTHROUGH;
            case SSH_AGAIN:
                break;
            case SSH_ERROR:
                LOG(LOG_WARNING, "atomicio sending request length failed: %s", strerror(errno));
                return -1;
            }
        }
        pos = 0;
        uint8_t * buf2 = request->get_pos_ptr();
        while (len > pos){
            ssize_t res = channel_write_common_server(server_session, this->channel, buf2 + pos, len - pos);
            switch (res){
            default:
                pos += res;
                REDEMPTION_CXX_FALLTHROUGH;
            case SSH_AGAIN:
                break;
            case SSH_ERROR:
                LOG(LOG_WARNING, "atomicio sending request length failed: %s", strerror(errno));
                return -1;
            }
        }

        uint8_t payload[1024] = {0};
        /* wait for response, read the length of the response packet */

        size_t pos2 = 0;

        while (4 > pos2){
            ssize_t res = ssh_channel_read_stdout_server(server_session, this->channel, payload + pos2, 4 - pos2);
            if (res == SSH_AGAIN){
              continue;
            }
            if (res == SSH_ERROR){
                LOG(LOG_WARNING, "atomicio read response length failed: %s", strerror(errno));
                return -1;
            }
            pos2 += static_cast<size_t>(res);
        }

        len = Parse(payload).in_uint32_be();
        if (len > 256 * 1024) {
            ssh_set_error(error, SSH_FATAL, "Authentication response too long: %u", static_cast<unsigned>(len));
            return -1;
        }
        LOG(LOG_INFO, "Response length: %u", static_cast<unsigned>(len));
        while (len > 0) {
            LOG(LOG_INFO, "%s len=%u", __FUNCTION__, static_cast<unsigned>(len));

            ssize_t res = ssh_channel_read_stdout_server(server_session, this->channel, payload,
                                (len > sizeof(payload))?sizeof(payload):len);
            LOG(LOG_INFO, "res=%u", static_cast<unsigned>(res));

            if (res == SSH_AGAIN){
                  LOG(LOG_INFO, "%s again", __FUNCTION__);
                  continue;
            }
            if ((res == SSH_ERROR)||(res < 0)){
                LOG(LOG_WARNING, "Error reading response from authentication socket.");
                return -1;
            }

            LOG(LOG_INFO, "%s sending reply", __FUNCTION__);
            reply->out_blob(payload, res);
            len -= res;
        }
        LOG(LOG_INFO, "%s reply ready", __FUNCTION__);
        return 0;
    }

    int ssh_agent_get_ident_count_channel_ssh2_server(ssh_session_struct * server_session, error_struct & error)
    {
        LOG(LOG_INFO, "%s ---", __FUNCTION__);
        ssh_buffer_struct request;
        /* send message to the agent requesting the list of identities */
        request.out_uint8(SSH2_AGENTC_REQUEST_IDENTITIES);

        ssh_buffer_struct* reply = new ssh_buffer_struct;
        try {

            if (this->agent_talk_channel_server(server_session, &request, reply, error) < 0) {
                LOG(LOG_INFO, "%s agent talk channel error ---", __FUNCTION__);
                error.error_code = SSH_NO_ERROR;
                error.eid = 0;
                throw error;
            }

            LOG(LOG_INFO, "%s back from agent talk channel 1 ---", __FUNCTION__);


            /* get message type and verify the answer */
            unsigned int type = reply->in_uint8();

            LOG(LOG_WARNING, "Answer type: %d, expected answer: SSH2_AGENT_IDENTITIES_ANSWER (%d)",
                type, SSH2_AGENT_IDENTITIES_ANSWER);

            if ((type == SSH_AGENT_FAILURE) || (type == SSH_COM_AGENT2_FAILURE) || (type == SSH2_AGENT_FAILURE)) {
                LOG(LOG_INFO, "%s type = FAILURE ---", __FUNCTION__);
                error.error_code = SSH_NO_ERROR;
                error.eid = 0;
                throw error;
            }

            if (type != SSH2_AGENT_IDENTITIES_ANSWER) {
                ssh_set_error(error, SSH_FATAL, "Bad authentication reply message type: %d", type);
                error.eid = -1;
                LOG(LOG_INFO, "%s type = not SSH2_AGENT_IDENTITIES_ANSWER ---", __FUNCTION__);
                throw error;
            }

            this->count = reply->in_uint32_be();
            LOG(LOG_INFO, "Agent count: %d", this->count);

            if (this->count > 1024) {
                ssh_set_error(error, SSH_FATAL, "Too many identities in authentication reply: %d", this->count);
                error.eid = -1;
                throw error;
            }
        } catch (error_struct & error) {
            delete reply;
            return error.eid;
        };

        if (this->ident) {
            this->ident->buffer_reinit();
        }
        this->ident = reply;
        return this->count;
    }

};

/* all OID begin with the tag identifier + length */
#define SSH_OID_TAG 06

typedef struct ssh_gssapi_struct *ssh_gssapi;


struct ssh_kbdint_struct {
    uint32_t nprompts;
    uint32_t nanswers;
    char *name;
    char *instruction;
    char **prompts;
    unsigned char *echo; /* bool array */
    char **answers;

    ssh_kbdint_struct()
        : nprompts(0)
        , nanswers(0)
        , name(nullptr)
        , instruction(nullptr)
        , prompts(nullptr)
        , echo(nullptr)
        , answers(nullptr)
    {
    }
};
typedef struct ssh_kbdint_struct* ssh_kbdint;

ssh_kbdint ssh_kbdint_new(void);
void ssh_kbdint_clean(ssh_kbdint kbd);
void ssh_kbdint_free(ssh_kbdint kbd);


/** @internal
 * States of authentication in the client-side. They describe
 * what was the last response from the server
 */
enum ssh_auth_state_e {
  /** No authentication asked */
  SSH_AUTH_STATE_NONE=0,
  /** Last authentication response was a partial success */
  SSH_AUTH_STATE_PARTIAL,
  /** Last authentication response was a success */
  SSH_AUTH_STATE_SUCCESS,
  /** Last authentication response was failed */
  SSH_AUTH_STATE_FAILED,
  /** Last authentication was erroneous */
  SSH_AUTH_STATE_ERROR,
  /** Last state was a keyboard-interactive ask for info */
  SSH_AUTH_STATE_INFO,
  /** Last state was a public key accepted for authentication */
  SSH_AUTH_STATE_PK_OK,
  /** We asked for a keyboard-interactive authentication */
  SSH_AUTH_STATE_KBDINT_SENT,
  /** We have sent an userauth request with gssapi-with-mic */
  SSH_AUTH_STATE_GSSAPI_REQUEST_SENT,
  /** We are exchanging tokens until authentication */
  SSH_AUTH_STATE_GSSAPI_TOKEN,
  /** We have sent the MIC and expecting to be authenticated */
  SSH_AUTH_STATE_GSSAPI_MIC_SENT,
};

/** @internal
 * @brief states of the authentication service request
 */
enum ssh_auth_service_state_e {
  /** initial state */
  SSH_AUTH_SERVICE_NONE=0,
  /** Authentication service request packet sent */
  SSH_AUTH_SERVICE_SENT,
  /** Service accepted */
  SSH_AUTH_SERVICE_ACCEPTED,
  /** Access to service denied (fatal) */
  SSH_AUTH_SERVICE_DENIED,
  /** Specific to SSH1 */
  SSH_AUTH_SERVICE_USER_SENT
};


/* These are the different states a SSH session can be into its life */
enum ssh_session_state_e {
	SSH_SESSION_STATE_NONE=0,
	SSH_SESSION_STATE_CONNECTING,
	SSH_SESSION_STATE_SOCKET_CONNECTED,
	SSH_SESSION_STATE_BANNER_RECEIVED,
	SSH_SESSION_STATE_INITIAL_KEX,
	SSH_SESSION_STATE_KEXINIT_RECEIVED,
	SSH_SESSION_STATE_DH,
	SSH_SESSION_STATE_AUTHENTICATING,
	SSH_SESSION_STATE_AUTHENTICATED,
	SSH_SESSION_STATE_ERROR,
	SSH_SESSION_STATE_DISCONNECTED
};

enum ssh_dh_state_e {
  DH_STATE_INIT=0,
  DH_STATE_INIT_SENT,
  DH_STATE_NEWKEYS_SENT,
  DH_STATE_FINISHED
};

enum ssh_pending_call_e {
	SSH_PENDING_CALL_NONE = 0,
	SSH_PENDING_CALL_CONNECT,
	SSH_PENDING_CALL_AUTH_NONE,
	SSH_PENDING_CALL_AUTH_PASSWORD,
	SSH_PENDING_CALL_AUTH_OFFER_PUBKEY,
	SSH_PENDING_CALL_AUTH_PUBKEY,
	SSH_PENDING_CALL_AUTH_AGENT,
	SSH_PENDING_CALL_AUTH_KBDINT_INIT,
	SSH_PENDING_CALL_AUTH_KBDINT_SEND,
	SSH_PENDING_CALL_AUTH_GSSAPI_MIC
};

/* libssh calls may block an undefined amount of time */
#define SSH_SESSION_FLAG_BLOCKING 1

/* Client successfully authenticated */
#define SSH_SESSION_FLAG_AUTHENTICATED 2


enum ssh_agent_state_e {
    SSH_AGENT_STATE_NONE = 0,
    SSH_AGENT_STATE_PUBKEY,
    SSH_AGENT_STATE_AUTH
};

struct ssh_agent_state_struct {
    enum ssh_agent_state_e state;
    ssh_key_struct *pubkey;
    char *comment;
    ssh_agent_state_struct()
    : state(SSH_AGENT_STATE_NONE)
    , pubkey(nullptr)
    , comment(nullptr)
    {
    }
};

#define FIRST_CHANNEL 42 // why not ? it helps to find bugs.

enum ssh_auth_auto_state_e {
    SSH_AUTH_AUTO_STATE_NONE=0,
    SSH_AUTH_AUTO_STATE_PUBKEY,
    SSH_AUTH_AUTO_STATE_KEY_IMPORTED,
    SSH_AUTH_AUTO_STATE_PUBKEY_ACCEPTED
};

struct ssh_auth_auto_state_struct {
    enum ssh_auth_auto_state_e state;
    int it;
    ssh_key_struct *privkey;
    ssh_key_struct *pubkey;

    ssh_auth_auto_state_struct()
        : state(SSH_AUTH_AUTO_STATE_NONE)
        , it(-1)
        , privkey(nullptr)
        , pubkey(nullptr)
    {
    }
};

#include "sashimi/ssh_session.hpp"

int ssh_poll_ctx_dopoll(struct ssh_poll_ctx_struct * ctx, int timeout);
int dopoll(ssh_poll_ctx_struct * ctx, int timeout);
