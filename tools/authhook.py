#!/usr/bin/python
# -*- coding: UTF-8 -*-

import select
import httplib, socket
from socket import error
from struct    import unpack
from struct    import pack
import datetime
# import base64

from password import PASSWORD1, PASSWORD2, PASSWORD3, PASSWORD4, PASSWORD5, PASSWORD6, PASSWORD7, PASSWORD8

def cut_message(message, width = 75, in_cr = '\n', out_cr = '<br>', margin = 6):
    result = []
    for line in message.split(in_cr):
        while len(line) > width:
            end = line[width:].split(' ')

            if len(end[0]) <= margin:
                result.append((line[:width] + end[0]).rstrip())
                end = end[1:]
            else:
                result.append(line[:width] + end[0][:margin] + '-')
                end[0] = '-' + end[0][margin:]

            line = ' '.join(end)

        result.append(line.rstrip())

    return out_cr.join(result)


MAGICASK = 'UNLIKELYVALUEMAGICASPICONSTANTS3141592926ISUSEDTONOTIFYTHEVALUEMUSTBEASKED'

LOREM_IPSUM0 = "Message<br>message"
LOREM_IPSUM1 = "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium <br>doloremque laudantium, totam rem aperiam, eaque<br> ipsa quae ab illo inventore veritatis et<br> quasi architecto beatae vitae dicta sunt explicabo.<br> Nemo enim ipsam voluptatem quia voluptas<br> sit aspernatur aut odit aut fugit, sed quia<br> consequuntur magni dolores eos qui<br> ratione voluptatem sequi nesciunt.<br> Neque porro quisquam est,<br> qui dolorem ipsum quia dolor sit amet,<br> consectetur, adipisci velit, sed<br> quia non numquam eius modi tempora<br> incidunt ut labore et dolore magnam<br> aliquam quaerat voluptatem. Ut enim<br> ad minima veniam, quis nostrum<br> exercitationem ullam corporis suscipit<br> laboriosam, nisi ut aliquid ex ea<br> commodi consequatur? Quis autem<br>Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium <br>doloremque laudantium, totam rem aperiam, eaque<br> ipsa quae ab illo inventore veritatis et<br> quasi architecto beatae vitae dicta sunt explicabo.<br>"
LOREM_IPSUM2 = "Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium <br>doloremque laudantium, totam rem aperiam, eaque<br> ipsa quae ab illo inventore veritatis et<br> quasi architecto beatae vitae dicta sunt explicabo.<br> Nemo enim ipsam voluptatem quia voluptas<br> sit aspernatur aut odit aut fugit, sed quia<br> consequuntur magni dolores eos qui<br> ratione voluptatem sequi nesciunt.<br> Neque porro quisquam est,<br> qui dolorem ipsum quia dolor sit amet,<br> consectetur, adipisci velit, sed<br> quia non numquam eius modi tempora<br> incidunt ut labore et dolore magnam<br> aliquam quaerat voluptatem. Ut enim<br> ad minima veniam, quis nostrum<br> exercitationem ullam corporis suscipit<br> laboriosam, nisi ut aliquid ex ea<br> commodi consequatur? Quis autem<br>Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium <br>doloremque laudantium, totam rem aperiam, eaque<br> ipsa quae ab illo inventore veritatis et<br> quasi architecto beatae vitae dicta sunt explicabo.<br> Nemo enim ipsam voluptatem quia voluptas<br> sit aspernatur aut odit aut fugit, sed quia<br> consequuntur magni dolores eos qui<br> ratione voluptatem sequi nesciunt.<br> Neque porro quisquam est,<br> qui dolorem ipsum quia dolor sit amet,<br> consectetur, adipisci velit, sed<br> quia non numquam eius modi tempora<br> incidunt ut labore et dolore magnam<br> aliquam quaerat voluptatem. Ut enim<br> ad minima veniam, quis nostrum<br> exercitationem ullam corporis suscipit<br> laboriosam, nisi ut aliquid ex ea<br> commodi consequatur? Quis autem<br>"

