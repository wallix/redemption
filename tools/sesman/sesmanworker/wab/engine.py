#!/usr/bin/python
# -*- coding: utf-8 -*-

from sesmanconf import TR, SESMANCONF, translations
from logger import Logger
FAKE=False
try:
    from wabengine.common.exception import AuthenticationFailed
    from wabengine.common.exception import AuthenticationChallenged
    from wabengine.common.exception import MustChangePassword
    from wallixgenericnotifier import Notify, CX_EQUIPMENT, PATTERN_FOUND, PRIMARY_CX_FAILED, SECONDARY_CX_FAILED, NEW_FINGERPRINT, WRONG_FINGERPRINT, RDP_PATTERN_FOUND, FILESYSTEM_FULL
    from wallixgenericnotifier import LICENCE_EXPIRED, LICENCE_PRIMARY_CX_ERROR, LICENCE_SECONDARY_CX_ERROR
    from wabconfig import Config
    from wabengine.client.sync_client import SynClient
    from wabx509 import AuthX509
except Exception, e:
    Logger().info("================================")
    Logger().info("==== Load Fake PROXY ENGINE ====")
    Logger().info("================================")
    FAKE = True
    from sshng.fake.proxyengine import *

import time

def _binary_ip(network, bits):
    # TODO need Ipv6 support
    # This is a bit too resilient, add check for obviously bad values
    a,b,c,d = [ int(x)&0xFF for x in network.split('.')]
    mask = (0xFFFFFFFF >> bits) ^ 0xFFFFFFFF
    return ((a << 24) + (b << 16) + (c << 8) + d) & mask

def is_device_in_subnet(device, subnet):

    if '/' in subnet:
        try:
            network, bits = subnet.rsplit('/')
            network_bits = _binary_ip(network, int(bits))
            device_bits = _binary_ip(device, int(bits))
            result = network_bits == device_bits
        except Exception, e:
            Logger().error("Bad host definition device '%s' subnet '%s': %s" % (device, subnet, str(e)))
            result = False
    else:
        result = device == subnet
    Logger().info("checking if device %s is in subnet %s -> %s" % (device, subnet, ['No', 'Yes'][result]))
    return result

