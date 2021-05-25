#!/usr/bin/env python3 -O
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

from struct import unpack_from, pack
from select     import select
import socket

# import uuid # for random rec_path


MAGICASK = u'UNLIKELYVALUEMAGICASPICONSTANTS3141592926ISUSEDTONOTIFYTHEVALUEMUSTBEASKED'
DEBUG = True

if DEBUG:
    import pprint

class AuthentifierSocketClosed(Exception):
    pass

class AuthentifierSharedData():
    def __init__(self, conn):
        self.proxy_conx = conn
        self.shared = {
            u'module':                  u'login',
            u'selector_group_filter':   u'',
            u'selector_device_filter':  u'',
            u'selector_proto_filter':   u'',
            u'selector':                u'False',
            u'selector_current_page':   u'1',
            u'selector_lines_per_page': u'0',

            u'target_login':    MAGICASK,
            u'target_device':   MAGICASK,
            u'target_host':     MAGICASK,
            u'login':           "admin",
            u'ip_client':       MAGICASK,
            u'proto_dest':      MAGICASK,
        }

    def send_data(self, data):
        u""" NB : Strings sent to the ReDemPtion proxy MUST be UTF-8 encoded """

        if DEBUG:
            Logger().info(u'================> send_data (update) =\n%s' % (pprint.pformat(data)))

        self.shared.update(data)

        def _toval(key, value):
            key = key.encode('utf-8')
            try:
                value = value.encode('utf-8')
            except:
                # int, etc
                value = str(value).encode('utf-8')
            key_len = len(key)
            value_len = len(value)
            return (
                True, key,
                pack(f'>1sB{key_len}sL{value_len}s',
                     b'!', key_len, key, value_len, value)
            )

        def _toask(key):
            key = key.encode('utf-8')
            key_len = len(key)
            return (
                False, key,
                pack(f'>1sB{key_len}s', b'?', key_len, key)
            )

        _list = [(_toval(key, value) if value != MAGICASK else _toask(key))
                   for key, value in data.items()]
        _list.sort()

        if DEBUG:
            Logger().info(u'send_data (on the wire) length = %s' % len(_list))

        _r_data = b''.join(s for t in _list for s in t[2])
        self.proxy_conx.sendall(pack('>H', len(_list)))
        self.proxy_conx.sendall(_r_data)

    def receive_data(self):
        u""" NB : Strings coming from the ReDemPtion proxy are UTF-8 encoded """

        def read_sck():
            try:
                d = self.proxy_conx.recv(65536)
                if len(d):
                    if DEBUG:
                        Logger().info(d)
                    return d

            except Exception:
                # Logger().info("%s <<<%s>>>" % (
                #     u"Failed to read data from rdpproxy authentifier socket",
                #     traceback.format_exc(e))
                # )
                raise AuthentifierSocketClosed()

            if DEBUG:
                Logger().info("received_buffer (empty packet)")
            raise AuthentifierSocketClosed()

        class Buffer:
            def __init__(self):
                self._data = read_sck()
                self._offset = 0

            def reserve_data(self, n):
                while len(self._data) - self._offset < n:
                    if DEBUG:
                        Logger().info("received_buffer (big packet) "\
                                      "old = %d / %d ; required = %d"
                                      % (self._offset, len(self._data), n))
                    self._data = self._data[self._offset:] + read_sck()
                    self._offset = 0

            def extract_name(self, n):
                self.reserve_data(n)
                _name = self._data[self._offset:self._offset+n].decode('utf-8')
                self._offset += n
                return _name

            def unpack(self, fmt, n):
                self.reserve_data(n)
                r = unpack_from(fmt, self._data, self._offset)
                self._offset += n
                return r

            def is_empty(self):
                return len(self._data) == self._offset

        _buffer = Buffer()
        _data = {}

        while True:
            _nfield, = _buffer.unpack('>H', 2)

            if DEBUG:
                Logger().info("received_buffer (nfield) = %d" % (_nfield,))

            for _ in range(0, _nfield):
                _type, _n = _buffer.unpack("BB", 2)
                _key = _buffer.extract_name(_n)

                if DEBUG:
                    Logger().info("received_buffer (key)   = %s%s"
                                  % ('?' if _type == 0x3f else '!', _key,))

                if _type == 0x3f: # b'?'
                    _data[_key] = MAGICASK
                else:
                    _n, = _buffer.unpack('>L', 4)
                    _data[_key] = _buffer.extract_name(_n)

                    if DEBUG:
                        Logger().info("received_buffer (value) = %s"
                                    % (_data[_key],))

            if _buffer.is_empty():
                break

            if DEBUG:
                Logger().info("received_buffer (several packet)")

        self.shared.update(_data)

        if DEBUG:
            Logger().info("receive_data (is asked): =\n%s" % (pprint.pformat(
                [e[0] for e in self.shared.items()])))

        return True, u''

    def get(self, key, default=None):
        return self.shared.get(key, default)

    def is_asked(self, key):
        return self.shared.get(key) == MAGICASK


