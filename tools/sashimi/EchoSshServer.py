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

try:
    from sshng import pysshct
except Exception, e:
    Logger().info("Error importing sashimi lib %s" % traceback.format_exc(e))

class Logger(object):
    def info(self, s): print "Info: %s" % s
    def error(self, s): print "Error: %s" % s
    def warning(self, s): print "Warning: %s" % s
    def debug(self, s): print "Debug: %s" % s
    def exception(self, s): print "Exception: %s" % s
    def trace(self, s):  print "Trace: %s" % s

host_key = '/etc/sashimi/server_rsa.key'

class EchoChannel(object):
    def __init__(self, session):
        self.session = session
        self.callbacks = pysshct.buildCallbacks(
            pysshct.ssh_channel_callbacks_struct, 
            pysshct.CB_CHANNEL_FUNCS, self)
        self.libssh_channel = pysshct.lib.ssh_new_channel(session.libssh_session, self.callbacks)

    def cb_eof(self):
        Logger().info("@@%s.cb_eof" % (self.__class__.__name__))
        return 0

    def isEof(self):
        Logger().info("@@%s.isEof" % (self.__class__.__name__))
        rc = pysshct.lib.ssh_channel_is_eof_server(self.session.libssh_session, self.libssh_channel)
        return rc != 0

    def sendEof(self):
        Logger().info("@@%s.sendEof" % (self.__class__.__name__))
        return pysshct.lib.ssh_channel_send_eof_server(self.session.libssh_session, self.libssh_channel)

    def isOpen_server(self):
        Logger().info("@@%s.isOpen_server" % (self.__class__.__name__))
        rc = pysshct.lib.ssh_channel_is_open_server(self.session.libssh_session, self.libssh_channel)
        return rc != 0

    def sendExitStatus(self, status):
        Logger().info("@@%s.sendExitStatus(%s)" % (self.__class__.__name__, status))
        return pysshct.lib.ssh_channel_request_send_exit_status_server(
            self.session.libssh_session, self.libssh_channel, status)

    def sendExitSignal(self, signalp, corep, errmsgp, langp):
        Logger().info("@@%s.sendExitSignal(%s, %s, %s, %s)" % (
                        self.__class__.__name__,
                        signalp, corep, errmsgp, langp))
        return pysshct.lib.ssh_channel_request_send_exit_signal_server(
            self.session.libssh_session, self.libssh_channel,
            signalp, corep, errmsgp, langp)

    def cb_close(self):
        Logger().info("@@%s.cb_close" % (self.__class__.__name__))
        return 0

    def close(self):
        Logger().info("@@%s.close" % (self.__class__.__name__))
        pysshct.lib.ssh_channel_close_server(self.session.libssh_session, self.libssh_channel)

    def write_stderr_server(self, data):
        return pysshct.lib.ssh_channel_write_stderr_server(self.session.libssh_session, self.libssh_channel, data, len(data))

    def write_stdout_server(self, data):
        return pysshct.lib.ssh_channel_write_server(self.session.libssh_session, self.libssh_channel, data, len(data))

    def __del__(self):
        if self.libssh_channel is not None:
            pysshct.lib.ssh_channel_free_server(self.session.libssh_session, self.libssh_channel)

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
        return pysshct.SSH_OK

    def cb_requestShell(self):
        Logger().info("@@@%s.cb_requestShell()" %
            (self.__class__.__name__,))
        return pysshct.SSH_OK

    def cb_requestAuthAgent(self):
        Logger().info("@@@%s.cb_requestAuthAgent()" % (self.__class__.__name__,))
        return pysshct.SSH_OK

    def cb_requestX11(self, single_conn, auth_proto, auth_cookie, screen_number):
        Logger().info("@@@%s.cb_requestX11(%s, %s, %s, %s)" % (
            self.__class__.__name__,
            single_conn, auth_proto, auth_cookie, screen_number))
        return pysshct.SSH_OK

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
        return pysshct.SSH_OK

    def cb_requestEnv(self, env_name, env_value):
        Logger().info("@@@%s.cb_requestEnv(%s=%s)" %
            (self.__class__.__name__, env_name, env_value))
        return pysshct.SSH_OK

    def cb_requestSubsystem(self, subsystem):
        Logger().info("@@@%s.cb_requestSubsystem(%s)" %
            (self.__class__.__name__, subsystem))
        return pysshct.SSH_ERROR

    def cb_data_stderr(self, data):
        Logger().info("@@@%s.cb_data_stderr(%s)" %
            (self.__class__.__name__, data))
        return len(data)

    def cb_data_stdout(self, data):
        Logger().info("@@@%s.cb_data_stdout(%s)" %
            (self.__class__.__name__, data))
        return pysshct.lib.ssh_channel_write_server(self.session.libssh_session, self.libssh_channel, data, len(data))

    def cb_wakeup(self):
        Logger().info("@@@%s.cb_wakeup(%s)" %
            (self.__class__.__name__))
        return

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

        self.client_addr = addr[0]
        self.client_port = addr[1]
        self.host_key_file = host_key_file

        Logger().info("Incoming ssh connection from %s " % self.client_addr)

        self.server_callbacks = pysshct.buildCallbacks(
            pysshct.ssh_server_callbacks_struct, 
            pysshct.CB_SERVER_SESSION_FUNCS, self)
        self.libssh_mainloop = pysshct.lib.ssh_new_poll_ctx()

        self.authmethods = (
#            pysshct.SSH_AUTH_METHOD_NONE
             pysshct.SSH_AUTH_METHOD_PASSWORD
#             | pysshct.SSH_AUTH_METHOD_GSSAPI_MIC
#             | pysshct.SSH_AUTH_METHOD_PUBLICKEY
#             | pysshct.SSH_AUTH_METHOD_INTERACTIVE
             )
        self.libssh_session = pysshct.lib.ssh_start_new_server_session(
            self.server_callbacks, 
            self.libssh_mainloop,
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
            polltime = time.time()
            print "Polling %s" % str(polltime)

    def __del__(self):
        if self.libssh_session is not None:
            pysshct.lib.ssh_free(self.libssh_session)

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
        return pysshct.SSH_ERROR

    def cb_serviceRequest(self, service):
        Logger().info("@@%s.cb_serviceRequest(%s)" % (
            self.__class__.__name__, service))
        return pysshct.SSH_OK

    def verify_target_host(self):
        Logger().info("@@%s.verify_target_host()" % (
            self.__class__.__name__))

    def managePassthrough(self, password):
        Logger().info("@@%s.managePassthrough()" % (
            self.__class__.__name__))

    def manageChallenge(self):
        Logger().info("@@%s.manageChallenge(%s)" % (
            self.__class__.__name__, service))

    def manageKbdIntPassword(self):
        Logger().info("@@%s.manageKbdIntPassword(%s)" % (
            self.__class__.__name__))

    def getInteractiveAnswers(self):
        Logger().info("@@%s.getInteractiveAnswers()" % (
            self.__class__.__name__))

    def cb_authInteractive(self, username, kbdint_response):
        Logger().info("@@%s.cb_authInteractive(%s,%s)" % (
            self.__class__.__name__,
            username,
            kbdint_response))

    # Server Session Callback API
    # ===========================
    def cb_authNone(self, username):
        Logger().info("@@%s.cb_authNone(%s)" % (self.__class__.__name__, username))
        pysshct.lib.ssh_set_auth_methods_server(self.libssh_session, self.authmethods)
        return pysshct.SSH_AUTH_DENIED

    def cb_authPassword(self, username, password):
        Logger().info("@@%s.cb_authPassword(%s, %s)" % (self.__class__.__name__, username, password))
        return pysshct.SSH_AUTH_SUCCESS

    def cb_authPubkey(self, username, key, sig_state):
        Logger().info("@@%s.cb_authPubkey(%s, %s)" % self.__class__.__name__, key, sig_state)
        return pysshct.SSH_AUTH_DENIED

    def cb_authGssapiMic(self, username, principal):
        Logger().info("@@%s.cb_authGssapiMic(%s, %s)" % (
                        self.__class__.__name__,
                        username,
                        principal))
        return pysshct.SSH_AUTH_DENIED

    def cb_channelOpenSessionRequest(self):
        Logger().info("@@%s.cb_channelOpenSessionRequest()" % self.__class__.__name__)
        channel = EchoChannel(self)
        return channel.libssh_channel

    def cb_channelOpenDirectTCPIPRequest(self, dest, dest_port, orig, orig_port):
        Logger().info("@@%s.cb_channelOpenDirectTCPIPRequest(%s:%s, %s:%s)" % (
            self.__class__.__name__, dest, dest_port, orig, orig_port))

    def channelOpenForwardedTCPIPRequest(self, dest, dest_port, orig, orig_port):
        Logger().info("@@%s.channelOpenForwardedTCPIPRequest(%s:%s, %s:%s)" % (
            self.__class__.__name__, dest, dest_port, orig, orig_port))

    def get_targetsession(self, channel, subprotocol):
        Logger().info("@@%s.get_targetsession(%s, %s)" % (
            self.__class__.__name__, channel, subprotocol))

        return self.target_session, "OK"

    def interactiveShellChannel(self, channel, target):
        Logger().info("@@%s.interactiveShellChannel(%s, %s)" % (
            self.__class__.__name__, channel, target))

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

    def start(self):
        pass

    def check_access_allowed(self, sub_protocol, right_protocol, right_subprotocols):
        Logger().info("@@%s.check_access_allowed()" % (
            self.__class__.__name__))

def serve():
    import signal
    signal.signal(signal.SIGCHLD, signal.SIG_IGN)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind(('0.0.0.0', 2200))
    s.listen(100)
    try:
        while 1:
            rfds, wfds, xfds = select.select([s], [], [], 1)
            for sck in rfds:
                if sck == s:
                    client_socket, client_addr = s.accept()
                    child_pid = os.fork()
                    if child_pid == 0:
                        signal.signal(signal.SIGCHLD, signal.SIG_DFL)
                        s.close()
                        server = SSHServer(client_socket, client_addr, host_key)
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

if __name__ == '__main__':
    serve()
