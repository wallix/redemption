#!/usr/bin/python

from sesmanconf import TR
from sesmanconf import TR, SESMANCONF, translations

from wabengine.common.exception import AuthenticationFailed
from wabengine.common.exception import AuthenticationChallenged
from wabengine.common.exception import MustChangePassword
from logger import Logger

from model import RightInfo, UserInfo
from sesmanconf import TR, SESMANCONF, translations

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
        self.session_id  = None
        self.auth_x509 = None
        self._trace_encryption = None
        self.challenge = None
        self.proxy_rights = None
        # Logger().info("=========================")
        # Logger().info("==== RDP ENGINE (WAB) ===")
        # Logger().info("=========================")

    def get_language(self):
        try:
            return self.wabuser.preferredLanguage
        except Exception, e:
            return 'en'

    def get_wabuser_name(self):
        try:
            return self.wabuser.cn
        except Exception, e:
            return ""

    def get_force_change_password(self):
        try:
            return self.wabuser.forceChangePwd
        except Exception, e:
            return False

    def get_trace_encryption(self):
        try:
            from wabconfig import Config
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
            from wabx509 import AuthX509
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
            from wabengine.client.sync_client import SynClient
            self.client = SynClient('localhost', 'tcp:8803')
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
            from wabengine.client.sync_client import SynClient
            self.client = SynClient('localhost', 'tcp:8803')
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
            from wallixgenericnotifier import Notify, LICENCE_EXPIRED, LICENCE_PRIMARY_CX_ERROR, LICENCE_SECONDARY_CX_ERROR
            lic_status = self.wabengine.get_license_status()

            if lic_status.is_expired():
                Logger().info("LICENCE_EXPIRED")
                Notify(self.wabengine, LICENCE_EXPIRED, u"")
                license_ok = False
            if lic_status.is_primary_limit_reached():
                Logger().info("PRIMARY LICENCE LIMIT")
                Notify(self.wabengine, LICENCE_PRIMARY_CX_ERROR, { u'nbPrimaryConnection' : lic_status.primary()[0] })
                license_ok = False
            if lic_status.is_secondary_limit_reached():
                Logger().info("SECONDARY LICENCE LIMIT")
                Notify(self.wabengine, LICENCE_SECONDARY_CX_ERROR, { u'nbSecondaryConnection' : lic_status.secondary()[0] })
                license_ok = False
        except Exception, e:
            """If calling get_license_status raise some error, user will be rejected as per invalid license"""
            import traceback
            Logger().info("Engine get_license_status failed: (((%s)))" % (traceback.format_exc(e)))
            license_ok = False

        return license_ok

    def NotifyConnectionToCriticalEquipment(self, protocol, user, source,
            ip_source, login, device, ip, time, url):
        try:
            notif_data = {
                   u'protocol' : protocol
                 , u'user'     : user
                 , u'source'   : source
                 , u'ip_source': ip_source
                 , u'login'    : login
                 , u'device'   : device
                 , u'ip'       : ip
                 , u'time'     : time
             }

            if not (url is None):
                notif_data[u'url'] = url

            from wallixgenericnotifier import Notify, CX_EQUIPMENT
            Notify(self.wabengine, CX_EQUIPMENT, notif_data)
        except Exception, e:
            import traceback
            Logger().info("Engine NotifyConnectionToCriticalEquipment failed: (((%s)))" % (traceback.format_exc(e)))

    def NotifyPrimaryConnectionFailed(self, user, ip):
        try:
            notif_data = {
                   u'user' : user
                 , u'ip'   : ip
             }

            from wallixgenericnotifier import Notify, PRIMARY_CX_FAILED
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

            from wallixgenericnotifier import Notify, SECONDARY_CX_FAILED
            Notify(self.wabengine, SECONDARY_CX_FAILED, notif_data)
        except Exception, e:
            import traceback
            Logger().info("Engine NotifySecondaryConnectionFailed failed: (((%s)))" % (traceback.format_exc(e)))

    def NotifyFilesystemIsFullOrUsedAtXPercent(self, filesystem, used):
        try:
            notif_data = {
                   u'filesystem' : filesystem
                 , u'used'       : used
             }

            from wallixgenericnotifier import Notify, FILESYSTEM_FULL
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

            from wallixgenericnotifier import Notify, RDP_PATTERN_FOUND
            Notify(self.wabengine, RDP_PATTERN_FOUND, notif_data)
        except Exception, e:
            import traceback
            Logger().info("Engine NotifyFindPatternInRDPFlow failed: (((%s)))" % (traceback.format_exc(e)))

    def get_targets_list(self, group_filter, device_filter, protocol_filter,
                         real_target_device):
        targets = []
        item_filtered = False
        for right in self.rights:
            if not right.resource.application:
                if (right.resource.device.host == u'autotest' or
                    right.resource.device.host == u'bouncer2' or
                    right.resource.device.host == u'widget2_message' or
                    right.resource.device.host == u'widgettest' or
                    right.resource.device.host == u'test_card'):
                    temp_service_login                = right.service_login.replace(u':RDP', u':INTERNAL', 1)
                    temp_resource_service_protocol_cn = 'INTERNAL'
                    temp_resource_device_cn           = right.resource.device.cn
                else:
                    temp_service_login                = right.service_login
                    temp_resource_service_protocol_cn = right.resource.service.protocol.cn
                    temp_resource_device_cn           = right.resource.device.cn
            else:
                temp_service_login                = right.service_login + u':APP'
                temp_resource_service_protocol_cn = u'APP'
                temp_resource_device_cn           = right.resource.application.cn

            if ((right.target_groups.find(group_filter) == -1)
                or (temp_service_login.find(device_filter) == -1)
                or (temp_resource_service_protocol_cn.find(protocol_filter) == -1)):
                item_filtered = True
                continue

            if real_target_device:
                if right.resource.application:
                    continue
                if (right.resource.device
                    and (not is_device_in_subnet(real_target_device,
                                                 right.resource.device.host))):
                    continue

            targets.append((right.target_groups # ( = concatenated list)
                            , temp_service_login
                            , temp_resource_service_protocol_cn
                            , (right.deconnection_time if right.deconnection_time[0:4] < "2034" else u"-")
                            , temp_resource_device_cn
                            )
                           )
        return targets, item_filtered

    def get_proxy_rights(self, protocols, target_device=None):
        if self.proxy_rights is not None:
            return
        self.proxy_rights = self.wabengine.get_proxy_rights(protocols, target_device)

        self.rights = self.proxy_rights.rights
        # gather target group names in one string
        for idx, r in enumerate(self.rights):
            tg = r.group_targets[0].cn
            for g in r.group_targets[1:]:
                tg = u"%s;%s" % (tg, g.cn)
            self.rights[idx].target_groups = tg

