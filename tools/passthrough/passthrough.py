#!/usr/bin/env -S python3 -O
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
from select import select
import socket

import uuid # for random rec_path and .log


MAGICASK = 'UNLIKELYVALUEMAGICASPICONSTANTS3141592926ISUSEDTONOTIFYTHEVALUEMUSTBEASKED'
DEBUG = True

if DEBUG:
    import pprint

class AuthentifierSocketClosed(Exception):
    pass

class AuthentifierSharedData():
    def __init__(self, conn):
        self.proxy_conx = conn
        self.shared = {
            'module':                  'login',
            'selector_group_filter':   '',
            'selector_device_filter':  '',
            'selector_proto_filter':   '',
            'selector':                'False',
            'selector_current_page':   '1',
            'selector_lines_per_page': '0',

            'target_login':    MAGICASK,
            'target_device':   MAGICASK,
            'target_host':     MAGICASK,
            'login':           "admin",
            'ip_client':       MAGICASK,
            'proto_dest':      MAGICASK,
        }

    def send_data(self, data):
        """ NB : Strings sent to the ReDemPtion proxy MUST be UTF-8 encoded """

        if DEBUG:
            Logger().info('================> send_data (update) =\n%s' % (pprint.pformat(data)))

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
            Logger().info('send_data (on the wire) length = %s' % len(_list))

        _r_data = b''.join(t[2] for t in _list)
        self.proxy_conx.sendall(pack('>H', len(_list)))
        self.proxy_conx.sendall(_r_data)

    def receive_data(self):
        """ NB : Strings coming from the ReDemPtion proxy are UTF-8 encoded """

        def read_sck():
            try:
                d = self.proxy_conx.recv(65536)
                if len(d):
                    if DEBUG:
                        Logger().info(d)
                    return d

            except Exception:
                # Logger().info("%s <<<%s>>>" % (
                #     "Failed to read data from rdpproxy authentifier socket",
                #     traceback.format_exc())
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
        data_to_send.update({ 'module' : 'interactive_target' })
        self.shared.send_data(data_to_send)
        self.shared.receive_data()
        if self.shared.get('display_message') != 'True':
            raise Exception('Connection closed by client')

    def selector_target(self, data_to_send):
        self.shared.send_data({
            'module': 'selector',
            'selector': '1',
            'login': self.shared.get('target_login')
        })
        self.shared.receive_data()
        self.shared.send_data(data_to_send)
        self.shared.receive_data()
        if self.shared.is_asked('proto_dest'):
            target = self.shared.get('login').split(':')
            target_device = target[0]
            target_login = target[1]
            # login = target[2]
            self.shared.shared['target_login'] = target_login
            self.shared.shared['target_host'] = target_device
            self.shared.shared['target_device'] = target_device
            self.shared.shared['real_target_device'] = target_device
            # self.shared.shared['target_password'] = '...'
            # self.shared.shared['proto_dest'] = 'RDP'
        else:
            # selector_current_page, .....
            pass



    def start(self):
        self.shared.receive_data()

        device = "<host>$<application path>$<working dir>$<args> for Application"
        login = self.shared.get('login', MAGICASK) or MAGICASK
        host = self.shared.get('real_target_device', MAGICASK) or MAGICASK
        password = self.shared.get('password', MAGICASK) or MAGICASK
        splitted = login.split('@', 1)
        if len(splitted) == 2:
            login = splitted[0]
            host = splitted[1]
            device = host
            if login == 'internal':
                password = ''

        interactive_data = {
            'target_password': password,
            'target_host': host,
            'target_login': login,
            'target_device': device
        }

        kv = {}

        if MAGICASK in (device, login, host, password):
            self.interactive_target(interactive_data)
        else:
            self.shared.shared['login'] = login
            self.shared.shared['target_login'] = login
            self.shared.shared['target_password'] = password
            self.shared.shared['target_host'] = host
            self.shared.shared['target_device'] = host
            self.shared.shared['real_target_device'] = host
            kv = interactive_data

        # uncomment the following to get the selector module
        # selector_data = {
        #     'target_login': 'Proxy\\Administrator\x01login 2\x01login 3',
        #     'target_device': '10.10.44.27\x01device 2\x01device 3',
        #     'proto_dest': 'RDP\x01VNC\x01RDP',
        # }
        # self.selector_target(selector_data)

        now = datetime.now()
        session_id = str(uuid.uuid4())

        kv = {}

        # kv['is_rec'] = '1'  # Enable recording
        kv['trace_type'] = '0'
        # kv['encryption_key'] = '1E' * 32  # 32 bytes string in hexadecimal format
        # kv['sign_key'] = '1E' * 32  # 32 bytes string in hexadecimal format

        # kv['hash_path'] = '...'
        # kv['record_path'] = '...'
        kv['record_subdirectory'] = now.strftime("%Y-%m-%d")
        kv['record_filebase'] = now.strftime("%H:%M-") + session_id
        kv['session_log:enable_session_log_file'] = '0'

        kv['login'] = self.shared.get('target_login')
        kv['proto_dest'] = "RDP"
        kv['target_port'] = "3389"
        kv['session_id'] = session_id
        kv['module'] = 'RDP' if self.shared.get('login') != 'internal' else host
        kv['target_password'] = self.shared.get('target_password')
        kv['target_login'] = self.shared.get('target_login')
        kv['target_host'] = self.shared.get('target_host')
        kv['target_device'] = self.shared.get('target_host')

        if '$' in kv['target_host']:
            app_params = kv['target_host']
            list_params = app_params.split('$', 3)
            kv['target_host'] = list_params[0]
            if len(list_params) > 3:
                kv['alternate_shell'] = list_params[1]
                kv['shell_working_directory'] = list_params[2]
                kv['target_application'] = list_params[1]
                kv['shell_arguments'] = list_params[3]

        self.shared.send_data(kv)

        try_next = False
        signal.signal(signal.SIGUSR1, self.kill_handler)
        try:
            self.shared.send_data(kv)

            # Looping on keepalived socket
            while True:
                r = []
                Logger().info("Waiting on proxy")
                got_signal = False
                try:
                    r, w, x = select([self.proxy_conx], [], [], 60)
                except Exception as e:
                    if DEBUG:
                        Logger().info("exception: '%s'" % e)
                        import traceback
                        Logger().info("<<<<%s>>>>" % traceback.format_exc())
                    if e[0] != 4:
                        raise
                    Logger().info("Got Signal %s" % e)
                    got_signal = True
                if self.proxy_conx in r:
                    self.shared.receive_data();

                    if self.shared.is_asked('keepalive'):
                        self.shared.send_data({'keepalive': 'True'})
                # r can be empty
                else: # (if self.proxy_conx in r)
                    Logger().info('Missing Keepalive')
                    Logger().error('break connection')
                    release_reason = 'Break connection'
                    break
            Logger().debug("End Of Keep Alive")


        except AuthentifierSocketClosed:
            if DEBUG:
                import traceback
                Logger().info("RDP/VNC connection terminated by client")
                Logger().info("<<<<%s>>>>" % traceback.format_exc())
        except Exception:
            if DEBUG:
                import traceback
                Logger().info("RDP/VNC connection terminated by client")
                Logger().info("<<<<%s>>>>" % traceback.format_exc())

        try:
            Logger().info("Close connection ...")

            self.proxy_conx.close()

            Logger().info("Close connection done.")
        except IOError:
            if DEBUG:
                Logger().info("Close connection: Exception")
                Logger().info("<<<<%s>>>>" % traceback.format_exc())
    # END METHOD - START


    def kill_handler(self, signum, frame):
        # Logger().info("KILL_HANDLER = %s" % signum)
        if signum == signal.SIGUSR1:
            self.kill()

    def kill(self):
        try:
            Logger().info("Closing a RDP/VNC connection")
            self.proxy_conx.close()
        except Exception:
            pass



from socket import (fromfd, AF_UNIX, AF_INET, SOCK_STREAM, SOL_SOCKET, SO_REUSEADDR)

# if this value changes, it should be synchronized with `[globals] authfile` in rdpproxy.ini
socket_path = '/tmp/redemption-sesman-sock'

def standalone():
    print('open socket at', socket_path)
    signal.signal(signal.SIGCHLD, signal.SIG_IGN)
    if os.path.exists(socket_path):
        os.unlink(socket_path)
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
