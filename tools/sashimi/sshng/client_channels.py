#!/usr/bin/python -O
# -*- coding: utf-8 -*-
#
# Copyright (c) 2015 WALLIX, SAS. All rights reserved.
#
# Licensed computer software. Property of WALLIX.

import traceback
import time

from logger import Logger

from sshng.pysshct import (
    lib,
    buildCallbacks,
    ssh_channel_callbacks_struct,
    CB_CHANNEL_FUNCS
    )

STARTED_INIT = 0
STARTED_AUTHENTIFIED = 1
STARTED_REJECTED = 2
STARTED_INTERACTIVE = 3
STARTED_AGAIN = 4

from sshng.common.proto_analyzer import (
    ProtoAnalyzer,
    SFTPAnalyzer,
    SCPAnalyzer,
    AnalyzerKill
    )


class KickStartClientChannel:
    """
    Internal Client Channel used to kickstart
    the secondary session authentication after
    opening of the primary channel inside primary
    session.
    """
    def __init__(self, session, linkedchannel, server, x11):
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
        self.x11 = x11
        self.session.server.device_channels.append(self)

    def close(self):
        Logger().info("@@@ %s.close()" % self.__class__.__name__)
        self.session.server.client_channels_to_close.append(self)
        self.channel_closed = True

    def isOpen_client(self):
        return True

    def write_stderr_client(self, data):
        return None

    def write_stdout_client(self, data):
        Logger().info("KS Client Channel")
        try:
            self.session.data = self.session.data + data
        except Exception:
            self.session.data = ""

        self.wakeup()

        return len(data)

    def wakeup(self):
        self.session.start_client()
        if self.session.started is STARTED_AUTHENTIFIED:
            targetchannel = SSHClientChannel(self.session,
                                                    self.linkedchannel,
                                                    self.x11)

            targetchannel.linkedchannel.linkedchannel = targetchannel
            targetchannel.linkedchannel = self.linkedchannel

            self.session.window_change_handler()

            lib.ssh_channel_request_shell_client(
                targetchannel.session.libssh_session, targetchannel.libssh_channel)

            self.server.device_channels.remove(self)

#            self.session.linkedchannel = targetchannel
        elif self.session.started is STARTED_REJECTED:
            Logger().info("reject")
            self.linkedchannel.write_stderr_server("\r\nFailed to connect to secondary target\r\n")
            self.linkedchannel.linkedchannel = None
            self.server.server_channels_to_close.append((self.linkedchannel, "OK"))
            self.server.wabengine_proxy.secondary_failed("Failed to connect to secondary target", self.server.wabuser, self.session.client_addr, self.session.user, self.session.host)

        return


class KickStartCmdClientChannel:
    """
    Internal Command Client Channel used to kickstart
    the secondary session authentication after
    opening of the primary channel inside primary
    session.
    """
    def __init__(self, session, linkedchannel, server, x11, cmd):
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
        self.x11 = x11
        self.cmd = cmd
        self.session.server.device_channels.append(self)


    def close(self):
        Logger().info("@@@ %s.close()" % self.__class__.__name__)
        self.session.server.client_channels_to_close.append(self)
        self.channel_closed = True

    def isOpen_client(self):
        return True

    def write_stderr_client(self, data):
        return None

    def write_stdout_client(self, data):
        Logger().info("KickStartCmdClientChannel write")

        try:
            self.session.data = self.session.data + data
        except Exception:
            self.session.data = ""

        self.wakeup()

        return len(data)

    def wakeup(self):
        self.session.start_client()
        if self.session.started is STARTED_AUTHENTIFIED:
            targetchannel = None
            if self.server.wabengine_proxy.detect_pattern(self.cmd):
                self.linkedchannel.write_stderr_server("\r\nUnauthorized Pattern Detected!!!\r\n")
                self.server.server_channels_to_close.append((self.linkedchannel, "OK"))
                self.server.wabengine_proxy.secondary_failed(
                    "Unauthorized Pattern Detected",
                    self.server.wabuser,
                    self.session.client_addr,
                    self.session.user,
                    self.session.host)
            else:
                self.server.wabengine_proxy.record(self.cmd.encode('utf-8'))
                targetchannel = SSHCmdClientChannel(self.session, self.linkedchannel, self.cmd, self.x11)

            self.server.device_channels.remove(self)

        elif self.session.started is STARTED_REJECTED:
            self.linkedchannel.write_stderr_server("\r\nFailed to connect to secondary target\r\n")
            self.server.server_channels_to_close.append((self.linkedchannel, "OK"))
            self.server.wabengine_proxy.secondary_failed("Failed to connect to secondary target", self.server.wabuser, self.session.client_addr, self.session.user, self.session.host)

        return


