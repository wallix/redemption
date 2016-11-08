#!/usr/bin/python -O
# -*- coding: utf-8 -*-
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.

#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#   GNU General Public License for more details.

#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#   Product name: redemption-sashimi
#   Copyright (c) 2016 by Wallix
#   Author(s): Christophe Grosjean


import socket
import sys
import traceback

from sys import stdin

from socket import fromfd
from socket import AF_INET
from socket import SOCK_STREAM
from socket import SOL_SOCKET
from socket import SO_REUSEADDR
from select import select
import os
import errno

import ctypes
import time
from socket import SOL_SOCKET, SO_KEEPALIVE

from logger import Logger

host_key = '/etc/sashimi/server_rsa.key'

pid_path = '/var/run/sashimi'

IP_TRANSPARENT = 19

try:
    from sshng import pysshct

    from sshng.pysshct import (
        lib,
        buildCallbacks,
        CB_SERVER_SESSION_FUNCS,
        ssh_server_callbacks_struct,
        SSH_OK,
        SSH_ERROR,
        SSH_AGAIN,
        SSH_AUTH_DENIED,
        SSH_AUTH_SUCCESS,
        SSH_AUTH_PARTIAL,
        SSH_PUBLICKEY_HASH_MD5,
        SSH_AUTH_METHOD_NONE,
        SSH_AUTH_METHOD_PASSWORD,
        SSH_AUTH_METHOD_GSSAPI_MIC,
        SSH_AUTH_METHOD_PUBLICKEY,
        SSH_AUTH_METHOD_INTERACTIVE,
        SSH_AUTH_METHOD_HOSTBASED,
        SSH_PUBLICKEY_STATE_VALID,
        SSH_PUBLICKEY_STATE_NONE,
        )

    from hello_channel import HelloChannel

except Exception, e:
    Logger().info("Error importing sashimi lib %s" % traceback.format_exc(e))


def serve():
    import signal
    signal.signal(signal.SIGCHLD, signal.SIG_IGN)
    s = socket.socket(AF_INET, SOCK_STREAM)
    s.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
#    s.setsockopt(socket.SOL_IP, IP_TRANSPARENT, 1)
    #TODO: move that to configuration file
    s.bind(('0.0.0.0', 2200))
    s.listen(100)
    try:
        while 1:
            rfds, wfds, xfds = select([s], [], [], 1)
            for sck in rfds:
                if sck == s:
                    client_socket, client_addr = s.accept()
                    child_pid = os.fork()
                    if child_pid == 0:
                        signal.signal(signal.SIGCHLD, signal.SIG_DFL)
                        s.close()
                        server = SSHServer(client_socket, client_addr, host_key, pid_path)
                        server.start()
                        sys.exit(0)
                    else:
                        client_socket.close()
                        #os.waitpid(child_pid, 0)

    except KeyboardInterrupt:
        if client_socket:
            client_socket.close()
        sys.exit(1)
    except socket.error, e:
        import traceback
        Logger().debug("%s" % traceback.format_exc(e))
    except Exception, e:
        import traceback
        Logger().info("%s" % traceback.format_exc(e))


