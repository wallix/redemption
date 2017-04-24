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
    def info(s): print("Info: %s" % s)
    @staticmethod 
    def error(s): print("Error: %s" % s)
    @staticmethod 
    def warning(s): print("Warning: %s" % s)
    @staticmethod 
    def debug(s): print("Debug: %s" % s)
    @staticmethod 
    def exception(s): print("Exception: %s" % s)
    @staticmethod 
    def trace(s):  print("Trace: %s" % s)

try:
    import pysshct
except Exception as e:
    s = "EXCEPTION" # traceback.format_exc(e)
    Logger.info("Error importing sashimi lib {s}".format(s=s))

host_key = '/etc/sashimi/server_rsa.key'

class EchoChannel(object):
    def __init__(self, session):
        self.buffer = []
        self.session = session
        self.callbacks = pysshct.buildCallbacks(
            pysshct.ssh_channel_callbacks_struct, 
            pysshct.CB_CHANNEL_FUNCS, self)
        self.libssh_channel = pysshct.lib.ssh_new_channel(
            session.libssh_session, self.callbacks)

    def flush(self):
        Logger.info("@@@%s.flush() data='%s'" %
            (self.__class__.__name__, str(self.buffer)))
        if self.buffer:
            try:
                pos = self.buffer.index('\r')
            except:
                return

            tosend = "".join(self.buffer[0:pos+1])
            res = pysshct.lib.ssh_channel_write_server(
                self.session.libssh_session,
                self.libssh_channel, tosend, len(tosend))
            if res > 0: 
                self.buffer[0:res+1] = []
            return
            

    def __del__(self):
        if self.libssh_channel is not None:
            pysshct.lib.ssh_channel_free_server(self.session.libssh_session, self.libssh_channel)

    # CHANNEL API
    def cb_data_stdout(self, data):
        Logger.info("@@@%s.cb_data_stdout(data='%s' len=%d)" %
            (self.__class__.__name__, data, len(data)))
        self.buffer.append(data)
        return len(data)

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

