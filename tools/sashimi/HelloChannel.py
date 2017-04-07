
# sys import
import traceback
import time
from datetime import datetime
import select

from sshng import pysshct
from sshng.pysshct import (
    lib,
    buildCallbacks,
    CB_CHANNEL_FUNCS,
    ssh_channel_callbacks_struct,
    SSH_ERROR,
    SSH_OK
    )

# wab proxy import
from sshng.common.sshselector import SSHSelector
from sshng.common.wabconsole import WABConsole
from sshng.common.utils import mdecode, parse_ssh_auth

from logger import Logger

from sshng.client_channels import (
    SCPClientChannel,
    KickStartClientChannel,
    KickStartCmdClientChannel,
    KickStartSCPClientChannel,
    KickStartSFTPClientChannel
    )

from sshng.client import (
    RloginClient,
    TelnetClient,
    SSHWABConsoleClient,
    KickStartTelnetClientChannel,
    STARTED_INIT,
    STARTED_AUTHENTIFIED,
    STARTED_REJECTED,
    STARTED_INTERACTIVE,
    STARTED_AGAIN
    )

from wab.engine import APPROVAL_ACCEPTED, APPROVAL_REJECTED, APPROVAL_PENDING, APPROVAL_NONE

from sshng.common.proto_analyzer import AnalyzerKill

import ctypes


class ServerChannel(object):
    def __init__(self, session):
        self.session = session
        self.callbacks = buildCallbacks(ssh_channel_callbacks_struct, CB_CHANNEL_FUNCS, self)
        self.libssh_channel = lib.ssh_new_channel(session.libssh_session, self.callbacks)
        self.linkedchannel = None
        self.channel_closed = False
        self.session.server_channels.append(self)

    def cb_eof(self):
        Logger().info("@@%s.cb_eof" % (self.__class__.__name__))
        if self.linkedchannel:
            self.linkedchannel.sendEof()
        return 0

    def isEof(self):
        Logger().info("@@%s.isEof" % (self.__class__.__name__))
        rc = lib.ssh_channel_is_eof_server(self.session.libssh_session, self.libssh_channel)
        return rc != 0

    def sendEof(self):
        Logger().info("@@%s.sendEof" % (self.__class__.__name__))
        return lib.ssh_channel_send_eof_server(self.session.libssh_session, self.libssh_channel)

    def isOpen_server(self):
        Logger().info("@@%s.isOpen_server" % (self.__class__.__name__))
        rc = lib.ssh_channel_is_open_server(self.session.libssh_session, self.libssh_channel)
        return rc != 0

    def sendExitStatus(self, status):
        return lib.ssh_channel_request_send_exit_status_server(self.session.libssh_session, self.libssh_channel, status)

    def sendExitSignal(self, signalp, corep, errmsgp, langp):
        return lib.ssh_channel_request_send_exit_signal_server(self.session.libssh_session, self.libssh_channel,
                                                            signalp, corep, errmsgp,
                                                            langp)

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
            self.session.server_channels_to_close.append((self, None))
        except Exception, e:
            Logger().info("@@%s.cb_close <<<%s>>>" % (self.__class__.__name__, traceback.format_exc(e)))
        return 0

    def close(self):
        try:
            Logger().info("@@%s.close" % (self.__class__.__name__))
            #I should only do that once, if I initiate several closes it's an error
            if not self.channel_closed:
                self.channel_closed = True            
                lib.ssh_channel_close_server(self.session.libssh_session, self.libssh_channel)
        except Exception, e:
            Logger().info("@@%s.close <<<%s>>>" % (self.__class__.__name__, traceback.format_exc(e)))

