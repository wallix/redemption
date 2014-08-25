#!/usr/bin/python
# -*- coding: UTF-8 -*-

from sesmanconf import TR

class AuthenticationFailed(Exception): pass
from logger import Logger
from targetconf import TargetConf

class ChallengeInfo(object):
    def __init__(self, message=None, promptEcho=None):
        self.message = message
        self.promptEcho = promptEcho

class Engine(object):

    def __init__(self):
        class User:
            def __init__(self, preferredLanguage = u'en'):
                self.preferredLanguage = preferredLanguage
                self.cn = None
        Logger().info("Engine constructor")
        self.wab_login = None
        self.user = User()
        self._trace_encryption = False
        self.challenge = None
        self.rights = None

    def get_force_change_password(self):
        return False

    def get_language(self):
        return self.user.preferredLanguage

    def get_wabuser_name(self):
        return self.wab_login

    def get_trace_encryption(self):
        return self._trace_encryption

    def password_expiration_date(self):
        #return false or number of days
        return False, 0

    def is_x509_connected(self, wab_login, ip_client, proxy_type, target, server_ip):
        """
        Ask if we are authentifying using x509
        (and ask user by opening confirmation popup if we are,
        session ticket will be asked later in x509_authenticate)
        """
        print ('is_x509_connected(%s %s %s %s)' % (wab_login, ip_client, proxy_type, target))
        res = False
        try:
            res = TargetConf().config_users[wab_login]['is_x509_connected']
            if res:
                self.wab_login = wab_login
        except:
            pass
        return res

    def x509_authenticate(self):
        print ('x509_authenticate(%s)' % self.wab_login)
        try:
            if TargetConf().config_users[self.wab_login]['x509_authenticate']:
                self.user.preferredLanguage = TargetConf().config_users[self.wab_login][u'preferredLanguage']
                return True
            return False
        except:
            self.wab_login = None
        return False

    def password_authenticate(self, wab_login, ip_client, password, server_ip):
        print ('password_authenticate(%s %s %s)' % (wab_login, ip_client, password))
        res = False
        try:
            # print ('::::: %s' % (TargetConf().config_users[wab_login]))
            if wab_login == 'challenge':
                res = self.challenge_manage(wab_login, password)
            else:
                res = TargetConf().config_users[wab_login]['password'] == password
            if res:
                Logger().info("password authenticated Succeed for %s" % wab_login)
                self.wab_login = wab_login
                self.user.preferredLanguage = TargetConf().config_users[wab_login][u'preferredLanguage']
        except:
            pass
        return res

    def passthrough_authenticate(self, wab_login, ip_client, server_ip):
        print ('passthrough_authenticate(%s %s)' % (wab_login, ip_client))
        try:
            if TargetConf().config_users[self.wab_login]['passthrough_authenticate']:
                self.user.preferredLanguage = TargetConf().config_users[self.wab_login][u'preferredLanguage']
                return True
            return False
        except:
            self.wab_login = None
        return False

    def get_license_status(self):
        return True

    def challenge_manage(self, wab_login, password):
        res = False
        if not self.challenge:
            res = TargetConf().config_users[wab_login]['password'] == password
            if res:
                self.challenge = ChallengeInfo("""When Littlefoot's Mother died in the original
'Land Before Time', did you feel sad ?
(Bots: No lying)""",
                                               False)
                res = False
        else:
            res = TargetConf().config_users[wab_login]['response'] == password
            self.challenge = None
        return res



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

        Logger().info(u"NotifyConnectionToCriticalEquipment: %r" % notif_data)

    def NotifyPrimaryConnectionFailed(self, user, ip):
        notif_data = {
               u'user' : user
             , u'ip'   : ip
         }

        Logger().info(u"NotifyPrimaryConnectionFailed: %r" % notif_data)

    def NotifySecondaryConnectionFailed(self, user, ip, account, device):
        notif_data = {
               u'user'   : user
             , u'ip'     : ip
             , u'account': account
             , u'device' : device
         }

        Logger().info(u"NotifySecondaryConnectionFailed: %r" % notif_data)

    def NotifyFilesystemIsFullOrUsedAtXPercent(self, filesystem, used):
        notif_data = {
               u'filesystem' : filesystem
             , u'used'       : used
         }

        Logger().info(u"NotifyFilesystemIsFullOrUsedAtXPercent: %r" % notif_data)

    def NotifyFindPatternInRDPFlow(self, regexp, string, user_login, user, host, cn):
        notif_data = {
               u'regexp'     : regexp
             , u'string'     : string
             , u'user_login' : user_login
             , u'user'       : user
             , u'host'       : host
             , u'device'     : cn
         }

        Logger().info(u"NotifyFindPatternInRDPFlow: %r" % notif_data)

    def get_proxy_rights(self, protocols, target_device=None):
        if self.rights:
            return
        self.rights = [ TargetConf().config_rights[r] for r in TargetConf().config_users[self.wab_login]['rights']]
        return self.rights

    def get_selected_target(self, target_login, target_device, target_protocol):
        if target_protocol == '':
            target_protocol = None
        selected_target = None
        self.get_proxy_rights([u'RDP', u'VNC'], target_device)
        for r in self.rights:
            if r.resource.application:
                if target_device != r.resource.application.cn:
                    continue
                if target_login != r.account.login:
                    continue
                if target_protocol != u'APP':
                    continue
            else:
                if target_device != r.resource.device.cn:
                    continue
                if target_login != r.account.login:
                    continue
                if target_protocol != r.resource.service.cn:
                    continue
            selected_target = r
            break

        return selected_target

    def get_effective_target(self, selected_target):
        service_login = selected_target.service_login
        Logger().info("Effective_target %s" % service_login)
        try:
            if selected_target.resource.application:
                res = [ TargetConf().config_effective_targets[r] for r in TargetConf().config_service_logins[service_login]['rights']]
                Logger().info("Engine get_effective_target done (application)")
                return effective_target
            else:
                Logger().info("Engine get_effective_target done (physical)")
                return [selected_target]

        except Exception, e:
            import traceback
            Logger().info("%s" % traceback.format_exc(e))
