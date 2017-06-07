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

#include "utils/log.hpp"
#include "sashimi/channels.hpp"

REDEMPTION_DIAGNOSTIC_PUSH
REDEMPTION_DIAGNOSTIC_GCC_IGNORE("-Wold-style-cast")

// ================================= SSH_MSG_SERVICE_ACCEPT =================================

/**
 * @brief Open an agent authentication forwarding channel. This type of channel
 * can be opened by a server towards a client in order to provide SSH-Agent services
 * to the server-side process. This channel can only be opened if the client
 * claimed support by sending a channel request beforehand.
 *
 * @param[in]  channel  An allocated channel.
 *
 * @return              SSH_OK on success,
 *                      SSH_ERROR if an error occurred,
 *                      SSH_AGAIN if in nonblocking mode and call has
 *                      to be done again.
 *
 * @see ssh_channel_open_forward()
 */
int SshServerSession::ssh_channel_open_auth_agent_server(ssh_channel channel){
    LOG(LOG_INFO, "%s ---", __PRETTY_FUNCTION__);
    LOG(LOG_INFO, "ssh_channel_open_auth_agent %s %s", channel->show(), this->show());

    if (this->session_state == SSH_SESSION_STATE_ERROR){
        LOG(LOG_INFO, "ssh_channel_open_auth_agent E2");
        return SSH_ERROR;
    }

    int err=SSH_ERROR;

    switch(channel->state){
    case ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_NOT_OPEN:
        LOG(LOG_WARNING, "CHANNEL_STATE_NOT OPEN");
        channel->channel_open(this->new_channel_id(),
                              "auth-agent@openssh.com",
                              CHANNEL_MAX_PACKET,
                              CHANNEL_INITIAL_WINDOW,
                              this->out_buffer);


        this->packet_send();

        REDEMPTION_CXX_FALLTHROUGH;
    // Beware, this is fallthrough behavior to opening
    // But maybe we should have some error if we call open and channel is already opening
    // instead of accepting it.
    case ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPENING:
        LOG(LOG_WARNING, "CHANNEL_STATE_OPENING");
        err = SSH_OK;
        while(1) {
            if (channel->state != ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPENING){
                err = SSH_OK;
                break;
            }
            if (this->socket == nullptr) {
                LOG(LOG_WARNING, "handle_packets early exit : no socket");
                err = SSH_ERROR;
                break;
            }

            // Waiting for input
            dopoll(this->ctx, (this->flags&SSH_SESSION_FLAG_BLOCKING)
                ? SSH_TIMEOUT_INFINITE
                : SSH_TIMEOUT_NONBLOCKING);

            if (this->session_state == SSH_SESSION_STATE_ERROR){
                break;
            }
            if (channel->state == ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPENING) {
                err = SSH_AGAIN;
            }
            if (!(this->flags&SSH_SESSION_FLAG_BLOCKING)){
                break;
            }
        }

        if(channel->state == ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN){
            err=SSH_OK;
        }

    break;
    case ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN:
        LOG(LOG_INFO, "ssh_channel_open_auth_agent SSH_CHANNEL_STATE_OPEN");
        // shouldn't it be an error: looks like double opening of a channel ?
        err = SSH_OK;
    break;
    case ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_CLOSED:
    case ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN_DENIED:
        LOG(LOG_INFO, "ssh_channel_open_auth_agent SSH_CHANNEL_STATE_CLOSED/DENIED");
        err = SSH_ERROR;
        break;
    default:
        ssh_set_error(this->error, SSH_FATAL,"Bad state in channel_open: %d", int(channel->state));
        err = SSH_ERROR;
    }
    return err;
}

// ==================================== SSH_MSG_DISCONNECT ================================

//    [RFC4253] 11.1.  Disconnection Message
//    ======================================

//          byte      SSH_MSG_DISCONNECT
//          uint32    reason code
//          string    description in ISO-10646 UTF-8 encoding [RFC3629]
//          string    language tag [RFC3066]

//       This message causes immediate termination of the connection.  All
//       implementations MUST be able to process this message; they SHOULD be
//       able to send this message.

//       The sender MUST NOT send or receive any data after this message, and
//       the recipient MUST NOT accept any data after receiving this message.
//       The Disconnection Message 'description' string gives a more specific
//       explanation in a human-readable form.  The Disconnection Message
//       'reason code' gives the reason in a more machine-readable format
//       (suitable for localization), and can have the values as displayed in
//       the table below.  Note that the decimal representation is displayed
//       in this table for readability, but the values are actually uint32
//       values.

