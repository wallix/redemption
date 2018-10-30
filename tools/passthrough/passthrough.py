#!/usr/bin/python -O
# -*- coding: utf-8 -*-
##
# Author(s): Meng Tan
# Module description:  Passthrough ACL
# Allows RDPProxy to connect to any server RDP.
##

import random
import os
import signal
import traceback
import sys
from datetime import datetime

from logger import Logger

from struct     import unpack
from struct     import pack
from select     import select
import socket

MAGICASK = u'UNLIKELYVALUEMAGICASPICONSTANTS3141592926ISUSEDTONOTIFYTHEVALUEMUSTBEASKED'
DEBUG = False

def rvalue(value):
    if value:
        return value
    return MAGICASK

class AuthentifierSocketClosed(Exception):
    pass

class ACLPassthrough():
    def __init__(self, conn, addr):
        self.cn = u'Unknown'

        self.proxy_conx  = conn
        self.addr        = addr
        self.full_path   = None

        self.shared                    = {}

        self.shared[u'module']                  = u'login'
        self.shared[u'selector_group_filter']   = u''
        self.shared[u'selector_device_filter']  = u''
        self.shared[u'selector_proto_filter']   = u''
        self.shared[u'selector']                = u'False'
        self.shared[u'selector_current_page']   = u'1'
        self.shared[u'selector_lines_per_page'] = u'0'
        self.pid = os.getpid()

        self.shared[u'target_login']    = MAGICASK
        self.shared[u'target_device']   = MAGICASK
        self.shared[u'target_host']     = MAGICASK
        self.shared[u'login']           = "admin"
        self.shared[u'ip_client']       = MAGICASK
        self.shared[u'target_protocol'] = MAGICASK

    def send_data(self, data):
        u""" NB : Strings sent to the ReDemPtion proxy MUST be UTF-8 encoded """

        if DEBUG:
            import pprint
            Logger().info(u'================> send_data (update)=%s' % (pprint.pformat(data)))
        # replace MAGICASK with ASK and send data on the wire
        _list = []
        for key, value in data.items():
            self.shared[key] = value
            if value != MAGICASK:
                _pair = u"%s\n!%s\n" % (key, value)
            else:
                _pair = u"%s\nASK\n" % key
            _list.append(_pair)

        if DEBUG:
           import pprint
           Logger().info(u'send_data (on the wire)=%s' % (pprint.pformat(_list)))

        _r_data = u"".join(_list)
        _r_data = _r_data.encode('utf-8')
        _len = len(_r_data)

        _chunk_size = 1024 * 64 - 1
        _chunks = _len // _chunk_size

        if _chunks == 0:
            self.proxy_conx.sendall(pack(">L", _len))
            self.proxy_conx.sendall(_r_data)
        else:
            if _chunks * _chunk_size == _len:
                _chunks -= 1
            for i in range(0, _chunks):
                self.proxy_conx.sendall(pack(">H", 1))
                self.proxy_conx.sendall(pack(">H", _chunk_size))
                self.proxy_conx.sendall(_r_data[i*_chunk_size:(i+1)*_chunk_size])
            _remaining = _len - (_chunks * _chunk_size)
            self.proxy_conx.sendall(pack(">L", _remaining))
            self.proxy_conx.sendall(_r_data[_len-_remaining:_len])

    def receive_data(self):
        u""" NB : Strings coming from the ReDemPtion proxy are UTF-8 encoded """

        _status, _error = True, u''
        _data = ''
        try:
            # Fetch Data from Redemption
            try:
                _packet_size, = unpack(">L", self.proxy_conx.recv(4))
                _data = self.proxy_conx.recv(_packet_size)
            except Exception as e:
                if DEBUG:
                    import traceback
                    Logger().info(u"Socket Closed : %s" % traceback.format_exc(e))
                raise AuthentifierSocketClosed()
            _data = _data.decode('utf-8')
        except AuthentifierSocketClosed as e:
            raise
        except Exception as e:
            raise AuthentifierSocketClosed()

        if _status:
            _elem = _data.split('\n')

            if len(_elem) & 1 == 0:
                Logger().info(u"Odd number of items in authentication protocol")
                _status = False

        if _status:
            try:
                _data = dict(zip(_elem[0::2], _elem[1::2]))
            except Exception as e:
                if DEBUG:
                    import traceback
                    Logger().info(u"Error while parsing received data %s" % traceback.format_exc(e))
                _status = False

            if DEBUG:
                import pprint
                Logger().info("received_data (on the wire) = %s" % (pprint.pformat(_data)))

        # may be actual socket error, or unpack or parsing failure
        # (because we got partial data). Whatever the case socket connection
        # with rdp proxy is now broken and must be terminated
        if not _status:
            raise socket.error()

        if _status:
            for key in _data:
                if (_data[key][:3] == u'ASK'):
                    _data[key] = MAGICASK
                elif (_data[key][:1] == u'!'):
                    _data[key] = _data[key][1:]
                else:
                    # _data[key] unchanged
                    pass
            self.shared.update(_data)

        return _status, _error


    def interactive_target(self, data_to_send):
        data_to_send.update({ u'module' : u'interactive_target' })
        self.send_data(data_to_send)
        _status, _error = self.receive_data()
        if self.shared.get(u'display_message') != u'True':
            _status, _error = False, u'Connection closed by client'
        return _status, _error

        # If we send close we should expect authentifier socket will be closed by the other end
        # No need to return some warning message if that happen
        self.send_data(data_to_send)
        _status, _error = self.receive_data()

        return _status, _error

    def start(self):
        _status, _error = self.receive_data()

        interactive_data = {}
        interactive_data[u'target_password'] = rvalue(self.shared.get(u'password'))
        interactive_data[u'target_host'] = rvalue(self.shared.get(u'real_target_device'))
        interactive_data[u'target_login'] = rvalue(self.shared.get(u'login'))
        interactive_data[u'target_device'] = (
            "<host>$<application path>$<working dir>$"
            "<args> for Application"
        )

        _status, _error = self.interactive_target(interactive_data)

        kv = {}
        kv[u'login'] = self.shared.get(u'target_login')
        kv[u'proto_dest'] = "RDP"
        kv[u'target_port'] = "3389"
        kv[u'session_id'] = "0000"
        kv[u'module'] = 'RDP' if self.shared.get(u'login') != 'internal' else 'INTERNAL'
        kv[u'mode_console'] = u"allow"
        kv[u'target_password'] = self.shared.get(u'target_password')
        kv[u'target_login'] = self.shared.get(u'target_login')
        kv[u'target_host'] = self.shared.get(u'target_host')
        kv[u'target_device'] = self.shared.get(u'target_host')
        kv[u'session_log_path'] = datetime.now().strftime(
            "session_log-%Y-%m-%d-%I:%M%p.log")

        if '$' in kv[u'target_host']:
            app_params = kv[u'target_host']
            list_params = app_params.split('$', 3)
            kv[u'target_host'] = list_params[0]
            if len(list_params) > 3:
                kv[u'alternate_shell'] = list_params[1]
                kv[u'shell_working_directory'] = list_params[2]
                kv[u'target_application'] = list_params[1]
                kv[u'shell_arguments'] = list_params[3]

        self.send_data(kv)

        try_next = False
        signal.signal(signal.SIGUSR1, self.kill_handler)
        try:
            self.send_data(kv)

            # Looping on keepalived socket
            while True:
                r = []
                Logger().info(u"Waiting on proxy")
                got_signal = False
                try:
                    r, w, x = select([self.proxy_conx], [], [], 60)
                except Exception as e:
                    if DEBUG:
                        Logger().info("exception: '%s'" % e)
                        import traceback
                        Logger().info("<<<<%s>>>>" % traceback.format_exc(e))
                    if e[0] != 4:
                        raise
                    Logger().info("Got Signal %s" % e)
                    got_signal = True
                if self.proxy_conx in r:
                    _status, _error = self.receive_data();

                    if self.shared.get(u'keepalive') == MAGICASK:
                        self.send_data({u'keepalive': u'True'})
                # r can be empty
                else: # (if self.proxy_conx in r)
                    Logger().info(u'Missing Keepalive')
                    Logger().error(u'break connection')
                    release_reason = u'Break connection'
                    break
            Logger().debug(u"End Of Keep Alive")


        except AuthentifierSocketClosed as e:
            if DEBUG:
                import traceback
                Logger().info(u"RDP/VNC connection terminated by client")
                Logger().info("<<<<%s>>>>" % traceback.format_exc(e))
        except Exception as e:
            if DEBUG:
                import traceback
                Logger().info(u"RDP/VNC connection terminated by client")
                Logger().info("<<<<%s>>>>" % traceback.format_exc(e))

        try:
            Logger().info(u"Close connection ...")

            self.proxy_conx.close()

            Logger().info(u"Close connection done.")
        except IOError:
            if DEBUG:
                Logger().info(u"Close connection: Exception")
                Logger().info("<<<<%s>>>>" % traceback.format_exc(e))
    # END METHOD - START


    def kill_handler(self, signum, frame):
        # Logger().info("KILL_HANDLER = %s" % signum)
        if signum == signal.SIGUSR1:
            self.kill()

    def kill(self):
        try:
            Logger().info(u"Closing a RDP/VNC connection")
            self.proxy_conx.close()
        except Exception:
            pass