class SSHServer(object):
    """
    SSH Server class implementation to initialize transport, negotiate
    client connections, and start site clients.
    """
    def __init__(self, client_socket, addr, host_key_file, pid_path=None):
    
        self.libssh_error = lib.ssh_new_error()
        self.username = None
        self.kbdint_response = None
        self.client_socket = client_socket
        self.client_socket.setsockopt(SOL_SOCKET, SO_KEEPALIVE, 1)
        self.local_ip, self.local_port = client_socket.getsockname()

        self.client_addr = addr[0]
        self.client_port = addr[1]
        self.host_key_file = host_key_file

        Logger().info("Incoming ssh connection from %s " % self.client_addr)

        self.buffer64 = ctypes.create_string_buffer(10000)

        self.server_callbacks = buildCallbacks(ssh_server_callbacks_struct, CB_SERVER_SESSION_FUNCS, self)
        self.libssh_mainloop = lib.ssh_new_poll_ctx()

        self.authmethods = (
#            SSH_AUTH_METHOD_NONE
             SSH_AUTH_METHOD_PASSWORD
#             | SSH_AUTH_METHOD_GSSAPI_MIC
#             | SSH_AUTH_METHOD_PUBLICKEY
#             | SSH_AUTH_METHOD_INTERACTIVE
             )
        self.libssh_session = lib.ssh_start_new_server_session(
            self.server_callbacks, 
            self.libssh_mainloop,
            self.client_socket.fileno(),
            self.host_key_file, # host keyfile contains RSA, DSA or ECDSA key
            self.authmethods
            )

        lib.ssh_event_set_session_server(self.libssh_mainloop, self.libssh_session)

        self.lastactiontime = time.time()

        while True:
            rc = lib.ssh_event_dopoll(self.libssh_mainloop, 20000)
            if rc == SSH_ERROR:
                break
            polltime = time.time()
            print "Polling %s" % str(polltime)

    def __del__(self):
        if self.libssh_session is not None:
            lib.ssh_free(self.libssh_session)

    # SSH Session Callback API
    # ========================
    def Log(self, priority, message):
        Logger().info("[sashimi] %s" % message)

    def cb_connectStatus(self, status):
        Logger().info("@@%s.cb_connectStatus(%s)" % (
                    self.__class__.__name__,
                    status))

    def cb_globalRequest(self, gtype, want_reply, bind_address, bind_port):
        Logger().info("@@%s.cb_globalRequest(%s, %s, %s, %s)" % (
                    self.__class__.__name__,
                    gtype, want_reply, bind_address, bind_port))
        return SSH_ERROR

    def cb_serviceRequest(self, service):
        Logger().info("@@%s.cb_serviceRequest(%s)" % (
            self.__class__.__name__, service))
        return SSH_OK

    def verify_target_host(self):
        Logger().info("@@%s.verify_target_host()" % (
            self.__class__.__name__))
#        if self.target:
#            target_login, target_device, target_service = self.target
#            target_device, self.target_context = self.wabengine_proxy.resolve_target_host(
#                target_device, target_login, target_service,
#                self.real_target_ip, self.target_context,
#                self.passthrough_mode, [u'SSH', u'TELNET', u'RLOGIN'])
#            if target_device is None:
#                self.target = None

    def managePassthrough(self, password):
        Logger().info("@@%s.managePassthrough()" % (
            self.__class__.__name__))
#        self.passthrough_login = self.wabuser
#        self.passthrough_password = password
#        primary_login = self.default_login or self.wabuser
#        Logger().info(
#            "[WAB] Passthrough authentication received from %s@%s" % \
#                (self.passthrough_login, self.client_addr)
#            )
#        if self.wabengine_proxy.gssapi_authenticate(mdecode(primary_login),
#                                                    mdecode(self.client_addr),
#                                                    mdecode(self.local_ip)):
#            res = True
#        if not res:
#            Logger().error(
#                "[WAB] Passthrough authentication failed for %s@%s" % \
#                    (self.passthrough_login, self.client_addr)
#                )
#            self._pwd_auth = SSH_AUTH_DENIED
#            return SSH_AUTH_DENIED
#        # log authentication success
#        Logger().info("[WAB] Passthrough authentication accepted for %s@%s" % \
#                      (self.passthrough_login, self.client_addr))
#        self._pwd_auth = SSH_AUTH_SUCCESS
#        return SSH_AUTH_SUCCESS

    def manageChallenge(self):
        Logger().info("@@%s.manageChallenge(%s)" % (
            self.__class__.__name__, service))
