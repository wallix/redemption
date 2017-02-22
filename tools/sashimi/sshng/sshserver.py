# -*- coding: utf-8 -*-
#
# Copyright (c) 2010 WALLIX, SAS. All rights reserved.
# Licensed computer software. Property of WALLIX.
# Author(s) : Christophe Grosjean <cgrosjean@wallix.com>,
#             Loïc Michaux <lmichaux@wallix.com>
# Id: $Id: sshserver.py 12638 2013-01-18 17:11:12Z efayol $
# URL: $URL$
# Module description:
# Compile with:
#


# sys import
import traceback
from logger import Logger
import ctypes
import time
from socket import SOL_SOCKET, SO_KEEPALIVE
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
except Exception, e:
    Logger().info("Error importing sashimi lib %s" % traceback.format_exc(e))

from sshng.common.utils import mdecode, parse_ssh_auth, SSHException
from sshng.common.wabconsole import WABConsole

from sshng.server_channels import (
    WABChannelDirectTCPIP, 
    WABChannelForwardedTCPIP, 
    WABChannel)

from sshng.client import ( 
    SSHClient,
    RloginClient, 
    TelnetClient, 
    SSHWABConsoleClient, 
    KickStartTelnetClientChannel,
    STARTED_AUTHENTIFIED, 
    STARTED_REJECTED, 
    STARTED_AGAIN
    )


from sshng.client_channels import (
    KickStartClientChannel,
    SSHTCPIPClientChannel
    )


from wab.engine import APPROVAL_ACCEPTED, APPROVAL_REJECTED, APPROVAL_PENDING, APPROVAL_NONE

import wab.engine
from wab.engine import Engine
from wab.engine import TargetContext

import os

