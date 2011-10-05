#!/usr/bin/python

import select
import httplib, socket
from socket import error
from struct    import unpack
from struct    import pack

class User(object):
    def __init__(self, name, password, services = None, messages = None, rec_path = None):
        self.name = name
        self.password = password
        if messages is None:
            self.messages = []
        self.rec_path = rec_path
        self.services = services if services else []

    def add(self, service):
        self.services.append(service)

    def get_service(self, dic):
        answer = {}
        if len(self.services) == 1:
            # Authenticated user only has access to one service, connect to it immediately
            service = self.services[0]
            answer['target_device'] = service.device
            answer['proto_login'] = service.login
            answer['target_password'] = service.password
            answer['proto_dest'] = service.protocol
            answer['target_port'] = service.port
        else:
            _selector = dic.get('selector')
            _device = dic.get('target_device')
            _login = dic.get('target_login')
            if (_device and _device[:3] != 'ASK' and _login and _login[:3] != 'ASK'):
                for service in self.services:
                    print("Testing target %s@%s in %s@%s" % (_login, _device, service.login, service.device))
                    if (service.login == _login and service.device == _device):
                        print("Target found %s@%s" % (_login, _device))
                        answer['selector'] = 'false'
                        answer['target_password'] = service.password
                        answer['proto_dest'] = service.protocol
                        answer['target_port'] = service.port
                        break
                else:
                    if (_selector == 'ASK'):
                        answer['selector'] = 'true'
                        all_services = []
                        all_groups = []
                        all_protos = []
                        all_endtimes = []
                        for service in self.services:
                            all_services.append("%s@%s" %(service.login, service.device))
                            all_groups.append(service.protocol.lower())
                            all_protos.append(service.protocol)
                            all_endtimes.append("-")
                        answer['proto_dest'] = " ".join(all_protos)
                        answer['endtime'] = " ".join(all_endtimes)
                        answer['target_login'] = " ".join(all_groups)
                        answer['target_device'] = " ".join(all_services)
                    else:
                        answer['login'] = 'ASK'
                        answer['password'] = 'ASK'
                        answer['target_device'] = 'ASK'
                        answer['target_login'] = 'ASK'
            else:
                answer['selector'] = 'true'
                all_services = []
                all_groups = []
                all_protos = []
                all_endtimes = []
                for service in self.services:
                    all_services.append("%s@%s" %(service.login, service.device))
                    all_groups.append(service.protocol.lower())
                    all_protos.append(service.protocol)
                    all_endtimes.append("-")
                answer['proto_dest'] = " ".join(all_protos)
                answer['endtime'] = " ".join(all_endtimes)
                answer['target_login'] = " ".join(all_groups)
                answer['target_device'] = " ".join(all_services)

        return answer

class Service(object):
    def __init__(self, name, device, login, password, protocol, port):
        self.name = name
        self.device = device
        self.login = login
        self.password = password
        self.protocol = protocol
        self.port = port

class Authentifier(object):
    # we should just transmit some kind of salted hash to get something
    # more secure for password transmission, but for now the authentication
    # protocol is just supposed to be used locally on a secure system.
    # It will certainly change to something stronger to avoid storing passwords
    # at all. Comparing hashes is enough anyway.
    def __init__(self, sck, users):
        self.sck = sck
        self.users = users
        self.dic = {'login':'ASK', 'password':'ASK'}

#    passwords = {
#        'v' : 'v',
#        'vnc' : 'vnc',
#        'vnc2' : 'vnc2',
#        'n' : 'n',
#        'cgr' : 'cgr',
#        'bouncer' : 'bouncer',
#        'test' : 'test',
#        'message' : 'message',
#        'card' : 'card',
#        'loop' : 'loop',
#        'selector' : 'selector',
#        'looop' : 'looop',
#        'error_invalid' : 'error_invalid'
#    }
#        'message' : {
#            'proxy_type': 'RDP',
#            'target_device' : 'test_card',
#            'display_message' : 'ASK',
#            'message' : 'Hello, World',
#            'proto_dest': 'INTERNAL',
#            'is_rec':'False',
#            'authenticated':'true'
#        },
#        'card' : {
#            'proxy_type': 'RDP',
#            'target_device' : 'test_card',
#            'proto_dest': 'INTERNAL',
#            'is_rec':'True',
#            'rec_path':'/tmp/test_card.cpp',
#        },
#        'selector' : {
#            'proxy_type': 'RDP',
#            'target_device' : 'selector',
#            'proto_dest': 'INTERNAL',
#            'authenticated':'true',

