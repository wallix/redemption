from logger import Logger

try:
    from wabengine.common.exception import (
        AuthenticationFailed,
        AuthenticationChallenged,
        MultiFactorAuthentication,
        AuthenticationUpdatePwd,
        LicenseException,
        MustChangePassword,
        AccountLocked,
        SessionAlreadyStopped,
    )
    from wallixconst.authentication import (
        IDENTIFICATION_TYPES as IDENT,
        AUTHENTICATION_TYPES as AUTH,
    )
    from wabengine.client.sync_client import SynClient
    from wabx509 import AuthX509
except Exception as e:
    import traceback
    tracelog = traceback.format_exc()
    try:
        from .fake.proxyengine import *
        from .fake.const import (
            IDENTIFICATION_TYPES as IDENT,
            AUTHENTICATION_TYPES as AUTH,
        )
    except Exception as e:
        Logger().info("Wabengine const LOADING FAILED %s" % tracelog)


from .challenge import (
    Challenge,
    wchallenge_to_challenge,
    mfa_to_challenge,
    ac_to_challenge,
)
from collections import namedtuple


class AuthState(object):
    KERBEROS = "KERBEROS"
    PASSWORD = "PASSWORD"
    OTP = "OTP"
    SSH_KEY = "SSH_KEY"
    PINGID = "PINGID"


COMPATIBILITY_PROXY = {
    # IDENT.X509: [AUTH.X509, AUTH.PASSWORD, AUTH.PINGID],
    AuthState.KERBEROS: [AUTH.KERBEROS, AUTH.PASSWORD, AUTH.PINGID],
    AuthState.OTP: [AUTH.TOKEN],
    AuthState.PASSWORD: [AUTH.PASSWORD, AUTH.PINGID],
    AuthState.SSH_KEY: [AUTH.SSH_KEY, AUTH.PASSWORD, AUTH.PINGID],
    AuthState.PINGID: [AUTH.PINGID, AUTH.PASSWORD],
}

EXPECTED_FIRST_COMPAT = {
    # IDENT.X509: [AUTH.X509, AUTH.PASSWORD, AUTH.PINGID],
    AuthState.KERBEROS: [AUTH.KERBEROS],
    AuthState.OTP: [AUTH.TOKEN],
    AuthState.PASSWORD: [AUTH.PASSWORD, AUTH.PINGID],
    AuthState.SSH_KEY: [AUTH.SSH_KEY],
    AuthState.PINGID: [AUTH.PINGID],
}

IDENT_PROXY = {
    # IDENT.X509: [AUTH.X509, AUTH.PASSWORD, AUTH.PINGID],
    AuthState.KERBEROS: IDENT.KERBEROS,
    AuthState.OTP: IDENT.TOKEN,
    AuthState.PASSWORD: IDENT.LOGIN,
    AuthState.SSH_KEY: IDENT.LOGIN,
    AuthState.PINGID: IDENT.LOGIN,
}

CHALLENGE_AUTH_STATE = {
    AUTH.PASSWORD: AuthState.PASSWORD,
    AUTH.PINGID: AuthState.PINGID,
}


def get_auth_priority(auth_state):
    expected = EXPECTED_FIRST_COMPAT.get(auth_state)
    return expected[0] if expected else None