class WABChannel(ServerChannel):
    def __init__(self, session):
        super(WABChannel, self).__init__(session)

        self.args = []
        self.client_launched = False
        self.sub_protocol = None
        self.client_addr = session.client_addr
        self.x11 = None
        self.trace = None

        self.sftp_requested = False
        self.sftp_to_close = False
        self.sftp_error_msg = None
        # fd_closed is used to know if a channel is really closed
        # ssh_channel_is_closed() have to be patched
        self.fd_closed = False
        self.pty_requested = False
        self.winscp = False
        self.env_requested = False
        self.envs_to_send = []
        # self.data_num = 0
                

    def write_stderr_server(self, data):
        return lib.ssh_channel_write_stderr_server(self.session.libssh_session, self.libssh_channel, data, len(data))

    def write_stdout_server(self, data):
        return lib.ssh_channel_write_server(self.session.libssh_session, self.libssh_channel, data, len(data))

    def __del__(self):
        if self.libssh_channel is not None:
            lib.ssh_channel_free_server(self.session.libssh_session, self.libssh_channel)

    def cb_signal(self, signal):
        Logger().info("@@@ WABChannel.cb_signal()")
        pass

    def cb_requestPty(self, term, width, height, pxwidth, pxheight):
        # Logger().info("========================== ServerChannel %s cb_requestPty" % id(self))
        if self.sub_protocol is None:
            self.sub_protocol = u'SSH_SHELL_SESSION'
        self.session.term = term
        self.session.width = width
        self.session.height = height
        self.session.pixelwidth = pxwidth
        self.session.pixelheight = pxheight
        self.pty_requested = True
        return SSH_OK

    def target_selected(self, target):
        status, message = self.session.interactiveShellChannel(self, target)
        if not status:
            #TODO: can't check before writing. Target may return some error
            #TODO: send message now, no need to wait
            if self.isOpen_server():
                self.session.server_channels_to_close.append((self, message))
            return None
        return True

    def cb_requestShell(self):
        Logger().info("========================== ServerChannel %s cb_requestShell" % id(self))
        try:
            Logger().info("[WAB] Shell session requested")
            self.session.setBlocking(False)

