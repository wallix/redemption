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

MAGICASK = u'UNLIKELYVALUEMAGICASPICONSTANTS3141592926ISUSEDTONOTIFYTHEVALUEMUSTBEASKED'
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

class AuthentifierSocketClosed(Exception):
    pass

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
        self.shared[u'login']           = MAGICASK
        self.shared[u'ip_client']       = MAGICASK
        self.shared[u'proxy_type  ']    = MAGICASK
        self.shared[u'target_protocol'] = MAGICASK
        self.shared[u'keyboard_layout'] = MAGICASK

        self.shared[u'auth_channel_answer'] = u''
        self.shared[u'auth_channel_result'] = u''
        self.shared[u'auth_channel_target'] = u''

        self.internal_mod = False
        self.check_session_parameters = False

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

        # else:
        #     if self.shared.get(u'password') == MAGICASK:
        #         data[u'password'] = u''
        #         Logger().info(u"Update password")
        #     data.update({})

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
        if ((SESMANCONF[u'sesman'][u'auth_mode_passthrough'].lower() == u'true')
            and (SESMANCONF[u'sesman'][u'use_default_login'].strip() == u'2')
            and len(SESMANCONF[u'sesman'][u'default_login'].strip())):
            target_login = wab_login
            wab_login = SESMANCONF[u'sesman'][u'default_login'].strip()
            effective_login = target_login
        else:
            level_0_items = wab_login.split(u':')
            if len(level_0_items) > 1:
                if len(level_0_items) > 3:
                    Logger().info(u"username parse error %s" % wab_login)
                    return False, (TR(u'Username_parse_error %s') % wab_login), wab_login, target_login, target_device, target_service, effective_login

                target_service = u'RDP' if len(level_0_items) <= 2 else level_0_items[-2]
                level_1_items, wab_login       = level_0_items[0].split(u'@'), level_0_items[-1]
                target_login, target_device = '@'.join(level_1_items[:-1]), level_1_items[-1]
        if SESMANCONF[u'sesman'][u'auth_mode_passthrough'].lower() == u'true':
            Logger().info(u'ip_target="%s" real_target_device="%s"' % (
                self.shared.get(u'ip_target'), self.shared.get(u'real_target_device')))
            if target_login == MAGICASK:
                target_login = wab_login
            target_device = self.shared.get(u'real_target_device')
            target_service = u'RDP'
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

        if self.shared.get(u'proxy_type') not in [u'RDP', u'VNC']:
            return False, TR(u'Unknown proxy type')

        if self.shared.get(u'login') == MAGICASK:
            return None, TR(u"Empty user, try again")

        (_status, _error,
         wab_login, target_login, target_device,
         self.target_service_name, self.effective_login) = self.parse_username(
            self.shared.get(u'login'),
            self.shared.get(u'target_login'),
            self.shared.get(u'target_device'),
            self.target_service_name
            )
        if not _status:
            return None, TR(u"Invalid user, try again")

        Logger().info(u"Continue with authentication (%s) -> %s" % (self.shared.get(u'login'), wab_login))

        try:
            target_info = None
            if (target_login and target_device and self.target_service_name and
                not target_login == MAGICASK and
                not target_device == MAGICASK and
                not self.target_service_name == MAGICASK):
                target_info = u"%s@%s:%s" % (target_login, target_device, self.target_service_name)
            try:
                target_info = target_info.encode('utf8')
            except Exception, e:
                target_info = None
            #Check if X509 Authentication is active
            if self.engine.is_x509_connected(
                        wab_login,
                        self.shared.get(u'ip_client'),
                        self.shared.get(u'proxy_type'),
                        target_info,
                        self.shared.get(u'ip_target')):
                # Prompt the user in proxy window
                # Wait for confirmation from GUI (or timeout)
                if not (self.interactive_ask_x509_connection() and self.engine.x509_authenticate()):
                    return False, TR(u"x509 browser authentication not validated by user")
            elif SESMANCONF[u'sesman'][u'auth_mode_passthrough'].lower() == u'true':
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

            # try:
            #     # might be too early, should be done just before accessing Right structure
            #     # Then get user rights (reachable targets)
            #     self.engine.get_proxy_rights([u'RDP', u'VNC'])
            # except Exception, e:
            #     if DEBUG:
            #         import traceback
            #         Logger().info("<<<%s>>>" % traceback.format_exc(e))
            #     # NB : this exception may be raised because the user must change his password
            #     return False, TR(u"Error while retreiving rights for user %s") % wab_login
        except Exception, e:
            if DEBUG:
                import traceback
                Logger().info("<<<%s>>>" % traceback.format_exc(e))
            _status, _error = None, TR(u'auth_failed_wab %s') % wab_login

        return _status, _error

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

        while _status is None:

            if (target_device and target_device != MAGICASK
            and (target_login or SESMANCONF[u'sesman'][u'auth_mode_passthrough'].lower() == u'true') and target_login != MAGICASK):
                # Target is provided at login
                self._full_user_device_account = u"%s@%s:%s" % ( target_login
                                                               , target_device
                                                               , wab_login
                                                               )
                data_to_send = { u'login'                   : wab_login
                               , u'target_login'            : target_login
                               , u'target_device'           : target_device
                               # , u'proto_dest'              : proto_dest if proto_dest != u'INTERNAL' else u'RDP'
                               , u'module'                  : u'transitory' if self.target_service_name != u'INTERNAL' else u'INTERNAL'
                               }
                if data_to_send.has_key(u'module') and not self.internal_mod:
                    self.internal_mod = True if data_to_send[u'module'] == u'INTERNAL' else False
                self.send_data(data_to_send)
                _status = True
            elif self.shared.get(u'selector') == MAGICASK:
                # filters ("Group" and "Account/Device") entered by user in selector are applied to raw services list
                self.engine.get_proxy_rights([u'RDP', u'VNC'])
                services, item_filtered = self.engine.get_targets_list(
                    group_filter = self.shared.get(u'selector_group_filter'),
                    device_filter = self.shared.get(u'selector_device_filter'),
                    protocol_filter = self.shared.get(u'selector_proto_filter'),
                    real_target_device = self.shared.get(u'real_target_device'))

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
                                           , u'proxy_type'              : self.shared.get(u'proxy_type')
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
                                           , u'proxy_type'              : self.shared.get(u'proxy_type')
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
                    s = services[0]
                    data_to_send = {}
                    data_to_send[u'module'] = u'transitory' if s[2] != u'INTERNAL' else u'INTERNAL'
                    if s[2] == u'APP':
                        data_to_send[u'target_login'] = '@'.join(s[1].split('@')[:-1])
                        data_to_send[u'target_device'] = s[4]
                        # data_to_send[u'proto_dest'] = s[2]

                        self._full_user_device_account = u"%s@%s:%s" % ( target_login
                                                                       , target_device
                                                                       , wab_login
                                                                       )
                    else:
                        data_to_send[u'target_login'] = '@'.join(s[1].split('@')[:-1])
                        data_to_send[u'target_device'] = s[4]
                        # data_to_send[u'proto_dest'] = s[2] if s[2] != u'INTERNAL' else u'RDP'

                        self._full_user_device_account = u"%s@%s:%s" % ( self.shared.get(u'target_login')
                                                                       , self.shared.get(u'target_device')
                                                                       , self.shared.get(u'login')
                                                                       )
                    if data_to_send.has_key(u'module') and not self.internal_mod:
                        self.internal_mod = True if data_to_send[u'module'] == u'INTERNAL' else False
                    self.send_data(data_to_send)
                    self.target_service_name = s[1].split(':')[-1]
                    # Logger().info("Only one target : service name %s" % self.target_service_name)
                    _status = True
                else:
                    _status, _error = False, TR(u"Target unreachable")

            else:
                self.send_data({u'login': MAGICASK})
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

    def start(self):
        _status, tries = None, 5
        while _status is None and tries > 0:

            ##################
            ### AUTHENTIFY ###
            ##################
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
                _status, _error = self.get_service()
                if not _status:
                    self.engine.reset_proxy_rights()

        if tries <= 0:
            Logger().info(u"Too many login failures")
            _status, _error = False, TR(u"Too many login failures or selector orders, closing")
            self.engine.NotifyPrimaryConnectionFailed(self.shared.get(u'login'),
                                                      self.shared.get(u'ip_client'))

        if _status:
            Logger().info(u"Asking service %s@%s" % (self.shared.get(u'target_login'), self.shared.get(u'target_device')))

        # Fetch Auth on given target account
        if _status:
            ###################
            ### FIND_TARGET ###
            ###################

            # The purpose of the snippet below is electing the first right that match the login AND device
            # AND service that have been passed in the connection string.
            # If service is blank take the first right that match login AND device (may happen with a command
            #  line or a mstsc '.rdp' file connections ; never happens if the selector is used).
            # NB : service names are supposed to be in alphabetical ascending order.
            selected_target = None
            target_device =  self.shared.get(u'target_device')
            target_login = self.shared.get(u'target_login')
            target_service = self.target_service_name if self.target_service_name != u'INTERNAL' else u'RDP'
            # proto_dest = self.shared.get(u'proto_dest')
            # protocols = [proto_dest] if proto_dest else [ u'APP', u'RDP', u'VNC']
            services = [target_service] if target_service else [ u'APP', u'RDP', u'VNC']

            found = False
            for service_name in services:
                selected_target = self.engine.get_selected_target(target_login,
                                                                  target_device,
                                                                  service_name)
                if selected_target:
                    found = True
                    break
            if not found:
                _target = u"%s@%s:%s" % ( target_login, target_device, target_service )
                _error_log = u"Targets %s not found in user rights" % _target
                _status, _error = False, TR(u"Target %s not found in user rights") % _target
                Logger().info("%s" % _error)

        #TODO: looks like the code below should be done in the instance of some "selected_target" class
        if _status:
            session_started = False

            _status, _error = self.check_video_recording(
                selected_target.authorization.isRecorded,
                mdecode(self.engine.get_username()) if self.engine.get_username() else self.shared.get(u'login'))

            Logger().info(u"Fetching protocol")

            kv = {}
            kv[u'proto_dest'] = selected_target.resource.service.protocol.cn
            kv[u'target_port'] = selected_target.resource.service.port
            kv[u'timezone'] = str(altzone if daylight else timezone)

            if _status:
                kv['password'] = 'pass'

                # register signal
                signal.signal(signal.SIGUSR1, self.kill_handler)
                signal.signal(signal.SIGUSR2, self.check_handler)

                Logger().info(u"Starting Session")
                # Add connection to the observer
                kv[u'session_id'] = self.engine.start_session(selected_target, self.pid,
                                                              self.effective_login)
                _status, _error = self.engine.write_trace(self.full_path)
                pattern_kill, pattern_notify = self.engine.get_restrictions(selected_target, "RDP")
                if pattern_kill:
                    self.send_data({ u'module' : u'transitory', u'pattern_kill': pattern_kill })
                if pattern_notify:
                    self.send_data({ u'module' : u'transitory', u'pattern_notify': pattern_notify })

            if _status:
                Logger().info(u"Checking timeframe")
                self.infinite_connection = False
                if (selected_target.deconnection_time == u"-"
                   or selected_target.deconnection_time[0:4] >= u"2034"
                   ):
                    selected_target.deconnection_time = u"2034-12-31 23:59:59"
                    self.infinite_connection = True

                now = datetime.strftime(datetime.now(), "%Y-%m-%d %H:%M:%S")
                if (selected_target.deconnection_time == u'-'
                or now < selected_target.deconnection_time):
                    # deconnection time to epoch
                    tt = datetime.strptime(selected_target.deconnection_time, "%Y-%m-%d %H:%M:%S").timetuple()
                    kv[u'timeclose'] = int(mktime(tt))
                    if not self.infinite_connection:
                        _status, _error = self.interactive_display_message(
                                {u'message': TR(u'session_closed_at %s') % selected_target.deconnection_time}
                                )

            module = kv.get(u'proto_dest')
            if not module in [ u'RDP', u'VNC', u'INTERNAL' ]:
                module = u'RDP'
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
                if not _status:
                    physical_target = None
                    break

                kv[u'disable_tsk_switch_shortcuts'] = u'no'
                if selected_target.resource.application:
                    self.cn = selected_target.resource.application.cn
                    app_params = self.engine.get_app_params(selected_target, physical_target)
                    if not app_params:
                        continue
                    kv[u'alternate_shell'] = (u"%s %s" % (app_params.program, app_params.params))
                    kv[u'shell_working_directory'] = app_params.workingdir
                    kv[u'target_application'] = selected_target.service_login
                    kv[u'disable_tsk_switch_shortcuts'] = u'yes'
                else:
                    self.cn = selected_target.resource.device.cn

                if self.shared.get(u'real_target_device'):
                    kv[u'target_device'] = self.shared.get(u'real_target_device')
                else:
                    kv[u'target_device'] = physical_target.resource.device.host
                    kv[u'target_port'] = physical_target.resource.service.port

                if SESMANCONF[u'sesman'][u'auth_mode_passthrough'].lower() != u'true':
                    kv[u'target_login'] = physical_target.account.login

                release_reason = u''

                try:
                    Logger().info("auth_mode_passthrough=%s" % SESMANCONF[u'sesman'][u'auth_mode_passthrough'])

                    if SESMANCONF[u'sesman'][u'auth_mode_passthrough'].lower() == u'true':
                        if self.shared.get(u'password') == MAGICASK:
                            password_of_target = u''
                        else:
                            password_of_target = self.shared.get(u'password')
                        #Logger().info("auth_mode_passthrough target_password=%s" % password_of_target)
                        kv[u'password'] = u'password'
                    else:
                        password_of_target = self.engine.get_target_password(physical_target)

                    kv[u'target_password'] = password_of_target
                    if not password_of_target:
                        kv[u'target_password'] = u''
                        Logger().info(u"auto logon is disabled")

                    if not _status:
                        break

                    if self.shared.get(u'real_target_device'):
                        self._physical_target_device = self.shared.get(u'real_target_device')
                    else:
                        self._physical_target_device = physical_target.resource.device.host

                    Logger().info(u"Send critic notification (every attempt to connect to some physical node)")
                    if selected_target.authorization.isCritical:
                        import socket
                        self.engine.NotifyConnectionToCriticalEquipment(
                            (u'APP' if selected_target.resource.application
                                 else selected_target.resource.service.protocol.cn),
                            self.shared.get(u'login'),
                            socket.getfqdn(self.shared.get(u'ip_client')),
                            self.shared.get(u'ip_client'),
                            self.shared.get(u'target_login'),
                            self.shared.get(u'target_device'),
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
                             # , u'authenticated': u'False'
                             # , u'selector': u"undefined"
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

                                    _message = (u"SET JOB\x01"
                                                u"To:%s\x01"
                                                u"\x01"
                                                u"Job:simple_webform_filling\x01"
                                                u"Application:C:\\Program Files\\Internet Explorer\\iexplore.exe\x01"
                                                u"Directory:%%HOMEDRIVE%%%%HOMEPATH%%\x01"
                                                u"WebsiteURL:10.10.47.32\x01"
                                                u"WebformURL:https://10.10.47.32/accounts/login/\x01"
                                                u"WebformName:login-form\x01"
                                                u"Input:user_name:admin\x01"
                                                u"Input:passwd:admin") % self.shared.get(u'auth_channel_target')

                                    self.send_data({u'auth_channel_answer': _message})

                                    Logger().info(u"Sending of auth channel answer ok")

                                    self.shared[u'auth_channel_target'] = u''
                            # r can be empty
                            # else: # (if self.proxy_conx in r)
                            #     if not self.internal_mod:
                            #         Logger().error(u'break connection')
                            #         release_reason = u'Break connection'
                            #         break
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
                        if (physical_target == selected_target):
                            #no application case
                            Logger().info("Calling release_target_password")
                            self.engine.release_target_password(physical_target, release_reason)
                        else:
                            #application case
                            #release application password
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
            self.engine.NotifyFindPatternInRDPFlow(regexp, string, self.shared.get(u'login'), self.shared.get(u'target_login'), self.shared.get(u'target_device'), self.cn)
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
