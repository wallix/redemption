#!/usr/bin/python -O
# -*- coding: utf-8 -*-
##
# Copyright (c) 2010-2014 WALLIX, SARL. All rights reserved.
# Licensed computer software. Property of WALLIX.
# Product name: WALLIX Admin Bastion V 2.x
# Author(s): Olivier Hervieu, Christophe Grosjean, Raphael Zhou, Meng Tan
# Id: $Id$
# URL: $URL$
# Module description:  Sesman Worker
##
from __future__ import with_statement

import random
import os
import urllib
import signal
import traceback
import json
from logger import Logger

from cutmessage import cut_message
from struct     import unpack
from struct     import pack
from select     import select
from time       import time
from time       import strftime
from time       import ctime
from time       import timezone
from time       import altzone
from time       import daylight
from time       import sleep
from time       import mktime
from datetime   import datetime
import socket
from socket     import gethostname

#TODO : remove these hardcoded strings
RECORD_PATH = u'/var/wab/recorded/rdp/'

from sesmanconf import TR, SESMANCONF, translations
import engine

from engine import APPROVAL_ACCEPTED, APPROVAL_REJECTED, \
    APPROVAL_PENDING, APPROVAL_NONE
from engine import APPREQ_REQUIRED, APPREQ_OPTIONAL


MAGICASK = u'UNLIKELYVALUEMAGICASPICONSTANTS3141592926ISUSEDTONOTIFYTHEVALUEMUSTBEASKED'
GENERICLOGIN = u'UNLIKELYVALUEWORKSASGENERICLOGIN'
def mundane(value):
    if value == MAGICASK:
        return u'Unknown'
    return value

DEBUG = False
def mdecode(item):
    if not item:
        return ""
    try:
        item = item.decode('utf8')
    except:
        pass
    return item

def truncat_string(item, maxsize=20):
    return (item[:maxsize] + '..') if len(item) > maxsize else item

class AuthentifierSocketClosed(Exception):
    pass

class TargetContext(object):
    def __init__(self, host=None, dnsname=None, login=None, show=None):
        self.host = host
        self.dnsname = dnsname
        self.login = login
        self.show = show
    def showname(self):
        return self.show or self.dnsname or self.host


################################################################################
class Sesman():
################################################################################

    # __INIT__
    #===============================================================================
    def __init__(self, conn, addr):
    #===============================================================================
        if SESMANCONF[u'sesman'][u'DEBUG'].lower() == u'true':
            global DEBUG
            DEBUG = True
        self.cn = u'Unknown'

        self.proxy_conx  = conn
        self.addr        = addr
        self.full_path   = None
        self._license_ok = None

        self.engine = engine.Engine()

        self.effective_login = None

        # shared should be read from sesman but never written except when sending
        self.shared                    = {}

        self._full_user_device_account = u'Unknown'
        self.target_service_name = None
        self.target_context = None

        self.shared[u'module']                  = u'login'
        self.shared[u'selector_group_filter']   = u''
        self.shared[u'selector_device_filter']  = u''
        self.shared[u'selector_proto_filter']   = u''
        self.shared[u'selector']                = u'False'
        self.shared[u'selector_current_page']   = u'1'
        self.shared[u'selector_lines_per_page'] = u'0'
        self.shared[u'real_target_device']      = MAGICASK
        self.shared[u'reporting']               = u''

        self._enable_encryption = self.engine.get_trace_encryption()
        self.language           = None
        self.pid = os.getpid()

        self.shared[u'target_login']    = MAGICASK
        self.shared[u'target_device']   = MAGICASK
        self.shared[u'target_host']     = MAGICASK
        self.shared[u'login']           = MAGICASK
        self.shared[u'ip_client']       = MAGICASK
        self.shared[u'target_protocol'] = MAGICASK
        self.shared[u'keyboard_layout'] = MAGICASK

        self.shared[u'auth_channel_answer'] = u''
        self.shared[u'auth_channel_result'] = u''
        self.shared[u'auth_channel_target'] = u''

        self.internal_target = False
        self.check_session_parameters = False

        # Passthrough context
        self.passthrough_mode = (SESMANCONF[u'sesman'][u'auth_mode_passthrough'].lower()
                                 == u'true')
        self.default_login = SESMANCONF[u'sesman'][u'default_login'].strip() if (
            SESMANCONF[u'sesman'][u'use_default_login'].strip() == u'2') else None
        self.passthrough_target_login = None

    def reset_session_var(self):
        self._full_user_device_account = u'Unknown'
        self.target_service_name = None
        self.internal_target = False
        self.passthrough_target_login = None
        self.target_context = None

    def set_language_from_keylayout(self):
        self.language = SESMANCONF.language
        french_layouts = [0x0000040C, # French (France)
                          0x00000C0C, # French (Canada) Canadian French (Legacy)
                          0x0000080C, # French (Belgium)
                          0x0001080C, # French (Belgium) Belgian (Comma)
                          0x0000100C] # French (Switzerland)
        keylayout = 0
        if self.shared.get(u'keyboard_layout') != MAGICASK:
            try:
                keylayout = int(self.shared.get(u'keyboard_layout'))
            except:
                pass
        if keylayout in french_layouts:
            self.language = 'fr'

    #TODO: is may be possible to delay sending data until the next input through receive_data
    def send_data(self, data):
        u""" NB : Strings sent to the ReDemPtion proxy MUST be UTF-8 encoded """

        if DEBUG:
            import pprint
            Logger().info(u'================> send_data (update)=%s' % (pprint.pformat(data)))


        #if current language changed, send translations
        if self.language != SESMANCONF.language:
            if not self.language:
                self.set_language_from_keylayout()
            SESMANCONF.language = self.language

            data[u'language'] = SESMANCONF.language
            # if self.shared.get(u'password') == MAGICASK:
            #     data[u'password'] = u''

            data.update(translations())

        # replace MAGICASK with ASK and send data on the wire
        _list = []
        for key, value in data.iteritems():
            self.shared[key] = value
            if value != MAGICASK:
                _pair =  u"%s\n%s\n" % (key, (u"!%s" % value))
            else:
                _pair = u"%s\nASK\n" % key
            _list.append(_pair)

        if DEBUG:
           import pprint
           Logger().info(u'send_data (on the wire)=%s' % (pprint.pformat(_list)))

        _r_data = u"".join(_list)
        _r_data = _r_data.encode('utf-8')
        _len = len(_r_data)

        self.proxy_conx.sendall(pack(">L", _len))
        self.proxy_conx.sendall(_r_data)

    def receive_data(self):
        u""" NB : Strings coming from the ReDemPtion proxy are UTF-8 encoded """

        _status, _error = True, u''
        _data = {}
        try:
            # Fetch Data from Redemption
            try:
                _packet_size, = unpack(">L", self.proxy_conx.recv(4))
                _data = self.proxy_conx.recv(_packet_size)
            except Exception, e:
                if DEBUG:
                    import traceback
                    Logger().info(u"Socket Closed : %s" % traceback.format_exc(e))
                raise AuthentifierSocketClosed()
            _data = _data.decode('utf-8')
        except AuthentifierSocketClosed, e:
            raise
        except Exception, e:
#            import traceback
#            Logger().info("%s <<<%s>>>" % (u"Failed to read data from rdpproxy authentifier socket", traceback.format_exc(e)))
            raise AuthentifierSocketClosed()

        if _status:
            _elem = _data.split('\n')

            if len(_elem) & 1 == 0:
                Logger().info(u"Odd number of items in authentication protocol")
                _status = False

        if _status:
            try:
                _data = dict(zip(_elem[0::2], _elem[1::2]))
            except Exception, e:
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

    def parse_username(self, wab_login, target_login, target_device, target_service):
        effective_login = None
        level_0_items = wab_login.split(u':')
        if len(level_0_items) > 1:
            if len(level_0_items) > 3:
                Logger().info(u"username parse error %s" % wab_login)
                return False, (TR(u'Username_parse_error %s') % wab_login), wab_login, target_login, target_device, target_service, effective_login
            target_service = u'' if len(level_0_items) <= 2 else level_0_items[-2]
            level_1_items, wab_login = level_0_items[0].split(u'@'), level_0_items[-1]
            target_login, target_device = '@'.join(level_1_items[:-1]), level_1_items[-1]
        if self.passthrough_mode:
            if not self.passthrough_target_login:
                self.passthrough_target_login = wab_login
            if self.default_login:
                effective_login = self.passthrough_target_login
                wab_login = self.default_login
            Logger().info(u'ip_target="%s" real_target_device="%s"' % (
                self.shared.get(u'ip_target'), self.shared.get(u'real_target_device')))
        return True, "", wab_login, target_login, target_device, target_service, effective_login

    def interactive_ask_x509_connection(self):
        """ Send a message to the proxy to prompt the user to validate x509 in his browser
            Wait until the user clicks Ok in Proxy prompt or until timeout
        """
        _status = False
        data_to_send = ({ u'message' : TR(u'valid_authorisation')
                       , u'password': u'x509'
                       , u'module' : u'confirm'
                       , u'display_message': MAGICASK
                       # , u'accept_message': u''
                      })

        self.send_data(data_to_send)

        # Wait for the user to click Ok in proxy

        while self.shared.get(u'display_message') == MAGICASK:
            Logger().info(u'wait user grant or reject connection')
            _status, _error = self.receive_data()
            if not _status:
                break

            Logger().info(u'Data received')
            if self.shared.get(u'display_message').lower() != u'true':
                _status = False

        return _status

    def interactive_display_message(self, data_to_send):
        u""" NB : Strings sent to the ReDemPtion proxy MUST be UTF-8 encoded """
        #TODO: we should not have to care about target login or device to display messages
        # we should be able to send messages before or after defining target seamlessly
        data_to_send.update({ u'module'        : u'confirm'
                            # , u'proto_dest'    : u'INTERNAL'
                            # , u'display_message': MAGICASK
                            # , u'accept_message': u''
                            })

        self.send_data(data_to_send)
        _status, _error = self.receive_data()

        if self.shared.get(u'display_message') != u'True':
            _status, _error = False, TR(u'not_display_message')

        return _status, _error

    def interactive_accept_message(self, data_to_send):
        data_to_send.update({ u'module'        : u'valid'
                            # , u'proto_dest'    : u'INTERNAL'
                            # , u'accept_message': MAGICASK
                            # , u'display_message': u''
                            })
        self.send_data(data_to_send)

        _status, _error = self.receive_data()
        if self.shared.get(u'accept_message') != u'True':
            _status, _error = False, TR(u'not_accept_message')

        return _status, _error

    def interactive_target(self, data_to_send):
        data_to_send.update({ u'module' : u'interactive_target' })
        self.send_data(data_to_send)
        _status, _error = self.receive_data()
        if self.shared.get(u'display_message') != u'True':
            _status, _error = False, TR(u'Connection closed by client')
        return _status, _error


    def complete_target_info(self, kv):
        keylist = [ u'target_password', u'target_login', u'target_host' ]
        extkv = dict((x, kv.get(x)) for x in keylist if kv.get(x) is not None)
        tries = 3
        _status, _error = None, None
        while (tries > 0) and (_status is None) :
            tries -= 1
            interactive_data = {}
            if not extkv[u'target_password']:
                interactive_data[u'target_password'] = MAGICASK
            if (extkv.get(u'target_login') == GENERICLOGIN or
                not extkv.get(u'target_login')):
                interactive_data[u'target_password'] = MAGICASK
                interactive_data[u'target_login'] = MAGICASK
            target_subnet = None
            if '/' in extkv.get(u'target_host'): # target_host is a subnet
                target_subnet = extkv.get(u'target_host')
                interactive_data[u'target_host'] = MAGICASK
                if _error:
                    host_note = TR("error %s") % _error
                else:
                    host_note = TR(u"in_subnet %s") % target_subnet
                interactive_data[u'target_device'] = host_note
            if interactive_data:
                Logger().info(u"Interactive Target Info asking")
                if not target_subnet:
                    interactive_data[u'target_host'] = extkv.get(u'target_host')
                    interactive_data[u'target_device'] = kv.get(u'target_device') if self.target_context else self.shared.get(u'target_device')
                if not interactive_data.get(u'target_password'):
                    interactive_data[u'target_password'] = ''
                if not interactive_data.get(u'target_login'):
                    interactive_data[u'target_login'] = extkv.get(u'target_login')
                _status, _error = self.interactive_target(interactive_data)
                if _status:
                    if interactive_data.get(u'target_password') == MAGICASK:
                        extkv[u'target_password'] = self.shared.get(u'target_password')
                    if interactive_data.get(u'target_login') == MAGICASK:
                        extkv[u'target_login'] = self.shared.get(u'target_login')
                    if interactive_data.get(u'target_host') == MAGICASK:
                        if self.check_hostname_in_subnet(self.shared.get(u'target_host'),
                                                         target_subnet):
                            extkv[u'target_host'] = self.shared.get(u'target_host')
                            extkv[u'target_device'] = self.shared.get(u'target_host')
                        else:
                            extkv[u'target_host'] = target_subnet
                            _status = None
                            _error = TR("no_match_subnet %s %s") % (
                                truncat_string(self.shared.get(u'target_host')),
                                target_subnet)
            else:
                _status, _error = True, "OK"
        return extkv, _status, _error


    def interactive_close(self, target, message):
        data_to_send = { u'error_message'  : message
                       , u'trans_ok'       : u'OK'
                       , u'module'         : u'close'
                       # , u'proto_dest'     : u'INTERNAL'
                       , u'target_device'  : u'close:%s' % target
                       , u'target_login'   : self.shared.get(u'target_login')
                       , u'target_password': u'Default'
                       }

        # If we send close we should expect authentifier socket will be closed by the other end
        # No need to return some warning message if that happen
        self.send_data(data_to_send)
        _status, _error = self.receive_data()

        return _status, _error


    def authentify(self):
        """ Authentify the user through password engine and then retreive his rights
             The user preferred language will be set as the language to use in
             interactive messages
        """
        _status, _error = self.receive_data()
        if not _status:
            return False, _error

        if self.shared.get(u'login') == MAGICASK:
            return None, TR(u"Empty user, try again")

        (_status, _error,
         wab_login, target_login, target_device,
         self.target_service_name, self.effective_login) = self.parse_username(
            self.shared.get(u'login'),
            self.shared.get(u'target_login'),
            self.shared.get(u'target_device'),
            self.target_service_name)
        if not _status:
            return None, TR(u"Invalid user, try again")

        Logger().info(u"Continue with authentication (%s) -> %s" % (self.shared.get(u'login'), wab_login))

        try:
            target_info = None
            if (target_login and target_device and
                not target_login == MAGICASK and
                not target_device == MAGICASK):
                if (self.target_service_name and
                    not self.target_service_name == MAGICASK):
                    target_info = u"%s@%s:%s" % (target_login, target_device, self.target_service_name)
                else:
                    target_info = u"%s@%s" % (target_login, target_device)
            try:
                target_info = target_info.encode('utf8')
            except Exception, e:
                target_info = None
            #Check if X509 Authentication is active
            if self.engine.is_x509_connected(
                        wab_login,
                        self.shared.get(u'ip_client'),
                        u"RDP",
                        target_info,
                        self.shared.get(u'ip_target')):
                # Prompt the user in proxy window
                # Wait for confirmation from GUI (or timeout)
                if not (self.interactive_ask_x509_connection() and self.engine.x509_authenticate()):
                    return False, TR(u"x509 browser authentication not validated by user")
            elif self.passthrough_mode:
                # Passthrough Authentification
                if not self.engine.passthrough_authenticate(
                        wab_login,
                        self.shared.get(u'ip_client'),
                        self.shared.get(u'ip_target')):
                    return False, TR(u"passthrough_auth_failed_wab %s") % wab_login
            else:
                # PASSWORD based Authentication
                if ((self.shared.get(u'password') == MAGICASK
                     and not wab_login.startswith('_OTP_'))  # one-time pwd
                    or not self.engine.password_authenticate(
                        wab_login,
                        self.shared.get(u'ip_client'),
                        self.shared.get(u'password'),
                        self.shared.get(u'ip_target'))):
                    if self.shared.get(u'password') == MAGICASK:
                        self.engine.challenge = None
                    return None, TR(u"auth_failed_wab %s") % wab_login

            # At this point, User is authentified.
            if wab_login.startswith('_OTP_'):
                real_wab_login = self.engine.get_username()
                self.shared[u'login'] = self.shared.get(u'login').replace(wab_login,
                                                                          real_wab_login)
            self.language = self.engine.get_language()
            if self.engine.get_force_change_password():
                self.send_data({u'rejected': TR(u'changepassword')})
                return False, TR(u'changepassword')

            Logger().info(u'lang=%s' % self.language)

            # TODO: Should be done by authentication methods
            # When user is authentified check if licence tokens are available
            Logger().info(u"Checking licence")
            if not self.engine.get_license_status():
                return False, TR(u'licence_blocker')

        except Exception, e:
            if DEBUG:
                import traceback
                Logger().info("<<<%s>>>" % traceback.format_exc(e))
            _status, _error = None, TR(u'auth_failed_wab %s') % wab_login

        return _status, _error

    def resolve_target_host(self, target_device, target_login):
        """ Resolve the right target host to use
        self.target_context.host will contains the target host.
        self.target_context.showname() will contains the target_device to show
        self.target_context.login will contains the target_login if not in
            passthrough mode.

        Returns None if target_device is a hostname,
                target_device in other cases
        """
        if self.shared.get(u'real_target_device'):
            # Transparent proxy
            if not self.target_context:
                self.target_context = TargetContext(host=self.shared.get(u'real_target_device'))
        elif target_device and target_device != MAGICASK:
            # This allow proxy to check if target_device is a device_name
            # or a hostname.
            # In case it is a hostname, we keep the target_login as a filter.
            valid = self.engine.valid_device_name([u'RDP', u'VNC'], target_device)
            Logger().info("Check Valid device '%s' : res = %s" %
                          (target_device, valid))
            if not valid:
                # target_device might be a hostname
                try:
                    login_filter = None
                    if (target_login and target_login != MAGICASK
                        and not self.passthrough_mode):
                        login_filter = target_login
                    dnsname, alias, ip_list = socket.gethostbyaddr(target_device)
                    Logger().info("Resolve DNS Hostname %s -> %s" % (target_device,
                                                                     ip_list))
                    host_ip = ip_list[0] if ip_list else None
                    self.target_context = TargetContext(host=host_ip,
                                                        dnsname=dnsname,
                                                        login=login_filter,
                                                        show=target_device)
                    return None
                except socket.error:
                    Logger().info("target_device is not a hostname")
        return target_device


    # GET SERVICE
    #===============================================================================
    def get_service(self):
    #===============================================================================
        u""" Send service pages to proxy until the selected service is returned.
        """

        Logger().info(u"get_service")

        (_status, _error,
         wab_login, target_login, target_device,
         self.target_service_name, self.effective_login) = self.parse_username(
            self.shared.get(u'login'),
            self.shared.get(u'target_login'),
            self.shared.get(u'target_device'),
            self.target_service_name
            )

        if not _status:
            Logger().info(u"Invalid user %s, try again" % self.shared.get(u'login'))
            return None, TR(u"Invalid user, try again")
        _status, _error = None, TR(u"No error")

        target_device = self.resolve_target_host(target_device, target_login)

        while _status is None:
            if (target_device and target_device != MAGICASK
                and (target_login or self.passthrough_mode)
                and target_login != MAGICASK):
                # Target is provided at login
                self._full_user_device_account = u"%s@%s:%s" % ( target_login
                                                               , target_device
                                                               , wab_login
                                                               )
                data_to_send = { u'login'                   : wab_login
                               , u'target_login'            : target_login
                               , u'target_device'           : target_device
                               , u'module'                  : u'transitory'
                               }
                if not self.internal_target:
                    self.internal_target = True if self.target_service_name == u'INTERNAL' else False
                self.send_data(data_to_send)
                _status = True
            elif self.shared.get(u'selector') == MAGICASK:
                # filters ("Group" and "Account/Device") entered by user in selector are applied to raw services list
                self.engine.get_proxy_rights([u'RDP', u'VNC'],
                                             check_timeframes=False)
                services, item_filtered = self.engine.get_targets_list(
                    group_filter = self.shared.get(u'selector_group_filter'),
                    device_filter = self.shared.get(u'selector_device_filter'),
                    protocol_filter = self.shared.get(u'selector_proto_filter'),
                    target_context = self.target_context)
                if (len(services) > 1) or item_filtered:
                    try:
                        _current_page = int(self.shared.get(u'selector_current_page')) - 1
                        _lines_per_page = int(self.shared.get(u'selector_lines_per_page'))

                        if not _lines_per_page:
                            target_login = u""
                            target_device = u""
                            proto_dest = u""

                            data_to_send = { u'login'                   : wab_login
                                           , u'target_login'            : target_login
                                           , u'target_device'           : target_device
                                           , u'proto_dest'              : proto_dest
                                           # , u'selector'                : u"True"
                                           , u'ip_client'               : self.shared.get(u'ip_client')
                                           , u'selector_number_of_pages': u"0"
                                           # No lines sent, reset filters
                                           , u'selector_group_filter'   : u""
                                           , u'selector_device_filter'  : u""
                                           , u'selector_proto_filter'   : u""
                                           , u'module'                  : u'selector'
                                           }

                        else:
                            _number_of_pages = 1 + (len(services)-1) / _lines_per_page
                            if _current_page >= _number_of_pages:
                                _current_page = _number_of_pages - 1
                            if _current_page < 0:
                                _current_page = 0
                            _start_of_page = _current_page * _lines_per_page
                            _end_of_page = _start_of_page + _lines_per_page

                            services = sorted(services, key=lambda x: x[1])[_start_of_page:_end_of_page]

                            all_target_login  = [s[0] for s in services]
                            all_target_device = [s[1] for s in services]
                            all_proto_dest    = [s[2] for s in services]
                            all_end_time      = ["-"  for s in services]

                            target_login = u"\x01".join(all_target_login)
                            target_device = u"\x01".join(all_target_device)
                            proto_dest = u"\x01".join(all_proto_dest)

                            data_to_send = { u'login'                   : wab_login
                                           , u'target_login'            : target_login
                                           , u'target_device'           : target_device
                                           , u'proto_dest'              : proto_dest
                                           , u'end_time'                : u";".join(all_end_time)
                                           # , u'selector'                : u'True'
                                           , u'ip_client'               : self.shared.get(u'ip_client')
                                           , u'selector_number_of_pages': "%s" % max(_number_of_pages, _current_page + 1)
                                           , u'selector_current_page'   : "%s" % (_current_page + 1)
                                           , u'selector_group_filter'   : self.shared.get(u'selector_group_filter')
                                           , u'selector_device_filter'  : self.shared.get(u'selector_device_filter')
                                           , u'selector_proto_filter'   : self.shared.get(u'selector_proto_filter')
                                           , u'module'                  : u'selector'
                                           }

                        self.send_data(data_to_send)

                        _status, _error = self.receive_data()

                        if self.shared.get(u'login') == MAGICASK:
                            self.send_data({
                                  u'login': MAGICASK
                                , u'selector_lines_per_page' : u'0'
                                , u'module'                  : u'login'})
                            Logger().info(u"Logout")
                            return None, u"Logout"

                        target_login = MAGICASK
                        target_device = MAGICASK
                        # proto_dest = MAGICASK
                        (_status, _error,
                         wab_login, target_login, target_device,
                         self.target_service_name,
                         self.effective_login) = self.parse_username(
                            self.shared.get(u'login'), target_login, target_device,
                            self.target_service_name)
                        if not _status:
                            Logger().info(u"Invalid user %s, try again" % self.shared.get(u'login'))
                            return None, TR(u"Invalid user, try again")

                        _status = None # One more loop
                    except Exception, e:
                        if DEBUG:
                            import traceback
                            Logger().info(u"Unexpected error in selector pagination %s" % traceback.format_exc(e))
                        return False, u"Unexpected error in selector pagination"
                elif len(services) == 1:
                    Logger().info(u"service len = 1")
                    s = services[0]
                    data_to_send = {}
                    data_to_send[u'login'] = wab_login
                    data_to_send[u'module'] = u'transitory' if s[2] != u'INTERNAL' else u'INTERNAL'
                    # service_login (s[1]) format:
                    # target_login@device_name:service_name
                    # target_login can contains '@'
                    # device_name and service_name can not contain ':', nor '@'

                    # target_split = [ *target_login* , device_name:service_name ]
                    target_split = s[1].split('@')
                    target_login = '@'.join(target_split[:-1])
                    # device_service_split = [ device_name, service_name ]
                    device_service_split = target_split[-1].split(':')
                    device_name = device_service_split[0]
                    service_name = device_service_split[-1]

                    data_to_send[u'target_login'] = target_login
                    data_to_send[u'target_device'] = device_name
                    self._full_user_device_account = u"%s@%s:%s" % (target_login,
                                                                    device_name,
                                                                    wab_login)
                    if not self.internal_target:
                        self.internal_target = True if s[2] == u'INTERNAL' else False
                    self.send_data(data_to_send)
                    self.target_service_name = service_name
                    # Logger().info("Only one target : service name %s" % self.target_service_name)
                    _status = True
                else:
                    _status, _error = False, TR(u"Target unreachable")

            else:
                self.send_data({u'login': MAGICASK,
                                u'module': 'login'
                                })
                return None, u"Logout"

        return _status, _error
    # END METHOD - GET_SERVICE

    def check_password_expiration_date(self):
        _status, _error = True, u''
        try:
            notify, days = self.engine.password_expiration_date()
            if notify:
                if days == 0:
                    message = TR(u'Your password will expire soon. Please change it.')
                else:
                    message = TR(u'Your password will expire in %s days. Please change it.') % days
                _status, _error = self.interactive_display_message({u'message': message})
        except Exception, e:
            if DEBUG:
                import traceback
                Logger().info("<<<<%s>>>>" % traceback.format_exc(e))
        return _status, _error


    def check_video_recording(self, isRecorded, user):
        Logger().info(u"Checking video")

        _status, _error = True, u''
        data_to_send = {
              u'is_rec'         : u'False'
            , u'rec_path'       : u""
            , u'file_encryption': u"False"
        }

        try:
            self.full_path = u""
            video_path = u""
            if isRecorded:
                try:
                    os.stat(RECORD_PATH)
                except OSError:
                    try:
                        os.mkdir(RECORD_PATH)
                    except Exception:
                        Logger().info(u"Failed creating recording path (%s)" % RECORD_PATH)
                        _status, _error = False, TR(u'error_creating_record_path %s') % RECORD_PATH
                if _status:
                    # Naming convention : {username}@{userip},{account}@{devicename},YYYYMMDD-HHMMSS,{wabhostname},{uid}
                    # NB :  backslashes are replaced by pipes for IE compatibility
                    random.seed(self.pid)

                    #keeping code synchronized with wabengine/src/common/data.py
                    video_path =  u"%s@%s," % (user, self.shared.get(u'ip_client'))
                    video_path += u"%s@%s," % (self.shared.get(u'target_login'), self.shared.get(u'target_device'))
                    video_path += u"%s," % (strftime("%Y%m%d-%H%M%S"))
                    video_path += u"%s," % gethostname()
                    video_path += u"%s" % random.randint(1000, 9999)
                    # remove all "dangerous" characters in filename
                    import re
                    video_path = re.sub(r'[^-A-Za-z0-9_@,.]', u"", video_path)

                    Logger().info(u"Session will be recorded in %s" % video_path)

                    self.full_path = RECORD_PATH + video_path
                    data_to_send[u'is_rec'] = True
                    data_to_send[u"file_encryption"] = u'True' if self._enable_encryption else u'False'
                    #TODO remove .flv extention and adapt ReDemPtion proxy code
                    data_to_send[u'rec_path'] = u"%s.flv" % (self.full_path)

                    record_warning = SESMANCONF[u'sesman'][u'record_warning'].lower()
                    if record_warning != 'false':
                        message =  u"Warning! Your remote session may be recorded and kept in electronic format."
                        try:
                            with open('/opt/wab/share/proxys/messages/motd.%s' % self.language) as f:
                                message = f.read().decode('utf-8')
                        except Exception, e:
                            pass
                        data_to_send[u'message'] = cut_message(message)

                        _status, _error = self.interactive_accept_message(data_to_send)
                        Logger().info(u"Session interactive")
                    else:
                        self.send_data(data_to_send)

        except Exception, e:
            if DEBUG:
                import traceback
                Logger().info("<<<<%s>>>>" % traceback.format_exc(e))
            _status, _error = False, TR(u"Connection closed by client")

        return _status, _error

    def select_target(self):
        ###################
        ### FIND_TARGET ###
        ###################
        """ The purpose of the snippet below is electing the first right that match
        the login AND device AND service that have been passed in the connection
        string.
        If service is blank take the first right that match login AND device
        (may happen with a command line or a mstsc '.rdp' file connections ;
        never happens if the selector is used).
        NB : service names are supposed to be in alphabetical ascending order.
        """
        selected_target = None
        target_device = self.shared.get(u'target_device')
        target_login = self.shared.get(u'target_login')
        target_service = self.target_service_name if self.target_service_name != u'INTERNAL' else u'RDP'

        # Logger().info("selected target ==> %s %s %s" % (target_login, target_device, target_service))
        selected_target = self.engine.get_selected_target(target_login,
                                                          target_device,
                                                          target_service)
        if not selected_target:
            _target = u"%s@%s:%s" % ( target_login, target_device, target_service )
            _error_log = u"Targets %s not found in user rights" % _target
            _status, _error = False, TR(u"Target %s not found in user rights") % _target
            Logger().info("%s" % _error)
            return None, _status, _error
        return selected_target, True, ""

    def check_target(self, selected_target):
        """ Checking selected target validity
        """
        ticket = None
        status = None
        info_message = None
        got_signal = False
        while True:
            Logger().info(u"Begin check_target ticket = %s..." % ticket)
            previous_status = status
            previous_info_message = info_message
            status, infos = self.engine.check_target(selected_target, self.pid, ticket)
            ticket = None
            info_message = infos.get('message')
            refresh_page = (got_signal
                            or (status != previous_status)
                            or (previous_info_message != info_message))
            Logger().info(u"End check_target ... refresh : %s" % refresh_page)
            if refresh_page:
                self.send_data({u'forcemodule' : True})
            if status == APPROVAL_ACCEPTED:
                return True, ""
            if refresh_page:
                self.interactive_display_waitinfo(status, infos)
            got_signal = False
            r = []
            try:
                Logger().info(u"Start Select ...")
                r, w, x = select([self.proxy_conx], [], [], 10)
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
                if self.shared.get(u'waitinforeturn') == "backselector":
                    # received back to selector
                    self.send_data({u'module' : u'selector', u'target_login': '',
                                    u'target_device' : ''})
                    return None, ""
                if self.shared.get(u'waitinforeturn') == "exit":
                    # received exit
                    self.send_data({u'module' : u'close'})
                    False, ""
                if self.shared.get(u'waitinforeturn') == "confirm":
                    # should parse the ticket info
                    desc = self.shared.get(u'comment')
                    ticketno = self.shared.get(u'ticket')
                    duration = self.parse_duration(self.shared.get(u'duration'))
                    ticket = { u"description": desc if desc else None,
                               u"ticket": ticketno if ticketno else None,
                               u"duration": duration}
        return False, ""

    def parse_duration(self, duration):
        if duration:
            try:
                import re
                mpat = re.compile("(\d+)m")
                hpat = re.compile("(\d+)h")
                hres = hpat.search(duration)
                mres = mpat.search(duration)
                duration = 0
                if mres:
                    duration += 60*int(mres.group(1))
                if hres:
                    duration += 60*60*int(hres.group(1))
                if duration == 0:
                    duration = None
            except Exception, e:
                duration = None
        return duration

    def interactive_display_waitinfo(self, status, infos):
        tosend = { u'module' : u'waitinfo',
                   u'message' : cut_message(infos.get('message')),
                   u'display_message' : MAGICASK,
                   u'waitinforeturn' : MAGICASK
                   }
        ticketfields = infos.get("ticket_fields")
        flag = 0
        if ticketfields:
            if ticketfields.get("description") == APPREQ_REQUIRED:
                flag += 1
            if ticketfields.get("ticket") == APPREQ_REQUIRED:
                flag += 2
            if ticketfields.get("duration") == APPREQ_REQUIRED:
                flag += 4
        if status == APPROVAL_NONE:
            tosend["showform"] = True
            tosend["formflag"] = flag
        else:
            tosend["showform"] = False
        self.send_data(tosend)

    def start(self):
        _status, tries = None, 5
        while _status is None and tries > 0:
            self.reset_session_var()

            ##################
            ### AUTHENTIFY ###
            ##################
            # [ LOGIN ]
            _status, _error = self.authentify()

            if _status is None and self.engine.challenge:
                # submit challenge:
                data_to_send = { u'authentication_challenge' : self.engine.challenge.promptEcho
                               , u'message' : cut_message(self.engine.challenge.message)
                               , u'module' : u'challenge'
                                 }
                self.send_data(data_to_send)
                continue

            tries = tries - 1
            if _status is None and tries > 0:
                Logger().info(
                    u"Wab user '%s' authentication from %s failed [%u tries remains]"  %
                    (mundane(self.shared.get(u'login')) , mundane(self.shared.get(u'ip_client')), tries)
                )

                (current_status, current_error,
                 current_wab_login, current_target_login, current_target_device,
                 self.target_service_name, self.effective_login) = self.parse_username(
                    self.shared.get(u'login'),
                    self.shared.get(u'target_login'),
                    self.shared.get(u'target_device'),
                    self.target_service_name
                    )

                data_to_send = { u'login': self.shared.get(u'login') if not current_wab_login.startswith('_OTP_') else MAGICASK
                               , u'password': MAGICASK
                               , u'module' : u'login'}
                self.send_data(data_to_send)
                continue

            if _status:
                tries = 5
                Logger().info(u"Wab user '%s' authentication succeeded" % mundane(self.shared.get(u'login')))

                # Warn password will expire soon for user
                _status, _error = self.check_password_expiration_date()

                # Get services for identified user
                _status = None
                while _status is None:
                    # [ SELECTOR ]
                    _status, _error = self.get_service()
                    Logger().info("get service end :%s" % _status)
                    if not _status:
                        # logout or error in selector
                        self.engine.reset_proxy_rights()
                        break
                    selected_target, _status, _error = self.select_target()
                    Logger().info("select_target end :%s" % _status)
                    if not _status:
                        # target not available
                        self.engine.reset_proxy_rights()
                        break
                    # [ WAIT INFO ]
                    _status, _error = self.check_target(selected_target)
                    Logger().info("check_target end :%s" % _status)

        if tries <= 0:
            Logger().info(u"Too many login failures")
            _status, _error = False, TR(u"Too many login failures or selector orders, closing")
            self.engine.NotifyPrimaryConnectionFailed(self.shared.get(u'login'),
                                                      self.shared.get(u'ip_client'))

        if _status:
            Logger().info(u"Asking service %s@%s" % (self.shared.get(u'target_login'), self.shared.get(u'target_device')))

        #TODO: looks like the code below should be done in the instance of some "selected_target" class
        if _status:
            #####################
            ### START_SESSION ###
            #####################
            session_started = False
            extra_info = self.engine.get_target_extra_info()
            _status, _error = self.check_video_recording(
                extra_info.is_recorded,
                mdecode(self.engine.get_username()) if self.engine.get_username() else self.shared.get(u'login'))

            Logger().info(u"Fetching protocol")

            kv = {}

            target_login_info = self.engine.get_target_login_info(selected_target)
            proto_info = self.engine.get_target_protocols(selected_target)
            kv[u'proto_dest'] = proto_info.protocol
            kv[u'target_port'] = target_login_info.service_port
            kv[u'timezone'] = str(altzone if daylight else timezone)

            if _status:
                kv['password'] = 'pass'

                # register signal
                signal.signal(signal.SIGUSR1, self.kill_handler)
                signal.signal(signal.SIGUSR2, self.check_handler)

                Logger().info(u"Starting Session, effective login='%s'" % self.effective_login)
                # Add connection to the observer
                kv[u'session_id'] = self.engine.start_session(selected_target, self.pid,
                                                              self.effective_login)
                _status, _error = self.engine.write_trace(self.full_path)
                _error = TR(_error)
                if not _status:
                    _error = _error % self.full_path
                pattern_kill, pattern_notify = self.engine.get_restrictions(selected_target, "RDP")
                if pattern_kill:
                    self.send_data({ u'module' : u'transitory', u'pattern_kill': pattern_kill })
                if pattern_notify:
                    self.send_data({ u'module' : u'transitory', u'pattern_notify': pattern_notify })

            if _status:
                Logger().info(u"Checking timeframe")
                self.infinite_connection = False
                deconnection_time = self.engine.get_deconnection_time(selected_target)
                if not deconnection_time:
                    Logger().error("No timeframe available, Timeframe has not been checked !")
                    _status = False
                if (deconnection_time == u"-"
                    or deconnection_time[0:4] >= u"2034"):
                    deconnection_time = u"2034-12-31 23:59:59"
                    self.infinite_connection = True

                now = datetime.strftime(datetime.now(), "%Y-%m-%d %H:%M:%S")
                if (deconnection_time == u'-'
                    or now < deconnection_time):
                    # deconnection time to epoch
                    tt = datetime.strptime(deconnection_time, "%Y-%m-%d %H:%M:%S").timetuple()
                    kv[u'timeclose'] = int(mktime(tt))
                    if not self.infinite_connection:
                        _status, _error = self.interactive_display_message(
                                {u'message': TR(u'session_closed_at %s') % deconnection_time}
                                )

            module = kv.get(u'proto_dest')
            if not module in [ u'RDP', u'VNC', u'INTERNAL' ]:
                module = u'RDP'
            if self.internal_target:
                module = u'INTERNAL'
            kv[u'module'] = module
            proto = u'RDP' if  kv.get(u'proto_dest') != u'VNC' else u'VNC'
            kv[u'device_redirection'] = SESMANCONF[proto][u'device_redirection']
            kv[u'clipboard'] = SESMANCONF[proto][u'clipboard']
            kv[u'mode_console'] = u"allow"

            self.reporting_reason  = None
            self.reporting_target  = None
            self.reporting_message = None

            try_next = False

            for physical_target in self.engine.get_effective_target(selected_target):
                physical_info = self.engine.get_physical_target_info(physical_target)
                if not _status:
                    physical_target = None
                    break

                kv[u'disable_tsk_switch_shortcuts'] = u'no'
                application = self.engine.get_application(selected_target)
                if application:
                    app_params = self.engine.get_app_params(selected_target, physical_target)
                    if not app_params:
                        continue
                    kv[u'alternate_shell'] = (u"%s %s" % (app_params.program, app_params.params))
                    kv[u'shell_working_directory'] = app_params.workingdir

                    kv[u'target_application'] = "%s@%s" % \
                        (target_login_info.account_login,
                         target_login_info.target_name)
                    # kv[u'target_application'] = selected_target.service_login
                    kv[u'disable_tsk_switch_shortcuts'] = u'yes'
                self.cn = target_login_info.target_name

                if self.target_context:
                    kv[u'target_host'] = self.target_context.host
                    kv[u'target_device'] = self.target_context.showname()
                else:
                    kv[u'target_host'] = physical_info.device_host
                    kv[u'target_port'] = physical_info.service_port

                if not self.passthrough_mode:
                    kv[u'target_login'] = physical_info.account_login

                release_reason = u''

                try:
                    Logger().info("auth_mode_passthrough=%s" % self.passthrough_mode)

                    if self.passthrough_mode:
                        kv[u'target_login'] = self.passthrough_target_login
                        if self.shared.get(u'password') == MAGICASK:
                            target_password = u''
                        else:
                            target_password = self.shared.get(u'password')
                        #Logger().info("auth_mode_passthrough target_password=%s" % target_password)
                        kv[u'password'] = u'password'
                    else:
                        target_password = self.engine.get_target_password(physical_target)

                    kv[u'target_password'] = target_password
                    extra_kv, _status, _error = self.complete_target_info(kv)
                    kv.update(extra_kv)

                    if self.target_context:
                        self._physical_target_device = self.target_context.host
                    else:
                        self._physical_target_device = physical_info.device_host

                    Logger().info(u"Send critic notification (every attempt to connect to some physical node)")
                    if extra_info.is_critical:
                        Logger().info("CRITICAL CONNECTION")
                        import socket
                        self.engine.NotifyConnectionToCriticalEquipment(
                            (u'APP' if application else proto_info.protocol),
                            self.shared.get(u'login'),
                            socket.getfqdn(self.shared.get(u'ip_client')),
                            self.shared.get(u'ip_client'),
                            self.shared.get(u'target_login'),
                            self.shared.get(u'target_host'),
                            self._physical_target_device,
                            ctime(),
                            None
                            )


                    self.engine.update_session(physical_target)

                    if not _status:
                        Logger().info( u"(%s):%s:REJECTED : User message: \"%s\""
                                       % ( mundane(self.shared.get(u'ip_client'))
                                         , mundane(self.shared.get(u'login'))
                                         , _error
                                         )
                                     )

                        kv = { u"login": u""
                             , u'password': u""
                             , u'target_login': u""
                             , u'target_password': u""
                             , u'target_device': u""
                             , u'target_host': u""
                             , u'rejected': _error
                             }

                    try_next = False

                    try:
                        ###########
                        # SEND KV #
                        ###########
                        self.send_data(kv)

                        Logger().info(u"Added connection to active WAB services")

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
                            if self.check_session_parameters:
                                self.update_session_parameters()
                                self.check_session_parameters = False
                            if self.proxy_conx in r:
                                _status, _error = self.receive_data();

                                if self.shared.get(u'keepalive') == MAGICASK:
                                    self.send_data({u'keepalive': u'True'})

                                if self.shared.get(u'reporting'):
                                    _reporting      = self.shared.get(u'reporting')
                                    _reporting_data = _reporting[_reporting.index(':') + 1:]

                                    _reporting_reason  = _reporting[:_reporting.index(':')]
                                    _reporting_target  = _reporting_data[:_reporting_data.index(':')]
                                    _reporting_message = _reporting_data[_reporting_data.index(':') + 1:]

                                    self.shared[u'reporting'] = u''

                                    Logger().info(u"Reporting: reason=\"%s\" target=\"%s\" message=\"%s\"" % (_reporting_reason, _reporting_target, _reporting_message))

                                    self.process_report(_reporting_reason, _reporting_target, _reporting_message)

                                    if _reporting_reason == u'CONNECTION_FAILED':
                                        self.reporting_reason  = _reporting_reason
                                        self.reporting_target  = _reporting_target
                                        self.reporting_message = _reporting_message

                                        try_next = True
                                        release_reason = u'Connexion failed'
                                        break

                                    elif _reporting_reason == u'FINDPATTERN_KILL':
                                        Logger().info(u"RDP connection terminated. Reason: Kill pattern detected")
                                        release_reason = u'Kill pattern detected'
                                        break

                                if self.shared.get(u'auth_channel_target'):
                                    Logger().info(u"Auth channel target=\"%s\"" % self.shared.get(u'auth_channel_target'))

                                    if self.shared.get(u'auth_channel_target') == u'GetWabSessionParameters':
                                        account_login = selected_target.account.login
                                        application_password = self.engine.get_target_password(selected_target)

                                        _message = { 'user' : account_login, 'password' : application_password }

                                        #Logger().info(u"GetWabSessionParameters (response):" % json.dumps(_message))
                                        self.send_data({u'auth_channel_answer': json.dumps(_message)})

                                        Logger().info(u"Sending of auth channel answer ok (GetWabSessionParameters)")

                                self.shared[u'auth_channel_target'] = u''
                            # r can be empty
                            else: # (if self.proxy_conx in r)
                                if not self.internal_target and not got_signal:
                                    Logger().info(u'Missing Keepalive')
                                    Logger().error(u'break connection')
                                    release_reason = u'Break connection'
                                    break
                        Logger().debug(u"End Of Keep Alive")


                    except AuthentifierSocketClosed, e:
                        if DEBUG:
                            import traceback
                            Logger().info(u"RDP/VNC connection terminated by client")
                            Logger().info("<<<<%s>>>>" % traceback.format_exc(e))
                        release_reason = u"RDP/VNC connection terminated by client"
                    except Exception, e:
                        if DEBUG:
                            import traceback
                            Logger().info(u"RDP/VNC connection terminated by client")
                            Logger().info("<<<<%s>>>>" % traceback.format_exc(e))
                        release_reason = u"RDP/VNC connection terminated by client: Exception"

                    if not try_next:
                        release_reason = u"RDP/VNC connection terminated by client"
                        break;
                finally:
                    if not (physical_target is None):
                        self.engine.release_target_password(physical_target, release_reason, selected_target)

            Logger().info(u"Stop session ...")

            # Notify WabEngine to stop connection if it has been launched successfully
            self.engine.stop_session(result=True, diag=u"success", title=u"End session")

            Logger().info(u"Stop session done.")

            # Error
            if try_next:
                _status, _error = self.interactive_close(self.reporting_target, self.reporting_message)

            try:
                Logger().info(u"Close connection ...")

                self.proxy_conx.close()

                Logger().info(u"Close connection done.")
            except IOError:
                if DEBUG:
                    Logger().info(u"Close connection: Exception")
                    Logger().info("<<<<%s>>>>" % traceback.format_exc(e))

    # END METHOD - START

    def process_report(self, reason, target, message):
        if   reason == u'CLOSE_SESSION_SUCCESSFUL':
            pass
        elif reason == u'CONNECTION_FAILED':
            self.engine.NotifySecondaryConnectionFailed(
                self.shared.get(u'login'),
                self.shared.get(u'ip_client'),
                self.shared.get(u'target_login'),
                self._physical_target_device)
        elif reason == u'CONNECTION_SUCCESSFUL':
            pass
        elif reason == u'OPEN_SESSION_FAILED':
            self.engine.NotifySecondaryConnectionFailed(
                self.shared.get(u'login'),
                self.shared.get(u'ip_client'),
                self.shared.get(u'target_login'),
                self._physical_target_device)
        elif reason == u'OPEN_SESSION_SUCCESSFUL':
            pass
        elif reason == u'FILESYSTEM_FULL':
            data = message.split(u'|')
            used       = data[0]
            filesystem = data[1]

            self.engine.NotifyFilesystemIsFullOrUsedAtXPercent(filesystem, used)
        elif reason == u'SESSION_EXCEPTION':
            pass
        elif (reason == u'FINDPATTERN_KILL') or (reason == u'FINDPATTERN_NOTIFY'):
            pattern = message.split(u'|')
            regexp = pattern[0]
            string = pattern[1]