#            if self.isLicenseStatusOK() is not True:
#                return SSH_OK

            if self.session.wabengine_proxy.get_force_change_password():
                #TODO: can't check before writing. Target may return some error
                #TODO: send message now, no need to wait
                if self.isOpen_server():
                    message = "You must change your password to access your accounts"
                    self.session.server_channels_to_close.append((self, message))
                    return SSH_OK
                return SSH_ERROR

            if not self.session.target:
                self.session.wabengine_proxy.get_proxy_rights(
                    [u'SSH', u'TELNET', u'RLOGIN'],
                    target_device=None, check_timeframes=False,
                    target_context=self.session.target_context)
                # TODO : target priority
                if len(self.session.wabengine_proxy.displaytargets) == 1:
                    tmp_target = self.session.wabengine_proxy.displaytargets[0]
                    self.session.target = (tmp_target.target_login,
                                           tmp_target.target_name,
                                           tmp_target.service_name)
                else:
                    if self.session.silent_mode is True:
                        return SSH_ERROR
                    self.linkedchannel = SSHSelector(self,
                                                     self.session.wabengine_proxy.displaytargets,
                                                     self.target_selected)
                    if self.linkedchannel.status == 3:
                        #TODO: can't check before writing. Target may return some error
                        #TODO: send message now, no need to wait
                        if self.isOpen_server():
                            self.session.server_channels_to_close.append((self,
                                                                      "No reachable target."))
                    return SSH_OK

            self.linkedchannel = SSHSelector(self,
                                             self.session.wabengine_proxy.displaytargets,
                                             self.target_selected,
                                             is_direct=True)
            if self.session.target == 'console':
                status, message = self.session.interactiveShellChannel(self, self.session.target)
                if not status:
                    #TODO: can't check before writing. Target may return some error
                    #TODO: send message now, no need to wait
                    if self.isOpen_server():
                        self.session.server_channels_to_close.append((self, message))
                        return SSH_OK
                    return SSH_ERROR
            else:
                status, message = self.linkedchannel.direct_approval(self.session.target)
                if status == APPROVAL_REJECTED:
                    #TODO: can't check before writing. Target may return some error
                    #TODO: send message now, no need to wait
                    if self.isOpen_server():
                        self.session.server_channels_to_close.append((self, message))
                        return SSH_OK
                    return SSH_ERROR
            return SSH_OK
            
        except Exception, e:
            import traceback
            Logger().info("Shell request exception: %s" % traceback.format_exc(e))

    def cb_requestAuthAgent(self):
        # Logger().info("========================== ServerChannel %s cb_requestAuthAgent" % id(self))
        self.session.ssh_agent_requested = True
        return SSH_OK

    def cb_requestX11(self, single_conn, auth_proto, auth_cookie, screen_number):
        Logger().info("========================== ServerChannel %s cb_requestX11" % id(self))
        Logger().info("[WAB] X11 forwarding requested")
        """ Set the x11 channel """
        self.session.x11_requested = True
        self.sub_protocol = u'SSH_SHELL_SESSION'

        class X11Info(object):
            def __init__(self,  single_conn, auth_proto, auth_cookie, screen_number):
                self.single_conn = single_conn
                self.auth_proto = auth_proto
                self.auth_cookie = auth_cookie
                self.screen_number = screen_number

        self.session.x11 = self.x11 = X11Info(single_conn, auth_proto, auth_cookie, screen_number)
        return SSH_OK

    def cb_ptyWindowChange(self, width, height, pxwidth, pxheight):
        #TODO: Pty should work either if linked channel is open before
        # or after calling cb_ptyWindowChange
        Logger().info("SSH Channel pty Window Change %s %s %s %s" % (width, height, pxwidth, pxheight))
        self.session.width = width
        self.session.height = height
        self.session.pixelwidth = pxwidth
        self.session.pixelheight = pxheight

        lib.ssh_channel_change_pty_size_client(
            self.linkedchannel.session.libssh_session, 
            self.linkedchannel.libssh_channel, width, height)

    def cb_requestExec(self, command):
        # Logger().info("======= requestExec = '%s'" % command)
        self.session.setBlocking(False)

        if (self.session.target is None and
                command.strip() in ["console", "wabconsole"]):
            self.session.target = 'console'

        if self.session.target == 'console':
            return self.cb_requestShell()

        if command[:4] == 'scp ':
            Logger().info("[WAB] SCP requested")
            self.session.scp_requested = True
            way_string = 'unknown'
            args = command.split()
            #find first non optional argument
            for i in range(1, len(args)):
                if args[i][0] == '-':
                    if args[i] == '-t':
                        scp_sub_protocol = u'SSH_SCP_UP'
                        way_string = 'upload'
                    if args[i] == '-f':
                        scp_sub_protocol = u'SSH_SCP_DOWN'
                        way_string = 'download'
                    continue
                path, target = parse_ssh_auth(args[i])

                if target:
                    if path == '':
                        path = '.'
                    args[i] = path
                    self.session.target = target
            command = ' '.join(args)

            Logger().info("[WAB] SCP %s" %args)
            Logger().info("SCP command: %s" % command)


            if self.session.real_target_ip: # transparent
                Logger().info("[WAB] SCP requested transparent")
                self.session.wabengine_proxy.get_proxy_rights(
                    [u'SSH'], target_device=None,
                    target_context=self.session.target_context)
                if len(self.session.wabengine_proxy.displaytargets) == 1:
                    tmp_target = self.session.wabengine_proxy.displaytargets[0]
                    self.session.target = (tmp_target.target_login,
                                           tmp_target.target_name,
                                           tmp_target.service_name)
                if target:
                    if (target[0] != self.session.target[0] or
                        target[1] != self.session.target[1] or
                        (target[2] and (target[2] != self.session.target[2]))):
                        self.session.target = None
                        return SSH_ERROR

            Logger().info("SCP target: %s" % str(self.session.target))

            target_login, target_device, target_service = self.session.target

            Logger().info("[WAB] SCP requested get_target_rights")
            self.session.target_rights, message = self.session.wabengine_proxy.get_target_rights(
                mdecode(target_login),
                mdecode(target_device),
                mdecode(target_service))

            if not self.session.target_rights:
                self.session.server_channels_to_close.append((self, message))
                return SSH_OK

            Logger().info("[WAB] SCP requested check_target")

            approval_status, approval_info = self.session.wabengine_proxy.check_target(self.session.target_rights, None)
            if approval_status != APPROVAL_ACCEPTED:
                self.session.server_channels_to_close.append((self, approval_info['message'].replace('\n', '\r\n').encode('utf-8')))
                return SSH_OK

            Logger().info("[WAB] SCP requested get_targetsession")
            targetsession, message = self.session.get_targetsession(self, scp_sub_protocol)

            if not targetsession:
                self.session.server_channels_to_close.append((self, message))
                return SSH_OK

            if not targetsession.started in [STARTED_AUTHENTIFIED, STARTED_REJECTED]:
                Logger().info("[WAB] SCP Kickstarting")
                targetchannel = KickStartSCPClientChannel(targetsession, self,
                                                          self.session, command,
                                                          way_string, path)
            else:
                Logger().info("[WAB] SCP Starting")
                self.session.wabengine_proxy.record("%s '%s'\r\n" % (way_string, path))
                targetchannel = SCPClientChannel(targetsession, self, command)

            self.session.device_channels.append(targetchannel)

            Logger().info("[WAB] End of SCP requested")
            
        else:
            Logger().info("[WAB] Exec requested")
            Logger().info("Exec command: %s" % command)
            args = command.split()
            if not self.session.target:
                _, target = parse_ssh_auth("%s:" % args[0])
                if target:
                    self.session.target = target
                    command = ' '.join(args[1:])
                    if command == '':
                        return self.cb_requestShell()

            Logger().info("[WAB] remote command requested")
            if self.session.real_target_ip: # transparent
                Logger().info("[WAB] remote command transparent")
                saved_target = self.session.target
                self.session.wabengine_proxy.get_proxy_rights(
                    [u'SSH'], target_device=None,
                    target_context=self.session.target_context)
                if len(self.session.wabengine_proxy.displaytargets) == 1:
                    tmp_target = self.session.wabengine_proxy.displaytargets[0]
                    self.session.target = (tmp_target.target_login,
                                           tmp_target.target_name,
                                           tmp_target.service_name)
                if saved_target is not None:
                    if (saved_target[0] != self.session.target[0] or
                        saved_target[1] != self.session.target[1] or
                        (saved_target[2] and
                         (saved_target[2] != self.session.target[2]))):
                        return SSH_ERROR

            if not self.session.target:
                self.session.server_channels_to_close.append((self, "invalid syntax"))
                return SSH_OK

            Logger().info("SSH channel opened for remote command execution")

            target_login, target_device, target_service = self.session.target