#        for r in self.rights:
#            rrr = RightInfo(r)
#            Logger().info("%r" % rrr)

    def get_selected_target(self, target_login, target_device, target_service):
        # Logger().info("%s@%s:%s" % (target_device, target_login, target_service))
        if target_service == '':
            target_service = None
        selected_target = None
        self.get_proxy_rights([u'RDP', u'VNC'], target_device)
        for r in self.rights:
            if r.resource.application:
                if target_device != r.resource.application.cn:
                    continue
                if target_login != r.account.login:
                    continue
                if target_service != u'APP':
                    continue
            else:
                #Logger().info("%s@%s:%s host=%s" % (r.account.login, r.resource.device.cn, r.resource.service.cn, r.resource.device.host))
                if SESMANCONF[u'sesman'][u'auth_mode_passthrough'].lower() == u'true':
                    if not is_device_in_subnet(target_device, r.resource.device.host):
                        continue
                    #Allow any user
                    if target_service != r.resource.service.cn:
                        continue
                else:
                    if target_device != r.resource.device.cn:
                        continue
                    if target_login != r.account.login:
                        continue
                    if target_service != r.resource.service.cn:
                        continue
            # Logger().info("Found %s@%s:%s" % (r.account.login, r.resource.device.cn, r.resource.service.cn))
            selected_target = r
            break

        return selected_target

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
            # Logger().info("effective_target=%r" % effective_target)

