#!/usr/bin/python -O
# -*- coding: iso-8859-1 -*-
#
# Copyright (c) 2008 WALLIX, SAS. All rights reserved.
#
# Licensed computer software. Property of WALLIX.

from logger import Logger

#import os
import select
import signal
import traceback
import time
import ctypes

import os
from os import waitpid, getpid
from time import sleep

import rtelnetlib as Telnet

# Wallix imported modules
from rloginlib import Rlogin

from sshng import pysshct
from sshng.pysshct import (
    lib,
    buildCallbacks,
    ssh_client_callbacks_struct, CB_CLIENT_SESSION_FUNCS,
    event_cb, CB_EVENT_FUNCS,
    SSH_ERROR,
    SSH_OK,
    SSH_AGAIN,
    SSH_PUBLICKEY_HASH_MD5,
    SSH_AUTH_SUCCESS,
    SSH_AUTH_DENIED,
    SSH_AUTH_AGAIN,
    SSH_AUTH_METHOD_GSSAPI_MIC,
    SSH_AUTH_METHOD_PUBLICKEY,
    SSH_AUTH_METHOD_PASSWORD,
    SSH_AUTH_METHOD_INTERACTIVE,
    SSH_AUTH_INFO,
    )

from sshng.common.utils import dotries, SSHException
from sshng.common.proto_analyzer import ProtoAnalyzer

STARTED_INIT = 0
STARTED_AUTHENTIFIED = 1
STARTED_REJECTED = 2
STARTED_INTERACTIVE = 3
STARTED_AGAIN = 4

from sshng.client_channels import (
    SSHClientError,
    AuthAgentClientChannel,
    X11ClientChannel
)


class SSHClient(object):
    def __init__(self, wabengine_proxy, target_rights, linkedchannel, svr_obj, x11session=False, use_agent=False, gssapi_token=None):
    
        self.userAuthNoneStatus = None
        self.libssh_error = lib.ssh_new_error()

        self.x11session = x11session
        self.use_agent = use_agent
        self.gssapi_token = gssapi_token

        self.wabengine_proxy = wabengine_proxy
        self.target_rights = target_rights
        self.server = svr_obj

        self.linkedchannel = linkedchannel

        self.pid = getpid()

        self.server._window_change_handler = self.window_change_handler

        self.client_addr = self.server.client_addr

        self.language = self.wabengine_proxy.get_language()

        login_info = self.wabengine_proxy.get_target_login_info()
        self.user = login_info.account_login
        self.autologon = login_info.autologon
        self.host = login_info.device_host
        self.port = login_info.service_port
        self.hname = login_info.target_name
        # self.conn_cmd = login_info.conn_cmd
        self.service_name = login_info.service_name

        self.started = STARTED_INIT
        
        # Complete target host context from transparent mode
        # if '/' in self.host and self.server.real_target_ip is not None:
        #     self.host = self.server.real_target_ip
        if '/' in self.host and self.server.target_context is not None:
            self.host = self.server.target_context.host
        # Complete target login context from passthrough mode
        if self.server.passthrough_login is not None:
            self.user = self.server.passthrough_login

        target_hostkeys = self.server.config.get('target_hostkeys_type')
        if not target_hostkeys:
            target_hostkeys = "ssh-rsa,ssh-dss"

        host = self.host.encode('utf-8')
        port = str(self.port)
        user = self.user.encode('utf-8')
        hostkeys = target_hostkeys
        verbosity = "4"
        hname = self.hname.encode('utf-8')
        service_name = self.service_name.encode('utf-8')

        self.callbacks = buildCallbacks(ssh_client_callbacks_struct, CB_CLIENT_SESSION_FUNCS, self)
        self.libssh_session = lib.ssh_new_client_session(self.callbacks, svr_obj.libssh_mainloop, host, port, user, hostkeys, "4", self.libssh_error)
        try:
            if self.libssh_session == None:
                raise SSHException(lib.ssh_get_error(self.libssh_error).decode('utf-8'))

            #Verify fingerprint
            buf = ctypes.create_string_buffer(32)
            rc = lib.ssh_get_server_publickey_hash_value_client(self.libssh_session,
                                                      SSH_PUBLICKEY_HASH_MD5,
                                                      buf, 0, self.libssh_error)
            if (rc == SSH_ERROR):
                raise SSHException(lib.ssh_get_error(self.libssh_error).decode('utf-8'))
            finger_raw = buf.raw[:16]

            status, message = self.wabengine_proxy.check_device_fingerprint(hname, service_name, finger_raw)