//               Symbolic name                                reason code
//               -------------                                -----------
//          SSH_DISCONNECT_HOST_NOT_ALLOWED_TO_CONNECT             1
//          SSH_DISCONNECT_PROTOCOL_ERROR                          2
//          SSH_DISCONNECT_KEY_EXCHANGE_FAILED                     3
//          SSH_DISCONNECT_RESERVED                                4
//          SSH_DISCONNECT_MAC_ERROR                               5
//          SSH_DISCONNECT_COMPRESSION_ERROR                       6
//          SSH_DISCONNECT_SERVICE_NOT_AVAILABLE                   7
//          SSH_DISCONNECT_PROTOCOL_VERSION_NOT_SUPPORTED          8
//          SSH_DISCONNECT_HOST_KEY_NOT_VERIFIABLE                 9
//          SSH_DISCONNECT_CONNECTION_LOST                        10
//          SSH_DISCONNECT_BY_APPLICATION                         11
//          SSH_DISCONNECT_TOO_MANY_CONNECTIONS                   12
//          SSH_DISCONNECT_AUTH_CANCELLED_BY_USER                 13
//          SSH_DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE         14
//          SSH_DISCONNECT_ILLEGAL_USER_NAME                      15

//       If the 'description' string is displayed, the control character
//       filtering discussed in [SSH-ARCH] should be used to avoid attacks by
//       sending terminal control characters.

//       Requests for assignments of new Disconnection Message 'reason code'
//       values (and associated 'description' text) in the range of 0x00000010
//       to 0xFDFFFFFF MUST be done through the IETF CONSENSUS method, as
//       described in [RFC2434].  The Disconnection Message 'reason code'
//       values in the range of 0xFE000000 through 0xFFFFFFFF are reserved for
//       PRIVATE USE.  As noted, the actual instructions to the IANA are in
//       [SSH-NUMBERS].


void ssh_disconnect_server(SshServerSession * server_session) {
    LOG(LOG_INFO, "%s ---", __FUNCTION__);

    if (server_session->socket != nullptr && server_session->socket->fd_in != INVALID_SOCKET) {
        server_session->out_buffer->out_uint8(SSH_MSG_DISCONNECT);
        server_session->out_buffer->out_uint32_be(SSH2_DISCONNECT_BY_APPLICATION);
        server_session->out_buffer->out_length_prefixed_cstr("Bye Bye");
        // TODO: we are not sending the packet... unlikely correct
        // we should send the disconnect message then wait for socket close
    }

    server_session->opts.fd = INVALID_SOCKET;
    server_session->session_state = SSH_SESSION_STATE_DISCONNECTED;

    if(server_session->current_crypto){
        delete server_session->current_crypto;
        server_session->current_crypto = nullptr;
    }
    if(server_session->in_buffer){
        server_session->in_buffer->buffer_reinit();
    }
    if(server_session->out_buffer){
        server_session->out_buffer->buffer_reinit();
    }
    server_session->auth_methods = 0;
}

inline int ssh_auth_reply_success_server(SshServerSession * server_session) {
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    server_session->out_buffer->out_uint8(SSH_MSG_USERAUTH_SUCCESS);
    server_session->packet_send();
    return SSH_OK;
}

/**
 * @brief remove the poll handle from session and assign them to a event,
 * when used in blocking mode.
 *
 * @param ctx     The poll_ctx_struct object
 * @param server_session   The session to add to the event.
 *
 */
void ssh_event_set_session_server(ssh_poll_ctx_struct * ctx, SshServerSession * server_session)
{
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    ctx->front_session = server_session;
}


/**
 * @brief Set temporary prompts for keyboard-interactive callback.
 *
 * @param[in] server_session      The ssh session to use.
 *
 * @param[in] name         The name.
 *
 * @param[in] instruction  The Instruction in UTF-8.
 *
 * @param[in] num_prompts  Number of prompts.
 *
 * @param[in] prompts      List of prompts in UTF-8 of size num_prompts.
 *
 * @param[in] echo         List of echo mode for each prompt.
 *
 * @returns                Status.
 */
