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
   Copyright (c) 2003-2013 by Aris Adamantiadis
   Copyright (c) 2009-2013 by Andreas Schneider <asn@cryptomilk.org>
*/

#include "sashimi/libssh/libssh.h"

#include "sashimi/buffer.hpp"
#include "sashimi/channels.hpp"

/**
 * @brief Allocate a new channel.
 *
 * @param[in]  session  The ssh session to use.
 *
 * @return              A pointer to a newly allocated channel, NULL on error.
 */
ssh_channel ssh_new_channel(ssh_session_struct * session, ssh_channel_callbacks cb)
{
    if(session == NULL) { return NULL; }

    ssh_channel_struct * channel = new ssh_channel_struct(session, cb, 0);
    channel->version = session->version;
    session->channels[session->nbchannels++] = channel;
    return channel;
}

void ssh_set_blocking(ssh_session_struct * session, int blocking) {
    if (!session) {
        syslog(LOG_INFO, "%s --- NULL SESSION", __FUNCTION__);
        return;
  }
  session->flags &= ~SSH_SESSION_FLAG_BLOCKING;
  session->flags |= blocking ? SSH_SESSION_FLAG_BLOCKING : 0;
}

void ssh_free(ssh_session_struct * session)
{
    delete session;
}

/**
 * @brief  Poll all the sockets and sessions associated through an event object.
 *         If any of the events are set after the poll, the
 *         call back functions of the sessions or sockets will be called.
 *         This function should be called once within the programs main loop.
 *
 * @param  ctx          The polling context
 * @param  timeout      An upper limit on the time for which the poll will
 *                      block, in milliseconds. Specifying a negative value
 *                      means an infinite timeout. This parameter is passed to
 *                      the poll() function.
 * @returns SSH_OK      No error.
 *          SSH_ERROR   Error happened during the poll.
 */
int ssh_event_dopoll(ssh_poll_ctx_struct * ctx, int timeout) {
    syslog(LOG_INFO, "%s ---", __FUNCTION__);

    if(ctx == NULL) {
        return SSH_ERROR;
    }
    return dopoll(ctx, timeout);
}
void ssh_event_set_session_client(ssh_poll_ctx_struct * ctx, ssh_session_struct * session)
{
    ssh_event_set_session_client(ctx, static_cast<SshClientSession*>(session));
}

int ssh_channel_open_auth_agent_server(ssh_session_struct * server_session, ssh_channel_struct * channel)
{
    return static_cast<SshServerSession*>(server_session)->ssh_channel_open_auth_agent_server(channel);
}

void ssh_event_set_session_server(ssh_poll_ctx_struct * ctx, ssh_session_struct * session)
{
    return ssh_event_set_session_server(ctx, static_cast<SshServerSession*>(session));
}

void ssh_disconnect_server(ssh_session_struct * session)
{
    return ssh_disconnect_server(static_cast<SshServerSession*>(session));
}

void ssh_disconnect_client(ssh_session_struct * client_session)
{
    return ssh_disconnect_client(static_cast<SshClientSession*>(client_session));
}

int ssh_userauth_none_client(ssh_session_struct * client_session, const char *username, error_struct * error)
{
    return static_cast<SshClientSession*>(client_session)->ssh_userauth_none_client(username, error);
}

int ssh_userauth_kbdint_settmpprompts_server(ssh_session_struct * server_session, const char * name,
                          const char * instruction, unsigned int num_prompts,
                          const char ** prompts, unsigned char * echo)
{
    return ssh_userauth_kbdint_settmpprompts_server(static_cast<SshServerSession*>(server_session), name, instruction, num_prompts, prompts, echo);
}

int ssh_userauth_kbdint_settmpprompt_server(ssh_session_struct * server_session, const char * name,
                         const char * instruction, const char * prompt,
                         unsigned char echo, error_struct * error)
{
    if (server_session==NULL) {
        return SSH_ERROR;
    }
    return ssh_userauth_kbdint_settmpprompt_server(static_cast<SshServerSession*>(server_session), name, instruction, prompt, echo, error);
}

int ssh_userauth_kbdint_getnanswers_server(ssh_session_struct * server_session, error_struct * error)
{
    return ssh_userauth_kbdint_getnanswers_server(static_cast<SshServerSession*>(server_session), error);
}