#            status, message = self.wabengine_proxy.check_device_fingerprint(self.hname, finger_raw)
            if not status:
                self.linkedchannel.write_stderr_server("Cannot connect to target\r\n")
                self.linkedchannel.write_stderr_server("%s\r\n" % message)
                raise SSHClientError(message)
        except SSHClientError, m:
            Logger().info("Cannot connect to target.")
            Logger().info("%s" % m)
            self.started = STARTED_REJECTED
        except Exception, exc:
            Logger().info("....Exception caught %s %s\n %s" % (type(exc), repr(exc), traceback.format_exc(exc)))
            self.started = STARTED_REJECTED

        Logger().info("Authentication on remote target")
        self.starter = self._start_client()

    def userauthNone(self):
        return lib.ssh_userauth_none_client(self.libssh_session, self.user, self.libssh_error)

    def userauthList(self):
        rc = lib.ssh_userauth_list_client(self.libssh_session, None, self.libssh_error)
        if (rc == SSH_ERROR):
            raise SSHException(lib.ssh_get_error(self.libssh_error).decode('utf-8'))
        return rc

    def userauthPassword(self, password):
        Logger().info("Connectiong with password %s" % password);
        rc = lib.ssh_userauth_password_client(self.libssh_session, None, password, self.libssh_error)
        if (rc == SSH_ERROR):
            raise SSHException(lib.ssh_get_error(self.libssh_error).decode('utf-8'))
        return rc

    def userauthAgent(self):
        rc = lib.ssh_userauth_agent_client(self.libssh_session, None, self.libssh_error)
        return rc

    def userauthGssapi(self):
        rc = lib.ssh_userauth_gssapi_client(self.libssh_session, self.libssh_error)
        if (rc == SSH_ERROR):
            raise SSHException(lib.ssh_get_error(self.libssh_error).decode('utf-8'))
        return rc

    def setGssapiCreds(self, creds):
        lib.ssh_gssapi_set_creds_client(self.libssh_session, creds)

    def setBlocking(self, blocking):
        lib.ssh_set_blocking(self.libssh_session, 1 if blocking else 0)


    def _start_client(self):
        auth = False
        try:
            Logger().info("=================== Start Client")
            Logger().info("[Target] 'None' authentication ...")
            self.userauthNone()
            Logger().info("Waiting to userauthNone answer")
            while not self.userAuthNoneStatus:
                yield
                if self.userAuthNoneStatus == SSH_AUTH_SUCCESS:
                    Logger().info("[Target] Authentication Success (None)")
                    auth = True

            if (self.userAuthNoneStatus == SSH_ERROR):
                raise SSHException("UserAuth failed")

            Logger().info("=================== Start Client None authStatus = %s" %
                (self.userAuthNoneStatus,))


            if not auth:
                Logger().info("[Target] GSSAPI authentication ...")
                auth_list = self.userauthList()

#                Logger().info("=================== userauthGSSAPI")
                if auth_list & SSH_AUTH_METHOD_GSSAPI_MIC and self.gssapi_token is not None:
                    self.setGssapiCreds(self.gssapi_token)
                    rc = self.userauthGssapi()
                    if rc == SSH_AUTH_SUCCESS:
                        Logger().info("[Target] Authentication Success (GSSAPI)")
                        auth = True

                # TODO FL [Mon Jul 25 14:38:39 2011]: must be config variables
                pubkey = '/etc/opt/wab/ssh/key.pub'
                prikey = '/etc/opt/wab/ssh/key'
                passphrase = ''

                # try publickey auth without failing
