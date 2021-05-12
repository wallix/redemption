#!/usr/bin/python
# -*- coding: utf-8 -*-

from __future__ import absolute_import
from logger import Logger
try:
    from wabengine.common.exception import AuthenticationFailed
    from wabengine.common.exception import AuthenticationChallenged
    from wabengine.common.exception import MultiFactorAuthentication
    from wabengine.common.exception import AuthenticationUpdatePwd
    from wabengine.common.exception import LicenseException
    from wabengine.common.exception import MustChangePassword
    from wabengine.common.exception import AccountLocked
    from wabengine.common.exception import SessionAlreadyStopped
    from wallixgenericnotifier import Notify, CX_EQUIPMENT, PATTERN_FOUND, \
        PRIMARY_CX_FAILED, SECONDARY_CX_FAILED, NEW_FINGERPRINT,\
        WRONG_FINGERPRINT, RDP_PATTERN_FOUND, RDP_PROCESS_FOUND, \
        RDP_OUTCXN_FOUND, FILESYSTEM_FULL
    from wabconfig import Config
    from wabengine.client.sync_client import SynClient
    from wallixconst.authentication import PASSWORD_VAULT, \
        PASSWORD_INTERACTIVE, PUBKEY_VAULT, PUBKEY_AGENT_FORWARDING, \
        KERBEROS_FORWARDING, PASSWORD_MAPPING, SUPPORTED_AUTHENTICATION_METHODS
    from wallixconst.account import AM_IL_DOMAIN
    from wallixconst.trace import LOCAL_TRACE_PATH_RDP
    from wabx509 import AuthX509
    CRED_DATA_LOGIN = "login"
    CRED_DATA_ACCOUNT_UID = "account_uid"
    CRED_INDEX = "credentials"
    APPREQ_REQUIRED = 1
    APPREQ_OPTIONAL = 0
except Exception:
    import traceback
    tracelog = traceback.format_exc()
    try:
        from .fake.proxyengine import *
        LOCAL_TRACE_PATH_RDP = u'/var/wab/recorded/rdp/'
        Logger().info("================================")
        Logger().info("==== Load Fake PROXY ENGINE ====")
        Logger().info("================================")
    except Exception:
        # Logger().info("FAKE LOADING FAILED>>>>>> %s" %
        #               traceback.format_exc())
        Logger().info("WABENGINE LOADING FAILED>>>>>> %s" % tracelog)


from .logtime import logtime_function_pause
import time
import socket
from .wallixauth import Authenticator
from .checkout import CheckoutEngine
from .checkout import (
    APPROVAL_ACCEPTED,
    APPROVAL_REJECTED,
    APPROVAL_PENDING,
    APPROVAL_NONE,
    SHADOW_ACCEPTED,
    SHADOW_REJECTED,
)
from . import targetaccuratefilter as taf

DEFAULT_CONF_DIR = "/var/wab/etc/"
DEFAULT_SPEC_DIR = "/opt/wab/share/conf/"

FINGERPRINT_SHA1 = 0
FINGERPRINT_MD5 = 1
FINGERPRINT_MD5_LEN = 16
FINGERPRINT_SHA1_LEN = 20


def _binary_ip(network, bits):
    # TODO need Ipv6 support
    # This is a bit too resilient, add check for obviously bad values
    a, b, c, d = [int(x) & 0xFF for x in network.split('.')]
    mask = (0xFFFFFFFF >> bits) ^ 0xFFFFFFFF
    return ((a << 24) + (b << 16) + (c << 8) + d) & mask


def is_device_in_subnet(device, subnet):
    if subnet is None:
        return False
    if '/' in subnet:
        try:
            network, bits = subnet.rsplit('/')
            network_bits = _binary_ip(network, int(bits))
            device_bits = _binary_ip(device, int(bits))
            result = network_bits == device_bits
        except Exception as e:
            Logger().error("Bad host definition device '%s' subnet '%s': %s" %
                           (device, subnet, str(e)))
            result = False
    else:
        result = device == subnet
    Logger().debug("checking if device %s is in subnet %s -> %s" %
                   (device, subnet, ['No', 'Yes'][result]))
    return result


def resolve_reverse_dns(ip_str):
    found_fqdn = None
    try:
        found_fqdn = socket.gethostbyaddr(ip_str)[0]
    except Exception:
        Logger().debug("Unable to reverse dns %s" % ip_str)
    else:
        Logger().debug("Found fqdn %s for %s" % (found_fqdn, ip_str))
    return found_fqdn


def read_config_file(modulename="sesman",
                     confdir=DEFAULT_CONF_DIR,
                     specdir=DEFAULT_SPEC_DIR):
    return Config(modulename=modulename, confdir=confdir, specdir=specdir)


def decode_rawtext_data(data):
    encoding_try = ['utf-8', 'iso-8859-1']
    for encoding in encoding_try:
        try:
            data = data.decode(encoding)
        except Exception:
            continue
        break
    return data