#        if self.wabengine_proxy.challenge:
#            rc = lib.ssh_userauth_kbdint_settmpprompt_server(
#                 self.libssh_session, "",
#                 "= Challenge =",
#                 "%s: " % self.wabengine_proxy.challenge.message,
#                 self.wabengine_proxy.challenge.promptEcho,
#                 self.libssh_error)
#            res = (rc == SSH_OK)
#            Logger().info("ManageChallenge res=%s" % res);

#            if res:
#                self.auth_method_str = "Challenge"
#            return res
#        return False

    def manageKbdIntPassword(self):
        Logger().info("@@%s.manageKbdIntPassword(%s)" % (
            self.__class__.__name__))
#        rc = lib.ssh_userauth_kbdint_settmpprompt_server(
#                self.libssh_session, "", "", "%s's password: " % self.wabuser, 0, self.libssh_error)
#        res = (rc == SSH_OK)
#        try:
#            Logger().info("= manageKbdIntPassword %s" % rc)

#            if res:
#                self.auth_method_str = "Password"
#                self.authmethods &= ~SSH_AUTH_METHOD_PASSWORD
#                self.setAuthMethods(self.authmethods)

#            Logger().info("= manageKbdIntPassword %s" % res)            
#        except Exception, e:
#            import traceback
#            Logger().info("= manageKbdIntPassword %s %s" % (res, traceback.format_exc(e)))            
#        return res

            

    def getInteractiveAnswers(self):
        Logger().info("@@%s.getInteractiveAnswers()" % (
            self.__class__.__name__))
#        Logger().info("= getInteractiveAnswers")                    
#        count = lib.ssh_userauth_kbdint_getnanswers_server(self.libssh_session, self.libssh_error)
#        return [ lib.ssh_userauth_kbdint_getanswer_server(self.libssh_session, i, self.libssh_error) \
#                     for i in xrange(count) ]

    def cb_authInteractive(self, username, kbdint_response):
        Logger().info("@@%s.cb_authInteractive(%s,%s)" % (
            self.__class__.__name__,
            username,
            kbdint_response))
#        # TODO: username check should ensure username never change!
#        if username and self.username != username:
#            self.username = username
#            self.wabuser, self.target = parse_ssh_auth(username)

#        if not self.kbdint_response:
#            self.kbdint_response = True
#            if self.manageChallenge():
#                return SSH_AUTH_PARTIAL
#            if self.manageKbdIntPassword():
#                return SSH_AUTH_PARTIAL

#        self.kbdint_response = None        
#        answers = self.getInteractiveAnswers()
#        if not answers:
#            return SSH_AUTH_DENIED
#        Logger().info("[WAB] %s (KbdInt) authentication received from %s@%s" %
#                      (self.auth_method_str, self.wabuser, self.client_addr))
#        # Logger().info("=> Answers = %s " % answers)
#        answer = answers[0]
#        if self.passthrough_mode is True:
#            return self.managePassthrough(answer)
#        res = self.wabengine_proxy.password_authenticate(mdecode(self.wabuser),
#                                                         mdecode(self.client_addr),
#                                                         mdecode(answer),
#                                                         mdecode(self.local_ip))
#        if not res:
#            if self.manageChallenge():
#                self.kbdint_response = True
#                return SSH_AUTH_PARTIAL
#            Logger().error("[WAB] %s (KbdInt) authentication failed for %s@%s" %
#                           (self.auth_method_str, self.wabuser, self.client_addr))
#            # self.setAuthMethods(self.authmethods) # for more tries
#            return SSH_AUTH_DENIED


#        self.verify_target_host()
#        Logger().info("[WAB] %s (KbdInt) authentication accepted for %s@%s" %
#                      (self.auth_method_str, self.wabuser, self.client_addr))