#                Logger().info("=======> userauth PUBLICKEY use_agent=%s" % (
#                            self.use_agent))
                if not auth:
                    if auth_list & SSH_AUTH_METHOD_PUBLICKEY:
                        if self.use_agent:
                            Logger().info("[Target] Auth forwarding authentication ...")

                            from sshng.server_channels import WABAgentChannel
                            #TODO: see what can be merged between WABAgentChannel and WABAuthAgentChannel
                            agentchannel = WABAgentChannel(self.server)

                            #TODO: there should be things to move to WABAgentChannel
                            if agentchannel.libssh_agentchannel:
                                #Open Front side Agent Channel
                                rc = SSH_AGAIN
                                while rc is SSH_AGAIN:
                                    self.started = STARTED_AGAIN
                                    yield
                                    rc = lib.ssh_channel_open_auth_agent_server(
                                        self.server.libssh_session,
                                        agentchannel.libssh_agentchannel)

                                if rc == SSH_OK:
                                    rc = lib.ssh_set_agent_channel_client(
                                          self.libssh_session,
                                          agentchannel.libssh_agentchannel)

                                    Logger().info("PYTHON: Authentication through Agent Forwarding!!!")

                                    rc = lib.ssh_userauth_agent_client(self.libssh_session, agentchannel.session.libssh_session, self.user, self.libssh_error)
                                    while rc is SSH_AUTH_AGAIN:
                                        Logger().info("PYTHON: Authentication through Agent Forwarding!!!")
                                        yield
                                        rc = lib.ssh_userauth_agent_client(self.libssh_session, agentchannel.session.libssh_session, self.user, self.libssh_error)

                                    Logger().info("PYTHON [Target] Authentication Agent forwarding DONE")


                                    if rc == SSH_AUTH_SUCCESS:
                                        Logger().info(" PYTHON [Target] Authentication Success (Agent forwarding)")
                                        auth = True
                                    else:
                                        Logger().info("PYTHON [Target] Authentication Failed (Agent forwarding)")
                                    agentchannel.close()

                if not auth:
                    Logger().info("[Target] Password authentication ...")
                    Logger().info("=================== userauth PASSWORD")
                    if auth_list & (SSH_AUTH_METHOD_PASSWORD | SSH_AUTH_METHOD_INTERACTIVE):
                        try:
                            if self.server.passthrough_mode is True:
                                # Complete target password context from passthrough mode
                                target_password = self.server.passthrough_password
                            else:
                                target_password = self.wabengine_proxy.get_target_password(self.target_rights)
                            if ((not target_password)
                                    and (not self.server.scp_requested)):
                                self.linkedchannel.write_stdout_server("%s@%s:%s's password: " % (
                                    self.user,
                                    self.hname,
                                    self.service_name
                                    ))
                                self.data = ''
                                self.linkedchannel.buffering = False
                                while not ('\n' in self.data or '\r' in self.data):
                                    yield

                                target_password = self.data.replace('\r', '\n').split('\n')[0].decode('utf-8')
                                self.linkedchannel.write_stdout_server("\r\n")

                            Logger().info("!!!!!!!!!!!!!!!Connectiong with password %s!!!!!!!!!!!!!" % target_password);


                            rc = SSH_AUTH_DENIED
                            if auth_list & SSH_AUTH_METHOD_PASSWORD:
                                rc = self.userauthPassword(target_password.encode('utf8'))
                            elif auth_list & SSH_AUTH_METHOD_INTERACTIVE:
                                Logger().info("KbdInt Authentication (Password)")
                                import ctypes
                                echo = ctypes.c_char()
                                rc = lib.ssh_userauth_kbdint_client(self.libssh_session, self.user, None, self.libssh_error)
                                while rc == SSH_AUTH_INFO:
                                    nb_prompts = lib.ssh_userauth_kbdint_getnprompts_client(self.libssh_session, self.libssh_error)
                                    for i in range(0, nb_prompts):
                                        lib.ssh_userauth_kbdint_getprompt_client(self.libssh_session, i, ctypes.byref(echo), self.libssh_error)
                                        rc = lib.ssh_userauth_kbdint_setanswer_client(self.libssh_session, i, target_password.encode('utf8'), self.libssh_error)
                                    rc = lib.ssh_userauth_kbdint_client(self.libssh_session, self.user, None, self.libssh_error)

                            if rc == SSH_AUTH_SUCCESS:
                                Logger().info("Authentication Success (Password)")
                                auth = True

                            self.wabengine_proxy.release_target_password(self.target_rights)

                        except Exception, e:
                            Logger().info("%s" % traceback.format_exc(e))

            if not auth:
                self.started = STARTED_REJECTED
                return
        except Exception, exc:
            Logger().info("....Exception caught %s %s\n %s" % (type(exc), repr(exc), traceback.format_exc(exc)))
            self.started = STARTED_REJECTED
            return