class Engine(object):
    def __init__(self, legacy_auth=False):
        self.wabengine = None
        self.checkout = None
        self.user_cn = None
        self.user_lang = None
        self.wabengine_conf = Config('wabengine')
        self.authenticator = Authenticator(
            self.wabengine_conf.get(
                'port',
                'unix:/run/wabengine/wabengine.sock'
            )
        )
        if legacy_auth:
            from .legacyauth import LegacyAuthenticator
            self.authenticator = LegacyAuthenticator(
                self.wabengine_conf.get(
                    'port',
                    'unix:/run/wabengine/wabengine.sock'
                )
            )
        self.session_id = None
        self.auth_x509 = None
        self._trace_type = None                 # local ?
        self._selector_banner = {}
        self.challenge = None
        self.session_record = None
        self.session_record_type = None
        self.deconnection_epoch = 0xffffffff
        self.deconnection_time = u"-"
        self.start_time = None

        self.proxy_rights = None
        self.rights = None
        self.targets = {}
        self.targetsdom = {}
        self.target_right = None

        self.physical_targets = []
        self.displaytargets = []
        self.proxyrightsinput = None
        self.pidhandler = None

        self.session_result = True
        self.session_diag = u'Success'
        self.trace_hash = None
        self.failed_secondary_set = False

        self.service = None

        self.checktarget_cache = None
        self.checktarget_infos_cache = None

    def _post_authentication(self):
        self.update_user()
        self.checkout = CheckoutEngine(self.wabengine)

    def update_user(self):
        userobj = self.wabengine.who_am_i()
        self.user_cn = userobj.cn
        self.user_lang = userobj.preferredLanguage

    def set_session_status(self, result=None, diag=None):
        # Logger().info("Engine set session status : result='%s', diag='%s'" %
        #               (result, diag))
        if result is not None:
            self.session_result = result
        if diag is not None:
            self.session_diag = diag

    def get_language(self):
        try:
            if not self.user_lang:
                self.update_user()
        except Exception:
            pass
        return self.user_lang or 'en'

    def get_username(self):
        try:
            if not self.user_cn:
                self.update_user()
        except Exception:
            pass
        return self.user_cn or ""

    def get_otp_client(self):
        try:
            return self.wabengine.get_otp_client()
        except Exception:
            return ''

    def check_license(self):
        res = self.wabengine.is_session_management_license()
        if not res:
            Logger().info("License Error: "
                          "Session management License is not available.")
        return res

    @staticmethod
    def format_terminal(message):
        message += '\n'
        message = message.replace('\n', '\r\n')
        return message

    def get_banner(self, lang=None, format_terminal=False):
        if not lang:
            lang = self.get_language()
        banner = (u"Warning! Your remotée session may be recorded and "
                  u"kept in electronic format.")
        try:
            motdfile = '/var/wab/etc/proxys/messages/motd.%s' % lang
            with open(motdfile, "rb") as f:
                banner = f.read()
            banner = decode_rawtext_data(banner)
        except IOError:
            pass
        if format_terminal:
            banner = self.format_terminal(banner)
        return banner

    def get_warning_message(self, lang=None, format_terminal=False):
        if lang == 'en':
            msg = (u"Warning! Unauthorizedé access to this system is"
                   u" forbidden and will be prosecuted by law.")
        elif lang == 'fr':
            msg = (u"Attention! Toute tentative d'acces sans"
                   u" autorisation ou de maintien frauduleux"
                   u" dans ce systeme fera l'objet de"
                   u" poursuites judiciaires.")
        try:
            loginfile = '/var/wab/etc/proxys/messages/login.%s' % lang
            with open(loginfile, "rb") as f:
                msg = f.read()
            msg = decode_rawtext_data(msg)
        except IOError:
            pass
        if format_terminal:
            msg = self.format_terminal(msg)
        return msg

    def get_deconnection_time_msg(self, lang):
        message = ""
        if self.deconnection_time and self.deconnection_time != '-':
            if lang == 'fr':
                message = (u"\033[1mAttention\033[0m: Cette connexion sera "
                           u"coupée à %s.\r\n" %
                           str(self.deconnection_time))
            else:
                message = (u"\033[1mWarning\033[0m: This connection will "
                           u"be closed at %s.\r\n" %
                           str(self.deconnection_time))
        return message

    def check_device_fingerprint(self, hname, service_cn, finger_raw,
                                 hash_type=FINGERPRINT_SHA1):
        finger_host = ''
        fingerprint_len = (FINGERPRINT_SHA1_LEN
                           if hash_type == FINGERPRINT_SHA1
                           else FINGERPRINT_MD5_LEN)
        for i, char in enumerate(finger_raw):
            try:
                finger_host += '%02x' % ord(char)
            except Exception:
                finger_host += '%02x' % char
            if i < fingerprint_len - 1:
                finger_host += ':'

        finger = self.wabengine.get_fingerprint(hname, service_cn)
        if not finger:
            self.wabengine.save_fingerprint(
                hname,
                service_cn,
                finger_host
            )
            Notify(self.wabengine,
                   NEW_FINGERPRINT,
                   {'device': hname})
        elif (finger != finger_host):
            Notify(self.wabengine,
                   WRONG_FINGERPRINT,
                   {'device': hname})
            return False, ("Host Key received is different from host key in "
                           "DB, please contact your administrator")
        return True, "OK"

    def init_timeframe(self, auth):
        if (auth['deconnection_time']
            and auth['deconnection_time'] != u"-"
            and auth['deconnection_time'][0:4] <= u"2034"):
            self.deconnection_time = auth['deconnection_time']
            self.deconnection_epoch = int(
                time.mktime(
                    time.strptime(
                        auth['deconnection_time'],
                        "%Y-%m-%d %H:%M:%S"
                    )
                )
            )

    def get_trace_type(self):
        try:
            self._trace_type = self.wabengine_conf.get('trace',
                                                       u'localfile_hashed')
            return self._trace_type
        except Exception:
            import traceback
            Logger().info("Engine get_trace_type failed: "
                          "configuration file section "
                          "'wabengine', key 'trace', (((%s)))" %
                          traceback.format_exc())
        return u'localfile_hashed'

    def get_selector_banner(self):
        try:
            self._selector_banner = self.wabengine_conf.get('banner')
        except Exception:
            import traceback
            Logger().info("Engine get_selector_banner failed: "
                          "configuration file section "
                          "'wabgine', key 'trace', (((%s)))" %
                          traceback.format_exc())
        return self._selector_banner

    def get_trace_encryption_key(self, path, old_scheme=False):
        return self.wabengine.get_trace_encryption_key(path, old_scheme)

    def get_trace_sign_key(self):
        return self.wabengine.get_trace_sign_key()

    def password_expiration_date(self):
        try:
            _data = self.wabengine.check_password_expiration_info()
            if _data[2]:
                Logger().info("Engine password_expiration_date=%s" % _data[0])
                return True, _data[0]
        except Exception:
            import traceback
            Logger().info("Engine password_expiration_date failed: "
                          "(((%s)))" % traceback.format_exc())
        return False, 0

    def is_x509_connected(self, wab_login, ip_client, proxy_type, target,
                          ip_server):
        return self.authenticator.is_x509_connected(
            wab_login, ip_client, proxy_type, target, ip_server
        )

    def is_x509_validated(self):
        return self.authenticator.is_x509_validated()

    @logtime_function_pause
    def x509_authenticate(self, ip_client=None, ip_server=None):
        return self.authenticator.x509_authenticate(
            self, ip_client, ip_server
        )

    def check_pingid(self, wab_login, ip_client, ip_server):
        return self.authenticator.check_pingid(
            wab_login, ip_client, ip_server
        )

    def pingid_authenticate(self):
        return self.authenticator.pingid_authenticate(self)

    def password_authenticate(self, wab_login, ip_client, password, ip_server):
        return self.authenticator.password_authenticate(
            self, wab_login, ip_client, password, ip_server
        )

    def passthrough_authenticate(self, wab_login, ip_client, ip_server):
        return self.authenticator.passthrough_authenticate(
            self, wab_login, ip_client, ip_server
        )

    def gssapi_authenticate(self, wab_login, ip_client, ip_server):
        return self.authenticator.gssapi_authenticate(
            self, wab_login, ip_client, ip_server
        )

    def pubkey_authenticate(self, wab_login, ip_client, pubkey, ip_server):
        return self.authenticator.pubkey_authenticate(
            self, wab_login, ip_client, pubkey, ip_server
        )

    def get_challenge(self):
        return self.authenticator.get_challenge()

    def reset_challenge(self):
        self.authenticator.reset_challenge()

    def resolve_target_host(self, target_device, target_login, target_service,
                            target_group, real_target_device, target_context,
                            passthrough_mode, protocols):
        """ Resolve the right target host to use
        target_context.host will contains the target host.
        target_context.showname() will contains the target_device to show
        target_context.login will contains the target_login if not in
            passthrough mode.

        Returns target_device: None target_device is a hostname,
                target_context
        """
        if real_target_device:
            # Transparent proxy
            if not target_context or not target_context.host:
                dnsname = resolve_reverse_dns(real_target_device)
                target_context = TargetContext(
                    host=real_target_device,
                    dnsname=dnsname
                )
                target_context.strict_transparent = True
        elif target_device:
            # This allow proxy to check if target_device is a device_name
            # or a hostname.
            # In case it is a hostname, we keep the target_login as a filter.
            valid = self.valid_device_name(protocols, target_device)
            Logger().info("Check Valid device '%s' : res = %s" %
                          (target_device, valid))
            if not valid:
                # target_device might be a hostname
                try:
                    login_filter, service_filter, group_filter = (
                        None, None, None
                    )
                    dnsname = None
                    if (target_login and not passthrough_mode):
                        login_filter = target_login
                    if (target_service and not passthrough_mode):
                        service_filter = target_service
                    if (target_group and not passthrough_mode):
                        group_filter = target_group
                    host_ip = socket.getaddrinfo(target_device, None)[0][4][0]
                    Logger().info("Resolve DNS Hostname %s -> %s" %
                                  (target_device, host_ip))
                    try:
                        socket.inet_pton(socket.AF_INET, target_device)
                    except socket.error:
                        dnsname = target_device
                    target_context = TargetContext(host=host_ip,
                                                   dnsname=dnsname,
                                                   login=login_filter,
                                                   service=service_filter,
                                                   group=group_filter,
                                                   show=target_device)
                    target_device = None
                except Exception:
                    # import traceback
                    # Logger().info("resolve_hostname: (((%s)))" %
                    #               (traceback.format_exc()))
                    Logger().info("target_device is not a hostname")
        return target_device, target_context

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
        except Exception:
            import traceback
            Logger().info("Engine NotifyConnectionToCriticalEquipment failed: "
                          "(((%s)))" % (traceback.format_exc()))

    def NotifyPrimaryConnectionFailed(self, user, ip):
        try:
            notif_data = {
                u'user': user,
                u'ip': ip
            }

            Notify(self.wabengine, PRIMARY_CX_FAILED, notif_data)
        except Exception:
            import traceback
            Logger().info("Engine NotifyPrimaryConnectionFailed failed: "
                          "(((%s)))" % (traceback.format_exc()))

    def NotifySecondaryConnectionFailed(self, user, ip, account, device):
        try:
            notif_data = {
                u'user': user,
                u'ip': ip,
                u'account': account,
                u'device': device,
            }

            Notify(self.wabengine, SECONDARY_CX_FAILED, notif_data)
        except Exception:
            import traceback
            Logger().info("Engine NotifySecondaryConnectionFailed failed: "
                          "(((%s)))" % (traceback.format_exc()))

    def NotifyFilesystemIsFullOrUsedAtXPercent(self, filesystem, used):
        try:
            notif_data = {
                u'filesystem': filesystem,
                u'used': used
            }

            Notify(self.wabengine, FILESYSTEM_FULL, notif_data)
        except Exception:
            import traceback
            Logger().info("Engine NotifyFilesystemIsFullOrUsedAtXPercent "
                          "failed: (((%s)))" % (traceback.format_exc()))

    def NotifyFindPatternInRDPFlow(self, regexp, string, user_login, user,
                                   host, cn, service):
        try:
            notif_data = {
                u'regexp': regexp,
                u'string': string,
                u'user_login': user_login,
                u'user': user,
                u'host': host,
                u'device': cn,
                u'service': service,
            }

            Notify(self.wabengine, RDP_PATTERN_FOUND, notif_data)

            text = (
                "%(regexp)s: The string '%(string)s' has been detected in the "
                "following RDP connection: "
                "%(user)s@%(device)s:%(service)s:%(user_login)s "
                "(%(host)s)\n"
            ) % notif_data
            Logger().info("%s" % text)
        except Exception:
            import traceback
            Logger().info("Engine NotifyFindPatternInRDPFlow failed: "
                          "(((%s)))" % (traceback.format_exc()))

    def notify_find_connection_rdp(self, rule, deny, app_name, app_cmd_line,
                                   dst_addr, dst_port, user_login, user,
                                   host, cn, service):
        try:
            notif_data = {
                u'rule': rule,
                u'deny': deny,
                u'app_name': app_name,
                u'app_cmd_line': app_cmd_line,
                u'dst_addr': dst_addr,
                u'dst_port': dst_port,
                u'user_login': user_login,
                u'user': user,
                u'host': host,
                u'device': cn,
                u'service': service
            }
            Notify(self.wabengine, RDP_OUTCXN_FOUND, notif_data)
            text = (
                "%(rule)s: The connection '%(dst_addr)s:%(dst_port)s' "
                "has been detected in the following RDP connection: "
                "%(user)s@%(device)s:%(service)s:%(user_login)s "
                "(%(host)s)\n"
            ) % notif_data
            Logger().info("%s" % text)
        except Exception:
            import traceback
            Logger().info("Engine NotifyFindConnectionInRDPFlow failed: "
                          "(((%s)))" % (traceback.format_exc()))

    def notify_find_process_rdp(self, regex, deny, app_name, app_cmd_line,
                                user_login, user, host, cn, service):
        try:
            notif_data = {
                u'regex': regex,
                u'deny': deny,
                u'app_name': app_name,
                u'app_cmd_line': app_cmd_line,
                u'user_login': user_login,
                u'user': user,
                u'host': host,
                u'device': cn,
                u'service': service
            }
            Notify(self.wabengine, RDP_PROCESS_FOUND, notif_data)
            text = (
                "%(regex)s: The application '%(app_name)s' has been detected "
                "in the following RDP connection: "
                "%(user)s@%(device)s:%(service)s:%(user_login)s "
                "(%(host)s)\n"
            ) % notif_data
            Logger().info("%s" % text)
        except Exception:
            import traceback
            Logger().info("Engine NotifyFindProcessInRDPFlow failed: "
                          "(((%s)))" % (traceback.format_exc()))

    def get_targets_list(self, group_filter, device_filter, protocol_filter,
                         case_sensitive):
        def fc(string):
            return string if case_sensitive else string.lower()

        targets = []
        selector_filter_mode = taf.get_selector_filter_mode(device_filter)

        if selector_filter_mode == taf.SelectorFilterMode.ADVANCED:
            try:
                filter_pattern_dict = (
                    taf.get_filter_pattern_dict(fc(device_filter)))
            except RuntimeError:
                return targets, True

        item_filtered = False

        for target_info in self.displaytargets:
            temp_service_login = target_info.service_login
            temp_resource_service_protocol_cn = target_info.protocol
            if not target_info.protocol == u"APP":
                if (target_info.target_name == u'autotest'
                    or target_info.target_name == u'bouncer2'
                    or target_info.target_name == u'widget2_message'
                    or target_info.target_name == u'widgettest'
                    or target_info.target_name == u'card'):
                    temp_service_login = target_info.service_login.replace(
                        u':RDP',
                        u':INTERNAL', 1)
                    temp_resource_service_protocol_cn = 'INTERNAL'

            if (not fc(group_filter) in fc(target_info.group)
                or not fc(protocol_filter) in fc(temp_resource_service_protocol_cn)):
                item_filtered = True
                continue

            if selector_filter_mode == taf.SelectorFilterMode.NORMAL:
                # apply target global filter mode

                if not fc(device_filter) in fc(temp_service_login):
                    item_filtered = True
                    continue
            elif selector_filter_mode == taf.SelectorFilterMode.ADVANCED:
                # apply target accurate filter mode

                target_login_real, sep, target_domain_real = (
                    target_info.target_login.rpartition('@'))

                if not sep:
                    target_login_real, target_domain_real = (
                        target_domain_real, "")

                target_device = target_info.target_name
                target_service = target_info.service_name
                target_field_dict = {
                    "account" : fc(target_login_real),
                    "domain" : fc(target_domain_real),
                    "device" : fc(target_device),
                    "service" : fc(target_service)}

                if not taf.is_filterable(filter_pattern_dict,
                                         target_field_dict):
                    item_filtered = True
                    continue

            targets.append((target_info.group,  # ( = concatenated list)
                            temp_service_login,
                            temp_resource_service_protocol_cn))
        # Logger().info("targets list = %s'" % targets)
        return targets, item_filtered

    def reset_proxy_rights(self):
        """
        Clean current fetched rights
        """
        self.proxy_rights = None
        self.rights = None
        self.target_right = None
        self.release_all_target()
        self.reset_target_session()

    def reset_target_session(self):
        """
        Clean Target session (if exist) rights and infos
        """
        self.target_right = None
        self.deconnection_epoch = 0xffffffff
        self.deconnection_time = u"-"
        self.start_time = None

        self.session_record = None
        self.session_record_type = None
        self.session_id = None
        self.pidhandler = None
        self.session_result = True
        self.session_diag = u'Success'
        self.failed_secondary_set = False
        self.trace_hash = None

        self.service = None

        self.checktarget_cache = None
        self.checktarget_infos_cache = None

    def proxy_session_logout(self):
        """
        Clean all information of current authenticated proxy session
        """
        self.reset_proxy_rights()
        if self.authenticator is not None:
            self.authenticator.reset()

    def get_proxy_user_rights(self, protocols, target_device, **kwargs):
        Logger().debug("** CALL Get_proxy_right ** proto=%s, target_device=%s"
                       % (protocols, target_device))
        urights = self.wabengine.get_proxy_user_rights(protocols,
                                                       target_device,
                                                       **kwargs)
        Logger().debug("** END Get_proxy_right **")
        if urights and (type(urights[0]) == str):
            import json
            urights = map(json.loads, urights)
        return urights

    def valid_device_name(self, protocols, target_device):
        try:
            # Logger().debug("** CALL VALIDATOR DEVICE NAME "
            #                "Get_proxy_right target=%s **"
            #                % target_device)
            prights = self.get_proxy_user_rights(
                protocols, target_device)
            # Logger().debug("** END VALIDATOR DEVICE NAME Get_proxy_right **")
        except Exception:
            # import traceback
            # Logger().info("valid_device_name failed: (((%s)))" %
            #               (traceback.format_exc()))
            return False
        rights = prights
        if rights:
            self.proxy_rights = prights
            return True
        return False

    def _filter_rights(self, target_context):
        from collections import defaultdict
        self.rights = self.proxy_rights
        # targets{(account, target)}{domain}[(service, group, right)]
        self.targets = defaultdict(lambda: defaultdict(list))
        self.targets_alias = defaultdict(lambda: defaultdict(list))
        self.displaytargets = []
        for right in self.rights:
            account_name = right['account_name']
            account_domain = right['domain_cn']
            account_login = right['account_login']
            account_logindom = self.get_account_login(right,
                                                      check_in_creds=False)
            account_namedom = account_name
            if account_domain and account_domain != AM_IL_DOMAIN:
                account_namedom = "%s@%s" % (account_name, account_domain)
            try:
                auth_name = right['auth_cn']
            except Exception:
                auth_name = right['target_group']
            if right['application_cn']:
                target_name = right['application_cn']
                service_name = u"APP"
                protocol = u"APP"
                host = None
                alias = None
                subprotocols = []
            else:
                target_name = right['device_cn']
                service_name = right['service_cn']
                protocol = right['service_protocol_cn']
                host = right['device_host']
                alias = right['device_alias']
                subprotocols = right['service_subprotocols']
            if target_context is not None:
                if target_context.host and host is None:
                    continue
                if (target_context.host
                    and not is_device_in_subnet(target_context.host, host)
                    and host != target_context.dnsname):
                    continue
                if (target_context.login
                    and account_login
                    and target_context.login not in [
                        account_login, account_logindom,
                        account_name, account_namedom]):
                    # match context login with login or name
                    # (with or without domain)
                    continue
                if (target_context.service
                    and service_name != target_context.service):
                    continue
                if (target_context.group
                    and target_context.group != auth_name):
                    continue

            target_value = (service_name, auth_name, right)
            # feed targets hashtable indexed on account_name and target_name
            # targets{(account, target)}{domain}[(service, group, right)]
            tuple_index = (account_name, target_name)
            self.targets[tuple_index][account_domain].append(target_value)
            if alias:
                alias_index = (account_name, alias)
                self.targets_alias[alias_index][account_domain].append(
                    target_value
                )

            self.displaytargets.append(DisplayInfo(account_namedom,
                                                   target_name,
                                                   service_name,
                                                   protocol,
                                                   auth_name,
                                                   subprotocols,
                                                   host))
        if target_context and target_context.strict_transparent:
            self._filter_subnet()

    def _filter_subnet(self):
        # in transparent mode, targets rights are already
        # filtered by a unique host
        filtered_subnet = [dit for dit in self.displaytargets
                           if '/' not in dit.host]
        if filtered_subnet:
            self.displaytargets = filtered_subnet

    def filter_app_rights(self, app_rights, account_name, domain_name,
                          app_name):
        _rs = [r for r in app_rights if (
            r['account_name'] == account_name
            and (not domain_name or r['domain_cn'] == domain_name)
            and r['application_cn'] == app_name)]
        return _rs

    def get_proxy_rights(self, protocols, target_device=None,
                         target_context=None):
        if self.proxy_rights is None:
            try:
                # Logger().debug("** CALL Get_proxy_right ** "
                #                "proto=%s, target_device=%s, "
                #                "checktimeframe=%s" %
                #                (protocols, target_device))
                self.proxy_rights = self.get_proxy_user_rights(
                    protocols, target_device)
                # Logger().debug("** END Get_proxy_right **")
            except Exception:
                # import traceback
                # Logger().info("traceback = %s" % traceback.format_exc())
                self.proxy_rights = None
                return
        if self.rights is not None:
            return
        # start = time.time()
        # Logger().debug("** BEGIN Filter_rights **")
        self._filter_rights(target_context)
        # Logger().debug("** END Filter_rights in %s sec **" %
        #                (time.time() - start))

    def _get_target_right_htable(self, target_account, target_device,
                                 t_htable):
        """
        Get target right list from t_htable
        filtered by target_account and target_device

        target_account = <login>@<domain> or <login>
        '@' might be present in login but not in domain
        t_htable = {(account, device)}{domain}[(service, group, right)]
        device can be an alias
        """
        try:
            acc_dom = target_account.rsplit('@', 1)
            account = acc_dom[0]
            domain = acc_dom[1] if len(acc_dom) == 2 else ''
            domres = t_htable.get((account, target_device), {})
            results = domres.get(domain)
            if not results:
                # domain might not be provided in target_account
                if domain:
                    # domain field is not empty so try again
                    # with target_account
                    domres = t_htable.get((target_account, target_device), {})
                if len(domres) == 1:
                    # no ambiguity, get first (and only) value
                    results = domres[next(iter(domres))]
                else:
                    # ambiguity on domain
                    results = []
        except Exception:
            results = []
        return results

    def _find_target_right(self, target_account, target_device,
                           target_service, target_group):
        try:
            Logger().debug("Find target %s@%s:%s:%s" %
                           (target_account, target_device,
                            target_service, target_group))
            results = self._get_target_right_htable(
                target_account, target_device, self.targets)
            if not results:
                results = self._get_target_right_htable(
                    target_account, target_device, self.targets_alias)
        except Exception:
            results = []
        right = None
        filtered = [
            (r_service, r)
            for (r_service, r_groups, r) in results if (
                ((not target_service) or (r_service == target_service))
                and ((not target_group) or (r_groups == target_group))
            )
        ]
        if filtered:
            filtered_service, right = filtered[0]
            # if ambiguity in group but not in service,
            # get the right without approval
            for (r_service, r) in filtered[1:]:
                if filtered_service != r_service:
                    right = None
                    break
                if r['auth_has_approval'] is False:
                    right = r
        if right:
            self.init_timeframe(right)
            self.target_right = right
        return right

    def get_target_rights(self, target_login, target_device, target_service,
                          target_group, target_context=None):
        try:
            self.get_proxy_rights([u'SSH', u'TELNET', u'RLOGIN', u'RAWTCPIP'],
                                  target_device=target_device,
                                  target_context=target_context)
            right = self._find_target_right(target_login, target_device,
                                            target_service, target_group)
            if right:
                return right, "OK"

            Logger().error("Bastion account %s couldn't log into %s@%s%s" % (
                self.user_cn,
                target_login,
                target_device,
                ":%s" % target_service if target_service else ""
            ))
        except Exception:
            import traceback
            Logger().info("traceback = %s" % traceback.format_exc())

        invalid_str = u"Invalid target %s\r\n"
        if self.get_language() == "fr":
            invalid_str = u"Cible %s invalide\r\n"
        target_str = u"%s@%s:%s (%s)" % (target_login, target_device,
                                         target_service, target_group)
        msg = invalid_str % target_str
        return (None, msg)

    def get_selected_target(self, target_login, target_device, target_service,
                            target_group, target_context=None):
        # Logger().info(
        #     ">>==GET_SELECTED_TARGET %s@%s:%s:%s" %
        #     (target_device, target_login, target_service, target_group)
        # )
        self.get_proxy_rights([u'RDP', u'VNC'], target_device,
                              target_context=target_context)
        return self._find_target_right(target_login, target_device,
                                       target_service, target_group)

    def get_effective_target(self, selected_target):
        application = selected_target['application_cn']
        try:
            if application and not self.is_shadow_session(selected_target):
                effective_target = self.wabengine.get_effective_target(
                    selected_target
                )
                Logger().info("Engine get_effective_target done (application)")
                return effective_target
            else:
                Logger().info("Engine get_effective_target done (physical)")
                return [selected_target]

        except Exception:
            import traceback
            Logger().info("Engine get_effective_target failed: (((%s)))" %
                          (traceback.format_exc()))
        return []

    def secondary_failed(self, reason, wabuser, ip_source, user, host):
        if self.failed_secondary_set:
            return
        self.failed_secondary_set = True
        if reason:
            self.session_diag = reason
        self.session_result = False
        Notify(self.wabengine, SECONDARY_CX_FAILED, {
            'user': wabuser,
            'ip': ip_source,
            'account': user,
            'device': host,
        })

    def get_app_params(self, selected_target, effective_target):
        # Logger().info("Engine get_app_params: "
        #               "service_login=%s effective_target=%s" %
        #               (service_login, effective_target))
        if self.is_shadow_session(selected_target):
            from collections import namedtuple
            status, infos = self.check_target(selected_target)
            AppParams = namedtuple('AppParams',
                                   ['program', 'params', 'workingdir'])
            token = infos.get("shadow_token", {})
            app_params = AppParams("", None, token.get("shadow_id"))
            Logger().info("Engine get_app_params shadow done")
            return app_params
        try:
            app_params = self.wabengine.get_app_params(
                selected_target,
                effective_target
            )
            Logger().info("Engine get_app_params done")
            return app_params
        except Exception:
            import traceback
            Logger().info("Engine get_app_params failed: (((%s)))" %
                          (traceback.format_exc()))
        return None

    def get_primary_password(self, target_device):
        Logger().debug("Engine get_primary_password ...")
        try:
            password = self.checkout.get_primary_password(target_device)
            Logger().debug("Engine get_primary_password done")
            return password
        except Exception:
            import traceback
            Logger().debug("Engine get_primary_password failed: "
                           "(((%s)))" % (traceback.format_exc()))
        return None

    def checkout_target(self, target):
        """
        Checkout target and get credentials object
        Deprecated (replaced by checkout_account called in check_target)
        """
        return True, "OK"

    def get_account_infos(self, account_name, domain_name, device_name):
        Logger().debug("Engine get_account_infos ...")
        try:
            return self.checkout.get_scenario_account_infos(
                account_name, domain_name, device_name
            )
        except Exception:
            import traceback
            Logger().debug("Engine get_account_infos failed:"
                           " %s" % (traceback.format_exc()))
        return None

    def get_account_infos_by_type(self, account_name, domain_name,
                                  device_name, account_type=None):
        try:
            return self.checkout.check_account_by_type(
                account_name=account_name,
                domain_name=domain_name,
                device_name=device_name,
                account_type=account_type
            )
        except Exception:
            import traceback
            Logger().debug("Engine get_account_infos_by_type failed:"
                           " %s" % (traceback.format_exc()))
        return None

    def get_target_passwords(self, target_device):
        Logger().debug("Engine get_target_passwords ...")
        try:
            return self.checkout.get_target_passwords(target_device)
        except Exception:
            import traceback
            Logger().debug("Engine get_target_passwords failed:"
                           " (((%s)))" % (traceback.format_exc()))
        return []

    def get_target_password(self, target_device):
        passwords = self.get_target_passwords(target_device)
        return passwords[0] if passwords else u""

    def get_target_privkeys(self, target_device):
        Logger().debug("Engine get_target_privkeys ...")
        try:
            return self.checkout.get_target_privkeys(target_device)
        except Exception:
            import traceback
            Logger().debug("Engine get_target_privkey failed:"
                           " (((%s)))" % (traceback.format_exc()))
        return []

    def release_target(self, target_device):
        try:
            self.checkout.release_target(target_device)
        except Exception:
            import traceback
            Logger().debug("Engine release_target failed:"
                           " (((%s)))" % (traceback.format_exc()))
        return True

    def release_account(self, acc_name, dom_name, dev_name):
        try:
            self.checkout.release_scenario_account(
                acc_name, dom_name, dev_name
            )
        except Exception:
            import traceback
            Logger().debug("Engine checkin_scenario_account failed: (%s)"
                           % (traceback.format_exc()))
        return True

    def release_account_by_type(self, account_name, domain_name,
                                device_name, account_type=None):
        try:
            self.checkout.release_account_by_type(
                account_name, domain_name, device_name,
                account_type=account_type
            )
        except Exception:
            import traceback
            Logger().debug("Engine checkin_account_by_type failed: (%s)"
                           % (traceback.format_exc()))
        return True

    def release_all_target(self):
        if self.checkout is not None:
            self.checkout.release_all()

    def start_session(self, auth, pid, effective_login=None, **kwargs):
        Logger().debug("**** CALL wabengine START SESSION ")
        try:
            self.session_id, self.start_time = self.wabengine.start_session(
                auth,
                pid=pid,
                effective_login=effective_login,
                **kwargs
            )
            self.failed_secondary_set = False
        except LicenseException:
            Logger().info("Engine start_session failed: License Exception")
            self.session_id, self.start_time = None, None
        except Exception:
            import traceback
            self.session_id, self.start_time = None, None
            Logger().info("Engine start_session failed: (((%s)))" %
                          (traceback.format_exc()))
        Logger().debug("**** END wabengine START SESSION ")
        return self.session_id, self.start_time

    def start_session_ssh(self, target, target_user, hname, host, client_addr,
                          pid, subproto, kill_handler, effective_login=None):
        """ Start session for new wabengine """
        self.hname = hname
        self.target_user = effective_login or target_user
        self.host = host

        if kill_handler:
            import signal
            signal.signal(signal.SIGUSR1, kill_handler)
        Logger().debug("**** CALL wabengine START SESSION ")
        try:
            self.session_id, self.start_time = self.wabengine.start_session(
                target,
                pid=pid,
                subprotocol=subproto,
                effective_login=effective_login,
                target_host=self.host
            )
        except LicenseException:
            Logger().info("Engine start_session failed: License exception")
            self.session_id, self.start_time = None, None
        except Exception:
            import traceback
            self.session_id, self.start_time = None, None
            Logger().info("Engine start_session failed: (((%s)))" %
                          (traceback.format_exc()))
        Logger().debug("**** END wabengine START SESSION ")
        if self.session_id is None:
            return None, None
        self.service = target['service_cn']
        is_critical = target['auth_is_critical']
        device_host = target['device_host']
        self.failed_secondary_set = False

        if not is_critical:
            return self.session_id, self.start_time
        # Notify start
        # if subproto in ['SSH_X11_SESSION', 'SSH_SHELL_SESSION']:
        #     subproto = 'SSH'
        notif_data = {
            'protocol': subproto,
            'user': self.user_cn,
            'source': socket.getfqdn(client_addr),
            'ip_source': client_addr,
            'login': self.get_account_login(target),
            'device': hname,
            'ip': device_host,
            'time': time.ctime()
        }

        Notify(self.wabengine,
               CX_EQUIPMENT,
               notif_data)

        return self.session_id, self.start_time

    def update_session_target(self, physical_target, **kwargs):
        """Update current session with target name.

        :param target physical_target: selected target
        :return: None
        """
        hosttarget = u"%s%s%s@%s:%s" % (
            physical_target['account_name'],
            '@' if physical_target['domain_cn'] else '',
            physical_target['domain_cn'],
            physical_target['device_cn'],
            physical_target['service_cn'])
        try:
            if self.session_id:
                self.wabengine.update_session(self.session_id,
                                              hosttarget=hosttarget,
                                              **kwargs)
        except Exception:
            import traceback
            Logger().info("Engine update_session_target failed: (((%s)))" %
                          (traceback.format_exc()))

    def update_session(self, **kwargs):
        """Update current session parameters to base.

        :return: None
        """
        try:
            if self.session_id:
                self.wabengine.update_session(self.session_id,
                                              **kwargs)
        except Exception:
            import traceback
            Logger().info("Engine update_session failed: (((%s)))" %
                          (traceback.format_exc()))

    def shadow_response(self, errcode, errmsg, token, userdata):
        try:
            status = SHADOW_ACCEPTED if errcode == '0' else SHADOW_REJECTED
            self.wabengine.make_session_shadowing_response(
                status=status, errmsg=errmsg, token=token, userdata=userdata
            )
        except Exception:
            import traceback
            Logger().info("Engine shadow_response failed: (((%s)))" %
                          (traceback.format_exc()))

    def stop_session(self, title=u"End session"):
        try:
            if self.session_id:
                # Logger().info(
                #     "Engine stop_session: result='%s', diag='%s', "
                #     "title='%s'" %
                #     (self.session_result, self.session_diag, title)
                # )
                self.wabengine.stop_session(
                    self.session_id,
                    result=self.session_result,
                    diag=self.session_diag,
                    title=title,
                    check=self.trace_hash
                )
                self.trace_hash = None
        except SessionAlreadyStopped:
            pass
        except Exception:
            import traceback
            Logger().info("Engine stop_session failed: (((%s)))" %
                          (traceback.format_exc()))
        Logger().debug("Engine stop session end")

    # RESTRICTIONS
    def get_all_restrictions(self, auth, proxytype):
        if proxytype == u"RDP":
            def matchproto(x):
                return x == u"RDP"
        elif proxytype == u"SSH":
            def matchproto(x):
                return x in ["SSH_SHELL_SESSION",
                             "SSH_REMOTE_COMMAND",
                             "SSH_SCP_UP",
                             "SSH_SCP_DOWN",
                             "SFTP_SESSION",
                             "RLOGIN", "TELNET"]
        else:
            return {}, {}
        try:
            restrictions = self.wabengine.get_proxy_restrictions(auth)
            kill_patterns = {}
            notify_patterns = {}
            for restriction in restrictions:
                if not restriction.subprotocol:
                    Logger().error("No subprotocol in restriction!")
                    continue
                subproto = restriction.subprotocol.cn
                if matchproto(subproto):
                    # Logger().debug("adding restriction %s %s %s" %
                    #                (restriction.action, restriction.data,
                    #                 restriction.subprotocol.cn))
                    if restriction.action == 'kill':
                        if not kill_patterns.get(subproto):
                            kill_patterns[subproto] = []
                        kill_patterns[subproto].append(restriction.data)
                    elif restriction.action == 'notify':
                        if not notify_patterns.get(subproto):
                            notify_patterns[subproto] = []
                        notify_patterns[subproto].append(restriction.data)

            Logger().info("patterns_kill = [%s]" % (kill_patterns))
            Logger().info("patterns_notify = [%s]" % (notify_patterns))
        except Exception:
            kill_patterns = {}
            notify_patterns = {}
            import traceback
            Logger().info("Engine get_restrictions failed: (((%s)))" %
                          (traceback.format_exc()))
        return (kill_patterns, notify_patterns)

    def get_restrictions(self, auth, proxytype):
        if self.is_shadow_session(auth):
            return None, None
        if proxytype == "RDP":
            def matchproto(x):
                return x == u"RDP"
            separator = u"\x01"
        elif proxytype == u"SSH":
            def matchproto(x):
                return x in ["SSH_SHELL_SESSION",
                             "SSH_REMOTE_COMMAND",
                             "SSH_SCP_UP",
                             "SSH_SCP_DOWN",
                             "SFTP_SESSION",
                             "RLOGIN", "TELNET"]
            separator = u"|"
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
                    Logger().debug("adding restriction %s %s %s" %
                                   (restriction.action, restriction.data,
                                    restriction.subprotocol.cn))
                    if restriction.action == 'kill':
                        kill_patterns.append(restriction.data)
                    elif restriction.action == 'notify':
                        notify_patterns.append(restriction.data)

            self.pattern_kill = separator.join(kill_patterns)
            self.pattern_notify = separator.join(notify_patterns)
            Logger().info("pattern_kill = [%s]" % (self.pattern_kill))
            Logger().info("pattern_notify = [%s]" % (self.pattern_notify))
        except Exception:
            self.pattern_kill = None
            self.pattern_notify = None
            import traceback
            Logger().info("Engine get_restrictions failed: (((%s)))" %
                          (traceback.format_exc()))
        return (self.pattern_kill, self.pattern_notify)

    # RESTRICTIONS: NOTIFIER METHODS
    def pattern_found_notify(self, action, regex_found, current_line):
        regex_found = regex_found
        current_line = current_line
        self.session_diag = u'Restriction pattern detected (%s)' % current_line
        data = {
            "regexp": regex_found,
            "string": current_line,
            "host": self.host,
            "user_login": self.user_cn,
            "user": self.target_user,
            "device": self.hname,
            "service": self.service,
            "action": action
        }
        Notify(self.wabengine, PATTERN_FOUND, data)
        text = (
            u"%(action)s: The string '%(string)s' has been detected "
            u"in the following SSH connection: "
            u"%(user)s@%(device)s:%(service)s:%(user_login)s (%(host)s)\n"
        ) % data
        Logger().info("%s" % text)
        if action.lower() == "kill":
            self.session_result = False

    def filesize_limit_notify(self, action, filesize, filename,
                              limit_filesize):
        restrictstr = u"file %s > %s" % (filename, limit_filesize)
        self.session_diag = u'Filesize restriction detected (%s)' % restrictstr
        data = {
            "regexp": u"filesize > %s" % filesize,
            "string": restrictstr,
            "host": self.host,
            "user_login": self.user_cn,
            "user": self.target_user,
            "device": self.hname,
            "service": self.service,
            "action": action
        }
        Notify(self.wabengine, PATTERN_FOUND, data)
        text = (
            u"%(action)s: The restriction '%(string)s' has been detected "
            u"in the following SSH connection: "
            u"%(user)s@%(device)s:%(service)s:%(user_login)s (%(host)s)\n"
        ) % data
        Logger().info("%s" % text)
        if action.lower() == "kill":
            self.session_result = False

    def globalsize_limit_notify(self, action, globalsize, limit_globalsize):
        self.session_diag = u'Filesize restriction detected'
        data = {
            "regexp": "globalsize > %s" % globalsize,
            "string": "globalsize > %s" % limit_globalsize,
            "host": self.host,
            "user_login": self.user_cn,
            "user": self.target_user,
            "device": self.hname,
            "service": self.service,
            "action": action
        }
        Notify(self.wabengine, PATTERN_FOUND, data)
        text = (
            u"%(action)s: The restriction '%(string)s' has been detected "
            u"in the following SSH connection: "
            u"%(user)s@%(device)s:%(service)s:%(user_login)s (%(host)s)\n"
        ) % data
        Logger().info("%s" % text)
        if action.lower() == "kill":
            self.session_result = False

    def start_tcpip_record(self, selected_target=None, filename=None):
        target = selected_target or self.target_right
        if not target:
            Logger().debug("start_record failed: missing target right")
            return False
        try:
            is_recorded = target['auth_is_recorded']
            if is_recorded:
                self.session_record = self.wabengine.get_trace_writer(
                    self.session_id,
                    filename=filename,
                    trace_type=u'pcap'
                )
                self.session_record_type = "pcap"
                self.session_record.initialize()
        except Exception:
            import traceback
            Logger().info("Engine start_record failed")
            Logger().debug("Engine get_trace_writer failed: %s" %
                           (traceback.format_exc()))
            return False
        return True

    def write(self, data):
        if self.session_record:
            self.session_record.writeraw(data)

    def start_record(self, selected_target=None, filename=None):
        target = selected_target or self.target_right
        if not target:
            Logger().debug("start_record failed: missing target right")
            return False
        try:
            is_recorded = target['auth_is_recorded']
            if is_recorded:
                self.session_record = self.wabengine.get_trace_writer(
                    self.session_id,
                    filename=filename,
                    trace_type=u'ttyrec'
                )
                self.session_record_type = "ttyrec"
                self.session_record.initialize()
        except Exception:
            import traceback
            Logger().info("Engine start_record failed")
            Logger().debug("Engine get_trace_writer failed: %s" %
                           (traceback.format_exc()))
            return False
        return True

    def record(self, data):
        """ Factorized record method to be used if isRecorded == True """
        if self.session_record:
            self.session_record.writeframe(data)

    def stop_record(self):
        if self.session_record:
            try:
                if self.session_record_type == "ttyrec":
                    # force last timestamp to match session duration
                    self.session_record.writeframe(b"")
                rec_hash = self.session_record.end()
                self.session_record = None
                return rec_hash
            except Exception as e:
                Logger().info("Stop record failed: %s" % e)
            self.session_record = None
        return None

    def write_trace(self, video_path):
        try:
            _status, _error = True, u"No error"
            if video_path:
                # Notify WabEngine with Trace file descriptor
                trace = self.wabengine.get_trace_writer(
                    self.session_id,
                    filename=video_path,
                    trace_type=u"rdptrc"
                )
                trace.initialize()
                trace.writeframe(b"%s.mwrm" % (video_path.encode('utf-8')))
                self.trace_hash = trace.end()
                self.session_record_type = "rdptrc"
        except Exception as e:
            Logger().info("Engine write_trace failed: %s" % e)
            _status, _error = False, u"Exception"
        return _status, _error

    def read_session_parameters(self, key=None):
        return self.wabengine.read_session_parameters(self.session_id, key=key)

    def check_target(self, target, pid=None, request_ticket=None):
        if self.checktarget_cache == (APPROVAL_ACCEPTED, target['target_uid']):
            # Logger().info("** CALL Check_target SKIPED**")
            return self.checktarget_cache[0], self.checktarget_infos_cache
        Logger().debug("** CALL Check_target ** ticket=%s" %
                       request_ticket)
        status, infos = self.checkout.check_target(target, request_ticket)
        Logger().debug("** END Check_target ** returns => "
                       "status=%s, info fields=%s" % (status, infos.keys()))
        self.checktarget_cache = (status, target['target_uid'])
        self.checktarget_infos_cache = infos
        # Logger().info("returns => status=%s, info=%s" % (status, infos))
        deconnection_time = infos.get("deconnection_time")
        if deconnection_time:
            target['deconnection_time'] = deconnection_time
            # update deconnection_time in right
        return status, infos

    def check_effective_target(self, app_right, effective_target):
        target_uid = effective_target['target_uid']
        for r in self.get_effective_target(app_right):
            if r['target_uid'] == target_uid:
                return True
        return False

    def get_application(self, selected_target=None):
        target = selected_target or self.target_right
        if not target:
            return None
        return target['application_cn']

    def get_target_protocols(self, selected_target=None):
        target = selected_target or self.target_right
        if not target:
            return None
        proto = target['service_protocol_cn']
        # subproto = [x.cn for x in target.resource.service.subprotocols]
        subproto = target['service_subprotocols']
        return ProtocolInfo(proto, subproto)

    def get_target_extra_info(self, selected_target=None):
        target = selected_target or self.target_right
        if not target:
            return None
        isRecorded = target['auth_is_recorded']
        isCritical = target['auth_is_critical']
        hasApproval = target['auth_has_approval']
        return ExtraInfo(isRecorded, isCritical, hasApproval)

    def get_deconnection_time(self, selected_target=None):
        target = selected_target or self.target_right
        if not target:
            return None
        if self.is_shadow_session(target):
            return u'-'
        return target['deconnection_time']

    def get_server_pubkey_options(self, selected_target=None):
        target = selected_target or self.target_right
        if not target:
            return {}

        conn_policy_data = self.get_target_conn_options(target)
        server_pubkey_options = conn_policy_data.get('server_pubkey', {})
        return server_pubkey_options

    def get_target_auth_methods(self, selected_target=None):
        target = selected_target or self.target_right
        if not target:
            return []
        try:
            # Logger().info("connectionpolicy")
            # Logger().info("%s" % target.resource.service.connectionpolicy)
            authmethods = target['connection_policy_methods']
        except Exception:
            Logger().error("Error: Connection policy has no methods field")
            authmethods = []
        return authmethods

    def get_target_conn_options(self, selected_target=None):
        target = selected_target or self.target_right
        if not target:
            return {}
        try:
            # Logger().info("connectionpolicy")
            # Logger().info("%s" % target.resource.service.connectionpolicy)
            conn_opts = target['connection_policy_data']
        except Exception:
            Logger().error("Error: Connection policy has no data field")
            conn_opts = {}
        return conn_opts

    def get_physical_target_info(self, physical_target):
        if self.is_shadow_session(physical_target):
            status, infos = self.check_target(physical_target)
            token = infos.get("shadow_token", {})
            return PhysicalTarget(
                device_host=token.get('shadow_ip'),
                account_login=physical_target.get('account_login'),
                service_port=token.get('shadow_port'),
                device_id=physical_target.get('device_uid')
            )
        port = physical_target['service_port']
        if not isinstance(port, int):
            port = int(port)
        return PhysicalTarget(
            device_host=physical_target['device_host'],
            account_login=self.get_account_login(physical_target),
            service_port=port,
            device_id=physical_target['device_uid']
        )

    def get_target_login_info(self, selected_target=None):
        target = selected_target or self.target_right
        if not target:
            return None
        if target['application_cn']:
            target_name = target['application_cn']
            device_host = None
        else:
            target_name = target['device_cn']
            device_host = target['device_host']

        account_login = self.get_account_login(target)
        account_name = target['account_name']
        domain_name = target['domain_cn']
        if domain_name == AM_IL_DOMAIN:
            domain_name = ""
        service_port = target['service_port']
        service_name = target['service_cn']
        auth_name = target['auth_cn']
        user_group_name = target.get('user_group_cn', "")
        target_group_name = target.get('target_group_cn', "")
        conn_opts = target['connection_policy_data']
        return LoginInfo(account_login=account_login,
                         account_name=account_name,
                         domain_name=domain_name,
                         target_name=target_name,
                         service_name=service_name,
                         auth_name=auth_name,
                         user_group_name=user_group_name,
                         target_group_name=target_group_name,
                         device_host=device_host,
                         service_port=service_port,
                         conn_opts=conn_opts)

    def get_account_login(self, right, check_in_creds=True):
        login = right['account_login']
        try:
            domain = right['domain_name']
        except Exception:
            domain = ""
        if check_in_creds:
            login = (self.checkout.get_target_login(right)
                     or login)
            domain = (self.checkout.get_target_domain(right)
                      or domain)
        if not login and right['domain_cn'] == AM_IL_DOMAIN:
            # Interactive Login
            return login
        trule = right['connection_policy_data'].get(
            "general", {}
        ).get(
            "transformation_rule"
        )
        if (trule and '${LOGIN}' in trule):
            return trule.replace(
                '${LOGIN}', login
            ).replace(
                '${DOMAIN}', domain or ''
            )
        if not domain:
            return login
        return "%s@%s" % (login, domain)

    def get_scenario_account_field(self, field, param, default=None):
        from .parsers import resolve_scenario_account
        return resolve_scenario_account(self, field, param, force_device=False, default=default)

    def get_crypto_methods(self):
        class crypto_methods(object):
            def __init__(self, proxy):
                self.proxy = proxy

            def get_trace_sign_key(self):
                return self.proxy.get_trace_sign_key()

            def get_trace_encryption_key(self, name, flag):
                return self.proxy.get_trace_encryption_key(name, flag)
        return crypto_methods(self.wabengine)

    def is_shadow_session(self, selected_target=None):
        target = selected_target or self.target_right
        return target.get('is_shadow') is True