const char *ssh_userauth_kbdint_getanswer_server(ssh_session_struct * server_session, unsigned int i, error_struct * error)
{
    return ssh_userauth_kbdint_getanswer_server(static_cast<SshServerSession*>(server_session), i, error);
}

int ssh_userauth_list_client(ssh_session_struct * client_session, const char *username, error_struct * error)
{
    (void)username;
    if (client_session == NULL) {
        return 0;
    }
    return ssh_userauth_list_client(
        static_cast<SshClientSession*>(client_session), error);
}

int ssh_userauth_agent_client(ssh_session_struct * client_session, ssh_session_struct * front_session, const char *username, error_struct * error)
{
    return ssh_userauth_agent_client(
        static_cast<SshClientSession*>(client_session),
        static_cast<SshServerSession*>(front_session), username, error);
}

int ssh_userauth_password_client(ssh_session_struct * client_session,
                          const char *username,
                          const char *password,
                          error_struct * error)
{
    return ssh_userauth_password_client(static_cast<SshClientSession*>(client_session), username, password, error);
}

int ssh_userauth_kbdint_client(ssh_session_struct * client_session, const char *user, const char *submethods, error_struct * error)
{
    return ssh_userauth_kbdint_client(static_cast<SshClientSession*>(client_session), user, submethods, error);
}

int ssh_userauth_kbdint_getnprompts_client(ssh_session_struct * client_session)
{
    return ssh_userauth_kbdint_getnprompts_client(static_cast<SshClientSession*>(client_session));
}

int ssh_userauth_gssapi_client(ssh_session_struct * client_session, error_struct * error)
{
    return ssh_userauth_gssapi_client(static_cast<SshClientSession*>(client_session), error);
}

int ssh_userauth_kbdint_setanswer_client(ssh_session_struct * client_session, unsigned int i, const char *answer, error_struct * error)
{
    return ssh_userauth_kbdint_setanswer_client(static_cast<SshClientSession*>(client_session), i, answer, error);
}

const char *ssh_userauth_kbdint_getprompt_client(ssh_session_struct * client_session, unsigned int i, char *echo, error_struct * error)
{
    return ssh_userauth_kbdint_getprompt_client(static_cast<SshClientSession*>(client_session), i, echo, error);
}


int ssh_sessionchannel_open_client(ssh_session_struct * client_session, ssh_channel channel)
{
    return ssh_sessionchannel_open_client(static_cast<SshClientSession*>(client_session), channel);
}

int ssh_channel_open_forward_client(ssh_session_struct * client_session, ssh_channel channel, const char *remotehost, int remoteport, const char *sourcehost, int localport)
{
    return ssh_channel_open_forward_client(static_cast<SshClientSession*>(client_session),
                                           channel,
                                           remotehost, remoteport, sourcehost, localport);
}

void ssh_channel_free_server(ssh_session_struct * server_session, ssh_channel channel)
{
    return ssh_channel_free_server(static_cast<SshServerSession*>(server_session), channel);
}

void ssh_channel_free_client(ssh_session_struct * client_session, ssh_channel channel)
{
    return ssh_channel_free_client(static_cast<SshClientSession*>(client_session), channel);
}

int ssh_channel_send_eof_client(ssh_session_struct * client_session, ssh_channel channel)
{
    return ssh_channel_send_eof_client(static_cast<SshClientSession*>(client_session), channel);
}

int ssh_channel_send_eof_server(ssh_session_struct * server_session, ssh_channel channel)
{
    return static_cast<SshServerSession*>(server_session)->ssh_channel_send_eof_server(channel);
}

int ssh_channel_close_client(ssh_session_struct * client_session, ssh_channel channel)
{
    return ssh_channel_close_client(static_cast<SshClientSession*>(client_session), channel);
}

int ssh_channel_close_server(ssh_session_struct * server_session, ssh_channel channel)
{
    return static_cast<SshServerSession*>(server_session)->ssh_channel_close_server(channel);
}

int ssh_get_server_publickey_hash_value_client(const ssh_session_struct * client_session,
                                                   enum ssh_publickey_hash_type type,
                                                   unsigned char *buf,
                                                   size_t *hlen, error_struct * error)
{
    return ssh_get_server_publickey_hash_value_client(
                static_cast<const SshClientSession*>(client_session), type, buf, hlen, error);
}