#        self._pwd_auth = SSH_AUTH_SUCCESS
#        return SSH_AUTH_SUCCESS
        return SSH_AUTH_SUCCESS


    # Server Session Callback API
    # ===========================
    def cb_authNone(self, username):
        Logger().info("@@%s.cb_authNone(%s)" % (self.__class__.__name__, username))
        lib.ssh_set_auth_methods_server(self.libssh_session, self.authmethods)
        return SSH_AUTH_DENIED

    def cb_authPassword(self, username, password):
        Logger().info("@@%s.cb_authPassword(%s, %s)" % (self.__class__.__name__, username, password))
        return SSH_AUTH_SUCCESS

    def cb_authPubkey(self, username, key, sig_state):
        Logger().info("@@%s.cb_authPubkey(%s, %s)" % self.__class__.__name__, key, sig_state)
        return SSH_AUTH_SUCCESS

    def cb_authGssapiMic(self, username, principal):
        Logger().info("@@%s.cb_authGssapiMic(%s, %s)" % (
                        self.__class__.__name__,
                        username,
                        principal))
        return SSH_AUTH_SUCCESS

    def cb_channelOpenSessionRequest(self):
        Logger().info("@@%s.cb_channelOpenSessionRequest()" % self.__class__.__name__)
        channel = HelloChannel(self)
        return channel.libssh_channel

    def cb_channelOpenDirectTCPIPRequest(self, dest, dest_port, orig, orig_port):
        Logger().info("@@%s.cb_channelOpenDirectTCPIPRequest(%s:%s, %s:%s)" % (
            self.__class__.__name__, dest, dest_port, orig, orig_port))
#        client_channel = WABChannelDirectTCPIP(self, dest, dest_port, orig, orig_port)

#        # if self.config['ip_transparent'] is True:
#        #     self.wabengine_proxy.filter_proxy_rights(self.real_target_ip)
#        #     if len(self.wabengine_proxy.displaytargets) == 1:
#        #         tmp_target = self.wabengine_proxy.displaytargets[0]
#        #         self.target = (tmp_target.target_login,
#        #                        tmp_target.target_name,
#        #                        tmp_target.service_name)

#        #Primary user and Target must already be known here
#        if not self.target or not self.wabuser:
#            client_channel.write_stderr("[WAB] Invalid syntax\r\n")
#            client_channel.close()
#            return None

#        if not self.target_rights:
#            target_login, target_device, target_service = self.target

#            self.target_rights, message = self.wabengine_proxy.get_target_rights(
#                mdecode(target_login),
#                mdecode(target_device),
#                mdecode(target_service))

#            if not self.target_rights:
#                Logger().info("NO target_right")
#                # client_channel.write_stderr("[WAB] Invalid syntax\r\n")
#                # client_channel.close()
#                return None

#            approval_status, approval_info = self.wabengine_proxy.check_target(self.target_rights, None)

#            if approval_status != APPROVAL_ACCEPTED:
#                self.disconnect()
#                return None

#        targetsession, message = self.get_targetsession(client_channel, 'SSH_X11_SESSION')
#        if not targetsession:
#            Logger().info("[WAB] Direct TCPIP Session rejected")
#            client_channel.write_stderr("[WAB] Secondary connection failed\r\n")
#            client_channel.close()
#            return None

#        try:
#            #TODO: most code below is client_channel init code and should be done in constructor
#            targetchannel = SSHTCPIPClientChannel(targetsession, client_channel)
#            targetsession.window_change_handler()
#            self.device_channels.append(targetchannel)
#        except Exception, exc:
#            Logger().info("Exception caught %s %s\n%s" % (type(exc), repr(exc), traceback.format_exc()))
#            client_channel.write_stderr("%s\r\n" % exc)
#            self.wabengine_proxy.secondary_failed(str(exc), self.wabuser, targetsession.client_addr, targetsession.user, targetsession.host)
#            return None

#        self.device_channels.append(client_channel)
#        return client_channel.libssh_channel

    # Server Session Callback API
    # ===========================
    def channelOpenForwardedTCPIPRequest(self, dest, dest_port, orig, orig_port):
        Logger().info("@@%s.channelOpenForwardedTCPIPRequest(%s:%s, %s:%s)" % (
            self.__class__.__name__, dest, dest_port, orig, orig_port))