class SSHServer(object):
    """
    SSH Server class implementation to initialize transport, negotiate
    client connections, and start site clients.
    """
    def __init__(self, client_socket, addr, host_key_file):
    
        self.libssh_error = pysshct.lib.ssh_new_error()
        self.username = None
        self.kbdint_response = None
        self.client_socket = client_socket
        self.client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
        self.local_ip, self.local_port = client_socket.getsockname()
        self.channels = []

        self.client_addr = addr[0]
        self.client_port = addr[1]
        self.host_key_file = host_key_file

        Logger.info("Incoming ssh connection from %s " % self.client_addr)

        self.server_callbacks = pysshct.buildCallbacks(
            pysshct.ssh_server_callbacks_struct, 
            pysshct.CB_SERVER_SESSION_FUNCS, self)
        self.libssh_mainloop = libssh_mainloop = pysshct.lib.ssh_new_poll_ctx()

        self.authmethods = (
#            pysshct.SSH_AUTH_METHOD_NONE
             pysshct.SSH_AUTH_METHOD_PASSWORD
#             | pysshct.SSH_AUTH_METHOD_GSSAPI_MIC
#             | pysshct.SSH_AUTH_METHOD_PUBLICKEY
#             | pysshct.SSH_AUTH_METHOD_INTERACTIVE
             )
        self.libssh_session = pysshct.lib.ssh_start_new_server_session(
            self.server_callbacks, 
            libssh_mainloop,
            self.client_socket.fileno(),
            self.host_key_file, # host keyfile contains RSA, DSA or ECDSA key
            self.authmethods
            )

        pysshct.lib.ssh_event_set_session_server(self.libssh_mainloop, self.libssh_session)

        self.lastactiontime = time.time()

        while True:
            rc = pysshct.lib.ssh_event_dopoll(self.libssh_mainloop, 20000)
            if rc == pysshct.SSH_ERROR:
                break
            for channel in self.channels:
                channel.flush()
            polltime = time.time()
            print("Polling %s" % str(polltime))

    def __del__(self):
        if self.libssh_session is not None:
            pysshct.lib.ssh_free(self.libssh_session)

    def Log(self, priority, message):
        Logger.info("[sashimi] %s" % message)

    # SERVER SESSION API
    def cb_connectStatus(self, status):
        Logger.info("@@%s.cb_connectStatus(%s)" % (
                    self.__class__.__name__,
                    status))

    # SERVER SESSION API
    def cb_globalRequest(self, gtype, want_reply, bind_address, bind_port):
        Logger.info("@@%s.cb_globalRequest(%s, %s, %s, %s)" % (
                    self.__class__.__name__,
                    gtype, want_reply, bind_address, bind_port))
        return pysshct.SSH_ERROR

    # SERVER SESSION API
    def cb_authPassword(self, username, password):
        Logger.info("@@%s.cb_authPassword(%s, %s)" % (self.__class__.__name__, username, password))
        if username == "good" and password == "good":
            return pysshct.SSH_AUTH_SUCCESS
        return pysshct.SSH_AUTH_DENIED

    # SERVER SESSION API
    def cb_authNone(self, username):
        Logger.info("@@%s.cb_authNone(%s)" % (self.__class__.__name__, username))
        pysshct.lib.ssh_set_auth_methods_server(self.libssh_session, self.authmethods)
        return pysshct.SSH_AUTH_DENIED

    # SERVER SESSION API
    def cb_authInteractive(self, username, kbdint_response):
        Logger.info("@@%s.cb_authInteractive(%s,%s)" % (
            self.__class__.__name__,
            username,
            kbdint_response))

    # SERVER SESSION API
    def cb_authGssapiMic(self, username, principal):
        Logger.info("@@%s.cb_authGssapiMic(%s, %s)" % (
                        self.__class__.__name__,
                        username,
                        principal))
        return pysshct.SSH_AUTH_DENIED

    # SERVER SESSION API
    def cb_authPubkey(self, username, key, sig_state):
        Logger.info("@@%s.cb_authPubkey(%s, %s)" % self.__class__.__name__, key, sig_state)
        return pysshct.SSH_AUTH_DENIED

    # SERVER SESSION API
    def cb_serviceRequest(self, service):
        Logger.info("@@%s.cb_serviceRequest(%s)" % (
            self.__class__.__name__, service))
        return pysshct.SSH_OK

    # SERVER SESSION API
    def cb_channelOpenSessionRequest(self):
        Logger.info("@@%s.cb_channelOpenSessionRequest()" % self.__class__.__name__)
        channel = EchoChannel(self)
        self.channels.append(channel)
        return channel.libssh_channel

    # SERVER SESSION API
    def cb_channelOpenDirectTCPIPRequest(self, dest, dest_port, orig, orig_port):
        Logger.info("@@%s.cb_channelOpenDirectTCPIPRequest(%s:%s, %s:%s)" % (
            self.__class__.__name__, dest, dest_port, orig, orig_port))

    # SERVER SESSION API
    def channelOpenForwardedTCPIPRequest(self, dest, dest_port, orig, orig_port):
        Logger.info("@@%s.channelOpenForwardedTCPIPRequest(%s:%s, %s:%s)" % (
            self.__class__.__name__, dest, dest_port, orig, orig_port))

def serve():
    import signal
    signal.signal(signal.SIGCHLD, signal.SIG_IGN)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    print("SshEchoServer listening on port 2200 user: good")
    s.bind(('0.0.0.0', 2200))
    s.listen(100)
    try:
        while 1:
            print('Waiting on Select')
            rfds, wfds, xfds = select.select([s], [], [], 10)
            print('Select woke up')
            for sck in rfds:
                if sck == s:
                    print("Incoming data")
                    client_socket, client_addr = s.accept()
                    child_pid = os.fork()
                    if child_pid == 0:
                        signal.signal(signal.SIGCHLD, signal.SIG_DFL)
                        s.close()
                        server = SSHServer(client_socket, client_addr, host_key)
                        sys.exit(0)
                    else:
                        client_socket.close()
                        #os.waitpid(child_pid, 0)

    except KeyboardInterrupt:
        if client_socket:
            client_socket.close()
        sys.exit(1)
    except socket.error as e:
        import traceback
        Logger.debug("%s" % traceback.format_exc(e))
    except Exception as e:
        import traceback
        Logger.info("%s" % traceback.format_exc(e))

if __name__ == '__main__':
    serve()
