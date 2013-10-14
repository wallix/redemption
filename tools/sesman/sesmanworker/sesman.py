#!/usr/bin/python -O
# -*- coding: utf-8 -*-
##
# Copyright (c) 2010 WALLIX, SARL. All rights reserved.
# Licensed computer software. Property of WALLIX.
# Product name: WALLIX Admin Bastion V 2.x
# Author(s): Olivier Hervieu
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

DEBUG = True

################################################################################
class Sesman():
################################################################################

    # __INIT__
    #===============================================================================
    def __init__(self, conn, addr):
    #===============================================================================
        self.proxy_conx  = conn
        self.addr        = addr
        self._error      = None
        self.path        = None
        self.full_path   = None
        self._license_ok = None

        self.engine = engine.Engine()
        self._full_user_device_account = u'Unknown'
        self._group_filter             = u''
        self._device_filter            = u''
        self._proto_filter             = u''
        self._selector                 = u'False'
        self.shared                    = {}
        self._enable_encryption        = self.engine.get_trace_encryption()
        self.language = None

        self._reporting                = u''

        self.pid = os.getpid()

        # Initialize connection
        # =====================
        self._target_login = MAGICASK
        self._target_device = MAGICASK
        self._wab_login = MAGICASK
        self._wab_password = MAGICASK
        self._ip_client = MAGICASK
        self._proxy_type = MAGICASK
        self._target_protocol = MAGICASK
        self._real_targer_device = MAGICASK

        self._saved_wab_login = self._wab_login

    def check_password_expiration_date(self):
        _status, _error = True, u''
        try:
            Logger().info("password_expiration_date = %s" % self.engine.password_expiration_date())
            if self.engine.password_expiration_date():
                _status, _error = self.interactive_display_message(
                    {u'message': u'Your password will expire %s. Please change it.' % "soon"}
                )
        except Exception, e:
            import traceback
            Logger().info("<<<<%e>>>>" % traceback.format_exc(e))
        return _status, _error


    def check_video_recording(self, isRecorded):
        _status, _error = True, u''
        data_to_send = {
              u'is_rec'         : u'False'
            , u'rec_path'       : u""
            , u'file_encryption': u"False"
        }
        
        try:
            self.full_path = u""
            self.path = u""
            if isRecorded:
                try:
                    os.stat(RECORD_PATH)
                except OSError:
                    try:
                        os.mkdir(RECORD_PATH)
                    except Exception:
                        _status, _error = False, TR(u'error_recording_path %s') % self._full_user_device_account
                if _status:
                    # Naming convention : {username}@{userip},{account}@{devicename},YYYYMMDD-HHMMSS,{wabhostname},{uid}
                    # NB :  backslashes are replaced by pipes for IE compatibility
                    random.seed(self.pid)

                    #keeping code synchronized with wabengine/src/common/data.py
                    video_path =  u"%s@%s," % ( self._wab_login, self._ip_client)
                    video_path += u"%s@%s," % (self._target_login, self._target_device)
                    video_path += u"%s," % (strftime("%Y%m%d-%H%M%S"))
                    video_path += u"%s," % gethostname()
                    video_path += u"%s" % random.randint(1000, 9999)
                    video_path = video_path.replace(unichr(92), u"")
                    self.path = video_path

                    Logger().debug(u"This session will be recorded in %s" % self.path)

                    self.full_path = RECORD_PATH + self.path
                    data_to_send[u'is_rec'] = True
                    data_to_send[u"file_encryption"] = u'True' if self._enable_encryption else u'False'
                    #TODO remove .flv extention and adapt ReDemPtion proxy code
                    data_to_send[u'rec_path'] = u"%s.flv" % (self.full_path)

                    record_warning = SESMANCONF[u'sesman'][u'record_warning'].lower()
                    if record_warning != 'false':
                        message =  u"Warning! Your remote session may be recorded and kept in electronic format."
                        try:
                            with open('/opt/wab/share/proxys/messages/motd.%s' % SESMANCONF.language) as f:
                                message = f.read().decode('utf-8')
                        except Exception, e:
                            pass
                        data_to_send[u'message'] = cut_message(message)

                    _status, _error = self.interactive_accept_message(data_to_send)

        except Exception, e:
            import traceback
            Logger().info("<<<<%s>>>>" % traceback.format_exc(e))
            _status, _error = False, u"Connection closed by client"

        return _status, _error


    def interactive_ask_x509_connection(self):
        """ Send a message to the proxy to prompt the user to validate x509 in his browser
            Wait until the user clicks Ok in Proxy prompt or until timeout
        """
        _status = False
        data_to_send = { u'message' : TR(u'valid_authorisation')
                       , u'password': u'x509'
                       , u'display_message': MAGICASK
                       , u'accept_message': u''
                        # ASK reset to u'' exists to avoid jamming the automaton states analysis in proxy side
                       , u'target_device': self._target_device if self._target_device != MAGICASK else u''
                       , u'target_login': self._target_login if self._target_login != MAGICASK else u''
                       }

        self.send_data(data_to_send)

        # Wait for the user to click Ok in proxy
        Logger().info(u'wait user grant or reject connection')
        _data, _status, _error = self.receive_data()

        Logger().info(u'Data received')
        if _data[u'display_message'] == u'True':
            _status = True

        return _status

    def interactive_display_message(self, data_to_send):
        u""" NB : Strings sent to the ReDemPtion proxy MUST be UTF-8 encoded """
        #TODO: we should not have to care about target login or device to display messages
        # we should be able to send messages before or after defining target seamlessly
        data_to_send.update({ u'proto_dest'    : u'INTERNAL'
                            , u'display_message': MAGICASK
                            , u'accept_message': u''
                            })

        self.send_data(data_to_send)
        _data, _status, _error = self.receive_data()

        if _data.get(u'display_message') != u'True':
            _status, _error = False, TR(u'not_display_message')

        return _status, _error

    def interactive_accept_message(self, data_to_send):
        u""" NB : Strings sent to the ReDemPtion proxy MUST be UTF-8 encoded """
        #TODO: we should not have to care about target login or device to display messages
        # we should be able to send messages before or after defining target seamlessly
        data_to_send.update({ u'proto_dest'    : u'INTERNAL'
                            , u'accept_message': MAGICASK
                            , u'display_message': u''
                            })
        self.send_data(data_to_send)

        _data, _status, _error = self.receive_data()
        if _data.get(u'accept_message') != u'True':
            _status, _error = False, TR(u'not_accept_message')

        return _status, _error


    def send_data(self, data):
        u""" NB : Strings sent to the ReDemPtion proxy MUST be UTF-8 encoded """

        if DEBUG:
            Logger().info(u'send_data (update)=%s' % (data))

        if DEBUG:
            Logger().info(u'send_data lang=%s sesman=%s' % (self.language, SESMANCONF.language))

        #if current language changed, send translations
        if self.language != SESMANCONF.language:
            if self.language:
                SESMANCONF.language = self.language
            else:
                self.language = SESMANCONF.language
            
            data[u'language'] = SESMANCONF.language
            data.update(translations())

        if DEBUG:
            Logger().info(u'send_data (full)=%s' % (data))

        # replace MAGICASK with ASK and send data on the wire            
        _list = []
        for key, value in data.iteritems():
            if value != MAGICASK:
                self.shared[key] = u'!%s' % value
                _pair =  u"%s\n%s\n" % (key, (u"!%s" % value))
            else:
                _pair = u"%s\nASK\n" % key
                self.shared[key] = 'ASK'
            _list.append(_pair)

        if DEBUG:
           Logger().info(u'send_data (on the wire)=%s' % (_list))

        _r_data = u"".join(_list)
        _r_data = _r_data.encode('utf-8')
        _len = len(_r_data)

        self.proxy_conx.sendall(pack(">L", _len))
        self.proxy_conx.sendall(_r_data)

    # SEND CLOSE
    #===========================================================================z====
    def interactive_close(self, target, message):
        u""" NB : Strings sent to the ReDemPtion proxy MUST be UTF-8 encoded """
        data_to_send = { u'error_message'  : message
                       , u'trans_ok'       : u'OK'
                       , u'proto_dest'     : u'INTERNAL'
                       , u'target_device'  : u'close:%s' % target
                       , u'target_login'   : self._target_login
                       , u'target_password': u'Default'
                       }

        # If we send close we should expect authentifier socket will be closed by the other end
        # No need to return some warning message if that happen
        self.send_data(data_to_send)
        _data, _status, _error = self.receive_data()

        return _status, _error

    # END METHOD - SEND MESSAGE

    def parse_username(self, _username):
        Logger().info(u"parse_username: ... _username=%s" % _username)

        level_0_items       = _username.split(u':')
        level_0_items_count = len(level_0_items)

        Logger().info(u'level_0_items=%s' % level_0_items)

        wab_login       = u''
        target_login    = u''
        target_device   = u''
        target_protocol = u''

        if level_0_items_count == 2:
            wab_login = level_0_items[level_0_items_count - 1]

            level_1_items       = level_0_items[0].split(u'@')
            level_1_items_count = len(level_1_items)

            target_login = '@'.join([level_1_items[i] for i in range(level_1_items_count - 1)])

            if level_1_items_count > 1:
                target_device = level_1_items[level_1_items_count - 1]