#        # Logger().debug("========================== Open forwarded TCP/IP Request")
#        client_channel = WABChannelForwardedTCPIP(self, dest, dest_port, orig, orig_port)

#        # if self.config['ip_transparent'] is True:
#        #     self.wabengine_proxy.filter_proxy_rights(self.real_target_ip)
#        #     if len(self.wabengine_proxy.displaytargets) == 1:
#        #         tmp_target = self.wabengine_proxy.displaytargets[0]
#        #         self.target = (tmp_target.target_login,
#        #                        tmp_target.target_name,
#        #                        tmp_target.service_name)

#        #Primary user and Target must already be known here
#        if not self.target or not self.wabuser:
#            client_channel.write_stderr("[WAB] Invalid syntax\r\n")
#            client_channel.close()
#            return None

#        if not self.target_rights:
#            target_login, target_device, target_service = self.target
#            self.target_rights, message = self.wabengine_proxy.get_target_rights(
#                mdecode(target_login),
#                mdecode(target_device),
#                mdecode(target_service))

#            if not self.target_rights:
#                Logger().info("NO target_right")
#                # client_channel.write_stderr("[WAB] Invalid syntax\r\n")
#                # client_channel.close()
#                return None

#            approval_status, approval_info = self.wabengine_proxy.check_target(self.target_rights, None)

#            if approval_status != APPROVAL_ACCEPTED:
#                return None

#        targetsession, message = self.get_targetsession(client_channel, 'SSH_X11_SESSION')
#        if not targetsession:
#            Logger().info("[WAB] Forwarded Session rejected")
#            client_channel.write_stderr(message)
#            client_channel.close()
#            return None

#        try:
#            targetchannel = SSHTCPIPClientChannel(targetsession, client_channel)
#            targetsession.window_change_handler()
#            self.device_channels.append(targetchannel)
#        except Exception, exc:
#            Logger().info("Exception caught %s %s\n%s" % (type(exc), repr(exc), traceback.format_exc()))
#            client_channel.write_stderr("%s\r\n" % exc)
#            self.wabengine_proxy.secondary_failed(str(exc), self.wabuser, targetsession.client_addr, targetsession.user, targetsession.host)

#        return client_channel.libssh_channel if not client_channel is None else None

    def get_targetsession(self, channel, subprotocol):
        Logger().info("@@%s.get_targetsession(%s, %s)" % (
            self.__class__.__name__, channel, subprotocol))
#        proto_info = self.wabengine_proxy.get_target_protocols()
#        allowed_subprotocol = self.check_access_allowed(subprotocol,
#                                                        proto_info.protocol,
#                                                        proto_info.subprotocols)
#        if not allowed_subprotocol:
#            if ((subprotocol == 'INTERACTIVE') and
#                (proto_info.protocol == u"SSH")):
#                subprotocol = u'SSH_SHELL_SESSION'
#            message = "WAB user %s can't access target using %s sub protocol" % (
#                self.wabuser, subprotocol)
#            Logger().info(message)
#            return None, message

#        channel.sub_protocol = allowed_subprotocol

#        if channel.sub_protocol in (u"TELNET", u"RLOGIN"):
#            if not self.target_session:
#                Target = {'TELNET': TelnetClient, 'RLOGIN': RloginClient}[channel.sub_protocol]

#                try:
#                    self.target_session = Target(channel,
#                                                 self.wabengine_proxy,
#                                                 self.target_rights,
#                                                 self)
#                    self.target_session.start_client()
#                except Exception:
#                    message = "Cannot access target using %s protocol" % (proto_info.protocol)
#                    Logger().info(message)
#                    return None, message