class Engine(object):
    def __init__(self):
        self.wabengine = None
        self.wabuser = None
        self.client = SynClient('localhost', 'tcp:8803')
        self.session_id  = None
        self.auth_x509 = None
        self._trace_encryption = None
        self.challenge = None
        self.deconnection_epoch = 0xffffffff
        self.deconnection_time = u"-"
        self.proxy_rights = None
        self.rights = None
        self.targets = {}
        self.target_right = None
        self.physical_targets = []
        self.displaytargets = []
        self.proxyrightsinput = None

    def get_language(self):
        try:
            if self.wabuser:
                return self.wabuser.preferredLanguage
            return self.wabengine.who_am_i().preferredLanguage
        except Exception, e:
            return 'en'

    def get_username(self):
        try:
            return self.wabuser.cn
        except Exception, e:
            return ""

    def get_force_change_password(self):
        try:
            return self.wabuser.forceChangePwd
        except Exception, e:
            return False

    def init_timeframe(self, auth):
        if (auth.deconnection_time != u"-"
            and auth.deconnection_time[0:4] <= u"2034"):
            self.deconnection_time = auth.deconnection_time
            self.deconnection_epoch = int(
                time.mktime(
                    time.strptime(
                        auth.deconnection_time,
                        "%Y-%m-%d %H:%M:%S"
                    )
                )
            )

    def get_trace_encryption(self):
        try:
            conf = Config("wabengine")
            self._trace_encryption = True if conf['trace'] == u'cryptofile' else False # u'localfile'
            return self._trace_encryption
        except Exception, e:
            import traceback
            Logger().info("Engine get_trace_encryption failed: configuration file section 'wabengine', key 'trace', (((%s)))" % traceback.format_exc(e))
        return False

    def password_expiration_date(self):
        try:
            _data = self.wabengine.check_password_expiration_info()
            if _data[2]:
                Logger().info("Engine password_expiration_date=%s" % _data[0])
                return True, _data[0]
        except Exception, e:
            import traceback
            Logger().info("Engine password_expiration_date failed: (((%s)))" % traceback.format_exc(e))
        return False, 0

    def is_x509_connected(self, wab_login, ip_client, proxy_type, target, server_ip):
        """
        Ask if we are authentifying using x509
        (and ask user by opening confirmation popup if we are,
        session ticket will be asked later in x509_authenticate)
        """
        try:
            self.auth_x509 = AuthX509(username = wab_login,
                                      ip = ip_client,
                                      requestor = proxy_type,
                                      target = target,
                                      server_ip = server_ip)
            result = self.auth_x509.is_connected()
            return result
        except Exception, e:
            import traceback
            Logger().info("Engine is_x509_connected failed: (((%s)))" % traceback.format_exc(e))
        return False

    def x509_authenticate(self):
        try:
            self.wabengine = self.auth_x509.get_proxy()
            if self.wabengine is not None:
                self.wabuser = self.wabengine.who_am_i()
                return True
        except AuthenticationFailed, e:
            pass
        except Exception, e:
            import traceback
            Logger().info("Engine x509_authenticate failed: (((%s)))" % traceback.format_exc(e))
        return False

    def password_authenticate(self, wab_login, ip_client, password, server_ip):
        try:
            self.wabengine = self.client.authenticate(username = wab_login,
                                                      password = password,
                                                      ip_source = ip_client,
                                                      challenge = self.challenge,
                                                      server_ip = server_ip)
            self.challenge = None
            if self.wabengine is not None:
                self.wabuser = self.wabengine.who_am_i()
                return True
        except AuthenticationChallenged, e:
            self.challenge = e.challenge
        except AuthenticationFailed, e:
            self.challenge = None
            pass
        except Exception, e:
            self.challenge = None
            import traceback
            Logger().info("Engine password_authenticate failed: (((%s)))" % traceback.format_exc(e))
        return False

    def passthrough_authenticate(self, wab_login, ip_client, server_ip):
        try:
            self.wabengine = self.client.authenticate_gssapi(username = wab_login,
                                                             realm = "realm",
                                                             ip_source = ip_client,
                                                             server_ip = server_ip)
            if self.wabengine is not None:
                self.wabuser = self.wabengine.who_am_i()
                return True
        except AuthenticationFailed, e:
            pass
        except Exception, e:
            import traceback
            Logger().info("Engine passthrough_authenticate failed: (((%s)))" % traceback.format_exc(e))
        return False

    def get_license_status(self):
        u""" Three checks : expiration, primary limits, secondary limit
        If at least one fails, user can't connect at all to any device,
        but all three checks are performed wether one as
        yet failed or not to send all relevant notifications to ADMIN.
        """
        license_ok = True
        try:
            lic_status = self.wabengine.get_license_status()

            if lic_status.is_expired():
                Logger().info("LICENCE_EXPIRED")
                Notify(self.wabengine, LICENCE_EXPIRED, u"")
                license_ok = False
            if lic_status.is_primary_limit_reached():
                Logger().info("PRIMARY LICENCE LIMIT")
                Notify(self.wabengine, LICENCE_PRIMARY_CX_ERROR, {u'nbPrimaryConnection': lic_status.primary()[0]})
                license_ok = False
            if lic_status.is_secondary_limit_reached():
                Logger().info("SECONDARY LICENCE LIMIT")
                Notify(self.wabengine, LICENCE_SECONDARY_CX_ERROR, {u'nbSecondaryConnection': lic_status.secondary()[0]})
                license_ok = False
        except Exception, e:
            """If calling get_license_status raise some error, user will be rejected as per invalid license"""
            import traceback
            Logger().info("Engine get_license_status failed: (((%s)))" % (traceback.format_exc(e)))
            license_ok = False

        return license_ok

    def NotifyConnectionToCriticalEquipment(self, protocol, user, source,
                                            ip_source, login, device, ip,
                                            time, url):
        try:
            notif_data = {
                u'protocol': protocol,
                u'user': user,
                u'source': source,
                u'ip_source': ip_source,
                u'login': login,
                u'device': device,
                u'ip': ip,
                u'time': time
                }

            if not (url is None):
                notif_data[u'url'] = url

            Notify(self.wabengine, CX_EQUIPMENT, notif_data)
        except Exception, e:
            import traceback
            Logger().info("Engine NotifyConnectionToCriticalEquipment failed: (((%s)))" % (traceback.format_exc(e)))

    def NotifyPrimaryConnectionFailed(self, user, ip):
        try:
            notif_data = {
                u'user': user,
                u'ip': ip
                }

            Notify(self.wabengine, PRIMARY_CX_FAILED, notif_data)
        except Exception, e:
            import traceback
            Logger().info("Engine NotifyPrimaryConnectionFailed failed: (((%s)))" % (traceback.format_exc(e)))

    def NotifySecondaryConnectionFailed(self, user, ip, account, device):
        try:
            notif_data = {
                   u'user'   : user
                 , u'ip'     : ip
                 , u'account': account
                 , u'device' : device
             }

            Notify(self.wabengine, SECONDARY_CX_FAILED, notif_data)
        except Exception, e:
            import traceback
            Logger().info("Engine NotifySecondaryConnectionFailed failed: (((%s)))" % (traceback.format_exc(e)))

    def NotifyFilesystemIsFullOrUsedAtXPercent(self, filesystem, used):
        try:
            notif_data = {
                u'filesystem': filesystem,
                u'used': used
                }

            Notify(self.wabengine, FILESYSTEM_FULL, notif_data)
        except Exception, e:
            import traceback
            Logger().info("Engine NotifyFilesystemIsFullOrUsedAtXPercent failed: (((%s)))" % (traceback.format_exc(e)))

    def NotifyFindPatternInRDPFlow(self, regexp, string, user_login, user, host, cn):
        try:
            notif_data = {
                   u'regexp'     : regexp
                 , u'string'     : string
                 , u'user_login' : user_login
                 , u'user'       : user
                 , u'host'       : host
                 , u'device'     : cn
             }

            Notify(self.wabengine, RDP_PATTERN_FOUND, notif_data)
        except Exception, e:
            import traceback
            Logger().info("Engine NotifyFindPatternInRDPFlow failed: (((%s)))" % (traceback.format_exc(e)))

    def get_targets_list(self, group_filter, device_filter, protocol_filter,
                         real_target_device):
        targets = []
        item_filtered = False
        for target_info in self.displaytargets:
            temp_service_login                = target_info.service_login
            temp_resource_service_protocol_cn = target_info.protocol
            if not target_info.protocol == u"APP":
                if (target_info.target_name == u'autotest' or
                    target_info.target_name == u'bouncer2' or
                    target_info.target_name == u'widget2_message' or
                    target_info.target_name == u'widgettest' or
                    target_info.target_name == u'test_card'):
                    temp_service_login = target_info.service_login.replace(u':RDP',
                                                                           u':INTERNAL', 1)
                    temp_resource_service_protocol_cn = 'INTERNAL'

            if ((target_info.group.find(group_filter) == -1)
                or (temp_service_login.find(device_filter) == -1)
                or (temp_resource_service_protocol_cn.find(protocol_filter) == -1)):
                item_filtered = True
                continue

            if real_target_device:
                if target_info.protocol == u"APP":
                    continue
                if (target_info.host
                    and (not is_device_in_subnet(real_target_device,
                                                 target_info.host))):
                    continue

            targets.append((target_info.group # ( = concatenated list)
                            , temp_service_login
                            , temp_resource_service_protocol_cn
                            )
                           )
        return targets, item_filtered

    def reset_proxy_rights(self):
        self.proxy_rights = None
        self.rights = None

    def get_proxy_rights(self, protocols, target_device=None):
        if self.proxy_rights is not None:
            return
        self.proxy_rights = self.wabengine.get_proxy_rights(protocols, target_device)

        self.rights = self.proxy_rights.rights
        self.targets = {}
        self.displaytargets = []
        for right in self.rights:
            if right.resource and right.account:
                if right.resource.application:
                    target_name = right.resource.application.cn
                    service_name = u"APP"
                    protocol = u"APP"
                    host = None
                else:
                    target_name = right.resource.device.cn
                    service_name = right.resource.service.cn
                    protocol = right.resource.service.protocol.cn
                    host = right.resource.device.host
                tuple_index = (right.account.login, target_name)
                if not self.targets.get(tuple_index):
                    self.targets[tuple_index] = {}
                self.targets[tuple_index][service_name] = right
                self.displaytargets.append(DisplayInfo(right.account.login,
                                                       target_name,
                                                       service_name,
                                                       protocol,
                                                       [x.cn for x in right.group_targets],
                                                       right.subprotocols,
                                                       host))

    def get_selected_target(self, target_login, target_device, target_service):
        # Logger().info("%s@%s:%s" % (target_device, target_login, target_service))
        if target_service == '':
            target_service = None
        selected_target = None
        self.get_proxy_rights([u'RDP', u'VNC'], target_device)
        right = None
        if SESMANCONF[u'sesman'][u'auth_mode_passthrough'].lower() == u'true':
            for r in self.rights:
                if not is_device_in_subnet(target_device, r.resource.device.host):
                    continue
                if target_service != r.resource.service.cn:
                    continue
                right = r
                break
        else:
            result = self.targets.get((target_login, target_device))
            if result:
                if (not target_service) and (len(result) == 1):
                    right = result.values()[0]
                if target_service and result.get(target_service):
                    right = result[target_service]
        if right:
            self.init_timeframe(right)
            self.target_right = right
        return right

    def get_effective_target(self, selected_target):
        Logger().info("Engine get_effective_target: service_login=%s" % selected_target.service_login)
        try:
            if selected_target.resource.application:
                effective_target = self.wabengine.get_effective_target(selected_target)
                Logger().info("Engine get_effective_target done (application)")
                return effective_target
            else:
                Logger().info("Engine get_effective_target done (physical)")
                return [selected_target]

        except Exception, e:
            import traceback
            Logger().info("Engine get_effective_target failed: (((%s)))" % (traceback.format_exc(e)))
        return []

    def get_app_params(self, selected_target, effective_target):