int ssh_userauth_kbdint_settmpprompts_server(SshServerSession * server_session, const char * name,
                                      const char * instruction, unsigned int num_prompts,
                                      const char ** prompts, unsigned char * echo) {
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    unsigned int i = 0;
    LOG(LOG_INFO, "set prompt %d %s", num_prompts, prompts[0]);
    if (server_session->tmp_kbdint == nullptr) {
        LOG(LOG_INFO, "%s A ---", __FUNCTION__);
        server_session->tmp_kbdint = new ssh_kbdint_struct;
        // TODO : check memory allocation
    }
    else {
        LOG(LOG_INFO, "%s B ---", __FUNCTION__);
        ssh_kbdint_clean(server_session->tmp_kbdint);
    }

    LOG(LOG_INFO, "%s C ---", __FUNCTION__);

    server_session->tmp_kbdint->name = strdup(name);
    // TODO : check memory allocation
    server_session->tmp_kbdint->instruction = strdup(instruction);
    // TODO : check memory allocation

    server_session->tmp_kbdint->nprompts = num_prompts;
    if(num_prompts > 0) {
        server_session->tmp_kbdint->prompts = static_cast<char**>(malloc(num_prompts * sizeof(char *)));
        // TODO : check memory allocation
        server_session->tmp_kbdint->echo = static_cast<unsigned char*>(malloc(num_prompts * sizeof(unsigned char)));
        // TODO : check memory allocation
        for (i = 0; i < num_prompts; i++) {
            server_session->tmp_kbdint->echo[i] = echo[i];
            server_session->tmp_kbdint->prompts[i] = strdup(prompts[i]);
            // TODO : check memory allocation
        }
    }
    else {
        server_session->tmp_kbdint->prompts = nullptr;
        server_session->tmp_kbdint->echo = nullptr;
    }
    return SSH_OK;
}


/**
 * @brief Set temporary unique (or none) prompt for keyboard-interactive callback.
 *
 * @param[in] server_session      The ssh session to use.
 *
 * @param[in] name         The name.
 *
 * @param[in] instruction  The Instruction in UTF-8.
 *
 * @param[in] prompt       The prompts in UTF-8 (can be nullptr).
 *
 * @param[in] echo         Echo mode for the prompt.
 *
 * @returns                Status.
 */
int ssh_userauth_kbdint_settmpprompt_server(SshServerSession * server_session, const char * name,
                                     const char * instruction, const char * prompt,
                                     unsigned char echo, error_struct * error) {
    (void)error;
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    unsigned int num_prompts = 0;
    const char ** prompts = nullptr;
    unsigned char * echos = nullptr;
    if (prompt != nullptr) {
        num_prompts = 1;
        prompts = &prompt;
        echos = &echo;
    }
    return ssh_userauth_kbdint_settmpprompts_server(server_session, name, instruction, num_prompts, prompts, echos);
}


/**
 * @brief Get the number of answers the client has given.
 *
 * @param[in]  server_session  The ssh session to use.
 *
 * @returns             The number of answers.
 */
int ssh_userauth_kbdint_getnanswers_server(SshServerSession * server_session, error_struct * error) {
    (void)error;
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    if(server_session == nullptr || server_session->kbdint == nullptr){
        return SSH_ERROR;
    }
    return server_session->kbdint->nanswers;
}

/**
 * @brief Get the answer for a question from a message block.
 *
 * @param[in]  server_session  The ssh session to use.
 *
 * @param[in]  i index  The number of the ith answer.
 *
 * @return              0 on success, < 0 on error.
 */
const char *ssh_userauth_kbdint_getanswer_server(SshServerSession * server_session, unsigned int i, error_struct * error) {
    (void)error;
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    if (server_session == nullptr
    || server_session->kbdint == nullptr
    || server_session->kbdint->answers == nullptr) {
        return nullptr;
    }
    if (i >= server_session->kbdint->nanswers) {
        return nullptr;
    }

    return server_session->kbdint->answers[i];
}




/**
 * @brief Close and free a channel.
 *
 * @param[in]  channel  The channel to free.
 *
 * @warning Any data unread on this channel will be lost.
 */