#        if channel.sub_protocol in [
#            'SSH_ALL',
#            'SSH_X11_SESSION',
#            'SSH_SHELL_SESSION',
#            'SSH_REMOTE_COMMAND',
#            'SSH_SCP_UP',
#            'SSH_SCP_DOWN',
#            'SFTP_SESSION'
#        ]:

#            #TODO: we should check it is no Telnet/Rlogin
#    #        Logger().info("========================== Server %s get_targetsession" % id(self))
#            if not self.target_session:
#                is_agent_forwardable = self.wabengine_proxy.get_target_agent_forwardable()
#                use_agent = self.ssh_agent_requested and is_agent_forwardable
#                if self.ssh_agent_requested:
#                    Logger().info("Agent Requested = yes")
#                if is_agent_forwardable:
#                    Logger().info("Agent Forwardable = yes")
#                is_x11_allowed = (allowed_subprotocol == u"SSH_X11_SESSION"
#                                  or (allowed_subprotocol == u"SSH_REMOTE_COMMAND"
#                                      and u"SSH_X11_SESSION" in proto_info.subprotocols))
#                use_x11 = is_x11_allowed and self.x11_requested
#                try:
#                    self.target_session = SSHClient(self.wabengine_proxy,
#                                                    self.target_rights,
#                                                    channel,
#                                                    self,
#                                                    use_x11,
#                                                    use_agent,
#                                                    self.gssapi_token)
#                    self.target_session.start_client()
##                    Logger().info("secondary session started")

#                except Exception, e:
#                    Logger().info("SSHClient authentication Failed <<<<<<<<<<<<<<<<<%s>>>>>>>>>>>>>>>>>>" % traceback.format_exc(e))
#                    self.target_session = None

#                # if not self.target_session:
#                if ((not self.target_session) or
#                        self.target_session.started is STARTED_REJECTED):
#                    return None, "Can't open target session\r\n"

#                self.target_session.start_client()

#                lib.ssh_event_set_session_client(self.libssh_mainloop, self.target_session.libssh_session)

        return self.target_session, "OK"

    def interactiveShellChannel(self, channel, target):
        Logger().info("@@%s.interactiveShellChannel(%s, %s)" % (
            self.__class__.__name__, channel, target))
#        try:
#            if not self.target:
#                self.target = target
#            if self.target == 'console':
#                # TODO: self.session.rights is not initialized for console
#                # we should have a wabuser but no target
#                self.target_session = SSHWABConsoleClient(self.wabengine_proxy,
#                                                          self.target_rights,
#                                                          channel, self)

#            #TODO: This should be moved in start_client ans start_client merged with constructor
#            # Or use a completely different client like Telnet and Rlogin
#                channel.linkedchannel = WABConsole(self, channel,
#                                                   self.wabuser,
#                                                   self.wabpassword,
#                                                   self.client_addr,
#                                                   self.local_ip)

#                self.device_channels.append(channel.linkedchannel)

#                return True, "OK"

##            if self.wabengine_proxy.get_license_status() is not True:
##                return False, "You cannot currently access any target. Please contact your administrator."

#            target_login, target_device, target_service = self.target
#            self.target_rights, message = self.wabengine_proxy.get_target_rights(
#                mdecode(target_login),
#                mdecode(target_device),
#                mdecode(target_service))

#            if not self.target_rights:
#                return False, message

#            approval_status, approval_info = self.wabengine_proxy.check_target(self.target_rights, None)

#            if approval_status != APPROVAL_ACCEPTED:
#                return False, approval_info['message'].replace('\n', '\r\n').encode('utf-8')

#            targetsession, message = self.get_targetsession(channel, 'INTERACTIVE')

#            if not targetsession:
#                return False, message