class Authenticator(object):
    __slots__ = (
        'synclient_port', 'client', 'auth_x509', 'challenge',
        'auth_state', 'auth_key', 'auth_ident', 'auth_challenge',
        'removed_auth_state'
    )

    def __init__(self, synclient_port):
        self.synclient_port = synclient_port
        self.client = None
        self.auth_x509 = None
        self.challenge = None
        self.auth_state = None
        self.auth_key = None
        self.auth_ident = None
        self.auth_challenge = None
        self.removed_auth_state = set()

    def reset(self):
        self.client = None
        self.auth_x509 = None
        self.challenge = None
        self.auth_state = None
        self.auth_key = None
        self.auth_ident = None
        self.auth_challenge = None
        self.removed_auth_state = set()

    def _init_client(self):
        if self.client is None:
            self.client = SynClient(
                'localhost',
                self.synclient_port
            )

    def _reset_auth(self, remove_auth_state=None, cancel=False):
        if remove_auth_state:
            Logger().debug(">> PA remove state %s" % remove_auth_state)
            # do not try this auth method again
            self.removed_auth_state.add(remove_auth_state)
        self.challenge = None
        self.auth_state = None
        self.auth_challenge = None
        self.auth_ident = None
        if cancel and self.auth_key and self.client:
            self.client.wa_cancel(self.auth_key)
        self.auth_key = None

    def _init_identify(self, ip_source, server_ip,
                       login=None, auth_state=None,
                       client_name="PROXY"):
        self._init_client()
        if auth_state in self.removed_auth_state:
            # this auth method has been definitively banned
            Logger().debug(">> PA method %s banned" % auth_state)
            return False
        if self.auth_key and (self.auth_state == auth_state):
            # already identified for this kind of auth method
            return True
        Logger().debug(">> PA init_identify current:%s  asked:%s" %
                       (self.auth_key, auth_state))
        self.auth_state = auth_state
        self.auth_ident = IDENT_PROXY[auth_state]
        data = {}
        if auth_state in [AuthState.PASSWORD,
                          AuthState.KERBEROS,
                          AuthState.SSH_KEY,
                          AuthState.PINGID]:
            data = {
                'login': login,
            }
        elif auth_state in [AuthState.OTP]:
            data = {
                'token': login,
            }
        if not data:
            Logger().debug(">> PA init_identify unknown state %s" % auth_state)
            self._reset_auth()
            return False
        auth_type = None
        try:
            Logger().debug("Call BEGIN wa_identify %s" % data)
            self.auth_key = self.client.wa_identify(
                data=data,
                identification=self.auth_ident,
                ip_source=ip_source,
                server_ip=server_ip,
                client=client_name,
            )
            # Logger().debug("Call END wa_identify %s" % self.auth_key)
            Logger().debug("Call END wa_identify")
            compatibility = COMPATIBILITY_PROXY[auth_state]
            Logger().debug("Call BEGIN wa_compatibility (%s)" %
                           compatibility)
            self.auth_challenge = self.client.wa_compatibility(
                key=self.auth_key,
                compatibility=compatibility,
                priority=get_auth_priority(auth_state)
            )
            Logger().debug("Call END wa_compatibility %s" %
                           self.auth_challenge)
            auth_type = self.auth_challenge.get('auth_type')
            if (auth_type not in EXPECTED_FIRST_COMPAT[auth_state]):
                Logger().debug(
                    ">> PA init_identify challenge auth type %s"
                    " not expected for %s" %
                    (auth_type, auth_state)
                )
                remove_state = None
                if auth_state in (AuthState.SSH_KEY, AuthState.KERBEROS):
                    remove_state = auth_state
                self._reset_auth(remove_auth_state=remove_state)
                return False
        except AuthenticationFailed as af:
            Logger().debug(">> PA init_identify AuthenticationFailed")
            self._reset_auth()
            return False
        if (auth_type == AUTH.SSH_KEY
            and not self.auth_challenge.get('ssh_keys')):
            # user does not have any key configured
            # -> reject any other sshkey try
            Logger().debug(">> PA init_identify No stored keys")
            self._reset_auth(remove_auth_state=AuthState.SSH_KEY)
            return False
        return True

    def _authentify(self, enginei, data, debug_str="None"):
        try:
            Logger().debug("Call BEGIN wa_authenticate (%s)" %
                           (debug_str))
            enginei.wabengine = self.client.wa_authenticate(
                key=self.auth_key,
                data=data,
            )
            Logger().debug("Call END wa_authenticate (%s)" % (debug_str))
            self.challenge = None
            if enginei.wabengine is not None:
                enginei._post_authentication()
                return True
        except AuthenticationChallenged as ac:
            Logger().debug("AuthenticationChallenged %s" % ac.__dict__)
            self.set_challenge(ac.challenge)
            if self.auth_state == AuthState.PINGID:
                return self._authentify(
                    enginei, {}, "PINGID"
                )
            # Logger().debug("Challenge %s" % self.challenge.__dict__)
            return False
        except AuthenticationFailed as exp:
            Logger().info("%s" % exp)
        except Exception as a:
            self._reset_auth()
            import traceback
            Logger().info(
                "Engine wa_authenticate (%s) failed: "
                "(((%s)))" % (debug_str, traceback.format_exc()))
            raise
        self._reset_auth()
        return False

    def set_challenge(self, challenge):
        auth_type = challenge.get("auth_type")
        self.auth_state = CHALLENGE_AUTH_STATE.get(auth_type)
        if self.auth_state is None:
            # Challenge only work for "PASSWORD" and PINGID
            self._reset_auth()
            return
        self.auth_challenge = challenge
        self.challenge = ac_to_challenge(challenge)
        return

    def get_challenge(self):
        return self.challenge

    def reset_challenge(self):
        self.challenge = None

    def is_x509_connected(self, wab_login, ip_client, proxy_type, target,
                          ip_server):
        """
        Ask if we are authentifying using x509
        (and ask user by opening confirmation popup if we are,
        session ticket will be asked later in x509_authenticate)
        """
        try:
            self.auth_x509 = AuthX509(username=wab_login,
                                      ip=ip_client,
                                      requestor=proxy_type,
                                      target=target,
                                      server_ip=ip_server)
            result = self.auth_x509.is_connected()
            return result
        except Exception:
            import traceback
            Logger().info("Engine is_x509_connected failed: "
                          "(((%s)))" % traceback.format_exc())
        return False

    def is_x509_validated(self):
        try:
            result = False
            if self.auth_x509 is not None:
                result = self.auth_x509.is_validated()
        except Exception:
            import traceback
            Logger().info("Engine is_x509_validated failed: (((%s)))" %
                          traceback.format_exc())
        return result

    def x509_authenticate(self, enginei, ip_client=None, ip_server=None):
        try:
            enginei.wabengine = self.auth_x509.get_proxy()
            if enginei.wabengine is not None:
                enginei._post_authentication()
                return True
        except AuthenticationChallenged as e:
            self.challenge = wchallenge_to_challenge(e.challenge)
        except MultiFactorAuthentication as mfa:
            self.challenge = mfa_to_challenge(mfa)
            if self.challenge and self.challenge.recall:
                return self.password_authenticate(
                    enginei,
                    self.challenge.username,
                    ip_client, "", ip_server
                )
        except AuthenticationFailed:
            self.challenge = None
        except LicenseException:
            self.challenge = None
        except Exception:
            import traceback
            Logger().info("Engine x509_authenticate failed: "
                          "(((%s)))" % traceback.format_exc())
        return False

    def otp_authenticate(self, enginei, otp, ip_client, ip_server):
        if not self._init_identify(ip_client, ip_server, login=otp,
                                   auth_state=AuthState.OTP):
            return False
        data = {}
        auth_type = self.auth_challenge.get('auth_type')
        if auth_type == AUTH.TOKEN:
            data['result'] = auth_type == self.auth_ident
        if data:
            return self._authentify(enginei, data, "otp")
        self._reset_auth()
        return False

    def check_pingid(self, wab_login, ip_client, ip_server):
        if not self._init_identify(ip_client, ip_server, login=wab_login,
                                   auth_state=AuthState.PINGID):
            return False
        auth_type = self.auth_challenge.get('auth_type')
        return auth_type == AUTH.PINGID

    def pingid_authenticate(self, enginei):
        if self.auth_state != AuthState.PINGID:
            self._reset_auth()
            return False
        try:
            data = {}
            auth_type = self.auth_challenge.get('auth_type')
            if auth_type == AUTH.PINGID:
                return self._authentify(enginei, data, "pingid")
        except Exception as a:
            self._reset_auth()
            import traceback
            Logger().info("Engine pingid_authenticate failed: "
                          "(((%s)))" % traceback.format_exc())
            raise
        self._reset_auth()
        return False

    def password_authenticate(self, enginei, wab_login, ip_client, password,
                              ip_server):
        if wab_login.startswith('_OTP_'):
            return self.otp_authenticate(enginei, wab_login, ip_client,
                                         ip_server)
        if not self._init_identify(ip_client, ip_server, login=wab_login,
                                   auth_state=AuthState.PASSWORD):
            return False
        try:
            data = {}
            auth_type = self.auth_challenge.get('auth_type')
            if auth_type == AUTH.PASSWORD:
                data['password'] = password
            if auth_type == AUTH.PINGID:
                return self._authentify(enginei, data, "pingid")
            if data:
                return self._authentify(enginei, data, "password")
        except Exception as a:
            self._reset_auth()
            import traceback
            Logger().info("Engine password_authenticate failed: "
                          "(((%s)))" % traceback.format_exc())
            raise
        self._reset_auth()
        return False

    def passthrough_authenticate(self, enginei, wab_login, ip_client,
                                 ip_server):
        # self._init_identify(ip_client, ip_server, login=wab_login)
        return False

    def gssapi_authenticate(self, enginei, wab_login, ip_client, ip_server):
        if not self._init_identify(ip_client, ip_server, login=wab_login,
                                   auth_state=AuthState.KERBEROS):
            return False
        try:
            data = {}
            auth_type = self.auth_challenge.get('auth_type')
            if auth_type == AUTH.KERBEROS:
                data['result'] = auth_type == self.auth_ident
            if data:
                return self._authentify(enginei, data, "gssapi")
        except Exception as a:
            self._reset_auth(cancel=False)
            import traceback
            Logger().info("Engine gssapi_authenticate failed: "
                          "(((%s)))" % traceback.format_exc())
            raise
        self._reset_auth()
        return False

    def pubkey_authenticate(self, enginei, wab_login, ip_client, pubkey,
                            ip_server):
        if not self._init_identify(ip_client, ip_server, login=wab_login,
                                   auth_state=AuthState.SSH_KEY):
            return False
        try:
            data = {}
            auth_type = self.auth_challenge.get('auth_type')
            if auth_type == AUTH.SSH_KEY:
                ssh_keys = self.auth_challenge.get('ssh_keys')
                for stored_key in ssh_keys:
                    # Logger().debug("stored key %s" % stored_key)
                    # Logger().debug("pubkey %s" % pubkey)
                    if compare_pubkeys_str(stored_key, pubkey):
                        data['result'] = True
                        break
            if data:
                return self._authentify(enginei, data, "pubkey")
        except Exception as a:
            self._reset_auth()
            import traceback
            Logger().info("Engine pubkey_authenticate failed: "
                          "(((%s)))" % traceback.format_exc())
            raise
        return False


def compare_pubkeys_str(pubkey1, pubkey2):
    return compare_pubkeys(
        extract_pubkey(pubkey1),
        extract_pubkey(pubkey2)
    )


def extract_pubkey(pubkeystr):
    pubkey = pubkeystr
    if pubkeystr and ' ' in pubkeystr:
        splitted_pubkey = pubkeystr.split()
        # Fetch the longuest element (== the key)
        pubkey = max(splitted_pubkey, key=len)
    return pubkey


def compare_pubkeys(pubkey1, pubkey2):
    class SshPublicKey:
        def __init__(self, pubkey):
            import base64
            import struct

            keydata = base64.b64decode(pubkey)

            parts = []
            while keydata:
                # read the length of the data
                dlen = struct.unpack('>I', keydata[:4])[0]
                # read in <length> bytes
                data, keydata = keydata[4:dlen + 4], keydata[4 + dlen:]
                parts.append(data)

            # remove leading zeros
            self.fields = [s.lstrip(b'\x00') for s in parts]

        def __cmp__(self, key):
            if self.fields == key.fields:
                return 0
            return -1

    return ((pubkey1 == pubkey2)
            or (SshPublicKey(pubkey1) == SshPublicKey(pubkey2)))