#            if self.isLicenseStatusOK() is not True:
#                return SSH_OK

            Logger().info("[WAB] remote command get_target_rights")
            self.session.target_rights, message = self.session.wabengine_proxy.get_target_rights(
                mdecode(target_login),
                mdecode(target_device),
                mdecode(target_service))

            if not self.session.target_rights:
                self.session.server_channels_to_close.append((self, message))
                return SSH_OK

            approval_status, approval_info = self.session.wabengine_proxy.check_target(self.session.target_rights, None)
            if approval_status != APPROVAL_ACCEPTED:
                self.session.server_channels_to_close.append((self, approval_info['message'].replace('\n', '\r\n').encode('utf-8')))
                return SSH_OK

            Logger().info("[WAB] remote command get_targetsession")
            targetsession, message = self.session.get_targetsession(self, u'SSH_REMOTE_COMMAND')
            if not targetsession:
                self.session.server_channels_to_close.append((self, message))
                Logger().info("no target session for remote command")
                return SSH_OK

            try:
                Logger().info("[WAB] Remote exec Kickstarting")
                targetchannel = KickStartCmdClientChannel(targetsession, self,
                                                          self.session,
                                                          self.session.x11, command)

                if targetsession.started in [STARTED_AUTHENTIFIED, STARTED_REJECTED]:
                    targetchannel.wakeup()

            except Exception, exc:
                Logger().info("Failed to start command client")
                self.write_stderr_server("Failed to start command client\r\n")
                Logger().info("Exception caught %s %s\n%s" % (type(exc), repr(exc), traceback.format_exc()))
                self.session.wabengine_proxy.secondary_failed(
                    "Failed to start command client", 
                    self.session.wabuser, 
                    targetsession.client_addr, 
                    targetsession.user, 
                    targetsession.host)

            self.session.device_channels.append(targetchannel)
            
        Logger().info("=== RequestExec or SCP Done OK")
        return SSH_OK

    def cb_requestEnv(self, env_name, env_value):
        # Logger().info("======== ServerChannel %s cb_requestEnv (%s = %s)" % (id(self),
        #                                                                      env_name,
        #                                                                      env_value))
        # return SSH_ERROR
        self.env_requested = True
        self.envs_to_send.append((env_name, env_value))
        return SSH_OK

    def cb_requestSubsystem(self, subsystem):
        # Logger().info("========================== ServerChannel %s cb_requestSubsystem" % id(self))
        Logger().info("[WAB] subsystem requested %s !!!" % subsystem)