LOREM_IPSUM = cut_message("""INFORMATION DESTINEE A L'UTILISATEUR AVANT CONNEXION

Dans un souci de sécurisation de l'accès et de l'utilisation des applications et des bases de données présentent sur le serveur, nous vous informons que l'intégralité de votre session de travail sera enregistrée dés votre clic OK du présent message.

L'enregistrement ainsi réalisé sera archivé pendant un délai de 6 mois à compter de l'ouverture de la présente session.

Conformément à la loi « informatique et libertés », le traitement des données qui sera effectué a fait l'objet d'une déclaration auprès de la CNIL.

En outre, en cliquant sur OK, vous reconnaissez avoir pris connaissance
 préalablement de la « Charte relative au bon usage des ressources
 d'information et de communication au sein de la société » notamment disponible sur les panneaux d'affichages réservé à l'information du personnel dans l'entreprise, ou si vous êtes prestataire, à la « Charte d’accès au SI » """)

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
            answer['target_login'] = service.login
            answer['target_password'] = service.password
            answer['proto_dest'] = service.protocol
            answer['target_port'] = service.port
            answer['timeclose'] = str(service.timeclose)
            answer['is_rec'] = service.is_rec
            answer['rec_path'] = service.rec_path
            if service.alternate_shell:
                answer['alternate_shell'] = service.alternate_shell
            if service.shell_working_directory:
                answer['shell_working_directory'] = service.shell_working_directory
            if service.display_message:
                answer['display_message'] = service.display_message
                answer['message'] = service.message
                answer['target_device'] = 'test_card'

        else:
            _selector = dic.get('selector')
            _device = dic.get('target_device')
            _login = dic.get('target_login')
            if (_device and _device != MAGICASK and _login and _login != MAGICASK):
                for service in self.services:
                    print("Testing target %s@%s in %s@%s" % (_login, _device, service.login, service.device))
                    if (service.login == _login and service.device == _device):
                        print("Target found %s@%s" % (_login, _device))
                        answer['selector'] = 'false'
                        answer['target_password'] = service.password
                        answer['proto_dest'] = service.protocol
                        answer['target_port'] = service.port
                        answer['timeclose'] = str(service.timeclose)
                        answer['is_rec'] = service.is_rec
                        answer['rec_path'] = service.rec_path
                        if service.alternate_shell:
                            answer['alternate_shell'] = service.alternate_shell
                        if service.shell_working_directory:
                            answer['shell_working_directory'] = service.shell_working_directory
                        if service.display_message:
                            answer['display_message'] = service.display_message
                            answer['message'] = service.message
                            answer['target_device'] = 'test_card'
                        break
                else:
                    if (_selector == MAGICASK):
                        self.prepare_selector(answer, dic)
                    else:
                        answer['login'] = MAGICASK
                        answer['password'] = MAGICASK
                        answer['target_device'] = MAGICASK
                        answer['target_login'] = MAGICASK
            else:
                self.prepare_selector(answer, dic)

        return answer

    def prepare_selector(self, answer, dic):
        try:
            _x = dic.get('selector_current_page', '1')
            if _x.startswith('!'):
                _x = _x[1:]
            _current_page = int(_x) - 1
        except:
            _current_page = 0

        try:
            _x = dic.get('selector_lines_per_page', '10')
            if _x.startswith('!'):
                _x = _x[1:]
            _lines_per_page = int(_x)
        except:
            _lines_per_page = 10

        _group_filter = dic.get('selector_group_filter', '')
        if _group_filter.startswith('!'):
            _group_filter = _group_filter[1:]
        _device_filter = dic.get('selector_device_filter', '')
        if _device_filter.startswith('!'):
            _device_filter = _device_filter[1:]
        _proto_filter = dic.get('selector_proto_filter', '')
        if _proto_filter.startswith('!'):
            _proto_filter = _proto_filter[1:]
        answer['selector'] = 'true'

        all_services = []
        all_groups = []
        all_protos = []
        all_endtimes = []
        for service in self.services:
            target = "%s@%s" %(service.login, service.device)
            if target.find(_device_filter) == -1:
                continue
            if service.protocol.lower().find(_group_filter) == -1:
                continue
            if service.protocol.find(_proto_filter.upper()) == -1:
                continue
            # multiply number of entries by 15 to test pagination
            all_services.append(target)
            all_groups.append(service.protocol.lower())
            all_protos.append(service.protocol)
            all_endtimes.append(service.endtime)
        _number_of_pages = 1
        if _lines_per_page != 0:
            _number_of_pages = 1 + (len(all_protos) - 1) / _lines_per_page
        if _current_page >= _number_of_pages:
            _current_page = _number_of_pages - 1
        if _current_page < 0:
            _current_page = 0
        print "lines per page = ",_lines_per_page
        _start_of_page = _current_page * _lines_per_page
        _end_of_page = _start_of_page + _lines_per_page
        answer['proto_dest'] = " ".join(all_protos[_start_of_page:_end_of_page])
        answer['end_time'] = ";".join(all_endtimes[_start_of_page:_end_of_page])
        answer['target_login'] = " ".join(all_groups[_start_of_page:_end_of_page])
        answer['target_device'] = " ".join(all_services[_start_of_page:_end_of_page])
        answer['selector_number_of_pages'] = str(_number_of_pages)
        answer['selector_current_page'] = _current_page + 1