#        Logger().info("=================== begin Start Session")
        # Start_session is the last thing we do when authentication is OK
        session_id = self.wabengine_proxy.start_session_ssh(
            self.target_rights, self.user, self.hname, self.host,
            self.server.client_addr, self.pid, self.linkedchannel.sub_protocol,
            self.kill_handler, effective_login=self.server.passthrough_login)

        if session_id:
            self.server.rename_pid_file_to_session_file_pid(session_id)


        self.started = STARTED_AUTHENTIFIED
#        Logger().info("=================== Session Started")
        Logger().info("[Target] Session Started")
        return

    def start_client(self):
        """The secondary authentication may still need some user interaction
        before getting a running session. The secondary session is not actually opened
        until start_client has confirmed it. (means secondary authentication success)
        """
        if self.started in [STARTED_AUTHENTIFIED, STARTED_REJECTED]:
            #TODO: check, this case should really never happen
            # because session start/authentication  is only performed once.
            return self.started
        try:
            self.starter.next()
        except StopIteration, e:
            pass
        except Exception, e:
            Logger().info("Client Starting Error... %s"
                          % traceback.format_exc(e))
            self.started = STARTED_REJECTED
        return self.started


    # SSH Session Callbacks API
    # =========================
    def cb_auth_none_reply_client(self, status):
        Logger().info("====================== %s::cb_auth_none_reply(status=%s)" %
            (self.__class__.__name__, status))
        self.userAuthNoneStatus = status
        return 0

    # SSH Session Callbacks API
    # =========================
    def auth(self, prompt, buf, length, echo, verify):
        Logger().info("====================== %s::cb_auth" %
            self.__class__.__name__)
        return 0

    # SSH Session Callbacks API
    # =========================
    def Log(self, priority, message):
        Logger().info("[sashimi client session] %s" % message)

    # SSH Session Callbacks API
    # =========================
    def connectStatus(self, status):
        Logger().info("====================== ClientSession connectStatus")
        pass

    # SSH Session Callbacks API
    # =========================
    def globalRequest(self, gtype, want_reply, bind_address, bind_port):
        return SSH_ERROR

    # SSH Session Callbacks API
    # =========================
    def channelOpenAuthAgentRequest(self):
        Logger().info("Channel open AuthAgent request")

        if not self.use_agent:
            self.linkedchannel.write_stdout_server("You don't have enough privileges to access to authentify through Agent Forwarding.\r\n")
            return None
        
        from sshng.server_channels import WABAuthAgentChannel
        Logger().info("New AuthAgent server channel")
        client_auth_agent_channel = WABAuthAgentChannel(self.server)
        Logger().info("New AuthAgent client channel")
        channel = AuthAgentClientChannel(self, client_auth_agent_channel)

        res = dotries(lambda: lib.ssh_channel_open_auth_agent_server(self.server.libssh_session, client_auth_agent_channel.libssh_channel))
        # Logger().info("libssh openx11 done is open = %s" % client_x11_channel.isOpen_server())
        if res != SSH_OK:
            Logger().info("Channel Open AuthAgent Failed")
            return None

        self.server.client_channels.append(client_auth_agent_channel)
        self.server.device_channels.append(channel)

        Logger().info("Returning AuthAgent client channel")
        return channel.libssh_channel
        


    # SSH Session Callbacks API
    # =========================
    def channelOpenX11Request(self, originator_address, originator_port, sender, window, packet_size):
        Logger().info("Channel open x11 request is %s" % self.x11session)
        if not self.x11session:
            self.linkedchannel.write_stdout_server("You don't have enough privileges to start X11 applications.\r\n")
            lib.ssh_event_x11_requested_channel_failure_client(self.libssh_session, sender)
            return
            
        try:
            from sshng.server_channels import WABX11Channel
            Logger().info("Creation of WABX11Channel")
            client_x11_channel = WABX11Channel(self.server)
            Logger().info("Creation of X11Channel")
            channel = X11ClientChannel(self, client_x11_channel)
            Logger().info("Call to openX11")
            client_x11_channel.openX11(channel, originator_address, originator_port, sender, window, packet_size)
        except Exception, e:
            import traceback
            Logger().info("OpenX11 request %s" % traceback.format_exc(e))
            

    def window_change_handler(self):
        """
        Override these method in client implementation to change the default
        behavior (no resize)
        """
        return False

    def kill(self):
        """
        Allow to close the channel and kill the current connection.
        """
        msg = "\r\nConnection closed by your administrator\r\n"
        self.linkedchannel.write_stderr_server(msg)
        self.wabengine_proxy.stop_session(unicode(msg))

    def kill_handler(self, signum, dummy):
        if signum == signal.SIGUSR1:
            self.kill()

    def __del__(self):
        """
        Call on client object deletion.
        Remove the client from the list of currently connected client.
        """
        self.wabengine_proxy.release_target_password(self.target_rights)
        Logger().info("SSH Disconnect")