#        self.setBlocking(False)
        
        Logger().info("[WAB] subsystem requested %s setBlocking False" % subsystem)
        
        try:
            Logger().info("Try")
            if not self.session.target:
                if self.session.real_target_ip: # transparent
                    self.session.wabengine_proxy.get_proxy_rights(
                        [u'SSH'], target_device=None,
                        target_context=self.session.target_context)
                    if len(self.session.wabengine_proxy.displaytargets) == 1:
                        tmp_target = self.session.wabengine_proxy.displaytargets[0]
                        self.session.target = (tmp_target.target_login,
                                               tmp_target.target_name,
                                               tmp_target.service_name)
                    else:
                        return SSH_ERROR
                else:
                    return SSH_ERROR

            target_login, target_device, target_service = self.session.target

#            if self.isLicenseStatusOK() is not True:
#                return SSH_OK

            Logger().info("get_target_rights")
            self.session.target_rights, message = self.session.wabengine_proxy.get_target_rights(
                mdecode(target_login),
                mdecode(target_device),
                mdecode(target_service))

            if not self.session.target_rights:
                self.session.server_channels_to_close.append((self, message))
                return SSH_OK

            approval_status, approval_info = self.session.wabengine_proxy.check_target(self.session.target_rights, None)
            if approval_status != APPROVAL_ACCEPTED:
                self.session.server_channels_to_close.append((self, approval_info['message'].replace('\n', '\r\n').encode('utf-8')))
                return SSH_OK

            if subsystem == "sftp":
                Logger().info("SFTP subsystem requested")
                targetsession, message = self.session.get_targetsession(self, 'SFTP_SESSION')
                Logger().info("got targetsession %s %s" % (targetsession, message))
                if not targetsession:
                    self.session.server_channels_to_close.append((self, message))
                    return SSH_OK

                targetchannel = KickStartSFTPClientChannel(targetsession, self, self.session)

                if targetsession.started in [STARTED_AUTHENTIFIED, STARTED_REJECTED]:
                    targetchannel.wakeup();

                return SSH_OK

        except Exception, e:
            Logger().info("SFTP traceback %s" % traceback.format_exc(e))

        return SSH_ERROR

    def cb_data_stderr(self, data):
        Logger().info("@@@ WABChannel.cb_data_stderr()")
        try:
#            Logger().info("========================== ServerChannel %s cb_data_stderr -> %s" % (id(self), id(self.linkedchannel)))
            return self.linkedchannel.write_stderr_client(data)
        except Exception, e:
            Logger().info(">>>>>> %s" % traceback.format_exc(e))
        return -1

    def cb_data_stdout(self, data):
        Logger().info("@@@ WABChannel.cb_data_stdout()")
        # if self.data_num < 10:
        #     dump = ':'.join(x.encode('hex') for x in data[:20])
        #     # dump = ":".join("{:02x}".format(ord(c)) for c in data[:20])
        #     datalen = len(data)
        #     Logger().info(
        #         "oooooo FRONT Packet %s cb_data_stdout dump = '%s%s' length = %s" %
        #         (self.data_num + 1, dump, ["",":..."][datalen > 20], datalen))
        #     self.data_num += 1

        try:
            self.session.lastactiontime = time.time()

            if not self.linkedchannel or not self.linkedchannel.isOpen_client():
                self.session.server_channels_to_close.append((self, None))
                return 0
                
            if self.session.scp_requested or self.sftp_requested:
                try:
                    self.linkedchannel.analyzer.parse_up_data(data)
                except AnalyzerKill, e:
                    if self.sftp_requested:
                        self.sftp_to_close = True
                        self.sftp_error_msg = e.msg
                        if e.tosend is not None:
                            self.linkedchannel.write_stdout_client(e.tosend)
                        return len(data)
                    self.session.server_channels_to_close.append((self, e.msg))
                    return len(data)
                except Exception, e:
                    Logger().info(">>>> %s" % traceback.format_exc(e))
            if (self.winscp
                    and data.startswith('echo "WinSCP: this is begin-of-file" ; scp -r')):
                scp_sub_protocol = None
                if '; scp -r -p -d -f ' in data:
                    scp_sub_protocol = u'SSH_SCP_DOWN'
                if '; scp -r  -d -t ' in data:
                    scp_sub_protocol = u'SSH_SCP_UP'
                if scp_sub_protocol is not None:
                    targetsession, message = self.session.get_targetsession(self, scp_sub_protocol)
                    if not targetsession:
                        self.session.server_channels_to_close.append((self, message))
                        return 0

            ########## Keyboard pattern detection ################
            if self.session.wabengine_proxy.detect_pattern(data):
                self.write_stderr_server("\r\nUnauthorized Pattern Detected!!!\r\n")
                self.session.server_channels_to_close.append((self, None))
                return len(data)

            res = self.linkedchannel.write_stdout_client(data)
        except Exception, e:
            import traceback
            Logger().info("@@@ WABChannel.cb_data_stdout() <<<%s>>>" % traceback.format_exc(e))
        
        Logger().info("@@@ WABChannel.cb_data_stdout() done")
        return res

    def cb_wakeup(self):
        self.linkedchannel.wakeup()
        return