#            if channel.sub_protocol in (u"TELNET", u"RLOGIN"):
#                if not targetsession.started in [STARTED_AUTHENTIFIED,
#                                                 STARTED_REJECTED]:
#                    targetchannel = KickStartTelnetClientChannel(targetsession, channel, self)
#                else:
#                    targetchannel = targetsession
#            elif channel.sub_protocol in ['SSH_ALL', 'SSH_X11_SESSION', 'SSH_SHELL_SESSION']:
#                targetchannel = KickStartClientChannel(targetsession, channel, self, self.x11)

#                if targetsession.started in [STARTED_AUTHENTIFIED, STARTED_REJECTED]:
#                    targetchannel.wakeup()
#            else:
#                targetchannel = None

#        except Exception, exc:
#            Logger().info("....Exception caught %s %s\n%s" % (type(exc), repr(exc), traceback.format_exc()))
#            return False, "Error"

#        if not targetchannel:
#            Logger().info("No target channel")
#            return False, "Error"

        return True, "OK"

    def check_timeframe(self):
        Logger().info("@@%s.check_timeframe()" % (
            self.__class__.__name__))
        return (time.time() < self.wabengine_proxy.deconnection_epoch)

    def _window_change_handler(self):
        Logger().info("@@%s._window_change_handler()" % (
            self.__class__.__name__))
        return True

    def get_real_target(self):
        Logger().info("@@%s.get_real_target()" % (
            self.__class__.__name__))
#        import re
##        r = '^tcp *\\d *\\d+ *ESTABLISHED *src=(\\d+\\.\\d+\\.\\d+\\.\\d+) *dst=(\\d+\\.\\d+\\.\\d+\\.\\d+) *sport=\\d+ *dport=\\d+ *src=\\d+\\.\\d+\\.\\d+\\.\\d+ *dst=\\d+\\.\\d+\\.\\d+\\.\\d+ *sport=\\d+ *dport=\\d+ *\\[ASSURED].*'
#        src_ip = self.client_addr
#        src_port = self.client_port
#        dst_ip = self.local_ip
#        dst_port = self.local_port

#        src_ip = src_ip.replace('.', '\\.')
#        dst_ip = dst_ip.replace('.', '\\.')
#        r = '^tcp *\\d *\\d+ *ESTABLISHED *src=(\\d+\\.\\d+\\.\\d+\\.\\d+) *dst=(\\d+\\.\\d+\\.\\d+\\.\\d+) *sport=\\d+ *dport=\\d+ *src=%s *dst=%s *sport=%i *dport=%i *\\[ASSURED].*' % (dst_ip, src_ip, dst_port, src_port)
#        f = open('/proc/net/ip_conntrack', 'r')
#        for l in f.readlines():
#            res = re.split(r, l)
#            if len(res) > 1:
#                real_target_ip = res[2]
#                if real_target_ip == self.local_ip:
#                    break
#                return res[2]
#        return None

    def start(self):
        pass

    def check_access_allowed(self, sub_protocol, right_protocol, right_subprotocols):
        Logger().info("@@%s.check_access_allowed()" % (
            self.__class__.__name__))

#        # Check if sub_protocol is allowed by current rights
#        # Logger().info("Check Access: sub_protocol = %s allowed=%s" % (sub_protocol, right_subprotocols))
#        if not right_protocol in [u"SSH", u"TELNET", u"RLOGIN"]:
#            return None

#        if (right_protocol in (u"TELNET", u"RLOGIN")
#                and sub_protocol == 'INTERACTIVE'):
#            sub_protocol = right_protocol

#        if sub_protocol == 'INTERACTIVE':
#            sub_protocol = u'SSH_SHELL_SESSION'

#        if (sub_protocol == u'SSH_SHELL_SESSION'
#            and u'SSH_X11_SESSION' in right_subprotocols):
#            sub_protocol = u'SSH_X11_SESSION'
#        # Logger().info("check subprotocol=%s in %s" % (sub_protocol, right_subprotocols))
#        if sub_protocol not in right_subprotocols:
#            return None

#        return sub_protocol



if __name__ == '__main__':
    serve()