class Service(object):
    def __init__(self, name, device, login, password, protocol, port, is_rec = 'False', rec_path = '/tmp/testxxx.png', alive=720000, clipboard = 'true', file_encryption = 'true', alternate_shell = '', shell_working_directory = ''):
        import time
        import datetime
        self.name = name
        self.device = device
        self.login = login
        self.password = password
        self.protocol = protocol
        self.port = port
        self.timeclose = int(time.time()+alive)
        self.endtime = datetime.datetime.strftime(datetime.datetime.fromtimestamp(self.timeclose), "%Y-%m-%d %H:%M:%S")
        self.is_rec = is_rec
        self.rec_path = rec_path
        self.display_message = None
        self.message = None
        self.clipboard = clipboard
        self.file_encryption = file_encryption
        self.alternate_shell = alternate_shell
        self.shell_working_directory = shell_working_directory

        if self.device == 'display_message':
            self.display_message = MAGICASK
            self.message = LOREM_IPSUM

class Authentifier(object):
    # we should just transmit some kind of salted hash to get something
    # more secure for password transmission, but for now the authentication
    # protocol is just supposed to be used locally on a secure system.
    # It will certainly change to something stronger to avoid storing passwords
    # at all. Comparing hashes is enough anyway.
    def __init__(self, sck, users):
        self.sck = sck
        self.users = users
        self.dic = {'login':MAGICASK, 'password':MAGICASK}
        self.tries = 5

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
        _data = dict((x, y) for x, y in zip(p, p) if (x[:6] != 'trans_'))
        for key in _data:
            print("Receiving %s=%s" % (key, _data[key]))
            if (_data[key][:3] == 'ASK'):
                _data[key] = MAGICASK
            elif (_data[key][:1] == '!'):
                # BASE64 TRY
                # _data[key] = base64.b64decode(_data[key][1:])
                _data[key] = _data[key][1:]
            else:
                # BASE64 TRY
                # _data[key] = base64.b64decode(_data[key])
                # _data[key] unchanged
                pass
        self.dic.update(_data)

        answer = {'authenticated': 'false', 'clipboard' : 'true', 'file_encryption' : 'true', 'trans_cancel' : 'Annuler', 'trans_ok' : 'Oui', 'width' : '1280', 'height' : '1024', 'bpp' : '8'}
        _login = self.dic.get('login')
        if _login != MAGICASK:
            for user in self.users:
                if user.name == _login:
                    _password = self.dic.get('password')
                    if _password and user.password == _password:
                        answer['authenticated'] = 'true'
                        print("Password OK for user %s" % user.name)
                    else:
                        answer['authenticated'] = 'false'
                        answer['password'] = MAGICASK
                        print("Wrong Password for user %s" % user.name)
                    break


        if answer['authenticated'] == 'true':
            self.tries = 5
            answer.update(user.get_service(self.dic))
        else:
            self.tries = self.tries - 1
            if self.tries == 0:
                answer['rejected'] = "Too many login failures"

        self.dic.update(answer)
        self.send()
        return True

    def send(self):
        self.dic['keepalive'] = 'true'
        # BASE64 TRY
        # _list = ["%s\n%s\n" % (key, ("!%s" % base64.b64encode(("%s" % value))) if value != MAGICASK else "ASK") for key, value in self.dic.iteritems()]
        _list = ["%s\n%s\n" % (key, ("!%s" % value) if value != MAGICASK else "ASK") for key, value in self.dic.iteritems()]

        for s in _list:
            print("Sending %s=%s" % tuple(s.split('\n')[:2]))

        _data = "".join(_list)
        _len = len(_data)
        print("len=", _len,)
        self.sck.send(pack(">L", _len))
        self.sck.send(_data)