class KickStartSCPClientChannel:
    """
    Internal SCP Client Channel used to kickstart
    the secondary session authentication after
    opening of the primary channel inside primary
    session.
    """
    def __init__(self, session, linkedchannel, server, cmd, way, path):
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
        self.cmd = cmd
        self.way = way
        self.path = path
        self.delayed_data = None
        self.session.server.device_channels.append(self)


    def close(self):
        Logger().info("@@@ %s.close()" % self.__class__.__name__)
        self.session.server.client_channels_to_close.append(self)
        self.channel_closed = True

    def isOpen_client(self):
        return True

    def write_stderr_client(self, data):
        # Logger().info("Kickstart SCP write stderr: '%s' len : %s" % (data, len(data)))
        return None

    def write_stdout_client(self, data):
        # what = ":".join(hex(ord(c)) for c in data)
        # Logger().info("Kickstart SCP write stdout: '%s' len : %s" % (what, len(data)))
        Logger().info("Kickstart SCP write stdout")
        try:
            self.session.data = self.session.data + data
        except Exception:
            self.session.data = ""
        if self.delayed_data is None:
            self.delayed_data = data
        else:
            self.delayed_data = self.delayed_data + data
        return len(data)

    def wakeup(self):
        self.session.start_client()
        if self.session.started is STARTED_AUTHENTIFIED:
            targetchannel = None
            if self.server.wabengine_proxy.detect_pattern(self.cmd):
                self.linkedchannel.write_stderr_server("\r\nUnauthorized Pattern Detected!!!\r\n")
                self.server.server_channels_to_close.append((self.linkedchannel, "OK"))
                self.server.wabengine_proxy.secondary_failed("Unauthorized Pattern Detected", self.server.wabuser, self.session.client_addr, self.session.user, self.session.host)
            else:
                self.server.wabengine_proxy.record("%s '%s'\r\n" % (self.way, self.path))
                targetchannel = SCPClientChannel(self.session, self.linkedchannel, self.cmd)
                if not self.delayed_data is None:
                    targetchannel.write_stdout_client(self.delayed_data)
            self.server.device_channels.remove(self)

        elif self.session.started is STARTED_REJECTED:
            self.linkedchannel.write_stderr_server("\r\nFailed to connect to secondary target\r\n")
            self.server.server_channels_to_close.append((self.linkedchannel, "OK"))
            self.server.wabengine_proxy.secondary_failed("Failed to connect to secondary target", self.server.wabuser, self.session.client_addr, self.session.user, self.session.host)

        return