class SSHServer(object):
    """
    SSH Server class implementation to initialize transport, negotiate
    client connections, and start site clients.
    """
    def __init__(self, client_socket, addr, host_key_file, pid_path=None):
        self.pid_path = pid_path
        self.create_pid_file_pid()
        """
        Create a new SSHServer instance.
        """
        
        try:
            self.libssh_error = lib.ssh_new_error()
            self.username = None
            self.kbdint_response = None
            self.client_socket = client_socket
            self.client_socket.setsockopt(SOL_SOCKET, SO_KEEPALIVE, 1)
            self.local_ip, self.local_port = client_socket.getsockname()

            self.client_addr = addr[0]
            self.client_port = addr[1]
            self.host_key_file = host_key_file
            self.client_channels_to_close = []
            self.server_channels_to_close = []

            if self.client_addr != "127.0.0.1":
                Logger().info("Incoming ssh connection from %s " % self.client_addr)

            self.buffer64 = ctypes.create_string_buffer(10000)

            self.delayed = []
            self.site = None

            self.running = False
            self.target_session = None
            self.exit_status = -1
            self.host_key = None
            self.target = None
            self.target_rights = None
            self.ssh_agent_requested = False

            self.max_timeout = 900
            self.lastactiontime = time.time()
            self.wabengine_proxy = Engine()

            self.host_key_file = host_key_file
            self.lastactiontime = time.time()
            self.args = []
            self.real_target_ip = None
            self.sftp_handler = None
            self.wabuser = None
            self.wabpassword = u''
            self.tokenid = None
            self.challenge = None
            self.auth_method_str = "Unknown"

            self.height = 24
            self.width = 80
            self.term = "xterm"

            self.sub_protocol = None

            self.sftp_requested = None
            self.x11_requested = None
            self.x509_requested = None
            self.keyboardinteractive_requested = None

            self.licenceError = None

            # Used for X11 FWD.
            self._transport = None

            self.server_channels = []
            self.device_channels = []
            self.sftp_channels = []
            self.is_sftp = False
            self.approvals = []
            self.scp_requested = False
            self.config = wab.engine.read_config_file('sashimi')

            self.passthrough_mode = self.config.get('auth_mode_passthrough', False)
            self.default_login = self.config.get('default_login', '').strip() or None
            self.passthrough_login = None
            self.passthrough_password = None

            self.max_timeout = self.config.get('session_timeout', 900)
            self.silent_mode = self.config.get('silent_mode', False)
            self.enable_ip_transparent = self.config.get('enable_ip_transparent', False)
            self.target_context = None

            self._pub_auth = SSH_AUTH_DENIED
            self._pwd_auth = None

            self.x11 = None
            self.ssh_agent = None
            self.gssapi_token = None
            self.force_password_change = None

            Logger().info("Incoming ssh connection from %s " % self.client_addr)

            self.args = []

            if self.enable_ip_transparent is True:
                self.real_target_ip = self.get_real_target()

            self.server_callbacks = buildCallbacks(ssh_server_callbacks_struct, CB_SERVER_SESSION_FUNCS, self)
            self.libssh_mainloop = lib.ssh_new_poll_ctx()

            # start_server_session replace all commented code above
            self.authmethods = (SSH_AUTH_METHOD_NONE
                # | SSH_AUTH_METHOD_PASSWORD
                 | SSH_AUTH_METHOD_GSSAPI_MIC
                 | SSH_AUTH_METHOD_PUBLICKEY
                 | SSH_AUTH_METHOD_INTERACTIVE
                 )
            self.libssh_session = lib.ssh_start_new_server_session(
                self.server_callbacks, 
                self.libssh_mainloop,
                self.client_socket.fileno(),
                self.host_key_file, # host keyfile contains RSA, DSA or ECDSA key
                self.authmethods
                )

            lib.ssh_event_set_session_server(self.libssh_mainloop, self.libssh_session)

            self.target_session = None
            self.lastactiontime = time.time()

            while self.checkSessions(self.target_session,
                                     self.device_channels,
                                     self.server_channels):
                rc = lib.ssh_event_dopoll(self.libssh_mainloop, 20000)
                if rc == SSH_ERROR:
                    break

                if self.target_session:
                    try:
                        if self.target_session.started is STARTED_AGAIN:
                            self.target_session.linkedchannel.cb_wakeup()
                    except Exception, e:
                        Logger().info("Exception caught : '%s'" % e)
                        pass
                polltime = time.time()

            # check inactivity timeout
                if polltime - self.lastactiontime > self.max_timeout:
                    for c in self.server_channels:
                        #TODO: can't check before writing. Target may return some error
                        if c.isOpen_server():
                            c.write_stderr("\r\nTimeout exceeded.\r\n")
                            if self.target_session:
                                self.target_session.disconnect()
                            self.disconnect()

            # check timeframe
                if not self.check_timeframe():
                    for c in self.server_channels:
                        #TODO: can't check before writing. Target may return some error
                        #TODO: send message now, no need to wait
                        if c.isOpen_server():
                            c.write_stderr("\r\nAllowed timeframe has been exceeded,"
                                           + " your connection will now be closed.\r\n")
                            c.close()

            Logger().info("End of Session")
            if self.target_session:
                self.wabengine_proxy.stop_session("End of Session")
        except RuntimeError, exc:
            Logger().info(">>>>>>> %s" % traceback.format_exc(exc))
            #cf #5134: catch begnin event_dopoll failed
            if str(exc) != 'event_dopoll failed':
                raise
            if str(exc) != 'event_dopoll failed':
                raise
        except SSHException, e:
            Logger().info("Exception %s" % e.msg)
            pass
        except Exception, exc:
            Logger().info(">>>>>>> %s" % traceback.format_exc(exc))
        finally:
            self.remove_file_pid()
            if self.client_addr != "127.0.0.1":
                Logger().info("End of ssh connection for %s" % self.client_addr)

    def disconnect(self):
        lib.ssh_disconnect_server(self.libssh_session)

    # remove file pid 'sshng.pid' or 'sashimi_<pid>.pid'
    def remove_file_pid(self):
        if self.pid_path is None:
            return
        try:
            os.remove("%s/%s" % (self.pid_path, self.pid_file))
        except OSError as e:
            Logger().info("Failed to remove pid file %s/%s" % (self.pid_path, self.pid_file))

    def create_pid_file_pid(self):
        if self.pid_path is None:
            return
        # Logger().info("Create session %s " % pid)
        try:
            pid = os.getpid()
            self.pid_file = "session_%s.pid" % pid
            with open("%s/%s" % (self.pid_path, self.pid_file), "w+") as fp:
                fp.write("%s" % pid)
                # Logger().info("write succeed session_%s" % pid)
        except IOError as e:
            import traceback
            Logger().info("Failed to create %s/%s (%s)" % (self.pid_path, self.pid_file, traceback.format_exc(e)))

    def rename_pid_file_to_session_file_pid(self, session_id):
        if self.pid_path is None:
            return
        # Logger().info("Renaming session %s to %s" % (os.getpid(), session_id))
        try:
            old_pid_file = "%s/%s" % (self.pid_path, self.pid_file)
            new_root_pid_file = "session_%s.pid" % session_id
            new_pid_file = "%s/%s" % (self.pid_path, new_root_pid_file)
            os.rename(old_pid_file, new_pid_file)
            self.pid_file = new_root_pid_file
        except OSError as e:
            import traceback
            Logger().info("Failed to rename pid_file using session_id %s (%s)" % (session_id, traceback.format_exc(e)))

    def __del__(self):
        if self.libssh_session is not None:
            lib.ssh_free(self.libssh_session)

    def setBlocking(self, blocking):
        if blocking:
            lib.ssh_set_blocking(self.libssh_session, 1)
        else:
            lib.ssh_set_blocking(self.libssh_session, 0)

    def __repr__(self):
        return "<SSHServer("+str(self)+")>"

    def __str__(self):
        return ("<SSHServer args=%s,client_addr=%s,wabuser=%s>" %
                (str(self.args),
                 str(self.client_addr),
                 str(self.wabuser)
                 ))

    def cancel_port_forward_request(self, address, port):
        pass

    def setAuthMethods(self, auth_methods):
        lib.ssh_set_auth_methods_server(self.libssh_session, auth_methods)

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
        if self.target:
            target_login, target_device, target_service = self.target
            target_device, self.target_context = self.wabengine_proxy.resolve_target_host(
                target_device, target_login, target_service,
                self.real_target_ip, self.target_context,
                self.passthrough_mode, [u'SSH', u'TELNET', u'RLOGIN'])
            if target_device is None:
                self.target = None

    def managePassthrough(self, password):
        self.passthrough_login = self.wabuser
        self.passthrough_password = password
        primary_login = self.default_login or self.wabuser
        Logger().info(
            "[WAB] Passthrough authentication received from %s@%s" % \
                (self.passthrough_login, self.client_addr)
            )
        if self.wabengine_proxy.gssapi_authenticate(mdecode(primary_login),
                                                    mdecode(self.client_addr),
                                                    mdecode(self.local_ip)):
            res = True
        if not res:
            Logger().error(
                "[WAB] Passthrough authentication failed for %s@%s" % \
                    (self.passthrough_login, self.client_addr)
                )
            self._pwd_auth = SSH_AUTH_DENIED
            return SSH_AUTH_DENIED
        # log authentication success
        Logger().info("[WAB] Passthrough authentication accepted for %s@%s" % \
                      (self.passthrough_login, self.client_addr))
        self._pwd_auth = SSH_AUTH_SUCCESS
        return SSH_AUTH_SUCCESS

    def manageChallenge(self):
        if self.wabengine_proxy.challenge:
            rc = lib.ssh_userauth_kbdint_settmpprompt_server(
                 self.libssh_session, "",
                 "= Challenge =",
                 "%s: " % self.wabengine_proxy.challenge.message,
                 self.wabengine_proxy.challenge.promptEcho,
                 self.libssh_error)
            res = (rc == SSH_OK)
            Logger().info("ManageChallenge res=%s" % res);

            if res:
                self.auth_method_str = "Challenge"
            return res
        return False

    def manageKbdIntPassword(self):
        rc = lib.ssh_userauth_kbdint_settmpprompt_server(
                self.libssh_session, "", "", "%s's password: " % self.wabuser, 0, self.libssh_error)
        res = (rc == SSH_OK)
        try:
            Logger().info("= manageKbdIntPassword %s" % rc)

            if res:
                self.auth_method_str = "Password"
                self.authmethods &= ~SSH_AUTH_METHOD_PASSWORD
                self.setAuthMethods(self.authmethods)

            Logger().info("= manageKbdIntPassword %s" % res)            
        except Exception, e:
            import traceback
            Logger().info("= manageKbdIntPassword %s %s" % (res, traceback.format_exc(e)))            
        return res

            

    def getInteractiveAnswers(self):
        Logger().info("= getInteractiveAnswers")                    
        count = lib.ssh_userauth_kbdint_getnanswers_server(self.libssh_session, self.libssh_error)
        return [ lib.ssh_userauth_kbdint_getanswer_server(self.libssh_session, i, self.libssh_error) \
                     for i in xrange(count) ]

    def cb_authInteractive(self, username, kbdint_response):
        Logger().info("@@%s.cb_authInteractive(%s,%s)" % (
            self.__class__.__name__,
            self.kbdint_response,
            kbdint_response))
        # TODO: username check should ensure username never change!
        if username and self.username != username:
            self.username = username
            self.wabuser, self.target = parse_ssh_auth(username)

        if not self.kbdint_response:
            self.kbdint_response = True
            if self.manageChallenge():
                return SSH_AUTH_PARTIAL
            if self.manageKbdIntPassword():
                return SSH_AUTH_PARTIAL

        self.kbdint_response = None        
        answers = self.getInteractiveAnswers()
        if not answers:
            return SSH_AUTH_DENIED
        Logger().info("[WAB] %s (KbdInt) authentication received from %s@%s" %
                      (self.auth_method_str, self.wabuser, self.client_addr))
        # Logger().info("=> Answers = %s " % answers)
        answer = answers[0]
        if self.passthrough_mode is True:
            return self.managePassthrough(answer)
        res = self.wabengine_proxy.password_authenticate(mdecode(self.wabuser),
                                                         mdecode(self.client_addr),
                                                         mdecode(answer),
                                                         mdecode(self.local_ip))
        if not res:
            if self.manageChallenge():
                self.kbdint_response = True
                return SSH_AUTH_PARTIAL
            Logger().error("[WAB] %s (KbdInt) authentication failed for %s@%s" %
                           (self.auth_method_str, self.wabuser, self.client_addr))
            # self.setAuthMethods(self.authmethods) # for more tries
            return SSH_AUTH_DENIED


        self.verify_target_host()
        Logger().info("[WAB] %s (KbdInt) authentication accepted for %s@%s" %
                      (self.auth_method_str, self.wabuser, self.client_addr))

        self._pwd_auth = SSH_AUTH_SUCCESS
        return SSH_AUTH_SUCCESS


    # Server Session Callback API
    # ===========================
    def cb_authNone(self, username):
        Logger().info("@@%s.cb_authNone" % self.__class__.__name__)
        if self.passthrough_mode is True:
            return SSH_AUTH_DENIED

        self.x509_requested = False
        res = False
        method_str = "None"
        if username and self.username != username:
            self.username = username
            self.wabuser, self.target = parse_ssh_auth(username)
        target_info = None
        if self.target:
            try:
                target_info = "%s@%s:%s" % self.target
            except Exception:
                target_info = self.target
        if self.wabuser.startswith('_OTP_'):  # OTP Authentication
            method_str = "OTP"
            if self.wabengine_proxy.password_authenticate(mdecode(self.wabuser),
                                                          mdecode(self.client_addr),
                                                          "", # password
                                                          mdecode(self.local_ip)):
                res = True
                self.wabuser = self.wabengine_proxy.get_username()
            else:
                # Below to force client to stop proceeding with next authentication
                self.setAuthMethods(SSH_AUTH_METHOD_HOSTBASED)
                return SSH_AUTH_DENIED
        elif self.wabengine_proxy.is_x509_connected(mdecode(self.wabuser),
                                                    mdecode(self.client_addr),
                                                    "SSH",
                                                    target_info,
                                                    mdecode(self.local_ip)):
            # X509 Authentication
            self.x509_requested = True
            method_str = "X509"

            # log X509 challenge
            Logger().info("X.509 authentication challenge from %s@%s" % (self.wabuser,
                                                                         self.client_addr))
            if self.wabengine_proxy.x509_authenticate():
                res = True
        else:
            self.authmethods = self.authmethods & ~SSH_AUTH_METHOD_NONE
            return SSH_AUTH_DENIED
        if not res:
            if self.wabengine_proxy.challenge is not None:
#                self.setAuthMethods(SSH_AUTH_METHOD_PASSWORD)
                self.setAuthMethods(SSH_AUTH_METHOD_INTERACTIVE)
                return SSH_AUTH_PARTIAL
            Logger().error(
                "%s authentication failed for %s@%s" % (method_str, self.wabuser,
                                                        self.client_addr)
                )
            return SSH_AUTH_DENIED
            
        self.verify_target_host()
        # log authentication success
        Logger().info(
            "%s authentication accepted for %s@%s" % (method_str, self.wabuser,
                                                      self.client_addr)
            )
        return SSH_AUTH_SUCCESS

    def cb_authPassword(self, username, password):
        Logger().info("@@%s.cb_authPassword" % self.__class__.__name__)
        # parse user authentication to set self.wabuser and self.args
        
        if username and self.username != username:
            self.username = username
            self.wabuser, self.target = parse_ssh_auth(username)

        res = False
        if self.passthrough_mode is True:
            return self.managePassthrough(password)

        # log password challenge
        Logger().info(
            "[WAB] Password authentication received from %s@%s" % (self.wabuser,
                                                                   self.client_addr)
        )

        if password is not None:
            if self.wabengine_proxy.password_authenticate(mdecode(self.wabuser),
                                                          mdecode(self.client_addr),
                                                          mdecode(password),
                                                          mdecode(self.local_ip)):
                self.wabpassword = mdecode(password)
                res = True
                Logger().info("Password authentication success")
        # initialize self.wabengine_proxy
        if not res:
            if self.wabengine_proxy.challenge is not None:
#               self.setAuthMethods(SSH_AUTH_METHOD_PASSWORD)
                self.setAuthMethods(SSH_AUTH_METHOD_INTERACTIVE)
                return SSH_AUTH_PARTIAL
            Logger().error(
                "[WAB] Password authentication failed for %s@%s" % (self.wabuser,
                                                                    self.client_addr)
            )
            self._pwd_auth = SSH_AUTH_DENIED
            return SSH_AUTH_DENIED

        self.verify_target_host()
        # log authentication success
        Logger().info("[WAB] Password authentication accepted for %s@%s" % (self.wabuser,
                                                                            self.client_addr))

        self._pwd_auth = SSH_AUTH_SUCCESS
        return SSH_AUTH_SUCCESS

    def cb_authPubkey(self, username, key, sig_state):
        Logger().info("@@%s.cb_authPubkey" % self.__class__.__name__)
        if self.passthrough_mode is True:
            return SSH_AUTH_DENIED

        rc = lib.ssh_pki_export_pubkey_base64_p(key, self.buffer64, 10000)
    #    rc = lib.ssh_pki_export_pubkey_base64_p(key)

        b64 = ""
        if rc == SSH_OK:
            b64 = self.buffer64.value

        if sig_state not in [SSH_PUBLICKEY_STATE_VALID, SSH_PUBLICKEY_STATE_NONE]:
            return SSH_AUTH_DENIED

        if username and self.username != username:
            self.username = username
            self.wabuser, self.target = parse_ssh_auth(username)

        # log publickey challenge
        Logger().info("[WAB] Public key authentication challenge from %s@%s key=%s" % (self.wabuser, self.client_addr, b64))

        res = False
        if self.wabpassword == b64 and sig_state == SSH_PUBLICKEY_STATE_VALID:
            res = True
        elif self.wabengine_proxy.pubkey_authenticate(mdecode(self.wabuser),
                                                      mdecode(self.client_addr),
                                                      b64,
                                                      mdecode(self.local_ip)):
            self.wabpassword = b64
            res = True
        else:
            self.wabpassword = u''

        # initialize self.wabengine_proxy
        if not res:
            Logger().info("[WAB] Public key authentication failed for %s@%s" % (self.wabuser, self.client_addr))
            if self.wabengine_proxy.challenge is not None:
#                self.setAuthMethods(SSH_AUTH_METHOD_PASSWORD)
                self.setAuthMethods(SSH_AUTH_METHOD_INTERACTIVE)
                return SSH_AUTH_PARTIAL
            Logger().info("[WAB] Public key authentication failed for %s@%s" % (
                    self.wabuser, self.client_addr))
            self._pub_auth = SSH_AUTH_DENIED
            return SSH_AUTH_DENIED

        # this is only an unsigned public key, so we are not authenticated yet
        if (sig_state == SSH_PUBLICKEY_STATE_NONE):
            Logger().info("[WAB] Public key authentication probe accepted for %s@%s" % (self.wabuser, self.client_addr))
            return SSH_AUTH_SUCCESS

        self.verify_target_host()
        # log authentication success
        Logger().info(
            "[WAB] Public key authentication accepted for %s@%s" % (self.wabuser, self.client_addr)
        )

        self._pub_auth = SSH_AUTH_SUCCESS
        return SSH_AUTH_SUCCESS

    def cb_authGssapiMic(self, username, principal):
        Logger().info("@@%s.cb_authGssapiMic(%s, %s)" % (
                        self.__class__.__name__,
                        username,
                        principal))
        if self.passthrough_mode is True:
            return SSH_AUTH_DENIED

        # parse the given username to extract proxying info
        if username and self.username != username:
            self.username = username
            self.wabuser, self.target = parse_ssh_auth(username)

        res = False
#8418: GSSAPIAuthentication: provide principal in place of username to wabengine gssapi authentication for Kerberos authentication (principal contains username and domain name)            
        if self.wabengine_proxy.gssapi_authenticate(mdecode(principal),
                                                    mdecode(self.client_addr),
                                                    mdecode(self.local_ip)):
            self.gssapi_token = lib.ssh_gssapi_get_creds_server(self.libssh_session)
            self.wabuser = self.wabengine_proxy.get_username()
            res = True

        if not res:
            if self.wabengine_proxy.challenge is not None:
#                self.setAuthMethods(SSH_AUTH_METHOD_PASSWORD)
                self.setAuthMethods(SSH_AUTH_METHOD_INTERACTIVE)
                return SSH_AUTH_PARTIAL
            Logger().error("[WAB] GSSAPI authentication failed for %s@%s"
                           % (self.wabuser, self.client_addr))
            return SSH_AUTH_DENIED

        self.verify_target_host()
        # log authentication success
        Logger().info("[WAB] GSSAPI authentication accepted for %s@%s" % (self.wabuser, self.client_addr))

        return SSH_AUTH_SUCCESS

    def cb_channelOpenSessionRequest(self):
        Logger().info("@@%s.cb_channelOpenSessionRequest" % self.__class__.__name__)
        channel = WABChannel(self)
        return channel.libssh_channel

    def cb_channelOpenDirectTCPIPRequest(self, dest, dest_port, orig, orig_port):
        Logger().info("@@%s.cb_channelOpenDirectTCPIPRequest" % self.__class__.__name__)
        client_channel = WABChannelDirectTCPIP(self, dest, dest_port, orig, orig_port)

        # if self.config['ip_transparent'] is True:
        #     self.wabengine_proxy.filter_proxy_rights(self.real_target_ip)
        #     if len(self.wabengine_proxy.displaytargets) == 1:
        #         tmp_target = self.wabengine_proxy.displaytargets[0]
        #         self.target = (tmp_target.target_login,
        #                        tmp_target.target_name,
        #                        tmp_target.service_name)

        #Primary user and Target must already be known here
        if not self.target or not self.wabuser:
            client_channel.write_stderr("[WAB] Invalid syntax\r\n")
            client_channel.close()
            return None

        if not self.target_rights:
            target_login, target_device, target_service = self.target

            self.target_rights, message = self.wabengine_proxy.get_target_rights(
                mdecode(target_login),
                mdecode(target_device),
                mdecode(target_service))

            if not self.target_rights:
                Logger().info("NO target_right")
                # client_channel.write_stderr("[WAB] Invalid syntax\r\n")
                # client_channel.close()
                return None

            approval_status, approval_info = self.wabengine_proxy.check_target(self.target_rights, None)

            if approval_status != APPROVAL_ACCEPTED:
                self.disconnect()
                return None

        targetsession, message = self.get_targetsession(client_channel, 'SSH_X11_SESSION')
        if not targetsession:
            Logger().info("[WAB] Direct TCPIP Session rejected")
            client_channel.write_stderr("[WAB] Secondary connection failed\r\n")
            client_channel.close()
            return None

        try:
            #TODO: most code below is client_channel init code and should be done in constructor
            targetchannel = SSHTCPIPClientChannel(targetsession, client_channel)
            targetsession.window_change_handler()
            self.device_channels.append(targetchannel)
        except Exception, exc:
            Logger().info("Exception caught %s %s\n%s" % (type(exc), repr(exc), traceback.format_exc()))
            client_channel.write_stderr("%s\r\n" % exc)
            self.wabengine_proxy.secondary_failed(str(exc), self.wabuser, targetsession.client_addr, targetsession.user, targetsession.host)
            return None

        self.device_channels.append(client_channel)
        return client_channel.libssh_channel

    # Server Session Callback API
    # ===========================
    def channelOpenForwardedTCPIPRequest(self, dest, dest_port, orig, orig_port):
        # Logger().debug("========================== Open forwarded TCP/IP Request")
        client_channel = WABChannelForwardedTCPIP(self, dest, dest_port, orig, orig_port)

        # if self.config['ip_transparent'] is True:
        #     self.wabengine_proxy.filter_proxy_rights(self.real_target_ip)
        #     if len(self.wabengine_proxy.displaytargets) == 1:
        #         tmp_target = self.wabengine_proxy.displaytargets[0]
        #         self.target = (tmp_target.target_login,
        #                        tmp_target.target_name,
        #                        tmp_target.service_name)

        #Primary user and Target must already be known here
        if not self.target or not self.wabuser:
            client_channel.write_stderr("[WAB] Invalid syntax\r\n")
            client_channel.close()
            return None

        if not self.target_rights:
            target_login, target_device, target_service = self.target
            self.target_rights, message = self.wabengine_proxy.get_target_rights(
                mdecode(target_login),
                mdecode(target_device),
                mdecode(target_service))

            if not self.target_rights:
                Logger().info("NO target_right")
                # client_channel.write_stderr("[WAB] Invalid syntax\r\n")
                # client_channel.close()
                return None

            approval_status, approval_info = self.wabengine_proxy.check_target(self.target_rights, None)

            if approval_status != APPROVAL_ACCEPTED:
                return None

        targetsession, message = self.get_targetsession(client_channel, 'SSH_X11_SESSION')
        if not targetsession:
            Logger().info("[WAB] Forwarded Session rejected")
            client_channel.write_stderr(message)
            client_channel.close()
            return None

        try:
            targetchannel = SSHTCPIPClientChannel(targetsession, client_channel)
            targetsession.window_change_handler()
            self.device_channels.append(targetchannel)
        except Exception, exc:
            Logger().info("Exception caught %s %s\n%s" % (type(exc), repr(exc), traceback.format_exc()))
            client_channel.write_stderr("%s\r\n" % exc)
            self.wabengine_proxy.secondary_failed(str(exc), self.wabuser, targetsession.client_addr, targetsession.user, targetsession.host)

        return client_channel.libssh_channel if not client_channel is None else None

    def get_targetsession(self, channel, subprotocol):
        proto_info = self.wabengine_proxy.get_target_protocols()
        allowed_subprotocol = self.check_access_allowed(subprotocol,
                                                        proto_info.protocol,
                                                        proto_info.subprotocols)
        if not allowed_subprotocol:
            if ((subprotocol == 'INTERACTIVE') and
                (proto_info.protocol == u"SSH")):
                subprotocol = u'SSH_SHELL_SESSION'
            message = "WAB user %s can't access target using %s sub protocol" % (
                self.wabuser, subprotocol)
            Logger().info(message)
            return None, message

        channel.sub_protocol = allowed_subprotocol

        if channel.sub_protocol in (u"TELNET", u"RLOGIN"):
            if not self.target_session:
                Target = {'TELNET': TelnetClient, 'RLOGIN': RloginClient}[channel.sub_protocol]

                try:
                    self.target_session = Target(channel,
                                                 self.wabengine_proxy,
                                                 self.target_rights,
                                                 self)
                    self.target_session.start_client()
                except Exception:
                    message = "Cannot access target using %s protocol" % (proto_info.protocol)
                    Logger().info(message)
                    return None, message

        if channel.sub_protocol in [
            'SSH_ALL',
            'SSH_X11_SESSION',
            'SSH_SHELL_SESSION',
            'SSH_REMOTE_COMMAND',
            'SSH_SCP_UP',
            'SSH_SCP_DOWN',
            'SFTP_SESSION'
        ]:

            #TODO: we should check it is no Telnet/Rlogin
    #        Logger().info("========================== Server %s get_targetsession" % id(self))
            if not self.target_session:
                is_agent_forwardable = self.wabengine_proxy.get_target_agent_forwardable()
                use_agent = self.ssh_agent_requested and is_agent_forwardable
                if self.ssh_agent_requested:
                    Logger().info("Agent Requested = yes")
                if is_agent_forwardable:
                    Logger().info("Agent Forwardable = yes")
                is_x11_allowed = (allowed_subprotocol == u"SSH_X11_SESSION"
                                  or (allowed_subprotocol == u"SSH_REMOTE_COMMAND"
                                      and u"SSH_X11_SESSION" in proto_info.subprotocols))
                use_x11 = is_x11_allowed and self.x11_requested
                try:
                    self.target_session = SSHClient(self.wabengine_proxy,
                                                    self.target_rights,
                                                    channel,
                                                    self,
                                                    use_x11,
                                                    use_agent,
                                                    self.gssapi_token)
                    self.target_session.start_client()
