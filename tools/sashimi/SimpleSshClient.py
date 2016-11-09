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

import os
import sys
import time
import traceback

import errno
import socket
import select
import ctypes

class Logger(object):
    @staticmethod 
    def info(s): print "Info: %s" % s
    @staticmethod 
    def error(s): print "Error: %s" % s
    @staticmethod 
    def warning(s): print "Warning: %s" % s
    @staticmethod 
    def debug(s): print "Debug: %s" % s
    @staticmethod 
    def exception(s): print "Exception: %s" % s
    @staticmethod 
    def trace(s):  print "Trace: %s" % s

try:
    from sshng import pysshct
except Exception, e:
    Logger.info("Error importing sashimi lib %s" % traceback.format_exc(e))

host_key = '/etc/sashimi/server_rsa.key'

class EchoChannel(object):
    def __init__(self, session):
        self.session = session
        self.callbacks = pysshct.buildCallbacks(
            pysshct.ssh_channel_callbacks_struct, 
            pysshct.CB_CHANNEL_FUNCS, self)
        self.libssh_channel = pysshct.lib.ssh_new_channel(
            session.libssh_session, self.callbacks)

    def __del__(self):
        if self.libssh_channel is not None:
            pysshct.lib.ssh_channel_free_server(self.session.libssh_session, self.libssh_channel)

    # CHANNEL API
    def cb_data_stdout(self, data):
        Logger.info("@@@%s.cb_data_stdout(%s)" %
            (self.__class__.__name__, data))
        return pysshct.lib.ssh_channel_write_server(
            self.session.libssh_session,
            self.libssh_channel, data, len(data))

    # CHANNEL API
    def cb_data_stderr(self, data):
        Logger.info("@@@%s.cb_data_stderr(%s)" %
            (self.__class__.__name__, data))
        return len(data)

    # CHANNEL API
    def cb_eof(self):
        Logger.info("@@%s.cb_eof" % (self.__class__.__name__))
        return 0

    # CHANNEL API
    def cb_close(self):
        Logger.info("@@%s.cb_close" % (self.__class__.__name__))
        return 0

    # CHANNEL API
    def cb_signal(self, signal):
        Logger.info("@@%s.cb_signal()"  % (self.__class__.__name__))

    # CHANNEL API
    def cb_exitSignal(self, signalp, corep, errmsgp, langp):
        Logger.info("@@%s.cb_exitSignal(%s, %s, %s, %s)"  % (
            self.__class__.__name__, signalp, corep, errmsgp, langp))

    # CHANNEL API
    def cb_exitStatus(self, status):
        Logger.info("@@%s.cb_exitStatus()"  % (
            self.__class__.__name__, status))

    # CHANNEL API
    def cb_requestPty(self, term, width, height, pxwidth, pxheight):
        Logger.info("@@@%s.cb_requestPty(%s, %s, %s, %s, %s)" %
            (self.__class__.__name__,
            term, width, height, pxwidth, pxheight))
        self.term = term
        self.width = width
        self.height = height
        self.pixelwidth = pxwidth
        self.pixelheight = pxheight
        self.pty_requested = True
        return pysshct.SSH_OK

    # CHANNEL API
    def cb_requestShell(self):
        Logger.info("@@@%s.cb_requestShell()" %
            (self.__class__.__name__,))
        return pysshct.SSH_OK

    # CHANNEL API
    def cb_requestAuthAgent(self):
        Logger.info("@@@%s.cb_requestAuthAgent()" % (self.__class__.__name__,))
        return pysshct.SSH_OK

    # CHANNEL API
    def cb_requestX11(self, single_conn, auth_proto, auth_cookie, screen_number):
        Logger.info("@@@%s.cb_requestX11(%s, %s, %s, %s)" % (
            self.__class__.__name__,
            single_conn, auth_proto, auth_cookie, screen_number))
        return pysshct.SSH_OK

    # CHANNEL API
    def cb_ptyWindowChange(self, width, height, pxwidth, pxheight):
        Logger.info("@@@%s.cb_ptyWindowChange(%s, %s, %s, %s)" %
            (self.__class__.__name__,
            width, height, pxwidth, pwheight))
        self.width = width
        self.height = height
        self.pixelwidth = pxwidth
        self.pixelheight = pxheight

    # CHANNEL API
    def cb_requestExec(self, command):
        Logger.info("@@@%s.cb_requestExec(%s)" %
            (self.__class__.__name__, command))
        return pysshct.SSH_OK

    # CHANNEL API
    def cb_requestEnv(self, env_name, env_value):
        Logger.info("@@@%s.cb_requestEnv(%s=%s)" %
            (self.__class__.__name__, env_name, env_value))
        return pysshct.SSH_OK

    # API checked
    def cb_requestSubsystem(self, subsystem):
        Logger.info("@@@%s.cb_requestSubsystem(%s)" %
            (self.__class__.__name__, subsystem))
        return pysshct.SSH_ERROR

    # API checked
    def cb_open_x11_server_status(self, status):
        Logger.info("@@@%s.cb_requestSubsystem(%s)" %
            (self.__class__.__name__, status))
        return pysshct.SSH_ERROR