void ssh_channel_free_server(SshServerSession * server_session, ssh_channel channel) {
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    if (channel == nullptr) {
        return;
    }

    if (channel->state == ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN) {
        ssh_channel_close_server(server_session, channel);
    }
    channel->flags |= SSH_CHANNEL_FLAG_FREED_LOCAL;

    /* The idea behind the flags is the following : it is well possible
     * that a client closes a channel that stills exists on the server side.
     * We definitively close the channel when we receive a close message *and*
     * the user closed it.
     */
    if((channel->flags & SSH_CHANNEL_FLAG_CLOSED_REMOTE)
       || (channel->flags & SSH_CHANNEL_FLAG_NOT_BOUND)){
        for (unsigned i = 0; i < server_session->nbchannels ; i++){
            if (server_session->channels[i] == channel){
                server_session->nbchannels--;
                server_session->channels[i] = server_session->channels[server_session->nbchannels];
                delete channel;
                break;
            }
        }
    }
}


/** @brief returns the client credentials of the connected client.
 * If the client has given a forwardable token, the SSH server will
 * retrieve it.
 * @returns gssapi credentials handle.
 * @returns nullptr if no forwardable token is available.
 */
ssh_gssapi_creds ssh_gssapi_get_creds_server(SshServerSession * server_session){
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    if (!server_session || !server_session->gssapi || server_session->gssapi->client_creds == GSS_C_NO_CREDENTIAL)
        return nullptr;
    return static_cast<ssh_gssapi_creds>(server_session->gssapi->client_creds);
}


/**
 * @brief Open a X11 channel.
 *
 * @param[in]  channel      An allocated channel.
 *
 * @param[in]  orig_addr    The source host (the local server).
 *
 * @param[in]  orig_port    The source port (the local server).
 *
 * @warning This function does not bind the local port and does not automatically
 *          forward the content of a socket to the channel. You still have to
 *          use channel_read and channel_write for this.
 */
void ssh_channel_open_x11_server(SshServerSession * server_session, ssh_channel channel, const char *orig_addr, int orig_port)
{
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    LOG(LOG_INFO, "ssh_channel_open_x11_server %s %s", channel->show(), server_session->show());

    if(channel == nullptr) {
        LOG(LOG_INFO, "ssh_channel_open_x11_server E1");
        return;
    }

    if ((orig_addr == nullptr)
    ||  (channel->state != ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_NOT_OPEN)){
            channel->callbacks->channel_open_x11_server_status_function(
                                server_session,
                                channel,
                                SSH_ERROR,
                                channel->callbacks->userdata);
        LOG(LOG_INFO, "ssh_channel_open_x11_server E2");
        return;
    }


    channel->local_channel = server_session->new_channel_id();
    channel->local_maxpacket = CHANNEL_MAX_PACKET;
    channel->local_window = CHANNEL_INITIAL_WINDOW;
    channel->state = ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPENING;

    LOG(LOG_INFO, "Creating channel #%d of type x11 with %d window and %d max packet",
                channel->local_channel, channel->local_window, channel->local_maxpacket);


    // TODO: shouldn't we copy the channel type locally (for debug purpose ?)
    server_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_OPEN);
    server_session->out_buffer->out_length_prefixed_cstr("x11");
    server_session->out_buffer->out_uint32_be(channel->local_channel);
    server_session->out_buffer->out_uint32_be(channel->local_window);
    server_session->out_buffer->out_uint32_be(channel->local_maxpacket);
    server_session->out_buffer->out_length_prefixed_cstr(orig_addr);
    server_session->out_buffer->out_uint32_be(orig_port);
    server_session->packet_send();

    class Event_ssh_channel_open_x11_server : public Event
    {
        ssh_session_struct * server_session;
        ssh_channel_struct * channel;
    public:
        Event_ssh_channel_open_x11_server(ssh_session_struct * server_session, ssh_channel_struct * channel)
            : server_session(server_session)
            , channel(channel)
        {
        }

        virtual ~Event_ssh_channel_open_x11_server(void)
        {
        }

        int trigger() override
        {
            LOG(LOG_INFO, "Event_ssh_channel_open_x11_server trigger");
            return this->channel->state != ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPENING;
        }

        void action() override
        {
            LOG(LOG_INFO, "Event_ssh_channel_open_x11_server action");
            channel->callbacks->channel_open_x11_server_status_function(
                                    this->server_session,
                                    this->channel,
                                   (this->channel->state
                                        == ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN)
                                            ? SSH_OK : SSH_ERROR,
                                    channel->callbacks->userdata);
        }
    } * event = new Event_ssh_channel_open_x11_server(server_session, channel);

    server_session->add_event(event);

    LOG(LOG_INFO, "ssh_channel_open_x11_server done loop");
    return;
}