#            for r in effective_target:
#                rrr = RightInfo(r)
#                Logger().info("%r" % rrr)

        except Exception, e:
            import traceback
            Logger().info("Engine get_effective_target failed: (((%s)))" % (traceback.format_exc(e)))
        return []

    def get_app_params(self, selected_target, effective_target):
#         Logger().info("Engine get_app_params: service_login=%s effective_target=%s" % (service_login, effective_target))
        Logger().info("Engine get_app_params: service_login=%s" % selected_target.service_login)
        try:
            app_params = self.wabengine.get_app_params(selected_target, effective_target)
            # Logger().info("app_params=%s" % (app_params.__dict__))
            # rrr = AppParamsInfo(app_params)
            # Logger().info("app_params=%r" % rrr)
            Logger().info("Engine get_app_params done")
            return app_params
        except Exception, e:
            import traceback
            Logger().info("Engine get_app_params failed: (((%s)))" % (traceback.format_exc(e)))
        return None

    def get_target_password(self, target_device):
#         Logger().info("Engine get_target_password: target_device=%s" % target_device)
        Logger().info("Engine get_target_password ...")
        try:
            target_password = self.wabengine.get_target_password(target_device)

            if not target_password:
                target_password = u''
            Logger().info("Engine get_target_password done")
            return target_password
        except Exception, e:
            import traceback
            Logger().info("Engine get_target_password failed: (((%s)))" % (traceback.format_exc(e)))
        return u''

    def release_target_password(self, target_device, reason, target_application = None):
        Logger().info("Engine release_target_password: reason=\"%s\"" % reason)
        try:
            self.wabengine.release_target_password(target_device, reason, target_application)
            Logger().info("Engine release_target_password done")
        except Exception, e:
            import traceback
            Logger().info("Engine release_target_password failed: (((%s)))" % (traceback.format_exc(e)))

    def start_session(self, auth, pid, effective_login):
        try:
            from wabengine.common.interface import IPBSessionHandler
            from wabengine.common.utils import ProcessSessionHandler
            wab_engine_session_handler = IPBSessionHandler(ProcessSessionHandler(int(pid)))
            self.session_id = self.wabengine.start_session(auth, wab_engine_session_handler, effective_login=effective_login)
        except Exception, e:
            import traceback
            Logger().info("Engine start_session failed: (((%s)))" % (traceback.format_exc(e)))

        return self.session_id

    def update_session(self, hosttarget):
        try:
            if self.session_id:
                self.wabengine.update_session(self.session_id, hosttarget)
        except Exception, e:
            import traceback
            Logger().info("Engine update_session failed: (((%s)))" % (traceback.format_exc(e)))

    def get_restrictions(self, auth):
        try:
            restrictions = self.wabengine.get_proxy_restrictions(auth)
            kill_patterns = []
            notify_patterns = []
            for restriction in restrictions:
                if not restriction.subprotocol:
                    Logger().error("No subprotocol in restriction!")
                    continue
                if restriction.subprotocol.cn == u'RDP':
                    Logger().debug("adding restriction %s %s %s" % (restriction.action, restriction.data, restriction.subprotocol.cn))
                    if restriction.action == 'kill':
                        kill_patterns.append(restriction.data)
                    elif restriction.action == 'notify':
                        notify_patterns.append(restriction.data)

            self.pattern_kill = u"\x01".join(kill_patterns)
            self.pattern_notify = u"\x01".join(notify_patterns)
            Logger().info("pattern_kill = [%s]" % (self.pattern_kill))
            Logger().info("pattern_notify = [%s]" % (self.pattern_notify))
        except Exception, e:
            import traceback
            Logger().info("Engine get_restrictions failed: (((%s)))" % (traceback.format_exc(e)))

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