#        },
#        'n' : {
#            'proxy_type': 'RDP',
#            'target_login' : r'vnc',
#            'target_device' : '10.10.4.13',
#            'target_password' : 'silver',
#            'target_port':'5901',
#            'proto_dest': 'VNC',
#            'is_rec':'True',
#            'rec_path':'/tmp/vnctest.cpp',
#        },
#        'vnc' : {
#            'proxy_type': 'RDP',
#            'target_login' : r'vnc',
#            'target_device' : '10.10.3.141',
#            'target_password' : 'SecureLinux',
#            'target_port':'5900',
#            'proto_dest': 'VNC',
#            'is_rec':'True',
#            'rec_path':'/tmp/vnctest.cpp',
#        },
#        'vnc2' : {
#            'proxy_type': 'RDP',
#            'target_login' : r'vnc',
#            'target_device' : '10.10.3.103',
#            'target_password' : 'SecureLinux',
#            'target_port':'5900',
#            'proto_dest': 'VNC',
#            'is_rec':'True',
#            'rec_path':'/tmp/vnctest.cpp',
#        },
#        'v' : {
#            'proxy_type': 'RDP',
#            'target_login' : r'vnc',
#            'target_device' : '192.168.1.28',
#            'target_password' : 'secure',
#            'target_port':'5900',
#            'proto_dest': 'VNC'
#        },
#        'cgr' : {
#            'proxy_type': 'RDP',
#            'target_login' : r'vnc',
#            'target_device' : '10.10.4.13',
#            'target_password' : 'secure',
#            'target_port':'5901',
#            'proto_dest': 'VNC'
#        },
#        'loop' : {
#            'proxy_type': 'RDP',
#            'target_login' : r'card',
#            'target_device' : '127.0.0.1',
#            'target_password' : 'card',
#            'target_port':'3389',
#            'proto_dest': 'RDP'
#        },
#        'looop' : {
#            'proxy_type': 'RDP',
#            'target_login' : r'loop',
#            'target_device' : '127.0.0.1',
#            'target_password' : 'loop',
#            'target_port':'3389',
#            'proto_dest': 'RDP'
#        },
#        'error_invalid' : {
#            'target_device': "10.10.3.54" ,
#            'target_login': "administrateur",
#            'authenticated': 'False',
#            'rejected':"Invalid IP Source"}
#    }

#    def __init__(self, sck):
#        self.sck = sck

    def read(self):
        print("Reading")
        try:
            _packet_size, = unpack(">L", self.sck.recv(4))
            print("Received Data length : %s" % _packet_size)
            _data = self.sck.recv(int(_packet_size))
        except Exception:
            # It's quick and dirty, but we do as if all possible errors
            # are authentifier socket was closed.
            return False

        p = iter(_data.split('\n'))
        self.dic.update(dict((x, y) for x, y in zip(p, p) if (x[:6] != 'trans_')))
        print(self.dic)


        answer = {'authenticated': 'false'}
        _login = self.dic.get('login')
        for user in self.users:
            if user.name == _login:
                _password = self.dic.get('password')
                if _password and user.password == _password:
                    answer['authenticated'] = 'true'
                    print("Password OK for user %s" % user.name)
                else:
                    answer['authenticated'] = 'false'
                    answer['login'] = 'ASK'
                    answer['password'] = 'ASK'
                    print("Wrong Password for user %s" % user.name)
                break

        if answer['authenticated'] == 'true':
            answer.update(user.get_service(self.dic))

        self.dic.update(answer)
        self.send()
        return True

    def send(self):
        print("Sending", self.dic)
        _data = ''.join(["%s\n" % v for tu in self.dic.iteritems() for v in tu])
        _len = len(_data)
        self.sck.send(pack(">L", _len+4))
        self.sck.send(_data)


server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind(('127.0.0.1', 3450))
server.listen(5)
wsockets = []
manager ={}

users = [
    User('x', 'x', [
        Service('xp', '10.10.14.111', r'qa\administrateur', 'S3cur3!1nux', 'RDP', '3389'),
        Service('w2008', '10.10.14.78', r'qa\administrateur', 'S3cur3!1nux', 'RDP', '3389'),
        Service('w7', '10.10.14.77', r'qa\administrateur', 'S3cur3!1nux', 'RDP', '3389'),
        Service('w2000', '10.10.14.64', r'administrateur', 'SecureLinux', 'RDP', '3389'),
        Service('Bouncer', 'bouncer2', 'internal', 'internal', 'INTERNAL', ''),
        Service('Card', 'test_card', 'internal', 'internal', 'INTERNAL', '')]),
    User('bouncer', 'bouncer', [
        Service('Bouncer', 'bouncer2', 'internal', 'internal', 'INTERNAL', '')]),
    User('selector', 'selector', [
        Service('Selector', 'selector', 'internal', 'internal', 'INTERNAL', '')])
]

#            'is_rec' : 'true',
#            'rec_path':'/tmp/trace2008.cpp',

while 1:
    rsockets = [server]+manager.keys()
    rfds, wfds, xfds = select.select(rsockets, [], [], 1)

    for s in rfds:
        if s == server:
            (sck, address) = server.accept()
            rsockets.append(sck)
            print("Accepting connection\n")
            manager[sck] = Authentifier(sck, users)
        else:
            if not manager[s].read():
                del manager[s]