/* TODO FIXME Fix the delayed close thing */
/* TODO FIXME Fix the blocking behaviours */

/**
 * @brief Reads data from a channel.
 *
 * @param[in]  channel  The channel to read from.
 *
 * @param[in]  dest     The destination buffer which will get the data.
 *
 * @param[in]  count    The count of bytes to be read.
 *
 * @return              The number of bytes read, 0 on end of file or SSH_ERROR
 *                      on error. In nonblocking mode it Can return 0 if no data
 *                      is available or SSH_AGAIN.
 *
 * @warning This function may return less than count bytes of data, and won't
 *          block until count bytes have been read.
 * @warning The read function using a buffer has been renamed to
 *          channel_read_buffer().
 */
int ssh_channel_read_stdout_server(ssh_session_struct * server_session, ssh_channel channel, void *dest, uint32_t count)
{
//    LOG(LOG_WARNING, "ssh_channel_read channel=%s", channel->show());
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    uint32_t len;
    int rc;

    if(channel == nullptr) {
        return SSH_ERROR;
    }
    if(dest == nullptr) {
        ssh_set_error(server_session->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return SSH_ERROR;
    }

    if (count == 0) {
        return 0;
    }

    /*
     * We may have problem if the window is too small to accept as much data
     * as asked
     */

    unsigned minimumsize = count - channel->stdout_buffer->in_remain();
    if (minimumsize > channel->local_window) {
        unsigned new_window = (minimumsize > WINDOWBASE) ? minimumsize : WINDOWBASE;
        if (new_window > channel->local_window){
            /* WINDOW_ADJUST packet needs a relative increment rather than an absolute
             * value, so we give here the missing bytes needed to reach new_window
             */
            server_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_WINDOW_ADJUST);
            server_session->out_buffer->out_uint32_be(channel->remote_channel);
            server_session->out_buffer->out_uint32_be(new_window - channel->local_window);
            server_session->packet_send();
            channel->local_window = new_window - channel->local_window ;
        }
    }

    /* block reading until at least one byte has been read
     *  and ignore the trivial case count=0
     */
     // CGR: why do we reset count to 1 ? Didn't the external caller asked for a specific count ?
//    count = 1;

    rc = SSH_OK;

    if ((server_session->flags & SSH_SESSION_FLAG_BLOCKING)){
        while (channel->stdout_buffer->in_remain() < count) {
            if (channel->remote_eof) {
                break;
            }
            if (server_session->session_state == SSH_SESSION_STATE_ERROR) {
                break;
            }
            dopoll(server_session->ctx, SSH_TIMEOUT_INFINITE);
            if (server_session->session_state == SSH_SESSION_STATE_ERROR) {
                break;
            }
        }
    }
    else {
            struct timeval start;
            gettimeofday(&start, nullptr);
            int timeout = TIMEOUT_DEFAULT_MS;

            while(channel->stdout_buffer->in_remain() < count){
                if (channel->remote_eof) {
                    break;
                }
                if (server_session->session_state == SSH_SESSION_STATE_ERROR) {
                    break;
                }
                dopoll(server_session->ctx, timeout);
                if (server_session->session_state == SSH_SESSION_STATE_ERROR) {
                    break;
                }
                struct timeval now;
                gettimeofday(&now, nullptr);
                long ms =   (now.tv_sec - start.tv_sec) * 1000
                        + (now.tv_usec < start.tv_usec) * 1000
                        + (now.tv_usec - start.tv_usec) / 1000;

                if (ms >= timeout) {
                    rc = (channel->stdout_buffer->in_remain() >= count
                        || channel->remote_eof
                        || server_session->session_state == SSH_SESSION_STATE_ERROR) ? SSH_OK : SSH_AGAIN;
                    break;
                }
                timeout -= ms;
            }
    }
    if (rc == SSH_ERROR
    || server_session->session_state == SSH_SESSION_STATE_ERROR){
        return SSH_ERROR;
    }

    if (channel->remote_eof && channel->stdout_buffer->in_remain() == 0) {
        return -1;
    }

    len = channel->stdout_buffer->in_remain();
    /* Read count bytes if len is greater, everything otherwise */
    len = (len > count ? count : len);

    memcpy(dest, channel->stdout_buffer->get_pos_ptr(), len);
    channel->stdout_buffer->in_skip_bytes(len);

    /* Authorize some buffering while userapp is busy */
    if ((channel->local_window < WINDOWLIMIT)
    && (WINDOWBASE > channel->local_window)) {
        /* WINDOW_ADJUST packet needs a relative increment rather than an absolute
         * value, so we give here the missing bytes needed to reach new_window
         */
        server_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_WINDOW_ADJUST);
        server_session->out_buffer->out_uint32_be(channel->remote_channel);
        server_session->out_buffer->out_uint32_be(WINDOWBASE - channel->local_window);
        server_session->packet_send();
        channel->local_window = WINDOWBASE - channel->local_window ;
    }
    return len;
}