class KickStartSFTPClientChannel:
    """
    Internal SFTP Client Channel used to kickstart
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
        self.delayed_data = None
        # Logger().info("SFTP kickstart started")
        self.session.server.device_channels.append(self)


    def close(self):
        Logger().info("@@@ %s.close()" % self.__class__.__name__)
        self.session.server.client_channels_to_close.append(self)
        self.channel_closed = True

    def isOpen_client(self):
        return True

    def write_stderr_client(self, data):
        return None

    def write_stdout_client(self, data):
        # what = ":".join(hex(ord(c)) for c in data)
        # Logger().info("Kickstart SFTP write stdout: '%s' len : %s" % (what, len(data)))
        Logger().info("Kickstart SFTP write stdout")
        try:
            self.session.data = self.session.data + data
        except Exception:
            self.session.data = ""
        if self.delayed_data is None:
            self.delayed_data = data
        else:
            self.delayed_data = self.delayed_data + data
        return len(data)

    def wakeup(self):
        Logger().info("WAKEUP kickstart SFTP %s, server %s, session %s" % (id(self),
                                                                       id(self.server),
                                                                       id(self.session)))
        self.session.start_client()

        if self.session.started is STARTED_AUTHENTIFIED:
            self.linkedchannel.linkedchannel = SFTPClientChannel(self.session, self.linkedchannel)
            self.server.setBlocking(True)
            # Logger().info("SFTP subsystem starting from kick start")
            # for SSH Session open new channel
        elif self.session.started is STARTED_REJECTED:
            self.linkedchannel.write_stderr_server("\r\nFailed to connect to secondary target\r\n")
            self.server.server_channels_to_close.append((self.linkedchannel, "OK"))
            self.server.wabengine_proxy.secondary_failed("Failed to connect to secondary target", self.server.wabuser, self.session.client_addr, self.session.user, self.session.host)

        return


class SSHClientError(Exception):
    """ SSHClient custom exception """
    pass


class ClientChannel(object):

    # ========== Common Channel Interface ===================
    def __init__(self, session, linkedchannel=None):
        self.session = session
        self.callbacks = buildCallbacks(ssh_channel_callbacks_struct, CB_CHANNEL_FUNCS, self)
        self.libssh_channel = lib.ssh_new_channel(session.libssh_session, self.callbacks)
        self.linkedchannel = linkedchannel
        linkedchannel.linkedchannel = self
        self.channel_closed = False
        self.session.server.device_channels.append(self)

    def cb_data_stdout(self, data):
        Logger().info("@@%s.cb_data_stdout" % (self.__class__.__name__))
        return self.linkedchannel.write_stdout_server(data)

    def write_stdout_client(self, data):
        return lib.ssh_channel_write_client(self.session.libssh_session, self.libssh_channel, data, len(data))

    def cb_data_stderr(self, data):
        Logger().info("@@%s.cb_data_stderr" % (self.__class__.__name__))
        return self.linkedchannel.write_stderr_server(data)

    def write_stderr_client(self, data):
        return lib.ssh_channel_write_stderr_client(self.session.libssh_session, self.libssh_channel, data, len(data))

    def cb_eof(self):
        Logger().info("@@%s.cb_eof" % (self.__class__.__name__))
        self.linkedchannel.sendEof()

    def sendEof(self):
        Logger().info("@@%s.sendEof" % (self.__class__.__name__))
        return lib.ssh_channel_send_eof_client(self.session.libssh_session, self.libssh_channel)

    def cb_close(self):
        try:
            Logger().info("@@%s.cb_close" % (self.__class__.__name__))
            if not self.channel_closed:
                # Close originated from remote side we have to propagate it
                # Otherwise it is an answer to a previous SSH_MSG_CLOSE IO sent
                # and there is no reason to propagate it
                self.close()
                if not self.linkedchannel.channel_closed:
                    self.linkedchannel.close()

            # Anyway we will have to close channels
            self.session.server.client_channels_to_close.append(self)
        except Exception, e:
            Logger().info("@@%s.cb_close <<<%s>>>" % (self.__class__.__name__, traceback.format_exc(e)))
        
        return 0

    def close(self):
        try:
            Logger().info("@@%s.close" % (self.__class__.__name__))
            #I should only do that once, if I initiate several closes it's an error
            if not self.channel_closed:
                self.channel_closed = True
                lib.ssh_channel_close_client(self.session.libssh_session, self.libssh_channel)
        except Exception, e:
            Logger().info("@@%s.cb_close <<<%s>>>" % (self.__class__.__name__, traceback.format_exc(e)))


    def isOpen_client(self):
        rc = lib.ssh_channel_is_open_client(self.session.libssh_session, self.libssh_channel)
        return rc != 0

    def isEof(self):
        rc = lib.ssh_channel_is_eof_client(self.libssh_channel)
        return rc != 0

    # ========== Client Channel Interface ===================

    def cb_exitStatus(self, status):
        Logger().info("@@%s.cb_exitStatus" % (self.__class__.__name__))
        self.linkedchannel.sendExitStatus(status)
        return 0

    def cb_exitSignal(self, signalp, corep, errmsgp, langp):
        Logger().info("@@%s.cb_exitSignal" % (self.__class__.__name__))
        self.linkedchannel.sendExitSignal(signalp, corep, errmsgp, langp)

    def requestAuthAgent(self):
        return lib.ssh_channel_request_auth_agent_client(self.session.libssh_session, self.libssh_channel)

    def requestSubsystem(self, subsystem):
        return lib.ssh_channel_request_subsystem_client(self.session.libssh_session, self.libssh_channel, subsystem)

    def __del__(self):
        if self.libssh_channel is not None:
            lib.ssh_channel_free_client(self.session.libssh_session, self.libssh_channel)


class X11ClientChannel(ClientChannel):
    def __init__(self, session, linkedchannel=None):
        super(X11ClientChannel, self).__init__(session, linkedchannel)
        Logger().info("[WAB] X11 channel created")
        self.analyzer = ProtoAnalyzer()

class AuthAgentClientChannel(ClientChannel):
    def __init__(self, session, linkedchannel=None):
        super(AuthAgentClientChannel, self).__init__(session, linkedchannel)
        self.analyzer = ProtoAnalyzer()

    def cb_close(self):
        Logger().info("@@%s.cb_close" % (self.__class__.__name__))
        self.linkedchannel.close()
        self.close()
        return 0

class SFTPClientChannel(ClientChannel):
    def __init__(self, session, linkedchannel):
        super(SFTPClientChannel, self).__init__(session, linkedchannel)
        self.analyzer = ProtoAnalyzer()

        lib.ssh_sessionchannel_open_client(self.session.libssh_session, self.libssh_channel)
        self.analyzer = SFTPAnalyzer(
            self.session.server.wabengine_proxy.record,
            self.session.server.wabengine_proxy.detect_filesize)
        self.requestSubsystem("sftp")

    def cb_kill_pattern(self, msg):
        self.linkedchannel.write_stderr_server("\r\n%s\r\n" % msg)
        self.session.setBlocking(False)
        self.linkedchannel.close()
        self.close()

    def cb_data_stdout(self, data):
        if self.linkedchannel.sftp_to_close:
            self.cb_kill_pattern(self.linkedchannel.sftp_error_msg)
            self.linkedchannel.sftp_to_close = False
        try:
            self.analyzer.parse_down_data(data)
        except AnalyzerKill, e:
            self.cb_kill_pattern(e.msg)
            return len(data)
        except Exception, e:
            Logger().info(">>>> %s" % traceback.format_exc(e))

        return super(SFTPClientChannel, self).cb_data_stdout(data)

    def wakeup(self):
        if not self.session.started is STARTED_AUTHENTIFIED:
            self.linkedchannel.session.channels_to_close.append((self, "OK"))


class SSHTCPIPClientChannel(ClientChannel):
    def __init__(self, session, linkedchannel):
        super(SSHTCPIPClientChannel, self).__init__(session, linkedchannel)
        self.analyzer = ProtoAnalyzer()

        lib.ssh_channel_open_forward_client(
            self.session.libssh_session,
            self.libssh_channel,
            linkedchannel.dest, linkedchannel.dest_port,
            linkedchannel.orig, linkedchannel.orig_port)


#TODO: merge with SSHCmd ?
class SCPClientChannel(ClientChannel):
    def __init__(self, session, linkedchannel, command):
        super(SCPClientChannel, self).__init__(session, linkedchannel)

        lib.ssh_sessionchannel_open_client(self.session.libssh_session, self.libssh_channel)

        self.analyzer = SCPAnalyzer(
            self.session.server.wabengine_proxy.record,
            self.session.server.wabengine_proxy.detect_filesize)

        if session.use_agent:
            self.requestAuthAgent()

        lib.ssh_channel_request_exec_client(self.session.libssh_session, self.libssh_channel, command)


    def cb_data_stdout(self, data):
        try:
            self.analyzer.parse_down_data(data)
        except AnalyzerKill, e:
            self.session.server.server_channels_to_close.append((self.linkedchannel, e.msg))
            return len(data)
        except Exception, e:
            Logger().info(">>>> %s" % traceback.format_exc(e))
        self.linkedchannel.session.lastactiontime = time.time()

        return super(SCPClientChannel, self).cb_data_stdout(data)


class SSHCmdClientChannel(ClientChannel):
    def __init__(self, session, linkedchannel, command, x11):
        super(SSHCmdClientChannel, self).__init__(session, linkedchannel)

        self.analyzer = ProtoAnalyzer()

        lib.ssh_sessionchannel_open_client(self.session.libssh_session, self.libssh_channel)

        if self.linkedchannel.pty_requested:
            lib.ssh_channel_request_pty_size_client(self.session.libssh_session, self.libssh_channel,
                linkedchannel.session.term, linkedchannel.session.width, linkedchannel.session.height)
        Logger().info("SSHCmdClientChannel session use_agent=%s" % session.use_agent)

        if session.use_agent:
            self.requestAuthAgent()

        if self.linkedchannel.env_requested:
            for (name, value) in self.linkedchannel.envs_to_send:
                lib.ssh_channel_request_env_client(self.session.libssh_session,
                                                   self.libssh_channel,
                                                   name, value)
        if x11:
            lib.ssh_channel_request_x11_client(
                        self.session.libssh_session,
                        self.libssh_channel,
                        x11.single_conn,
                        x11.auth_proto,
                        x11.auth_cookie,
                        x11.screen_number)

        lib.ssh_channel_request_exec_client(self.session.libssh_session, self.libssh_channel, command)

    def cb_data_stdout(self, data):
        self.session.wabengine_proxy.record(data)
        return super(SSHCmdClientChannel, self).cb_data_stdout(data)

    def cb_data_stderr(self, data):
        self.session.wabengine_proxy.record(data)
        return super(SSHCmdClientChannel, self).cb_data_stderr(data)


class SSHClientChannel(ClientChannel):
    def __init__(self, session, linkedchannel, x11):
        super(SSHClientChannel, self).__init__(session, linkedchannel)
        self.analyzer = ProtoAnalyzer()

        lib.ssh_sessionchannel_open_client(self.session.libssh_session, self.libssh_channel)

        if self.linkedchannel.pty_requested:
            lib.ssh_channel_request_pty_size_client(self.session.libssh_session, self.libssh_channel,
                linkedchannel.session.term, linkedchannel.session.width, linkedchannel.session.height)

        if session.use_agent:
            self.requestAuthAgent()

        if self.linkedchannel.env_requested:
            for (name, value) in self.linkedchannel.envs_to_send:
                lib.ssh_channel_request_env_client(self.session.libssh_session,
                                                   self.libssh_channel,
                                                   name, value)
        if x11:
            lib.ssh_channel_request_x11_client(
                    self.session.libssh_session,
                    self.libssh_channel,
                    x11.single_conn,
                    x11.auth_proto,
                    x11.auth_cookie,
                    x11.screen_number)

        if self.session.server.silent_mode is not True:
            self.linkedchannel.write_stdout_server("%s%s" %(
                 session.wabengine_proxy.get_ssh_banner(session.language),
                 session.wabengine_proxy.get_deconnection_time_msg(session.language)))

    def cb_data_stdout(self, data):
        try:
            if (not self.linkedchannel.winscp
            and not self.linkedchannel.pty_requested
            and "WinSCP: this is end-of-file:" in data):
                self.linkedchannel.winscp = True

            if not self.linkedchannel.winscp:
                self.session.wabengine_proxy.record(data)
        except Exception, e:
            Logger().info(">>>>>> %s" % traceback.format_exc(e))
            return 0

        return super(SSHClientChannel, self).cb_data_stdout(data)

    def cb_data_stderr(self, data):
        try:
            self.session.wabengine_proxy.record(data)
        except Exception, e:
            Logger().info(">>>>>> %s" % traceback.format_exc(e))
            return 0

        return super(SSHClientChannel, self).cb_data_stderr(data)