class TargetContext(object):
    def __init__(self, host=None, dnsname=None, login=None, service=None,
                 group=None, show=None):
        self.host = host
        self.dnsname = dnsname
        self.login = login
        self.service = service
        self.group = group
        self.show = show
        self.strict_transparent = False

    def showname(self):
        return self.show or self.dnsname or self.host

    def is_empty(self):
        return not (self.host or self.login or self.service or self.group)


class DisplayInfo(object):
    __slots__ = ("target_login", "target_name", "service_name", "protocol",
                 "group", "subprotocols", "service_login", "host")

    def __init__(self, target_login, target_name, service_name,
                 protocol, group, subproto, host):
        self.target_login = target_login
        self.target_name = target_name
        self.service_name = service_name
        self.protocol = protocol
        self.group = group
        self.subprotocols = subproto or []
        self.service_login = "%s@%s:%s" % (
            self.target_login, self.target_name, self.service_name
        )
        self.host = host

    def get_target_tuple(self):
        return (self.target_login,
                self.target_name,
                self.service_name,
                self.group)


class ProtocolInfo(object):
    __slots__ = (
        "protocol", "subprotocols",
    )

    def __init__(self, protocol, subprotocols=[]):
        self.protocol = protocol
        self.subprotocols = subprotocols


class ExtraInfo(object):
    __slots__ = (
        "is_recorded", "is_critical", "has_approval",
    )

    def __init__(self, is_recorded, is_critical, has_approval):
        self.is_recorded = is_recorded
        self.is_critical = is_critical
        self.has_approval = has_approval