class WABAgentChannel(ServerChannel):
    def __init__(self, session):
        self.session = session
        self.callbacks = buildCallbacks(ssh_channel_callbacks_struct, CB_CHANNEL_FUNCS, self)
        self.libssh_agentchannel = lib.ssh_new_channel(session.libssh_session, self.callbacks)
        
    def close(self):
        Logger().info("@@@%s.close" % (self.__class__.__name__))
        #I should only do that once, if I initiate several closes it's an error
        self.channel_closed = True            
        lib.ssh_channel_close_server(
                self.session.libssh_session, 
                self.libssh_agentchannel)


class WABAuthAgentChannel(WABChannel):
    def __init__(self, session):
        super(WABAuthAgentChannel, self).__init__(session)

    def cb_data_stdout(self, data):
        Logger().info("@@ %s.cb_data_stdout" % self.__class__.__name__)
        self.session.lastactiontime = time.time()
        return self.linkedchannel.write_stdout_client(data)

    def cb_data_stderr(self, data):
        Logger().info("@@ %s.cb_data_stderr" % self.__class__.__name__)
        return self.linkedchannel.write_stderr_client(data)

    def close(self):
        lib.ssh_channel_close_server(self.session.libssh_session, self.libssh_channel)

    def write_stderr_server(self, data):
        return lib.ssh_channel_write_stderr_server(self.session.libssh_session, self.libssh_channel, data, len(data))

    def write_stdout_server(self, data):
        return lib.ssh_channel_write_server(self.session.libssh_session, self.libssh_channel, data, len(data))

    def __del__(self):
        if self.libssh_channel is not None:
            lib.ssh_channel_free_server(self.session.libssh_session, self.libssh_channel)


    def cb_signal(self, signal):
        pass


class WABX11Channel(WABChannel):
    def __init__(self, session):
        WABChannel.__init__(self, session)

    def close(self):
        lib.ssh_channel_close_server(self.session.libssh_session, self.libssh_channel)

    def write_stderr_server(self, data):
        return lib.ssh_channel_write_stderr_server(self.session.libssh_session, self.libssh_channel, data, len(data))

    def write_stdout_server(self, data):
        return lib.ssh_channel_write_server(self.session.libssh_session, self.libssh_channel, data, len(data))

    def __del__(self):
        if self.libssh_channel is not None:
            lib.ssh_channel_free_server(self.session.libssh_session, self.libssh_channel)

    def cb_data_stdout(self, data):
        Logger().info("@@@ WABX11Channel.cb_data_stdout()")
        self.session.lastactiontime = time.time()
        if not self.linkedchannel.isOpen_client():
            self.session.server_channels_to_close.append((self, None))
            return 0
        return self.linkedchannel.write_stdout_client(data)

    def cb_data_stderr(self, data):