class SSHClient(object):
    """
    Simple SSH Client class
    """
    def __init__(self, host, port, user, hostkeys):
        self.libssh_error = pysshct.lib.ssh_new_error()
        self.libssh_mainloop = pysshct.lib.ssh_new_poll_ctx()

        self.callbacks = pysshct.buildCallbacks(
            pysshct.ssh_client_callbacks_struct,
            pysshct.CB_CLIENT_SESSION_FUNCS, self)
        self.libssh_session = lib.ssh_new_client_session(
            self.callbacks,
            self.libssh_mainloop,
            host, port, user, hostkeys, "4", self.libssh_error)

        self.lastactiontime = time.time()

        while True:
            rc = pysshct.lib.ssh_event_dopoll(self.libssh_mainloop, 20000)
            if rc == pysshct.SSH_ERROR:
                break
            polltime = time.time()
            print "Polling %s" % str(polltime)

    def __del__(self):
        if self.libssh_session is not None:
            pysshct.lib.ssh_free(self.libssh_session)

    # CLIENT SESSION API
    def cb_auth_none_reply_client(self, status):
        Logger.info("@@%s.cb_auth_none_reply_client(%s)" % (
                    self.__class__.__name__,
                    status))

    # CLIENT SESSION API
    def auth(self, promptp, bufp, length, echo, verify):
        Logger.info("@@%s.cb_auth_none_reply_client(%s, %s, %s, %s, %s)" % (
                    self.__class__.__name__,
                    prompt, bufp, length, echo, verify))

    # CLIENT SESSION API
    def connectStatus(self, statusp):
        Logger.info("@@%s.connectStatus(%s)" % (
                    self.__class__.__name__,
                    statusp))

    # CLIENT SESSION API
    def globalRequest(self, gtype, want_reply, bind_address, bind_port):
        Logger.info("@@%s.globalRequest(%s,%s,%s,%)" % (
                    self.__class__.__name__,
                    gtype, want_reply, bind_address, bind_ports))

    # CLIENT SESSION API
    def channelOpenX11Request(self, originator, originator_port, sender, window, packet_size):
        Logger.info("@@%s.channelOpenX11Request%s,%s,%s,%s,%s)" % (
                    self.__class__.__name__,
                    originator, originator_port, sender, window, packet_size))

    # CLIENT SESSION API
    def channelOpenAuthAgentRequest(self):
        Logger.info("@@%s.channelOpenAuthAgentRequest()" % (
                    self.__class__.__name__))
    
    


if __name__ == '__main__':
    SSHClient("127.0.0.1", "22", cgrosjean, "ssh-rsa,ssh-dss")
