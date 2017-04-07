
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

from logger import Logger

import ctypes

class HelloChannel(object):
    def __init__(self, session):
        self.session = session
        self.callbacks = buildCallbacks(ssh_channel_callbacks_struct, CB_CHANNEL_FUNCS, self)
        self.libssh_channel = lib.ssh_new_channel(session.libssh_session, self.callbacks)

    def cb_eof(self):
        Logger().info("@@%s.cb_eof" % (self.__class__.__name__))
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
        Logger().info("@@%s.sendExitStatus(%s)" % (self.__class__.__name__, status))
        return lib.ssh_channel_request_send_exit_status_server(
            self.session.libssh_session, self.libssh_channel, status)

    def sendExitSignal(self, signalp, corep, errmsgp, langp):
        Logger().info("@@%s.sendExitSignal(%s, %s, %s, %s)" % (
                        self.__class__.__name__,
                        signalp, corep, errmsgp, langp))
        return lib.ssh_channel_request_send_exit_signal_server(
            self.session.libssh_session, self.libssh_channel,
            signalp, corep, errmsgp, langp)

    def cb_close(self):
        Logger().info("@@%s.cb_close" % (self.__class__.__name__))
        return 0

    def close(self):
        Logger().info("@@%s.close" % (self.__class__.__name__))
        lib.ssh_channel_close_server(self.session.libssh_session, self.libssh_channel)

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
        Logger().info("@@@%s.cb_requestPty(%s, %s, %s, %s, %s)" %
            (self.__class__.__name__,
            term, width, height, pxwidth, pxheight))
        self.term = term
        self.width = width
        self.height = height
        self.pixelwidth = pxwidth
        self.pixelheight = pxheight
        self.pty_requested = True
        return SSH_OK

    def cb_requestShell(self):
        Logger().info("@@@%s.cb_requestShell()" %
            (self.__class__.__name__,))
        return SSH_OK

    def cb_requestAuthAgent(self):
        Logger().info("@@@%s.cb_requestAuthAgent()" % (self.__class__.__name__,))
        return SSH_OK

    def cb_requestX11(self, single_conn, auth_proto, auth_cookie, screen_number):
        Logger().info("@@@%s.cb_requestX11(%s, %s, %s, %s)" % (
            self.__class__.__name__,
            single_conn, auth_proto, auth_cookie, screen_number))
        return SSH_OK

    def cb_ptyWindowChange(self, width, height, pxwidth, pxheight):
        Logger().info("@@@%s.cb_ptyWindowChange(%s, %s, %s, %s)" %
            (self.__class__.__name__,
            width, height, pxwidth, pwheight))
        self.width = width
        self.height = height
        self.pixelwidth = pxwidth
        self.pixelheight = pxheight

    def cb_requestExec(self, command):
        Logger().info("@@@%s.cb_requestExec(%s)" %
            (self.__class__.__name__, command))
        return SSH_OK

    def cb_requestEnv(self, env_name, env_value):
        Logger().info("@@@%s.cb_requestEnv(%s=%s)" %
            (self.__class__.__name__, env_name, env_value))
        return SSH_OK

    def cb_requestSubsystem(self, subsystem):
        Logger().info("@@@%s.cb_requestSubsystem(%s)" %
            (self.__class__.__name__, subsystem))
        return SSH_ERROR

    def cb_data_stderr(self, data):
        Logger().info("@@@%s.cb_data_stderr(%s)" %
            (self.__class__.__name__, data))
        return len(data)

    def cb_data_stdout(self, data):
        Logger().info("@@@%s.cb_data_stdout(%s)" %
            (self.__class__.__name__, data))
        return lib.ssh_channel_write_server(self.session.libssh_session, self.libssh_channel, data, len(data))

    def cb_wakeup(self):
        Logger().info("@@@%s.cb_wakeup(%s)" %
            (self.__class__.__name__))
        return


