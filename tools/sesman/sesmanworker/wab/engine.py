#!/usr/bin/python

from wabengine.common.exception import AuthenticationFailed
from logger import Logger

from model import RightInfo, UserInfo

class Engine(object):

    def __init__(self):
        self.wabengine = None
        self.session_id  = None
        self.auth_x509 = None
        self._trace_encryption = None

    def get_trace_encryption(self):
        try:
            from wabconfig import Config
            conf = Config("wabengine")
            self._trace_encryption = True if conf['trace'] == u'cryptofile' else False # u'localfile'
            return self._trace_encryption
        except Exception, e:
            import traceback
            Logger().info("Reading configuration file section 'wabengine', key 'trace' failed : %s" % traceback.format_exc(e))
        return False

    def password_expiration_date(self):
        _data = self.wabengine.check_password_expiration_info()
        if _data[2]:
            if _data[0] > 1:
                #TODO: Add translations for these messages
                return u'Your password will expire in %s days. Please change it.' % _data[0]
            else:
                #TODO: Add translations for these messages
                return u'Your password will expire soon. Please change it.'
        return False

    def is_x509_connected(self, wab_login, ip_client, proxy_type):
        """
        Ask if we are authentifying using x509
        (and ask user by opening confirmation popup if we are,
        session ticket will be asked later in x509_authenticate)
        """
        try:
            from wabx509 import AuthX509
            self.auth_x509 = AuthX509(wab_login, ip_client, proxy_type)
            result = self.auth_x509.is_connected()
            return result
        except Exception, e:
            import traceback
            Logger().info("Engine is_x509_connected failed (((%s)))" % traceback.format_exc(e))
        return False

    def x509_authenticate(self):
        try:
            self.wabengine = self.auth_x509.get_proxy()
            if self.wabengine is not None:
                self.user = self.wabengine.who_am_i()
                return True
        except AuthenticationFailed, e:
            pass
        except Exception, e:
            import traceback
            Logger().info("Engine x509_authenticate failed (((%s)))" % traceback.format_exc(e))
        return False


    def password_authenticate(self, wab_login, ip_client, password):
        try:
            from wabengine.client.sync_client import SynClient
            self.client = SynClient('localhost', 'tcp:8803')
            self.wabengine = self.client.authenticate(wab_login, password, ip_client)
            if self.wabengine is not None:
                self.user = self.wabengine.who_am_i()
                return True
        except AuthenticationFailed, e:
            pass
        except Exception, e:
            import traceback
            Logger().info("Engine password_authenticate failed (((%s)))" % traceback.format_exc(e))
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
            Logger().info("Unexpected licence error %s" % (traceback.format_exc(e)))
            license_ok = False

        return license_ok

    def NotifyConnectionToCriticalEquipment(self, protocol, user, source,
            ip_source, login, device, ip, time, url):
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

    def NotifyPrimaryConnectionFailed(self, user, ip):
        notif_data = {
               u'user' : user
             , u'ip'   : ip
         }

        from wallixgenericnotifier import Notify, PRIMARY_CX_FAILED
        Notify(self.wabengine, PRIMARY_CX_FAILED, notif_data)

    def NotifySecondaryConnectionFailed(self, user, device):
        notif_data = {
               u'user'   : user
             , u'device' : device
         }

        from wallixgenericnotifier import Notify, SECONDARY_CX_FAILED
        Notify(self.wabengine, SECONDARY_CX_FAILED, notif_data)

    def NotifyFilesystemIsFullOrUsedAtXPercent(self, filesystem, used):
        notif_data = {
               u'filesystem' : filesystem
             , u'used'       : used
         }

        from wallixgenericnotifier import Notify, FILESYSTEM_FULL
        Notify(self.wabengine, FILESYSTEM_FULL, notif_data)

    def NotifyFindPatternInRDPFlow(self, regexp, string, user_login, user, host):
        notif_data = {
               u'regexp'     : regexp
             , u'string'     : string
             , u'user_login' : user_login
             , u'user'       : user
             , u'host'       : host
         }

        from wallixgenericnotifier import Notify, RDP_PATTERN_FOUND
        Notify(self.wabengine, RDP_PATTERN_FOUND, notif_data)

    def get_proxy_rights(self, protocols):
        self.proxy_rights = self.wabengine.get_proxy_rights(protocols)

        self.user = self.proxy_rights.user
        #u = UserInfo(user)
        #Logger().info("%r" % u)

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

    def get_effective_target(self, service_login):
        Logger().info("service_login=%s" % service_login)
        effective_target = self.wabengine.get_effective_target(service_login)
        # Logger().info("effective_target=%r" % effective_target)

#        for r in effective_target:
#            rrr = RightInfo(r)
#            Logger().info("%r" % rrr)

        return effective_target

    def get_app_params(self, service_login, effective_target):
        Logger().info("service_login=%s effective_target=%s" % (service_login, effective_target))
        app_params = self.wabengine.get_app_params(service_login, effective_target)
        Logger().info("app_params=%s" % (app_params.__dict__))
        # rrr = AppParamsInfo(app_params)
        # Logger().info("app_params=%r" % rrr)
        return app_params

    def start_session(self, auth, pid):
        try:
            from wabengine.common.interface import IPBSessionHandler
            from wabengine.common.utils import ProcessSessionHandler
            wab_engine_session_handler = IPBSessionHandler(ProcessSessionHandler(int(pid)))
            self.session_id = self.wabengine.start_session(auth, wab_engine_session_handler)
        except Exception, e:
            import traceback
            Logger().info("<<<<%s>>>>" % e)

        return self.session_id

    def update_session(self, hosttarget):
        try:
            if self.session_id:
                self.wabengine.update_session(self.session_id, hosttarget)
        except Exception, e:
            import traceback
            Logger().info("<<<<%s>>>>" % e)


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

            self.patterns_kill = u"|".join(kill_patterns)
            self.patterns_notify = u"|".join(notify_patterns)
            Logger().info("kill_patterns = [%s]" % (self.patterns_kill))
            Logger().info("notify_patterns = [%s]" % (self.patterns_notify))
        except Exception, e:
            import traceback
            Logger().info("<<<<%s>>>>" % e)
        


    def stop_session(self, result=True, diag=u"success", title=u"End session"):
        if self.session_id:
            self.wabengine.stop_session(self.session_id, result=result, diag=diag, title=title)

    def write_trace(self, video_path):
        try:
            _status, _error = True, u"no error"
            if video_path:
                # Notify WabEngine with Trace file descriptor
                trace = self.wabengine.get_trace_writer(self.session_id, trace_type=u"rdptrc")
                trace.writeframe(str("%s.mwrm" % (video_path.encode('utf-8')) ) )
                trace.end()
        except Exception, e:
            import traceback
            Logger().info("<<<<<<%s>>>>>" % traceback.format_exc(e))
            _status, _error = False, u"Trace writer failed for %s" % video_path

        return _status, _error