int channel_write_common_server(ssh_session_struct * server_session, ssh_channel channel, const uint8_t *data, uint32_t len) {
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    uint32_t origlen = len;

    /*    LOG(LOG_INFO, "channel write len=%d", len);*/
    /*
     * Handle the max packet len from remote side, be nice
     * 10 bytes for the headers
     */
    size_t maxpacketlen = channel->remote_maxpacket - 10;

    if (channel->local_eof) {
        ssh_set_error(server_session->error, SSH_REQUEST_DENIED,
                      "Can't write to channel %d:%d  after EOF was sent",
                      channel->local_channel,
                      channel->remote_channel);
        server_session->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_ERROR;
    }

    if (channel->state != ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN
    || channel->delayed_close != 0) {
        LOG(LOG_INFO, "Remote channel is closed");
        ssh_set_error(server_session->error, SSH_REQUEST_DENIED, "Remote channel is closed");
        return SSH_ERROR;
    }

    if (server_session->session_state == SSH_SESSION_STATE_ERROR){
        return SSH_ERROR;
    }

    while (len > 0) {
        while (channel->remote_window == 0) {
            if (server_session->session_state == SSH_SESSION_STATE_ERROR){
                LOG(LOG_INFO, "Wait for a growing window message terminated on error: exiting");
                return SSH_ERROR;
            }
            if (server_session->socket == nullptr){
                return SSH_ERROR;
            }

            // Waiting for input
            dopoll(server_session->ctx, (server_session->flags&SSH_SESSION_FLAG_BLOCKING)?SSH_TIMEOUT_INFINITE:SSH_TIMEOUT_NONBLOCKING);

            if (server_session->session_state == SSH_SESSION_STATE_ERROR
            || server_session->session_state == SSH_SESSION_STATE_ERROR){
                LOG(LOG_INFO, "Wait for a growing window message terminated on error: exiting");
                return SSH_ERROR;
            }
        }
        uint32_t effectivelen = len > channel->remote_window ? channel->remote_window : len;
        if (effectivelen > maxpacketlen) {
            effectivelen = maxpacketlen;
        }

        server_session->out_buffer->out_uint8(SSH_MSG_CHANNEL_DATA);
        server_session->out_buffer->out_uint32_be(channel->remote_channel);
        server_session->out_buffer->out_uint32_be(effectivelen);
        server_session->out_buffer->out_blob(data, effectivelen);

        server_session->packet_send();

        channel->remote_window -= effectivelen;
        len -= effectivelen;
        data = data + effectivelen;
    }
    return static_cast<int>(origlen - len);

}


/**
 * @internal
 *
 * @brief Send a global request (needed for forward listening) and wait for the
 * result.
 *
 * @param[in]  request  The type of request (defined in RFC).
 *
 * @param[in]  buffer   Additional data to put in packet.
 *
 * @param[in]  reply    Set if you expect a reply from server.
 *
 * @return              SSH_OK on success,
 *                      SSH_ERROR if an error occurred,
 *                      SSH_AGAIN if in nonblocking mode and call has
 *                      to be done again.
 */