#        lib.ssh_disconnect_client(self.libssh_session)
        # Should not have to call recorder destructor...
        self.wabengine_proxy.stop_session(u"Client deletion")
        try:
            waitpid(-1, 1)  # 1 is the value for os.WNOHANG
#        except Exception, exc:
        except:
            #No child for this process
            pass
        if self.libssh_session is not None:
            lib.ssh_free(self.libssh_session)


class SSHWABConsoleClient(object):
    def __init__(self, wabengine_proxy, target_rights, linkedchannel, svr_obj):

        self.wabengine_proxy = wabengine_proxy
        self.target_rights = target_rights
        self.server = svr_obj

        self.linkedchannel = linkedchannel

        self.pid = getpid()

        self.server._window_change_handler = self.window_change_handler

        self.client_addr = self.server.client_addr

        self.language = wabengine_proxy.get_language()
        self.started = STARTED_AUTHENTIFIED

    def window_change_handler(self):
        """
        Override these method in client implementation to change the default
        behavior (no resize)
        """
        return False

    def kill(self):
        """
        Close the channel and kill the current connection.
        """
        msg = "\r\nConnection closed.\r\n"
        self.linkedchannel.write_stderr_server(msg)
        self.linkedchannel.linkedchannel = None
        self.linkedchannel.close()

    def kill_handler(self, signum, dummy):
        if signum == signal.SIGUSR1:
            self.kill()

    def __del__(self):
        """
        Call on client object deletion.
        Remove the client from the list of currently connected client.
        """
        try:
            waitpid(-1, 1)  # 1 is the value for os.WNOHANG
#        except Exception, exc:
        except:
            #No child for this process
            pass