#        Logger().info("@@@ WABX11Channel.cb_data_stderr()")
        return self.linkedchannel.write_stderr_client(data)

    def cb_signal(self, signal):
        pass

    def openX11(self, channel, originator_address, originator_port, sender, window, packet_size):
        Logger().info("WAB open X11 %s %s" % (originator_address, originator_port))
        self.linkedchannel = channel
        self.sender = sender
        self.window = window
        self.packet_size = packet_size
        return lib.ssh_channel_open_x11_server(self.session.libssh_session, self.libssh_channel,
                                            str(originator_address),
                                            int(originator_port))

    def cb_openX11Acknowledged(self, status):
        Logger().info("WAB open X11 Acknowledged status is %s" % status)
        try:
            if status == SSH_OK:
                Logger().info("Call to event_x11_requested_channel_client")
                lib.ssh_event_x11_requested_channel_client(
                    self.linkedchannel.session.libssh_session,
                    self.linkedchannel.libssh_channel, 
                    self.sender, 
                    self.window, 
                    self.packet_size)
            else:
                Logger().info("event_x11_requested_channel_failure_client")
                lib.ssh_event_x11_requested_channel_failure_client(
                    self.linkedchannel.session.libssh_session,
                    self.linkedchannel.libssh_channel, 
                    self.sender)
        except Exception, e:
            Logger().info("<<<%s>>>" % traceback.format_exc(e))


class WABChannelDirectTCPIP(WABChannel):
    def __init__(self, session, dest, dest_port, orig, orig_port):
        WABChannel.__init__(self, session)
#        Logger().info("========================== WABChannelDirectTCPIP %s __init__" % id(self))
        self.dest = dest
        self.dest_port = dest_port
        self.orig = orig
        self.orig_port = orig_port
        self.sub_protocol = u'SSH_X11_SESSION'

    def cb_data_stdout(self, data):
        Logger().info("@@@ %s.cb_data_stdout()" % self.__class__.__name__)
        self.session.lastactiontime = time.time()
        if self.linkedchannel and self.linkedchannel.isOpen_client():
            return self.linkedchannel.write_stdout_client(data)
        self.session.server_channels_to_close.append((self, None))
        return 0

    def cb_data_stderr(self, data):
        Logger().info("@@@ %s.cb_data_stderr()" % self.__class__.__name__)
        try:
#            Logger().info("========================== WABChannelDirectTCPIP %s cb_data_stderr -> %s" % (id(self), id(self.linkedchannel)))
            self.session.lastactiontime = time.time()
            if self.linkedchannel and self.linkedchannel.isOpen_client():
                written = self.linkedchannel.write_stderr_client(data)
                return written
            self.session.server_channels_to_close.append((self, None))
            return 0
        except Exception, e:
            Logger().info(">>>>>> %s" % traceback.format_exc(e))
        return 0


class WABChannelForwardedTCPIP(WABChannel):
    def __init__(self, session, dest, dest_port, orig, orig_port):
        WABChannel.__init__(self, session)
#        Logger().info("========================== WABChannelForwardedTCPIP %s __init__ -> %s" % (id(self), None if not self.linkedchannel else id(self.linkedchannel)))
        self.forwarded_tcpip_requested = True
        self.dest = dest
        self.dest_port = dest_port
        self.orig = orig
        self.orig_port = orig_port
        self.sub_protocol = u'SSH_X11_SESSION'

    def cb_data_stdout(self, data):
        try:
            Logger().info("========================== WABChannelForwardedTCPIP %s cb_data_stdout -> %s" % (id(self), id(self.linkedchannel)))
            self.session.lastactiontime = time.time()
            return self.linkedchannel.write_stdout_client(data)
        except Exception, e:
            Logger().info(">>>>>> %s" % traceback.format_exc(e))
        return 0

    def cb_data_stderr(self, data):
        try:
#            Logger().info("========================== WABChannelForwardedTCPIP %s cb_data_stderr -> %s" % (id(self), id(self.linkedchannel)))
            self.session.lastactiontime = time.time()
            return self.linkedchannel.write_stderr_client(data)
        except Exception, e:
            Logger().info(">>>>>> %s" % traceback.format_exc(e))
        return 0
        
        
        
        