int SshServerSession::global_request_server(const char *request, ssh_buffer_struct* buffer, int reply) {
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    LOG(LOG_INFO, "> GLOBAL_REQUEST %s", request);
    int rc = SSH_OK;

    if (this->global_req_state == SSH_CHANNEL_REQ_STATE_NONE){
        this->out_buffer->out_uint8(SSH_MSG_GLOBAL_REQUEST);
        this->out_buffer->out_length_prefixed_cstr(request);
        this->out_buffer->out_uint8(reply != 0);

        if (buffer != nullptr) {
            this->out_buffer->out_blob(buffer->get_pos_ptr(), buffer->in_remain());
        }

        this->global_req_state = SSH_CHANNEL_REQ_STATE_PENDING;
        this->packet_send();

        LOG(LOG_INFO,
                "Sent a SSH_MSG_GLOBAL_REQUEST %s", request);

        if (reply == 0) {
            this->global_req_state = SSH_CHANNEL_REQ_STATE_NONE;

            return SSH_OK;
        }
    }

    if (this->session_state == SSH_SESSION_STATE_ERROR){
        this->global_req_state = SSH_CHANNEL_REQ_STATE_NONE;
        return SSH_ERROR;
    }

    while(1){
        if (this->global_req_state != SSH_CHANNEL_REQ_STATE_PENDING){
            break;
        }

        // Waiting for input
        dopoll(this->ctx, (this->flags&SSH_SESSION_FLAG_BLOCKING)?SSH_TIMEOUT_INFINITE:SSH_TIMEOUT_NONBLOCKING);

        if (this->session_state == SSH_SESSION_STATE_ERROR){
            this->global_req_state = SSH_CHANNEL_REQ_STATE_NONE;
            return SSH_ERROR;
        }

        if (!(this->flags & SSH_SESSION_FLAG_BLOCKING)){
            break;
        }
    }

    switch(this->global_req_state){
    case SSH_CHANNEL_REQ_STATE_ACCEPTED:
        LOG(LOG_INFO, "Global request %s success",request);
        rc = SSH_OK;
        break;
    case SSH_CHANNEL_REQ_STATE_DENIED:
        LOG(LOG_INFO, "Global request %s failed", request);
        ssh_set_error(this->error, SSH_REQUEST_DENIED,
                      "Global request %s failed", request);
        rc = SSH_ERROR;
        break;
    case SSH_CHANNEL_REQ_STATE_ERROR:
    case SSH_CHANNEL_REQ_STATE_NONE:
        rc = SSH_ERROR;
        break;
    case SSH_CHANNEL_REQ_STATE_PENDING:
        return SSH_AGAIN;
    }
    this->global_req_state = SSH_CHANNEL_REQ_STATE_NONE;
    return rc;
}

/**
 * @brief Blocking write on a channel stderr.
 *
 * @param[in]  channel  The channel to write to.
 *
 * @param[in]  data     A pointer to the data to write.
 *
 * @param[in]  len      The length of the buffer to write to.
 *
 * @return              The number of bytes written, SSH_ERROR on error.
 *
 */
int SshServerSession::ssh_channel_write_stderr_server(ssh_channel channel, const uint8_t *data, uint32_t len)
{
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    if(channel == nullptr) {
        LOG(LOG_WARNING, "Invalid channel");
        return SSH_ERROR;
    }
    if(data == nullptr) {
        LOG(LOG_WARNING, "Invalid data");
        return SSH_ERROR;
    }

    if (len > INT_MAX) {
        LOG(LOG_WARNING, "Length (%u) is bigger than INT_MAX", len);
        return SSH_ERROR;
    }

    uint32_t origlen = len;

    /*    LOG(LOG_INFO, "channel write len=%d", len);*/
    /*
     * Handle the max packet len from remote side, be nice
     * 10 bytes for the headers
     */
    size_t maxpacketlen = channel->remote_maxpacket - 10;

    if (channel->local_eof) {
        ssh_set_error(this->error, SSH_REQUEST_DENIED,
                      "Can't write to channel %d:%d  after EOF was sent",
                      channel->local_channel,
                      channel->remote_channel);
        this->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_ERROR;
    }

    if (channel->state != ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN
    || channel->delayed_close != 0) {
        LOG(LOG_INFO, "Remote channel is closed");
        ssh_set_error(this->error, SSH_REQUEST_DENIED, "Remote channel is closed");
        return SSH_ERROR;
    }

    if (this->session_state == SSH_SESSION_STATE_ERROR){
        return SSH_ERROR;
    }

    while (len > 0) {
        while (channel->remote_window == 0) {
            if (this->session_state == SSH_SESSION_STATE_ERROR){
                LOG(LOG_INFO, "Wait for a growing window message terminated on error: exiting");
                return SSH_ERROR;
            }
            LOG(LOG_WARNING, "Waiting for growing window Call to handle_packets session_state=%d channel_state=%d", this->session_state, static_cast<int>(channel->state));
            if (this->socket == nullptr){
                return SSH_ERROR;
            }

            // Waiting for input
            dopoll(this->ctx, (this->flags&SSH_SESSION_FLAG_BLOCKING)?SSH_TIMEOUT_INFINITE:SSH_TIMEOUT_NONBLOCKING);

            if (this->session_state == SSH_SESSION_STATE_ERROR){
                LOG(LOG_INFO, "Wait for a growing window message terminated on error: exiting");
                return SSH_ERROR;
            }
        }
        uint32_t effectivelen = len > channel->remote_window ? channel->remote_window : len;
        if (effectivelen > maxpacketlen) {
            effectivelen = maxpacketlen;
        }

        /* stderr message has an extra field */
        this->out_buffer->out_uint8(SSH_MSG_CHANNEL_EXTENDED_DATA);
        this->out_buffer->out_uint32_be(channel->remote_channel);
        this->out_buffer->out_uint32_be(SSH2_EXTENDED_DATA_STDERR);
        this->out_buffer->out_uint32_be(effectivelen);
        this->out_buffer->out_blob(data, effectivelen);

        this->packet_send();

//        LOG(LOG_INFO,
//                "channel_write wrote %ld bytes", (long int) effectivelen);

        channel->remote_window -= effectivelen;
        len -= effectivelen;
        data += effectivelen;
    }
    return static_cast<int>(origlen - len);
}