#                    Logger().info("secondary session started")

                except Exception, e:
                    Logger().info("SSHClient authentication Failed <<<<<<<<<<<<<<<<<%s>>>>>>>>>>>>>>>>>>" % traceback.format_exc(e))
                    self.target_session = None

                # if not self.target_session:
                if ((not self.target_session) or
                        self.target_session.started is STARTED_REJECTED):
                    return None, "Can't open target session\r\n"

                self.target_session.start_client()

                lib.ssh_event_set_session_client(self.libssh_mainloop, self.target_session.libssh_session)

        return self.target_session, "OK"

    def interactiveShellChannel(self, channel, target):
        try:
            if not self.target:
                self.target = target
            if self.target == 'console':
                # TODO: self.session.rights is not initialized for console
                # we should have a wabuser but no target
                self.target_session = SSHWABConsoleClient(self.wabengine_proxy,
                                                          self.target_rights,
                                                          channel, self)

            #TODO: This should be moved in start_client ans start_client merged with constructor
            # Or use a completely different client like Telnet and Rlogin
                channel.linkedchannel = WABConsole(self, channel,
                                                   self.wabuser,
                                                   self.wabpassword,
                                                   self.client_addr,
                                                   self.local_ip)

                self.device_channels.append(channel.linkedchannel)

                return True, "OK"

#            if self.wabengine_proxy.get_license_status() is not True:
#                return False, "You cannot currently access any target. Please contact your administrator."

            target_login, target_device, target_service = self.target
            self.target_rights, message = self.wabengine_proxy.get_target_rights(
                mdecode(target_login),
                mdecode(target_device),
                mdecode(target_service))

            if not self.target_rights:
                return False, message

            approval_status, approval_info = self.wabengine_proxy.check_target(self.target_rights, None)

            if approval_status != APPROVAL_ACCEPTED:
                return False, approval_info['message'].replace('\n', '\r\n').encode('utf-8')

            targetsession, message = self.get_targetsession(channel, 'INTERACTIVE')

            if not targetsession:
                return False, message

            if channel.sub_protocol in (u"TELNET", u"RLOGIN"):
                if not targetsession.started in [STARTED_AUTHENTIFIED,
                                                 STARTED_REJECTED]:
                    targetchannel = KickStartTelnetClientChannel(targetsession, channel, self)
                else:
                    targetchannel = targetsession
            elif channel.sub_protocol in ['SSH_ALL', 'SSH_X11_SESSION', 'SSH_SHELL_SESSION']:
                targetchannel = KickStartClientChannel(targetsession, channel, self, self.x11)

                if targetsession.started in [STARTED_AUTHENTIFIED, STARTED_REJECTED]:
                    targetchannel.wakeup()
            else:
                targetchannel = None

        except Exception, exc:
            Logger().info("....Exception caught %s %s\n%s" % (type(exc), repr(exc), traceback.format_exc()))
            return False, "Error"

        if not targetchannel:
            Logger().info("No target channel")
            return False, "Error"

        return True, "OK"

    def check_timeframe(self):
        return (time.time() < self.wabengine_proxy.deconnection_epoch)

    def _window_change_handler(self):
        return True

    def get_real_target(self):
        import re