from socket import fromfd
from socket import AF_UNIX
from socket import AF_INET
from socket import SOCK_STREAM
from socket import SOL_SOCKET
from socket import SO_REUSEADDR
from select import select
from logger import Logger

socket_path = '/tmp/redemption-sesman-sock'

def standalone():
    print('open socket at', socket_path)
    signal.signal(signal.SIGCHLD, signal.SIG_IGN)
    # create socket from bounded port
    s1 = socket.socket(AF_UNIX, SOCK_STREAM)
    s1.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
    s1.bind(socket_path)
    s1.listen(100)

    s2 = socket.socket(AF_INET, SOCK_STREAM)
    s2.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
    s2.bind(('127.0.0.1', 3450))
    s2.listen(100)

    try:
        while 1:
            rfds, wfds, xfds = select([s1, s2], [], [], 1)
            for sck in rfds:
                if sck in [s1, s2]:
                    client_socket, client_addr = sck.accept()
                    child_pid = os.fork()
                    if child_pid == 0:
                        signal.signal(signal.SIGCHLD, signal.SIG_DFL)
                        sck.close()
                        server = ACLPassthrough(client_socket, client_addr)
                        server.start()
                        sys.exit(0)
                    else:
                        client_socket.close()
                        #os.waitpid(child_pid, 0)

    except KeyboardInterrupt:
        if client_socket:
            client_socket.close()
        sys.exit(1)
    except socket.error as e:
        pass
    except AuthentifierSocketClosed as e:
        Logger().info("Authentifier Socket Closed")
    # except Exception as e:
        # Logger().exception("%s" % e)

if __name__ == '__main__':
    standalone()