class PhysicalTarget(object):
    __slots__ = (
        "device_host", "account_login", "service_port", "device_id",
    )

    def __init__(self, device_host, account_login, service_port, device_id):
        self.device_host = device_host
        self.account_login = account_login
        self.service_port = service_port
        self.device_id = device_id


class LoginInfo(object):
    __slots__ = (
        "account_login", "account_name", "domain_name", "service_name",
        "target_name", "auth_name", "user_group_name", "target_group_name",
        "device_host", "service_port", "conn_opts",
    )

    def __init__(self, account_login, account_name, domain_name, target_name,
                 service_name, auth_name, user_group_name, target_group_name,
                 device_host, service_port, conn_opts):
        self.account_login = account_login
        self.account_name = account_name
        self.domain_name = domain_name
        self.target_name = target_name
        self.service_name = service_name
        self.auth_name = auth_name
        self.user_group_name = user_group_name
        self.target_group_name = target_group_name
        self.device_host = device_host
        self.service_port = service_port
        self.conn_opts = conn_opts

    def get_target_str(self):
        return "%s%s@%s:%s:%s" % (self.account_name,
                                  "@%s" % self.domain_name if self.domain_name
                                  else '',
                                  self.target_name,
                                  self.service_name,
                                  self.auth_name)

    def get_target_dict(self):
        return {
            "user_group": self.user_group_name,
            "target_group": self.target_group_name,
            "authorization": self.auth_name,
            "account": self.account_name,
            "account_domain": self.domain_name,
            "device": self.target_name,
            "service": self.service_name,
        }