class ACLPassthrough():
    def __init__(self, conn, addr):
        self.proxy_conx = conn
        self.addr       = addr
        self.shared = AuthentifierSharedData(conn)

    def interactive_target(self, data_to_send):
        data_to_send.update({ u'module' : u'interactive_target' })
        self.shared.send_data(data_to_send)
        _status, _error = self.shared.receive_data()
        if self.shared.get(u'display_message') != u'True':
            _status, _error = False, u'Connection closed by client'
        return _status, _error

    def receive_data(self):
        status, error = self.shared.receive_data()
        if not status:
            raise Exception(error)


    def selector_target(self, data_to_send):
        self.shared.send_data({
            u'module': u'selector',
            u'selector': '1',
            u'login': self.shared.get(u'target_login')
        })
        self.receive_data()
        self.shared.send_data(data_to_send)
        self.receive_data()
        if self.shared.is_asked(u'proto_dest'):
            target = self.shared.get(u'login').split(':')
            target_device = target[0]
            target_login = target[1]
            # login = target[2]
            self.shared.shared[u'target_login'] = target_login
            self.shared.shared[u'target_host'] = target_device
            self.shared.shared[u'target_device'] = target_device
            self.shared.shared[u'real_target_device'] = target_device
            # self.shared.shared[u'target_password'] = '...'
            # self.shared.shared[u'proto_dest'] = 'RDP'
        else:
            # selector_current_page, .....
            pass



    def start(self):
        _status, _error = self.shared.receive_data()

        device = "<host>$<application path>$<working dir>$<args> for Application"
        login = self.shared.get(u'login', MAGICASK) or MAGICASK
        host = self.shared.get(u'real_target_device', MAGICASK) or MAGICASK
        password = self.shared.get(u'password', MAGICASK) or MAGICASK
        splitted = login.split('@')
        if len(splitted) > 1:
            login = splitted[0]
            host = ''.join(splitted[1:])
            device = host

        interactive_data = {
            u'target_password': password,
            u'target_host': host,
            u'target_login': login,
            u'target_device': device
        }

        kv = {}

        if MAGICASK in (device, login, host, password):
            _status, _error = self.interactive_target(interactive_data)
        else:
            self.shared.shared[u'login'] = login
            self.shared.shared[u'target_login'] = login
            self.shared.shared[u'target_password'] = password
            self.shared.shared[u'target_host'] = host
            self.shared.shared[u'target_device'] = host
            self.shared.shared[u'real_target_device'] = host
            kv = interactive_data

        # selector_data = {
        #     u'target_login': 'Proxy\\Administrator\x01login 2\x01login 3',
        #     u'target_device': '10.10.44.27\x01device 2\x01device 3',
        #     u'proto_dest': 'RDP\x01VNC\x01RDP',
        # }
        # self.selector_target(selector_data)

        kv = {}
        # kv[u'is_rec'] = u'1'
        # kv[u'record_filebase'] = datetime.now().strftime("%Y-%m-%d/%H:%M-") + str(uuid.uuid4())
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
        kv[u'session_probe'] = u'0'

        if '$' in kv[u'target_host']:
            app_params = kv[u'target_host']
            list_params = app_params.split('$', 3)
            kv[u'target_host'] = list_params[0]
            if len(list_params) > 3:
                kv[u'alternate_shell'] = list_params[1]
                kv[u'shell_working_directory'] = list_params[2]
                kv[u'target_application'] = list_params[1]
                kv[u'shell_arguments'] = list_params[3]

        self.shared.send_data(kv)

        try_next = False
        signal.signal(signal.SIGUSR1, self.kill_handler)
        try:
            self.shared.send_data(kv)

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
                    _status, _error = self.shared.receive_data();

                    if self.shared.is_asked(u'keepalive'):
                        self.shared.send_data({u'keepalive': u'True'})
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