#            Logger().info(u"regexp=\"%s\" string=\"%s\" user_login=\"%s\" user=\"%s\" host=\"%s\"" %
#                (regexp, string, self.shared.get(u'login'), self.shared.get(u'target_login'), self.shared.get(u'target_device')))
            self.engine.NotifyFindPatternInRDPFlow(regexp, string, self.shared.get(u'login'), self.shared.get(u'target_login'), self.shared.get(u'target_device'), self.cn, self.target_service_name)
        else:
            Logger().info(
                u"Unexpected reporting reason: \"%s\" \"%s\" \"%s\"" % (reason, target, message))

    def kill_handler(self, signum, frame):
        # Logger().info("KILL_HANDLER = %s" % signum)
        if signum == signal.SIGUSR1:
            self.kill()

    def check_handler(self, signum, frame):
        # Logger().info("CHECK_HANDLER = %s" % signum)
        if signum == signal.SIGUSR2:
            self.check_session_parameters = True

    def kill(self):
        try:
            Logger().info(u"Closing a RDP/VNC connection")
            self.proxy_conx.close()
        except Exception:
            pass

    def check_hostname_in_subnet(self, host, subnet):
        try:
            dnsname, alias, ip_list = socket.gethostbyaddr(host)
            Logger().info("Resolve DNS Hostname %s -> %s" % (host,
                                                             ip_list))
            host_ip = ip_list[0] if ip_list else None
        except socket.error:
            try:
                host_ip = socket.gethostbyname(host)
            except Exception, e:
                return False
        except Exception, e:
            return False
        return engine.is_device_in_subnet(host_ip, subnet)
    def update_session_parameters(self):
        params = self.engine.read_session_parameters()
        res = params.get("rt_display")
        Logger().info("rt_display=%s" % res)
        if res:
            Logger().info("shared rt_display=%s" % self.shared.get("rt_display"))
            if self.shared.get("rt_display") != res:
                Logger().info("sending rt_display=%s !" % res)
                self.send_data({ "rt_display": res })

# END CLASS - Sesman


# This little main permets to run the Sesman Server Alone for one connection
#if __name__ == u'__main__':
#    sck = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#    sck.bind(('', 3350))
#    sck.listen(100)
#    connection, address = sck.accept()

#    Sesman(connection, address)

# EOF