#            target_protocol = u'RDP'
        elif level_0_items_count == 3:
            wab_login = level_0_items[level_0_items_count - 1]

            level_1_items       = level_0_items[0].split(u'@')
            level_1_items_count = len(level_1_items)

            target_login = '@'.join([level_1_items[i] for i in range(level_1_items_count - 1)])

            if level_1_items_count > 1:
                target_device = level_1_items[level_1_items_count - 1]

            target_protocol = level_0_items[level_0_items_count - 2]
        else:
            wab_login = _username

            Logger().info(u"parse_username: done. wab_login=\"%s\"" % wab_login)

            self._wab_login = wab_login

            return

        Logger().info(
            u"parse_username: done. wab_login=\"%s\" target_login=\"%s\" target_device=\"%s\" target_protocol=\"%s\"" % (
                wab_login, target_login, target_device, target_protocol))

        self._wab_login       = wab_login
        self._target_login    = target_login
        self._target_device   = target_device
        self._target_protocol = target_protocol

    def receive_data(self):
        u""" NB : Strings coming from the ReDemPtion proxy are UTF-8 encoded """

        _status, _error = True, u''
        _data = {}
        try:
            # Fetch Data from Redemption
            _packet_size, = unpack(">L", self.proxy_conx.recv(4))
            _data = self.proxy_conx.recv(_packet_size)
            _data = _data.decode('utf-8')
        except Exception:
            Logger().info(u"recv failed")
            _status = False
            _error = u"Failed to read data from rdpproxy authentifier socket"

        if _status:
            _elem = _data.split('\n')

            if len(_elem) & 1 == 0:
                _status = False
                _error = u"Odd number of items in authentication protocol"
        if _status:
            try:
                _data = dict(zip(_elem[0::2], _elem[1::2]))
            except Exception, e:
                _status = False
                _error = u"Error on parsing data %s" % e

            if DEBUG:
                Logger().info("received_data (on the wire) = %s" % _data)

        # may be actual socket error, or unpack or parsing failure
        # (because we got partial data). Whatever the case socket connection
        # with rdp proxy is now broken and must be terminated
        if not _status:
            raise socket.error()

        if _status:
            self.shared.update(_data)
            _data = self.shared
            for key in _data:
                if (_data[key][:3] == u'ASK'):
                    _data[key] = MAGICASK
                elif (_data[key][:1] == u'!'):
                    _data[key] = _data[key][1:]
                else:
                    # _data[key] unchanged
                    pass
                    
            self.context = { 
                  u'login': self._wab_login
                , u'password': self._wab_password
                , u'ip_client': self._ip_client
                , u'proxy_type': self._proxy_type
                , u'selector': self._selector
                , u'selector_group_filter': self._group_filter
                , u'selector_device_filter': self._device_filter
                , u'selector_proto_filter': self._proto_filter
                , u'selector_current_page': u'1'
                , u'selector_lines_per_page': u'0'
                , u'target_login': self._target_login
                , u'target_device': self._target_device
                , u'target_protocol': self._target_protocol
                , u'real_target_device': u''
                , u'reporting': u''
            }
            
            self.context.update(_data)

            if DEBUG:
                Logger().info("received_data (with cached) = %s" % self.context)


            self._wab_login =          _data.get(u'login', self._wab_login)
            self._wab_password =       _data.get(u'password', self._wab_password)
            self._ip_client =          _data.get(u'ip_client', self._ip_client)
            self._proxy_type =         _data.get(u'proxy_type', self._proxy_type)
            self._selector =           _data.get(u'selector', self._selector)
            self._group_filter =       _data.get(u'selector_group_filter', self._group_filter)
            self._device_filter =      _data.get(u'selector_device_filter', self._device_filter)
            self._proto_filter =       _data.get(u'selector_proto_filter', self._proto_filter)
            self._current_page =       _data.get(u'selector_current_page', u'1')
            self._lines_per_page =     _data.get(u'selector_lines_per_page', u'0')
            self._target_login =       _data.get(u'target_login', self._target_login)
            self._target_device =      _data.get(u'target_device', self._target_device)
            self._target_protocol =    _data.get(u'proto_dest', self._target_protocol)
            self._real_target_device = _data.get(u'real_target_device', u'')
            self._reporting =          _data.get(u'reporting', u'')

            self._saved_wab_login = self._wab_login
            
            self.parse_username(self._wab_login)

        return _data, _status, _error

    # END METHOD - RECEIVE DATA

    # AUTHENTIFY
    #===============================================================================
    def authentify(self):
    #===============================================================================
        """ Authentify the user through password engine and then retreive his rights
             The user preferred language will be set as the language to use in
             interactive messages
        """
        Logger().info(u"Waiting for data");
        _data, _status, _error = self.receive_data()

        if not _status:
            return False, _error

        if self._proxy_type not in [u'RDP', u'VNC']:
            return False, TR(u'Unknown proxy type')

        if self._wab_login == MAGICASK:
            return None, TR(u"Empty user, try again")

        try:
            #Check if X509 Authentication is active
            if self.engine.is_x509_connected(self._wab_login, self._ip_client, self._proxy_type):
                # Prompt the user in proxy window
                 # Wait for confirmation from GUI (or timeout)
                if not (self.interactive_ask_x509_connection() and self.engine.x509_authenticate()):
                    return False, TR(u"x509 browser authentication not validated by user")
            else:
                # PASSWORD based Authentication
                if (self._wab_password == MAGICASK
                or not self.engine.password_authenticate(self._wab_login, self._ip_client, self._wab_password)):
                    return None, TR(u"auth_failed_wab %s") % self._wab_login

            try:
                if self.engine.user:
                    self.language = self.engine.user.preferredLanguage
            except Exception, e:
                import traceback
                Logger().info("<<<%s>>>" % traceback.format_exc(e))
                
            Logger().info(u'lang=%s sesman=%s' % (self.language, self.engine.user.preferredLanguage))
                

            # When user is authentified check if licence tokens are available
            Logger().info(u"Checking licence")
            if not self.engine.get_license_status():
                return False, TR(u'licence_blocker')

            try:
                # Then get user rights (reachable targets)
                self.engine.get_proxy_rights([u'RDP', u'VNC'])

            except Exception, e:
                import traceback
                Logger().info("<<<%s>>>" % traceback.format_exc(e))
                # NB : this exception may be raised because the user must change his password
                return False, TR(u"Error while retreiving rights for user %s" % self._wab_login)

        except engine.AuthenticationFailed, e:
            import traceback
            Logger().info("<<<%s>>>" % traceback.format_exc(e))
            _status, _error = None, TR(u'auth_failed_wab %s') % self._wab_login

        except Exception, e:
            import traceback
            Logger().info("<<<%s>>>" % traceback.format_exc(e))
            _status, _error = None, TR(u'auth_failed_wab %s') % self._wab_login

        return _status, _error

    # END METHOD - AUTHENTIFY


    # GET SERVICE
    #===============================================================================
    def get_service(self):
    #===============================================================================
        u""" Send service pages to proxy until the selected service is returned.
        """

        _status = None
        _error = u"No error"
        authentified_user = self._wab_login

        Logger().info(u"get_service %s" % (_status))

        while _status is None:
            if (self._target_device and self._target_device != MAGICASK
            and self._target_login  and self._target_login  != MAGICASK):
                self._full_user_device_account = u"%s@%s:%s" % ( self._target_login
                                                                  , self._target_device
                                                                  , self._wab_login
                                                                  )
                _status = True
            elif self._selector == MAGICASK and self._wab_login == authentified_user:
                # filters ("Group" and "Account/Device") entered by user in selector are applied to raw services list
                item_filtered = False
                services = []
                for right in self.engine.rights:
                    if not right.resource.application:
                        temp_service_login                = right.service_login
                        temp_resource_service_protocol_cn = right.resource.service.protocol.cn
                        temp_resource_device_cn           = right.resource.device.cn
                    else:
                        temp_service_login                = right.service_login + u':APP'
                        temp_resource_service_protocol_cn = u'APP'
                        temp_resource_device_cn           = right.resource.application.cn

                    if (  (right.target_groups.find(self._group_filter) == -1)
                       or (temp_service_login.find(self._device_filter) == -1)
                       or (temp_resource_service_protocol_cn.find(self._proto_filter) == -1)):
                        item_filtered = True
                        continue

                    if (self._real_target_device
                       and (right.resource.device.host != self._real_target_device)):
                       continue

                    services.append(( right.target_groups # ( = concatenated list)
                                    , temp_service_login
                                    , temp_resource_service_protocol_cn
                                    , (right.deconnection_time if right.deconnection_time[0:4] < "2034" else u"-")
                                    , temp_resource_device_cn
                                    )
                                   )

                if (len(services) > 1) or item_filtered:
                    try:
                        _current_page = int(self._current_page) - 1
                        _lines_per_page = int(self._lines_per_page)