int ssh_set_agent_channel_client(ssh_session_struct * client_session, ssh_channel channel)
{
    return ssh_set_agent_channel_client(static_cast<SshClientSession*>(client_session), channel);
}

void ssh_gssapi_set_creds_client(ssh_session_struct * client_session, const ssh_gssapi_creds creds)
{
    return ssh_gssapi_set_creds_client(static_cast<SshClientSession*>(client_session), creds);
}

int ssh_channel_request_env_client(ssh_session_struct * client_session, ssh_channel channel, const char *name, const char *value)
{
    return ssh_channel_request_env_client(static_cast<SshClientSession*>(client_session), channel, name, value);
}

ssh_gssapi_creds ssh_gssapi_get_creds_server(ssh_session_struct * server_session)
{
    return ssh_gssapi_get_creds_server(static_cast<SshServerSession*>(server_session));
}

void ssh_channel_open_x11_server(ssh_session_struct * server_session, ssh_channel channel, const char *orig_addr, int orig_port)
{
    return ssh_channel_open_x11_server(static_cast<SshServerSession*>(server_session), channel, orig_addr, orig_port);
}

void ssh_event_x11_requested_channel_client(ssh_session_struct * client_session, ssh_channel channel, uint32_t sender, uint32_t window, uint32_t packet_size)
{
    return static_cast<SshClientSession*>(client_session)->ssh_event_x11_requested_channel_client(channel, sender, window, packet_size);
}

void ssh_event_x11_requested_channel_failure_client(ssh_session_struct * client_session, uint32_t sender)
{
    return static_cast<SshClientSession*>(client_session)->ssh_event_x11_requested_channel_failure_client(sender);
}

int ssh_channel_request_x11_client(ssh_session_struct * client_session, ssh_channel channel, int single_connection, const char *protocol, const char *cookie, int screen_number)
{
    return ssh_channel_request_x11_client(static_cast<SshClientSession*>(client_session), channel, single_connection, protocol, cookie, screen_number);
}

int ssh_channel_request_send_signal_client(ssh_session_struct * client_session, ssh_channel channel, const char *signum)
{
    return ssh_channel_request_send_signal_client(static_cast<SshClientSession*>(client_session), channel, signum);
}

int ssh_channel_change_pty_size_client(ssh_session_struct * client_session, ssh_channel channel, int cols, int rows)
{
    return ssh_channel_change_pty_size_client(static_cast<SshClientSession*>(client_session), channel, cols, rows);
}

int ssh_channel_request_exec_client(ssh_session_struct * client_session, ssh_channel channel, const char *cmd)
{
    return ssh_channel_request_exec_client(static_cast<SshClientSession*>(client_session), channel, cmd);
}

int ssh_channel_request_pty_size_client(ssh_session_struct * client_session, ssh_channel channel, const char *term, int cols, int rows)
{
    return ssh_channel_request_pty_size_client(static_cast<SshClientSession*>(client_session), channel, term, cols, rows);
}

int ssh_channel_request_shell_client(ssh_session_struct * client_session, ssh_channel channel)
{
    return static_cast<SshClientSession*>(client_session)->ssh_channel_request_shell_client(channel);
}

int ssh_channel_write_client(ssh_session_struct * client_session, ssh_channel channel, const uint8_t *data, uint32_t len)
{
    return static_cast<SshClientSession*>(client_session)->ssh_channel_write_client(channel, data, len);
}

int ssh_channel_write_stderr_client(ssh_session_struct * client_session, ssh_channel channel, const uint8_t *data, uint32_t len)
{
    return static_cast<SshClientSession*>(client_session)->ssh_channel_write_stderr_client(channel, data, len);
}