#         Logger().info("Engine get_app_params: service_login=%s effective_target=%s" % (service_login, effective_target))
        Logger().info("Engine get_app_params: service_login=%s" % selected_target.service_login)
        try:
            app_params = self.wabengine.get_app_params(selected_target, effective_target)
            Logger().info("Engine get_app_params done")
            return app_params
        except Exception, e:
            import traceback
            Logger().info("Engine get_app_params failed: (((%s)))" % (traceback.format_exc(e)))
        return None

    def get_target_password(self, target_device):
#         Logger().info("Engine get_target_password: target_device=%s" % target_device)
        Logger().debug("Engine get_target_password ...")
        try:
            target_password = self.wabengine.get_target_password(target_device)

            if not target_password:
                target_password = u''
            Logger().debug("Engine get_target_password done")
            return target_password
        except Exception, e:
            import traceback
            Logger().debug("Engine get_target_password failed: (((%s)))" % (traceback.format_exc(e)))
        return u''

    def release_target_password(self, target_device, reason, target_application = None):
        Logger().debug("Engine release_target_password: reason=\"%s\"" % reason)
        try:
            self.wabengine.release_target_password(target_device, reason, target_application)
            Logger().debug("Engine release_target_password done")
        except Exception, e:
            import traceback
            Logger().info("Engine release_target_password failed: (((%s)))" % (traceback.format_exc(e)))

    def start_session(self, auth, pid, effective_login):
        try:
            try:
                from wabengine.common.interface import IPBSessionHandler
                from wabengine.common.utils import ProcessSessionHandler
                wab_engine_session_handler = IPBSessionHandler(ProcessSessionHandler(int(pid)))
            except Exception, e:
                wab_engine_session_handler = None
            self.session_id = self.wabengine.start_session(auth, wab_engine_session_handler, effective_login=effective_login)
        except Exception, e:
            import traceback
            Logger().info("Engine start_session failed: (((%s)))" % (traceback.format_exc(e)))

        return self.session_id

    def update_session(self, physical_target):
        hosttarget = u"%s@%s:%s" % ( physical_target.account.login
                                   , physical_target.resource.device.cn
                                   , physical_target.resource.service.protocol.cn)
        try:
            if self.session_id:
                self.wabengine.update_session(self.session_id, hosttarget)
        except Exception, e:
            import traceback
            Logger().info("Engine update_session failed: (((%s)))" % (traceback.format_exc(e)))

    def get_restrictions(self, auth, proxytype):
        if proxytype == "RDP":
            separator = u"\x01"
            matchproto = lambda x: x == u"RDP"
        elif proxytype == "SSH":
            separator = u"|"
            matchproto = lambda x: (x == self.subprotocol
                                    or (x == u'SSH_SHELL_SESSION'
                                        and self.subprotocol == u'SSH_X11_SESSION'))
        else:
            return None, None
        try:
            restrictions = self.wabengine.get_proxy_restrictions(auth)
            kill_patterns = []
            notify_patterns = []
            for restriction in restrictions:
                if not restriction.subprotocol:
                    Logger().error("No subprotocol in restriction!")
                    continue
                if matchproto(restriction.subprotocol.cn):
                    Logger().debug("adding restriction %s %s %s" % (restriction.action, restriction.data, restriction.subprotocol.cn))
                    if restriction.action == 'kill':
                        kill_patterns.append(restriction.data)
                    elif restriction.action == 'notify':
                        notify_patterns.append(restriction.data)

            self.pattern_kill = separator.join(kill_patterns)
            self.pattern_notify = separator.join(notify_patterns)
            Logger().info("pattern_kill = [%s]" % (self.pattern_kill))
            Logger().info("pattern_notify = [%s]" % (self.pattern_notify))
        except Exception, e:
            self.pattern_kill = None
            self.pattern_notify = None
            import traceback
            Logger().info("Engine get_restrictions failed: (((%s)))" % (traceback.format_exc(e)))
        return (self.pattern_kill, self.pattern_notify)

    def stop_session(self, result=True, diag=u"success", title=u"End session"):
        try:
            if self.session_id:
                self.wabengine.stop_session(self.session_id, result=result, diag=diag, title=title)
        except Exception, e:
            import traceback
            Logger().info("Engine stop_session failed: (((%s)))" % (traceback.format_exc(e)))

    def write_trace(self, video_path):
        try:
            _status, _error = True, TR(u"No error")
            if video_path:
                # Notify WabEngine with Trace file descriptor
                trace = self.wabengine.get_trace_writer(self.session_id, trace_type=u"rdptrc")
                trace.writeframe(str("%s.mwrm" % (video_path.encode('utf-8')) ) )
                trace.end()
        except Exception, e:
            Logger().info("Engine write_trace failed: %s" % e)
            _status, _error = False, TR(u"Trace writer failed for %s") % video_path

        return _status, _error

    def read_session_parameters(self, key=None):
        return self.wabengine.read_session_parameters(self.session_id, key=key)

    def get_target_protocols(self):
        if not self.target_right:
            return None
        proto = self.target_right.resource.service.protocol.cn
        subproto = [x.cn for x in self.target_right.subprotocols]
        return ProtocolInfo(proto, subproto)

    def get_target_extra_info(self):
        if not self.target_right:
            return None
        isRecorded = self.target_right.authorization.isRecorded
        isCritical = self.target_right.authorization.isCritical
        return ExtraInfo(isRecorded, isCritical)


    def get_target_agent_forwardable(self):
        if not self.target_right:
            return None
        return self.target_right.account.isAgentForwardable

    def get_physical_target_info(self, physical_target):
        return PhysicalTarget(device_host=physical_target.resource.device.host,
                              account_login=physical_target.account.login,
                              service_port=int(physical_target.resource.service.port))

    def get_target_login_info(self):
        if not self.target_right:
            return None
        physical_target = PhysicalTarget(
            device_host=self.target_right.resource.device.host,
            account_login=self.target_right.account.login,
            service_port=self.target_right.resource.service.port)
        if self.target_right.resource.application:
            target_name = self.target_right.resource.application.cn
        else:
            target_name = self.target_right.resource.device.cn
        service_name = self.target_right.resource.service.cn
        conn_cmd = self.target_right.resource.service.authmechanism.data
        autologon = self.target_right.account.password or self.target_right.account.isAgentForwardable
        return LoginInfo(physical_target=physical_target,
                         target_name=target_name,
                         service_name=service_name,
                         conn_cmd=conn_cmd,
                         autologon=autologon)


class DisplayInfo(object):
    def __init__(self, target_login, target_name, service_name,
                 protocol, group, subproto, host):
        self.target_login = target_login
        self.target_name = target_name
        self.service_name = service_name
        self.protocol = protocol
        # self.group = ";".join([x.cn for x in group])
        self.group = ";".join(group)
        self.service_login = "%s@%s:%s" % (self.target_login, self.target_name, self.service_name)
        self.subprotocols = [x.cn for x in subproto] if subproto else []
        self.host = host

class ProtocolInfo(object):
    def __init__(self, protocol, subprotocols=[]):
        self.protocol = protocol
        self.subprotocols = subprotocols

class ExtraInfo(object):
    def __init__(self, is_recorded, is_critical):
        self.is_recorded = is_recorded
        self.is_critical = is_critical

class PhysicalTarget(object):
    def __init__(self, device_host, account_login, service_port):
        self.device_host = device_host
        self.account_login = account_login
        self.service_port = service_port

class LoginInfo(object):
    def __init__(self, physical_target, target_name, service_name,
                 conn_cmd, autologon):
        self.physical_target = physical_target
        self.target_name = target_name
        self.service_name = service_name
        self.conn_cmd = conn_cmd
        self.autologon = autologon