#                        Logger().info( u"current_page=%s\n" % _current_page)
#                        Logger().info( u"lines_per_page=%s\n" % _lines_per_page)

                        if not _lines_per_page:
                            data_to_send = { u'target_login'            : u""
                                           , u'target_device'           : u""
                                           , u'proto_dest'              : u""
                                           , u'end_time'                : u""
                                           , u'selector'                : u"True"
                                           , u'ip_client'               : self._ip_client
                                           , u'proxy_type'              : self._proxy_type
                                           , u'authenticated'           : u"True"
                                           , u'selector_number_of_pages': u"0"
                                           # No lines sent, reset filters
                                           , u'selector_group_filter'   : u""
                                           , u'selector_device_filter'  : u""
                                           , u'selector_proto_filter'   : u""
                                           }
#                            Logger().info( u"data_to_send=%s\n" % data_to_send)

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
                            all_end_time      = [s[3] for s in services]

                            data_to_send = { u'target_login'            : u"\x01".join(all_target_login)
                                           , u'target_device'           : u"\x01".join(all_target_device)
                                           , u'proto_dest'              : u"\x01".join(all_proto_dest)
                                           , u'end_time'                : u";".join(all_end_time)
                                           , u'selector'                : u'True'
                                           , u'ip_client'               : self._ip_client
                                           , u'proxy_type'              : self._proxy_type
                                           , u'authenticated'           : u'True'
                                           , u'selector_number_of_pages': "%s" % _number_of_pages
                                           , u'selector_current_page'   : "%s" % (_current_page + 1)
                                           , u'selector_group_filter'   : self._group_filter
                                           , u'selector_device_filter'  : self._device_filter
                                           , u'selector_proto_filter'   : self._proto_filter
                                           }

                        self.send_data(data_to_send)

                        _data, _status, _error = self.receive_data()

                        if self._wab_login == MAGICASK:
                            self.send_data({
                                  u'login': MAGICASK
                                , u'selector_lines_per_page' : u'0'})
                            Logger().info(u"Logout %s" % (None))
                            return None, u"Logout"
                        _status = None # One more loop
                    except Exception, e:
                        import traceback
                        Logger().info(u"Unexpected error in selector pagination %s" % traceback.format_exc(e))
                        return False, u"Unexpected error in selector pagination"
                elif len(services) == 1:
                    s = services[0]
                    if s[2] == u'APP':
                        self._target_login = '@'.join(s[1].split('@')[:-1])
                        self._target_device = s[4]
                        self._target_protocol = s[2]

                        self._full_user_device_account = u"%s@%s:%s" % ( self._target_login
                                                                       , self._target_device
                                                                       , self._wab_login
                                                                       )
                    else:
                        self._target_login = '@'.join(s[1].split('@')[:-1])
                        self._target_device = s[4]
                        self._target_protocol = s[2]

                        self._full_user_device_account = u"%s@%s:%s" % ( self._target_login
                                                                       , self._target_device
                                                                       , self._wab_login
                                                                       )
                    _status = True
                else:
                    _status, _error = False, u"Target unreachable"

            else:
                self.send_data({u'login': MAGICASK})
                return None, u"Logout"

        return _status, _error
    # END METHOD - GET_SERVICE

    def get_selected_target(self, target_protocol):
        selected_target = None
        for r in self.engine.rights:
            if r.resource.application:
#                Logger().info(u"%s %s == %s %s" % (self._target_device, self._target_login, r.resource.application.cn, r.account.login))
                if self._target_device != r.resource.application.cn:
                    continue
                if self._target_login != r.account.login:
                    continue
                if target_protocol != u'APP':
                    continue
            else:
#                Logger().info(u"%s %s %s == %s %s %s" % (self._target_device, self._target_login, self._target_protocol, r.resource.device.cn, r.account.login, r.resource.service.cn))
                if self._target_device != r.resource.device.cn:
                    continue
                if self._target_login != r.account.login:
                    continue
                if target_protocol != r.resource.service.cn:
                    continue
#            if r.resource.application:
#                Logger().info(u"%s %s ==? %s %s" % (self._target_device, self._target_login, r.resource.application.cn, r.account.login))
#            else:
#                Logger().info(u"%s %s %s ==? %s %s %s" % (self._target_device, self._target_login, self._target_protocol, r.resource.device.cn, r.account.login, r.resource.service.cn))
            selected_target = r
            break

        return selected_target

    def start(self):
        _status, tries = None, 5
        while _status is None and tries > 0:

            ##################
            ### AUTHENTIFY ###
            ##################
            _status, _error = self.authentify()

            if _status is None:
                self.engine.NotifyPrimaryConnectionFailed(self._wab_login, self._ip_client)

            tries = tries - 1
            if _status is None and tries > 0:
                Logger().info(
                    u"Wab user '%s' authentication from %s failed [%u tries remains]"  %
                    (mundane(self._wab_login) , mundane(self._ip_client), tries)
                )

                data_to_send = {u'login': self._saved_wab_login, u'password': MAGICASK}
                self.send_data(data_to_send)
                continue

            if _status:
                tries = 5
                Logger().info(u"Wab user '%s' authentication succeeded" % mundane(self._wab_login))

                # Warn password will expire soon for user
                _status, _error = self.check_password_expiration_date()

                # Get services for identified user
                _status, _error = self.get_service()

        if tries <= 0:
            Logger().info(u"Too many login failures %s" % (_status))
            _status, _error = False, u"Too many login failures or selector orders, closing"

        if _status:
            Logger().info(u"Asking service %s@%s" % (self._target_login, self._target_device))

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
            try:
                selected_target = None
                if self._target_protocol:
                    selected_target = self.get_selected_target(self._target_protocol)
                else:
                    selected_target = self.get_selected_target(u'APP')
                    if not selected_target:
                        selected_target = self.get_selected_target(u'RDP')
                    if not selected_target:
                        selected_target = self.get_selected_target(u'VNC')

                if not selected_target:
                    _status = False
                    _error = u"Target (%s@%s:%s) not found in user rights" % ( self._target_login
                                                                        , self._target_device
                                                                        , self._target_protocol
                                                                        )
                    Logger().info(u"%s" % _error)

            except Exception, e:
                _status, _error = False, u"Failed to get authorisations for %s" % self._full_user_device_account

        # Dictionnary that will contain answer to client


        #TODO: looks like the code below should be done in the instance of some "selected_target" class
        if _status:
            session_started = False

            Logger().info(u"Checking video")
            _status, _error = self.check_video_recording(selected_target.authorization.isRecorded)

            if not _status:
                Logger().info(u"Check video error : %s" % _error)


            Logger().info(u"Fetching protocol")

            kv = {}
            kv[u'proto_dest'] = selected_target.resource.service.protocol.cn
            kv[u'target_port'] = selected_target.resource.service.port
            kv[u'timezone'] = str(altzone if daylight else timezone)

            if _status:
                kv[u'authenticated'] = u'True'

                # register signal
                signal.signal(signal.SIGUSR1, self.kill_handler)

                Logger().info(u"Starting Session")
                # Add connection to the observer
                kv[u'session_id'] = self.engine.start_session(selected_target, self.pid)
                _status, _error = self.engine.write_trace(self.full_path)

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

            proto = 'RDP' if self._target_protocol != 'VNC' else 'VNC'
            kv[u'device_redirection'] = SESMANCONF[proto][u'device_redirection']
            kv[u'clipboard'] = SESMANCONF[proto][u'clipboard']
            kv[u'mode_console'] = u"allow"

            self.reporting_reason  = None
            self.reporting_target  = None
            self.reporting_message = None

            try_next = False

            for physical_target in (self.engine.get_effective_target(selected_target.service_login)
                                     if selected_target.resource.application else [selected_target]):
                if not _status:
                    break

                if selected_target.resource.application:
                    app_params = self.engine.get_app_params(selected_target.service_login, physical_target)
                    kv[u'alternate_shell'] = (u"%s %s" % (app_params.program, app_params.params))
                    kv[u'shell_working_directory'] = app_params.workingdir
                    kv[u'target_application'] = selected_target.service_login

                kv[u'target_device'] = physical_target.resource.device.host
                kv[u'target_login'] = physical_target.account.login

                kv[u'target_password'] = physical_target.account.password
                if not physical_target.account.password:
                    kv[u'target_password'] = u''
                    Logger().info(u"auto logon is disabled")

                if not _status:
                    break

                self._physical_target_device = physical_target.resource.device.host

                Logger().info(u"Send critic notification (every attempt to connect to some physical node)")
                if selected_target.authorization.isCritical:
                    import socket
                    self.engine.NotifyConnectionToCriticalEquipment(
                        (u'APP' if selected_target.resource.application
                             else selected_target.resource.service.protocol.cn),
                        self._wab_login,
                        socket.getfqdn(self._ip_client),
                        self._ip_client,
                        self._target_login,
                        self._target_device,
                        physical_target.resource.device.host,
                        ctime(),
                        None
                        )

    # >>> DLA WabEngine
        #TODO DLA : Réécrire le test de péremption prochaine du mot de passe

    #        if _status:
    #            Logger().info(u'Checking for password obsolescence')

    #            try:
    #                if self._auth.password_in_warn_age:
    #                    Logger().info(u"Password will expire soon")
    #                    try:
    #                        message = TR(u'password_expire')
    #                    except:
    #                        message = u'Your password will expire soon. Please change it.'
    #                    _data, _status, _error = self.interactive_message(message)

    #            except Exception:
    #                _status = False
    #                _error = u"Failed to get if password will expire"
    # <<< FIN DLA WabEngine

                if not _status:
                    Logger().info( u"(%s):%s:REJECTED : %s" \
                                   % ( mundane(self._ip_client)
                                     , mundane(self._wab_login)
                                     , _error
                                     )
                                 )

                    kv = { u"login": u""
                         , u'password': u""
                         , u'target_login': u""
                         , u'target_password': u""
                         , u'target_device': u""
                         , u'authenticated': u'False'
                         , u'selector': u"undefined"
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
                        Logger().info(u"Waiting on proxy")
                        r, w, x = select([self.proxy_conx], [], [], 60)

                        if self.proxy_conx in r:
                            _proxy_data = self.proxy_conx.recv(4)
                            if not _proxy_data:
                                Logger().info(
                                    u"RDP/VNC connection terminated by client: Receive failed")
                                break
                            _packet_size, = unpack(">L", _proxy_data)

                            Logger().info(
                                u"Receive %s byte(s)" % _packet_size)

                            _proxy_data = self.proxy_conx.recv(_packet_size)
                            if not len(_proxy_data) == _packet_size:
                                Logger().info(
                                    u"RDP/VNC connection terminated by client: Invalid packet size")
                                break

                            _proxy_data = _proxy_data.decode('utf-8')

                            _elem = _proxy_data.split(u'\n')
                            _proxy_data = dict(zip(_elem[0::2], _elem[1::2]))

                            for key in _proxy_data:
                                if (_proxy_data[key][:3] == u'ASK'):
                                    _proxy_data[key] = MAGICASK
                                elif (_proxy_data[key][:1] == u'!'):
                                    _proxy_data[key] = _proxy_data[key][1:]
                                else:
                                    # _proxy_data[key] unchanged
                                    pass

                            if _proxy_data.get(u'keepalive') == MAGICASK:
                                self.send_data({u'keepalive': u'True'})

                            if _proxy_data.get(u'reporting'):
                                _reporting      = _proxy_data.get(u'reporting')
                                _reporting_data = _reporting[_reporting.index(':') + 1:]

                                _reporting_reason  = _reporting[:_reporting.index(':')]
                                _reporting_target  = _reporting_data[:_reporting_data.index(':')]
                                _reporting_message = _reporting_data[_reporting_data.index(':') + 1:]

                                Logger().info(u"Reporting: reason=\"%s\" target=\"%s\" message=\"%s\"" % (_reporting_reason, _reporting_target, _reporting_message))

                                self.process_report(_reporting_reason, _reporting_target, _reporting_message)

                                if _reporting_reason == u'CONNECTION_FAILED':
                                    self.reporting_reason  = _reporting_reason
                                    self.reporting_target  = _reporting_target
                                    self.reporting_message = _reporting_message

                                    try_next = True

                                    break

                        else: # (if self.proxy_conx in r)
                            Logger().error(u'break connection')
                            break

                    Logger().debug(u"End Of Keep Alive")

                except Exception, e:
                    Logger().info(u"RDP/VNC connection terminated by client: Exception")
                    Logger().info("<<<<%s>>>>" % traceback.format_exc(e))

                if not try_next:
                    break;

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
                Logger().info(u"Close connection: Exception")
                Logger().info("<<<<%s>>>>" % traceback.format_exc(e))

    # END METHOD - START

    def process_report(self, reason, target, message):
        if   reason == u'CLOSE_SESSION_SUCCESSFUL':
            pass
        elif reason == u'CONNECTION_FAILED':
            self.engine.NotifySecondaryConnectionFailed(self._target_login,
                self._physical_target_device)
        elif reason == u'CONNECTION_SUCCESSFUL':
            pass
        elif reason == u'OPEN_SESSION_FAILED':
            self.engine.NotifySecondaryConnectionFailed(self._target_login,
                self._physical_target_device)
        elif reason == u'OPEN_SESSION_SUCCESSFUL':
            pass
        elif reason == u'FILESYSTEM_FULL':
            self.engine.NotifyFilesystemIsFullOrUsedAtXPercent(message, 100)
        elif reason == u'SESSION_EXCEPTION':
            pass
        else:
            Logger().info(
                u"Unexpected reporting reason: \"%s\" \"%s\" \"%s\"" % (reason, target, message))

    def kill_handler(self, signum, frame):
        if signum == signal.SIGUSR1:
            self.kill()

    def kill(self):
        try:
            Logger().info(u"Closing a RDP/VNC connection")
            self.proxy_conx.close()
        except Exception:
            pass

# END CLASS - Sesman


# This little main permets to run the Sesman Server Alone for one connection
if __name__ == u'__main__':
    sck = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sck.bind(('', 3350))
    sck.listen(100)
    connection, address = sck.accept()

    Sesman(connection, address)

# EOF