class KickStartTelnetClientChannel:
    """
    Internal Client Channel used to kickstart
    the secondary session authentication after
    opening of the primary channel inside primary
    session.
    """
    def __init__(self, session, linkedchannel, server):
        """
        Primary Channel is already opened (linkedchannel),
        but secondary channel can't be opened yet because
        secondary session authentication is ongoing.
        Actual Opening of secondary channel will be
        delayed until the secondary session is OK.
        """
        self.session = session
        self.linkedchannel = linkedchannel
        linkedchannel.linkedchannel = self
        self.server = server

    def close(self):
        Logger().info("@@@ %s.close()" % self.__class__.__name__)
        self.session.server.client_channels_to_close.add(self)
        self.channel_closed = True
        self.linkedchannel.close()

    def isOpen_client(self):
        return True

    def write_stderr_client(self, data):
        return None

    def write_stdout_client(self, data):
        Logger().info("KS Telnet Client write")
        try:
            self.session.data += data
        except Exception:
            self.session_data = ""

        self.session.start_client()
        if self.session.started is STARTED_AUTHENTIFIED:
            # for Telnet/Rlogin session is channel
            self.linkedchannel.linkedchannel = self.session
            self.session.linkedchannel = self.linkedchannel
            self.server.device_channels.append(self.session)
            self.server.device_channels.remove(self)
        elif self.session.started is STARTED_REJECTED:
            self.linkedchannel = None
            self.server.device_channels.remove(self)

        return len(data)