#        r = '^tcp *\\d *\\d+ *ESTABLISHED *src=(\\d+\\.\\d+\\.\\d+\\.\\d+) *dst=(\\d+\\.\\d+\\.\\d+\\.\\d+) *sport=\\d+ *dport=\\d+ *src=\\d+\\.\\d+\\.\\d+\\.\\d+ *dst=\\d+\\.\\d+\\.\\d+\\.\\d+ *sport=\\d+ *dport=\\d+ *\\[ASSURED].*'
        src_ip = self.client_addr
        src_port = self.client_port
        dst_ip = self.local_ip
        dst_port = self.local_port

        src_ip = src_ip.replace('.', '\\.')
        dst_ip = dst_ip.replace('.', '\\.')
        r = '^tcp *\\d *\\d+ *ESTABLISHED *src=(\\d+\\.\\d+\\.\\d+\\.\\d+) *dst=(\\d+\\.\\d+\\.\\d+\\.\\d+) *sport=\\d+ *dport=\\d+ *src=%s *dst=%s *sport=%i *dport=%i *\\[ASSURED].*' % (dst_ip, src_ip, dst_port, src_port)
        f = open('/proc/net/ip_conntrack', 'r')
        for l in f.readlines():
            res = re.split(r, l)
            if len(res) > 1:
                real_target_ip = res[2]
                if real_target_ip == self.local_ip:
                    break
                return res[2]
        return None

    def start(self):
        pass

    def check_access_allowed(self, sub_protocol, right_protocol, right_subprotocols):

        # Check if sub_protocol is allowed by current rights
        # Logger().info("Check Access: sub_protocol = %s allowed=%s" % (sub_protocol, right_subprotocols))
        if not right_protocol in [u"SSH", u"TELNET", u"RLOGIN"]:
            return None

        if (right_protocol in (u"TELNET", u"RLOGIN")
                and sub_protocol == 'INTERACTIVE'):
            sub_protocol = right_protocol

        if sub_protocol == 'INTERACTIVE':
            sub_protocol = u'SSH_SHELL_SESSION'

        if (sub_protocol == u'SSH_SHELL_SESSION'
            and u'SSH_X11_SESSION' in right_subprotocols):
            sub_protocol = u'SSH_X11_SESSION'
        # Logger().info("check subprotocol=%s in %s" % (sub_protocol, right_subprotocols))
        if sub_protocol not in right_subprotocols:
            return None

        return sub_protocol

    def checkSessions(self, target_session, device_channels, server_channels):
        try:
            Logger().info("====== %s ==== FRONT %d:[%s] %d:[%s] ==== TARGET %d:[%s] %d:[%s] ============" % (
                 id(self.target_session),
                 len(server_channels), 
                 str(" ".join(["%s<%s>"%(x.__class__.__name__, id(x)) 
                    for x in server_channels])), 
                 len(self.server_channels_to_close), 
                 str(" ".join(["%s<%s>:%s"%(x.__class__.__name__, id(x), msg) 
                    for x, msg in self.server_channels_to_close])), 
                 len(device_channels),
                 str(" ".join(["%s<%s>"%(x.__class__.__name__, id(x)) 
                    for x in device_channels])), 
                len(self.client_channels_to_close),
                str(" ".join(["%s<%s>"%(x.__class__.__name__, id(x)) 
                    for x in self.client_channels_to_close]))
                ))

            if len(server_channels) > 0:
                self.running = True
            
            if not self.running:
                return True

            # TODO: why is it here ? We should be able to do that inside selector.
            for app in self.approvals:
                app.handle_approval()

            while len(self.client_channels_to_close)+len(self.server_channels_to_close) > 0:
            
                for channel in self.client_channels_to_close:
                    Logger().info("REMOVING CLIENT CHANNEL!!!!!!!!!!!!!")
                    device_channels.remove(channel)
                    if channel.linkedchannel:
                        channel.linkedchannel.linkedchannel = None
                self.client_channels_to_close = []

                for channel, msg in self.server_channels_to_close:
                    Logger().info("REMOVING SERVER CHANNEL!!!!!!!!!!!!!")
                    if channel.linkedchannel:
                        channel.linkedchannel.linkedchannel = None

    # TODO: this should be done by caller (where channels is put in close list) not here
    #                if self.silent_mode is not True:
    #                    if msg:
    #                        for c in msg:
    #                            channel.write_stderr_server(c)
    #                    channel.write_stderr_server('\r\n')
    #    #            channel.sendEof()
    #                channel.sendExitStatus(255)

                    server_channels.remove(channel)
                self.server_channels_to_close = []

                # This is a hack to propagate closes to other side
                # Remove as soon as we understand why close is not working properly
                for channel in server_channels:
                    if not channel.linkedchannel:
                        self.server_channels_to_close.append((channel, "Remote closed"))

                for channel in device_channels:
                    if not channel.linkedchannel:
                        self.client_channels_to_close.append(channel)
                
        except Exception, e:
                import traceback
                Logger().info("<<<%s>>>" % traceback.format_exc(e))

        Logger().info("Check session DONE ====== %s ==== FRONT %d:[%s] %d:[%s] ==== TARGET %d:[%s] %d:[%s] ============" % (
                 id(self.target_session),
                 len(server_channels), 
                 str(" ".join(["%s<%s>"%(x.__class__.__name__, id(x)) 
                    for x in server_channels])), 
                 len(self.server_channels_to_close), 
                 str(" ".join(["%s<%s>:%s"%(x.__class__.__name__, id(x), msg) 
                    for x, msg in self.server_channels_to_close])), 
                 len(device_channels),
                 str(" ".join(["%s<%s>"%(x.__class__.__name__, id(x)) 
                    for x in device_channels])), 
                len(self.client_channels_to_close),
                str(" ".join(["%s<%s>"%(x.__class__.__name__, id(x)) 
                    for x in self.client_channels_to_close]))
                ))
            
        return len(server_channels) + len(device_channels) > 0