server3350 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server3350.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server3350.bind(('127.0.0.1', 3350))
server3350.listen(5)

server3450 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server3450.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server3450.bind(('127.0.0.1', 3450))
server3450.listen(5)

servers = [server3350, server3450]
wsockets = []
manager ={}

users = [
    User('one', 'one', [
        Service('Test', 'autotest', 'replay', 'password', 'INTERNAL', '')]),

    User('x', 'x', [
        Service('VirtualBox', '127.0.0.1', 'redjenkins-vbox', 'x', 'RDP', '6389'),
        Service('Message', 'display_message', 'internal', 'internal', 'INTERNAL', '', alive=120),
        Service('Test', 'autotest', 'replay', 'password', 'INTERNAL', ''),
        Service('w2003', '10.10.46.73', r'qa\administrateur', PASSWORD1, 'RDP', '3389'),
        Service('loop', '10.10.43.13', r'x', 'notx', 'RDP', '3389'),
#        Service('loop', '10.10.43.13', r'x', 'notx', 'RDP', '3389'),
        Service('w2008', '10.10.46.114', r'administrator', PASSWORD6, 'RDP', '3389'),
        Service('Vnc', '10.10.46.70', 'any', PASSWORD2, 'VNC', '5900'),
        Service('Vnc', '10.10.46.70', 'nobody', PASSWORD3, 'VNC', '5900'),
        Service('xp', '10.10.46.111', r'qa\administrateur', PASSWORD1, 'RDP', '3389'),
        Service('seven', '10.10.46.116', r'qa\administrateur', PASSWORD1, 'RDP', '3389'),
        Service('w2003', '10.10.46.70', r'test_unicode', PASSWORD4, 'RDP', '3389'),
        Service('xp', '10.10.47.58', r'xavier', PASSWORD7, 'RDP', '3389'),
        Service('sqlserver', '10.10.46.66', r'qa\administrateur', PASSWORD1, 'RDP', '3389'),
        Service('w2008', '10.10.46.78', r'administrateur@qa', PASSWORD1, 'RDP', '3389'),
        Service('w2008', '10.10.46.70', r'administrateur@qa', PASSWORD1, 'RDP', '3389'),
        Service('w2000', '10.10.46.64', r'administrateur', PASSWORD5, 'RDP', '3389'),
        Service('ok2008', '10.10.3.172', r'Administrateur', PASSWORD6, 'RDP', '3389'),
#        Service('cert2008', '10.10.3.171', r'Administrateur', PASSWORD6, 'RDP', '3389'),
#        Service('TLS2008', '10.10.46.88', r'Administrateur@qa', PASSWORD1, 'RDP', '3389'),
        Service('TLS2008', '10.10.46.88', r'administrateur@qa', PASSWORD1, 'RDP', '3389'),
        Service('w7', '10.10.46.77', r'qa\administrateur', PASSWORD1, 'RDP', '3389'),
        Service('w2000', '10.10.46.64', r'administrateur', PASSWORD7, 'RDP', '3389'),
        # this one wont answer
        Service('w2000', '10.90.14.64', r'administrateur', PASSWORD7, 'RDP', '3389'),
        Service('Bouncer', 'bouncer2', 'internal', 'internal', 'INTERNAL', ''),
        Service('Card', 'test_card', 'internal', 'internal', 'INTERNAL', ''),
        Service('xp', '10.10.2.139', r'xavier', PASSWORD7, 'RDP', '3389'),
        Service('w2003', '10.10.3.28', r'administrator', PASSWORD6, 'RDP', '3389'),
        Service('Vnc', '10.10.3.103', 'any', PASSWORD7, 'VNC', '5900'),
        Service('Vnc', '192.168.10.11', 'any', 'titi', 'VNC', '5900'),
        Service('Vnc', '10.10.43.13', 'any', PASSWORD7, 'VNC', '5900')]),

    User('timeout', 'timeout', [
        Service('xp2m', '10.10.46.111', r'qa\administrateur', PASSWORD1, 'RDP', '3389', alive=120),
        Service('w2008', '10.10.46.78', r'qa\administrateur', PASSWORD1, 'RDP', '3389', alive=120),
        Service('w2008-2', '10.10.46.78', r'administrateur@qa', PASSWORD1, 'RDP', '3389', alive=120),
        Service('TLS2008', '10.10.46.88', r'Administrateur@qa', PASSWORD1, 'RDP', '3389', alive=120),
        Service('w7', '10.10.46.77', r'qa\administrateur', PASSWORD1, 'RDP', '3389', alive=120),
        Service('w2000', '10.10.46.64', r'administrateur', PASSWORD5, 'RDP', '3389', alive=120),
        Service('Bouncer', 'bouncer2', 'internal', 'internal', 'INTERNAL', '', alive=120),
        Service('Test', 'test', 'internal', 'internal', 'INTERNAL', '', alive=120),
        Service('Card', 'test_card', 'internal', 'internal', 'INTERNAL', '', alive=120),
        Service('Message', 'display_message', 'internal', 'internal', 'INTERNAL', '', alive=120),
        Service('Vnc', '10.10.3.103', 'any', PASSWORD7, 'VNC', '5900', alive=120),
        Service('Vnc', '10.10.43.13', 'any', PASSWORD7, 'VNC', '5900', alive=120)]),

    User('rec', 'rec', [
        Service('w2008', '10.10.46.114', r'administrator', PASSWORD6, 'RDP', '3389', is_rec = 'True', rec_path='/tmp/replay'),
        Service('Vnc', '10.10.46.70', 'any', PASSWORD2, 'VNC', '5900', is_rec = 'True', rec_path='/tmp/replay'),
        Service('xp', '10.10.46.111', r'qa\administrateur', PASSWORD1, 'RDP', '3389', is_rec = 'True', rec_path='/tmp/replay'),
        Service('seven', '10.10.46.116', r'qa\administrateur', PASSWORD1, 'RDP', '3389', is_rec = 'True', rec_path='/tmp/replay'),
        Service('w2003', '10.10.46.73', r'qa\administrateur', PASSWORD1, 'RDP', '3389', is_rec = 'True', rec_path='/tmp/replay'),
        Service('sqlserver', '10.10.46.66', r'qa\administrateur', PASSWORD1, 'RDP', '3389', is_rec = 'True', rec_path='/tmp/replay'),
        Service('xp2', '10.10.47.58', r'xavier', PASSWORD8, 'RDP', '3389', is_rec = 'True', rec_path='/tmp/replay'),
        Service('w2003', '10.10.46.70', r'test_unicode', PASSWORD4, 'RDP', '3389', is_rec = 'True', rec_path='/tmp/replay.wrm'),
        Service('w2003', '10.10.46.70', r'administrateur@qa', PASSWORD1, 'RDP', '3389', is_rec = 'True', rec_path='/tmp/replay.wrm'),
        Service('w2008', '10.10.46.78', r'qa\administrateur', PASSWORD1, 'RDP', '3389', is_rec = 'True', rec_path='/tmp/replay.wrm'),
        Service('w2008-2', '10.10.46.78', r'administrateur@qa', PASSWORD1, 'RDP', '3389', is_rec = 'True', rec_path='/tmp/replay'),
        Service('ok2008', '10.10.3.172', r'Administrateur', PASSWORD6, 'RDP', '3389', is_rec = 'True', rec_path='/tmp/replay'),
        Service('TLS2008', '10.10.46.88', r'Administrateur@qa', PASSWORD1, 'RDP', '3389', is_rec = 'True', rec_path='/tmp/replay'),
        Service('w7', '10.10.46.77', r'qa\administrateur', PASSWORD1, 'RDP', '3389', is_rec = 'True', rec_path='/tmp/replay'),
        Service('w2000', '10.10.46.64', r'administrateur', PASSWORD5, 'RDP', '3389', is_rec = 'True', rec_path='/tmp/replay'),
        Service('xp', '10.10.2.139', r'xavier', PASSWORD7, 'RDP', '3389', is_rec = 'True', rec_path='/tmp/test_xp'),
        Service('w2003', '10.10.3.28', r'administrator', PASSWORD6, 'RDP', '3389', is_rec = 'True', rec_path='/tmp/replay'),
        Service('Bouncer', 'bouncer2', 'internal', 'internal', 'INTERNAL', '', is_rec = 'True', rec_path='/tmp/replay'),
        Service('Test', 'test', 'internal', 'internal', 'INTERNAL', '', is_rec = 'True', rec_path='/tmp/replay'),
        Service('Card', 'test_card', 'internal', 'internal', 'INTERNAL', '', is_rec = 'True', rec_path='/tmp/replay'),
        Service('Vnc', '10.10.3.103', 'any', PASSWORD7, 'VNC', '5900', is_rec = 'True', rec_path='/tmp/replay'),
        Service('Vnc', '10.10.43.13', 'any', PASSWORD2, 'VNC', '5900', is_rec = 'True', rec_path='/tmp/replay')]),

    User('xp', 'xp', [
        Service('xp', '10.10.46.111', r'qa\administrateur', PASSWORD1, 'RDP', '3389')]),

    User('card', 'card', [
        Service('Card', 'test_card', 'internal', 'internal', 'INTERNAL', '')]),
    User('loop', 'loop', [
        Service('Card', '127.0.0.1', 'card', 'card', 'RDP', '3389')]),
    User('bouncer', 'bouncer', [
        Service('Bouncer', 'bouncer2', 'internal', 'internal', 'INTERNAL', '')]),
    User('selector', 'selector', [
        Service('Selector', 'selector', 'internal', 'internal', 'INTERNAL', '')])
]

while 1:
    rsockets = servers + manager.keys()
    rfds, wfds, xfds = select.select(rsockets, [], [], 1)

    for s in rfds:
        if s in servers:
            (sck, address) = s.accept()
            rsockets.append(sck)
            print("Accepting connection\n")
            import os
#            os.system("./replay_last.pl")
            manager[sck] = Authentifier(sck, users)
        else:
            if not manager[s].read():
                del manager[s]