class RloginTelnetClient(object):
    def __init__(self, channel, wabengine_proxy, target_rights, svr_obj):
        self.analyzer = ProtoAnalyzer()
        self.linkedchannel = channel
        channel.linkedchannel = self
        # As RloginTelnetClient have both aspects of session and channel
        # We add a session member to it to get back the session from the channel
        # in code working for both ssh and telnet
        self.session = self

        self.closed = False
        self.wabengine_proxy = wabengine_proxy
        self.target_rights = target_rights
        self.server = svr_obj  # Reference to the SSH Server to set/or not window change size handler
        self.transport = None

        self.pid = getpid()

        # Set the behavior for window size request :
        self.server._window_change_handler = self.window_change_handler

        self.client_addr = self.server.client_addr

        self.language = wabengine_proxy.get_language()

        login_info = self.wabengine_proxy.get_target_login_info()
        self.user = login_info.account_login
        self.autologon = login_info.autologon
        self.host = login_info.device_host
        self.port = login_info.service_port
        self.hname = login_info.target_name
        self.service_name = login_info.service_name
        self.conn_cmd = login_info.conn_cmd.scenario

        self.target_password = u''
        if self.autologon:
            self.target_password = self.wabengine_proxy.get_target_password(self.target_rights)
        if self.target_password is None:
            self.target_password = u''

        self.connect_transport()
        self.callback = event_cb(CB_EVENT_FUNCS['event_cb'])

        lib.ssh_event_set_fd_client(channel.session.libssh_mainloop,
                                 self.transport.fileno(),
                                 self.callback,
                                 self)
        self.data = ""
        self.started = STARTED_INIT
        self.starter = self._start_client()

    def _start_client(self):
        status = False
        for line in self.conn_cmd.splitlines():
            if not line:
                continue

            cmd, val = line.strip().split(":", 1)
            cmd = cmd.strip()
            val = val.strip()
            val = val.replace("\\n", "\n").replace("\\r", "\r")
            val = val.replace("$login", self.user)

            if "$password" in val:
                if not self.target_password:
                    self.linkedchannel.write_stdout_server("%s@%s:%s's password: " % (
                        self.user,
                        self.host,
                        self.service_name
                    ))

                    while not ('\n' in self.data or '\r' in self.data):
                        # Session still starting
                        yield

                    self.target_password = self.data.replace('\r', '\n').split('\n')[0].decode('utf-8')
                    self.linkedchannel.write_stdout_server("\r\n")

            val = val.replace("$password", self.target_password)

            if cmd.upper() == "SEND":
                if not val.endswith("\r\n"):
                    val = val.rstrip()
                    val += "\r\n"
                self.transport.write(val.encode('utf-8'))
            elif cmd.upper() == "EXPECT":
                Logger().info("EXPECT %s " % (val))
                prompt = self.transport.expect([val], 3)
                Logger().info("EXPECT %s result prompt=%s" % (val, str(prompt)))
                if prompt[0] == -1:
                    status, message = False, "ERROR: Couldn't connect. Timeout reached"
                    Logger().info("%s" % message)
                    break
                status = True
            else:
                status, message = False, "Invalid auth mechanism, telnet/rlogin scenario failed"
                break

        if self.autologon:
            self.wabengine_proxy.release_target_password(self.target_rights)

        if not status:
            self.started, self.message = STARTED_REJECTED, message
            return
        if self.server.silent_mode is not True:
            self.linkedchannel.write_stdout_server("%s%s" % (
                self.wabengine_proxy.get_ssh_banner(self.language),
                self.wabengine_proxy.get_deconnection_time_msg(self.language)))

        self.wabengine_proxy.start_session_ssh(
            self.target_rights, self.user, self.hname, self.host,
            self.server.client_addr, self.pid, self.linkedchannel.sub_protocol,
            self.kill_handler, effective_login=self.server.passthrough_login)

        self.started, self.message = STARTED_AUTHENTIFIED, "OK"

        return

    def start_client(self):
        if self.started in [STARTED_AUTHENTIFIED, STARTED_REJECTED]:
            #TODO: check, this case should really never happen
            # because session start/authentication  is only performed once.
            return self.started
        try:
            self.starter.next()
        except StopIteration, e:
            pass
        except Exception, e:
            Logger().info("Client Starting Error... %s" % traceback.format_exc(e))
            self.started = STARTED_REJECTED
        return self.started

    def isOpen_client(self):
        #TODO: check what value should be returned
        return not self.closed

    def close(self):
        if not self.closed:
            self.transport.close()
            self.closed = True

    def connect_transport(self):
        Logger().info("Implemented by subclasses")

    def check_eof(self, data):
        pass
        
    def pollEvent(self, fd, revent):
        try:
            if(self.transport.fileno() != fd):
                raise RuntimeError("fd differs in callbacks")
        except Exception, e:
            self.linkedchannel.close()
            self.close()

        if (revent & select.POLLIN):
            try:
                data = self.transport.read()
                self.linkedchannel.write_stdout_server(data)
                self.server.wabengine_proxy.record(data)
                self.check_eof(data)
            except EOFError:
                self.linkedchannel.close()
                self.close()

        if (revent & select.POLLERR):
            self.linkedchannel.close()
            self.close()

        if (revent & select.POLLOUT):
            pass

        return 0

    def window_change_handler(self):
        """
        Override these method in client implementation to change the default
        behavior (no resize)
        """
        return False

    def kill(self):
        """
        Allow to close the channel and kill the current connection.
        """
        msg = "\r\nConnection closed by your administrator\r\n"
        self.linkedchannel.write_stderr_server(msg)
        self.wabengine_proxy.stop_session(unicode(msg))
        self.linkedchannel.close()

    def kill_handler(self, signum, dummy):
        """ TBD """
        if signum == signal.SIGUSR1:
            self.kill()

    def __del__(self):
        """
        Call on client object deletion.
        Remove the client from the list of currently connected client.
        """
        # Should not have to call recorder destructor...
        self.wabengine_proxy.stop_session(u"Client deletion")
        try:
            waitpid(-1, 1)  # 1 is the value for os.WNOHANG
        except:
            #No child for this process
            pass


class RloginClient(RloginTelnetClient):
    def connect_transport(self):
        self.transport = Rlogin(self.user, self.host, self.port)
        self.transport.connect(self.host)
        self.transport.shell_identification_procedure()

    def write_stdout_client(self, data):
        Logger().info("RloginClient write")
        return self.transport.write(data)

    def write_stderr_client(self, data):
        return self.transport.write(data)

    def check_eof(self, data):
        if not data:
            self.linkedchannel.close()
            self.close()


class TelnetClient(RloginTelnetClient):
    def connect_transport(self):
        self.transport = Telnet.Telnet()
        self.transport.open(self.host, int(self.port))

    def write_stdout_client(self, data):
        Logger().info("RloginClient write")
        return self.transport.write(data)

    def write_stderr_client(self, data):
        return self.transport.write(data)
        
        