int ssh_channel_write_server(ssh_session_struct * server_session, ssh_channel channel, const uint8_t *data, uint32_t len)
{
    error_struct * error = &server_session->error;

    // #TODO: this is not the place for these check
    // Better to do that when we get channel
    if (channel->local_eof) {
        ssh_set_error(*error, SSH_REQUEST_DENIED,
                      "Can't write to channel %d:%d  after EOF was sent",
                      channel->local_channel,
                      channel->remote_channel);
        static_cast<SshServerSession*>(server_session)->session_state = SSH_SESSION_STATE_ERROR;
        return SSH_ERROR;
    }

    if (channel->state != ssh_channel_struct::ssh_channel_state_e::SSH_CHANNEL_STATE_OPEN
    || channel->delayed_close != 0) {
        syslog(LOG_INFO, "Remote channel is closed");
        ssh_set_error(*error, SSH_REQUEST_DENIED, "Remote channel is closed");
        return SSH_ERROR;
    }

    if (static_cast<SshServerSession*>(server_session)->session_state == SSH_SESSION_STATE_ERROR){
        syslog(LOG_INFO, "Remote channel is closed : session error");
        return SSH_ERROR;
    }

    return static_cast<SshServerSession*>(server_session)->ssh_channel_write_server(channel, data, len);
}

int ssh_channel_write_stderr_server(ssh_session_struct * server_session, ssh_channel channel, const uint8_t *data, uint32_t len)
{
    return static_cast<SshServerSession*>(server_session)->ssh_channel_write_stderr_server(channel, data, len);
}

int ssh_channel_is_open_client(ssh_session_struct * client_session, ssh_channel channel)
{
    return static_cast<SshClientSession*>(client_session)->ssh_channel_is_open_client(channel);
}

int ssh_channel_is_open_server(ssh_session_struct * server_session, ssh_channel channel)
{
    return static_cast<SshServerSession*>(server_session)->ssh_channel_is_open_server(channel);
}

int ssh_forward_cancel_server(ssh_session_struct * server_session, const char *address, int port)
{
    return static_cast<SshServerSession*>(server_session)->ssh_forward_cancel_server(address, port);
}

int ssh_forward_listen_server(ssh_session_struct * server_session, const char *address, int port, int *bound_port)
{
    return static_cast<SshServerSession*>(server_session)->ssh_forward_listen_server(address, port, bound_port);
}

int ssh_channel_is_closed_client(ssh_session_struct * client_session, ssh_channel channel)
{
    return static_cast<SshClientSession*>(client_session)->ssh_channel_is_closed_client(channel);
}

int ssh_channel_is_closed_server(ssh_session_struct * server_session, ssh_channel channel)
{
    return static_cast<SshServerSession*>(server_session)->ssh_channel_is_closed_server(channel);
}

int ssh_channel_is_eof_client(ssh_session_struct * client_session, ssh_channel channel)
{
    return static_cast<SshClientSession*>(client_session)->ssh_channel_is_eof_client(channel);
}

int ssh_channel_request_auth_agent_client(ssh_session_struct * client_session, ssh_channel channel)
{
    return static_cast<SshClientSession*>(client_session)->ssh_channel_request_auth_agent_client(channel);
}

int ssh_channel_request_subsystem_client(ssh_session_struct * client_session, ssh_channel channel, const char *subsys)
{
    return static_cast<SshClientSession*>(client_session)->ssh_channel_request_subsystem_client(channel, subsys);
}

int ssh_channel_is_eof_server(ssh_session_struct * server_session, ssh_channel channel)
{
    return static_cast<SshServerSession*>(server_session)->ssh_channel_is_eof_server(channel);
}

int ssh_channel_open_reverse_forward_server(ssh_session_struct * server_session, ssh_channel channel, const char *remotehost, int remoteport, const char *sourcehost, int localport)
{
    return static_cast<SshServerSession*>(server_session)->ssh_channel_open_reverse_forward_server(channel, remotehost, remoteport, sourcehost, localport);
}

int ssh_channel_request_send_exit_status_server(ssh_session_struct * server_session, ssh_channel channel, int exit_status)
{
    return static_cast<SshServerSession*>(server_session)->ssh_channel_request_send_exit_status_server(channel, exit_status);
}

int ssh_channel_request_send_exit_signal_server(ssh_session_struct * server_session, ssh_channel channel, const char *signum, int core, const char *errmsg, const char *lang)
{
    return static_cast<SshServerSession*>(server_session)->ssh_channel_request_send_exit_signal_server(channel, signum, core, errmsg, lang);
}

int ssh_set_auth_methods_server(ssh_session_struct * server_session, int authmethods)
{
    syslog(LOG_INFO, "%s ---", __FUNCTION__);
    return static_cast<SshServerSession*>(server_session)->ssh_set_auth_methods_server(authmethods);
}