#        Logger().info("Effective_target ok %r" % res)
        return res

    def get_app_params(self, selected_target, effective_target):
        service_login = selected_target.service_login
        res = None
#        Logger().info("get_app_params %s" % service_login)
        try:
            for key, value in TargetConf().config_effective_targets.items():
#                Logger().info('key=%s value=%s' % (key, value))
                if value  is effective_target:
                    res = TargetConf().config_app_params[service_login][key]
        except Exception, e:
            import traceback
            Logger().info("%s" % traceback.format_exc(e))
#        Logger().info("get_app_params done = %s" % res)
        return res

    def get_target_password(self, target_device):
        res = None
        Logger().info("get_target_password ...")
        Logger().info("account =%s" % target_device.account.login)
        Logger().info("resource=%s" % target_device.resource.device.cn)
        Logger().info("protocol=%s" % target_device.resource.service.protocol.cn)
        try:
            for p in TargetConf().config_target_password:
                if (p.account == target_device.account.login and
                   p.resource == target_device.resource.device.cn and
                   p.protocol == target_device.resource.service.protocol.cn):
                    res = p.password
        except Exception, e:
            import traceback
            Logger().info("%s" % traceback.format_exc(e))
        Logger().info("get_target_password done = %s" % res)
        return res

    def release_target_password(self, target_device, reason, target_application = None):
        Logger().info("release_target_password done: target_device=\"%s\" reason=\"%s\"" %
            (target_device, reason))

    def start_session(self, target, pid, effective_login):
        return "SESSIONID-0000"

    def get_restrictions(self, target):
        self.pattern_kill = u""
        self.pattern_notify = u""
        return

    def update_session(self, target31):
        pass

    def stop_session(self, result=True, diag=u"success", title=u"End session"):
        pass

    def write_trace(self, video_path):
        return True, ""

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

    def read_session_parameters(self, key=None):
        return {"rt_display": "1"}