/**
 * @brief Open a TCP/IP reverse forwarding channel.
 *
 * @param[in]  channel  An allocated channel.
 *
 * @param[in]  remotehost The remote host to connected (host name or IP).
 *
 * @param[in]  remoteport The remote port.
 *
 * @param[in]  sourcehost The source host (your local computer). It's optional
 *                        and for logging purpose.
 *
 * @param[in]  localport  The source port (your local computer). It's optional
 *                        and for logging purpose.
 *
 * @return              SSH_OK on success,
 *                      SSH_ERROR if an error occurred,
 *                      SSH_AGAIN if in nonblocking mode and call has
 *                      to be done again.
 *
 * @warning This function does not bind the local port and does not automatically
 *          forward the content of a socket to the channel. You still have to
 *          use channel_read and channel_write for this.
 */
int SshServerSession::ssh_channel_open_reverse_forward_server(ssh_channel channel, const char *remotehost,
                                     int remoteport, const char *sourcehost, int localport) {
    LOG(LOG_INFO, "%s ---", __FUNCTION__);
    int err = SSH_ERROR;

    if(channel == nullptr) {
        return err;
    }
    if(remotehost == nullptr || sourcehost == nullptr) {
        ssh_set_error(this->error, SSH_FATAL, "Invalid argument in %s", __FUNCTION__);
        return err;
    }

    if (channel->state != ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_NOT_OPEN){
        return SSH_ERROR;
    }


    channel->channel_open(this->new_channel_id(),
        "forwarded-tcpip", CHANNEL_MAX_PACKET, CHANNEL_INITIAL_WINDOW, this->out_buffer);

    this->out_buffer->out_length_prefixed_cstr(remotehost);
    this->out_buffer->out_uint32_be(remoteport);
    this->out_buffer->out_length_prefixed_cstr(sourcehost);
    this->out_buffer->out_uint32_be(localport);

    this->packet_send();

    /* wait until channel is opened by server */
    err = SSH_OK;
    while(1) {
        if (this->session_state == SSH_SESSION_STATE_ERROR){
            err = SSH_ERROR;
            break;
        }
        if (channel->state != ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPENING){
            err = SSH_OK;
            break;
        }

        // Waiting for input
        dopoll(this->ctx,
            (this->flags&SSH_SESSION_FLAG_BLOCKING)
                ?SSH_TIMEOUT_INFINITE
                :SSH_TIMEOUT_NONBLOCKING);
        if (this->session_state == SSH_SESSION_STATE_ERROR){
            break;
        }
        if (channel->state == ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPENING) {
            err = SSH_AGAIN;
        }
        if (!(this->flags&SSH_SESSION_FLAG_BLOCKING)){
            break;
        }
        if (this->session_state == SSH_SESSION_STATE_ERROR){
            err = SSH_ERROR;
            break;
        }
    }

    if(channel->state == ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN){
        err=SSH_OK;
    }
    return err;
}

REDEMPTION_DIAGNOSTIC_POP
