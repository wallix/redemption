import select
import httplib, socket
from socket import error
from struct    import unpack
from struct    import pack



class Authentifier(object):
    # we should just transmit some kind of salted hash to get something
    # more secure for password transmission, but for now the authentication
    # protocol is just supposed to be used locally on a secure system.
    # It will certainly change to something stronger to avoid storing passwords
    # at all. Comparing hashes is enough anyway.
    passwords = {
        'xp': 'xp',
        'v' : 'v',
        'n' : 'n',
        'cgr' : 'cgr',
    }
    targets = {
        'xp' : {
            'proxy_type': 'RDP',
            'target_login' : r'qa\administrateur',
            'target_device' : '10.10.14.111',
            'target_password' : 'S3cur3!1nux',
            'target_port':'3389'
        },
        'n' : {
            'proxy_type': 'RDP',
            'target_login' : r'vnc',
            'target_device' : '10.10.4.163',
            'target_password' : 'tabra',
            'target_port':'5900',
            'proto_dest': 'VNC'
        },
        'v' : {
            'proxy_type': 'RDP',
            'target_login' : r'vnc',
            'target_device' : '192.168.1.28',
            'target_password' : 'secure',
            'target_port':'5900',
            'proto_dest': 'VNC'
        },
        'cgr' : {
            'proxy_type': 'RDP',
            'target_login' : r'vnc',
            'target_device' : '10.10.4.13',
            'target_password' : 'secure',
            'target_port':'5901',
            'proto_dest': 'VNC'
        }
    }

    def __init__(self, sck):
        self.sck = sck

    def read(self):
        print "Reading"
        try:
            _packet_size, = unpack(">L", self.sck.recv(4))
            print("Received Data length : %s" % _packet_size)
            _data = self.sck.recv(int(_packet_size))
        except Exception, e:
            # It's quick and dirty, but we do as if all possible errors
            # are authentifier socket was closed.
            return False

        p = iter(_data.split('\n'))
        self.dic = dict((x, y) for x, y in zip(p, p) if (x[:6] != 'trans_'))

        print self.dic
        _login = self.dic.get('login')
        _password = self.dic.get('password')
        if _password and self.passwords.get(_login) == _password:
            print "Password OK"
            self.dic.update(self.targets.get(_login, {}))
        else:
            print "Wrong Password"
            self.dic = {'login' : 'ASK' }
        self.send()
        return True

    def send(self):
        print "Sending", self.dic
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

while 1:
    rsockets = [server]+manager.keys()
    rfds, wfds, xfds = select.select(rsockets, [], [], 1)

    for s in rfds:
        if s == server:
            (sck, address) = server.accept()
            rsockets.append(sck)
            manager[sck] = Authentifier(sck)
            print "Accepting connection\n"
        else:
            if not manager[s].read():
                del manager[s]
