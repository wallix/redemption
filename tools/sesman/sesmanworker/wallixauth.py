import traceback

from logger import Logger
from wabengine.client.checker import Checker
from wabengine.common.exception import (
    LicenseException,
)
from wallixauthentication.api import (
    AuthenticationFailed,
    AuthenticationChallenge,
    IdentificationFailed,
)
from wallixconst.authentication import (
    IDENTIFICATION_TYPES as IDENT,
    AUTHENTICATION_TYPES as AUTH,
    PROXY_CLIENT,
)
from wabx509 import AuthX509

from .challenge import (
    Challenge,
    md_to_challenge,
    ac_to_challenge,
    ur_to_challenge,
)
from typing import Dict, Optional, Set, Union, Any


class AuthState:
    KERBEROS = "KERBEROS"
    PASSWORD = "PASSWORD"
    OTP = "OTP"
    SSH_KEY = "SSH_KEY"
    MOBILE_DEVICE = "MOBILE_DEVICE"
    PASSTHROUGH = "PASSTHROUGH"
    URL_REDIRECT = "URL_REDIRECT"
    KBDINT_CHECK = "KBDINT_CHECK"


COMPATIBILITY_PROXY = {
    # IDENT.X509: [AUTH.X509, AUTH.PASSWORD, AUTH.MOBILE_DEVICE],
    AuthState.KERBEROS: [AUTH.KERBEROS, AUTH.PASSWORD, AUTH.MOBILE_DEVICE],
    AuthState.OTP: [AUTH.TOKEN],
    AuthState.PASSWORD: [AUTH.PASSWORD, AUTH.MOBILE_DEVICE],
    AuthState.SSH_KEY: [AUTH.SSH_KEY, AUTH.PASSWORD, AUTH.MOBILE_DEVICE],
    AuthState.MOBILE_DEVICE: [AUTH.MOBILE_DEVICE, AUTH.PASSWORD],
    AuthState.PASSTHROUGH: [AUTH.PASSTHROUGH],
    AuthState.URL_REDIRECT: [AUTH.URL_REDIRECT],
    AuthState.KBDINT_CHECK: [AUTH.URL_REDIRECT, AUTH.PASSWORD,
                             AUTH.MOBILE_DEVICE],
}

EXPECTED_FIRST_COMPAT = {
    # IDENT.X509: [AUTH.X509, AUTH.PASSWORD, AUTH.MOBILE_DEVICE],
    AuthState.KERBEROS: [AUTH.KERBEROS],
    AuthState.OTP: [AUTH.TOKEN],
    AuthState.PASSWORD: [AUTH.PASSWORD, AUTH.MOBILE_DEVICE],
    AuthState.SSH_KEY: [AUTH.SSH_KEY],
    AuthState.MOBILE_DEVICE: [AUTH.MOBILE_DEVICE],
    AuthState.PASSTHROUGH: [AUTH.PASSTHROUGH],
    AuthState.URL_REDIRECT: [AUTH.URL_REDIRECT],
    AuthState.KBDINT_CHECK: [AUTH.PASSWORD, AUTH.URL_REDIRECT,
                             AUTH.MOBILE_DEVICE],
}

IDENT_PROXY = {
    # IDENT.X509: [AUTH.X509, AUTH.PASSWORD, AUTH.MOBILE_DEVICE],
    AuthState.KERBEROS: IDENT.KERBEROS,
    AuthState.OTP: IDENT.TOKEN,
    AuthState.PASSWORD: IDENT.LOGIN,
    AuthState.SSH_KEY: IDENT.LOGIN,
    AuthState.MOBILE_DEVICE: IDENT.LOGIN,
    AuthState.PASSTHROUGH: IDENT.PASSTHROUGH,
    AuthState.URL_REDIRECT: IDENT.LOGIN,
    AuthState.KBDINT_CHECK: IDENT.LOGIN,
}

CHALLENGE_AUTH_STATE = {
    AUTH.PASSWORD: AuthState.PASSWORD,
    AUTH.MOBILE_DEVICE: AuthState.MOBILE_DEVICE,
    AUTH.URL_REDIRECT: AuthState.URL_REDIRECT,
}

KBDINT_PENDING_STATUS = {
    AuthState.PASSWORD: "password",
    AuthState.MOBILE_DEVICE: "mobile_device",
    AuthState.URL_REDIRECT: "url_redirect",
}


BANNABLE_STATES = (AuthState.SSH_KEY, AuthState.KERBEROS)


def get_auth_priority(auth_state: str) -> Optional[str]:
    if auth_state == AuthState.KBDINT_CHECK:
        return None
    expected = EXPECTED_FIRST_COMPAT.get(auth_state)
    return expected[0] if expected else None


class Authenticator:
    __slots__ = (
        'checker', 'auth_x509', 'challenge',
        'auth_state', 'auth_key', 'auth_ident', 'auth_challenge',
        'removed_auth_state', 'current_login',
    )

    def __init__(self) -> None:
        self.reset()

    def reset(self) -> None:
        self.auth_x509: Optional[AuthX509] = None
        self.challenge: Optional[Challenge] = None
        self.auth_state: Optional[str] = None
        self.auth_key: Optional[str] = None
        self.auth_ident: Optional[str] = None
        self.auth_challenge: Optional[Dict[str, str]] = None
        self.current_login: Optional[str] = None
        self.removed_auth_state: Set[str] = set()
        self.checker: Optional[Checker] = None

    def _init_client(self) -> None:
        if self.checker is None:
            self.checker = Checker()

    def _reset_auth(self, remove_auth_state: str = '', cancel: bool = True) -> None:
        if remove_auth_state:
            Logger().debug(f">> PA remove state {remove_auth_state}")
            # do not try this auth method again
            self.removed_auth_state.add(remove_auth_state)
        self.challenge = None
        self.auth_state = None
        self.auth_challenge = None
        self.auth_ident = None
        if cancel and self.auth_key and self.checker:
            try:
                self.checker.cancel(self.auth_key)
            except Exception:
                pass
        self.auth_key = None

    def _init_identify(self,
                       ip_source: str,
                       server_ip: str,
                       login: Optional[str],
                       auth_state: str,
                       client_name: str = PROXY_CLIENT) -> bool:
        self._init_client()

        # Conditions to continue

        if (self.current_login is not None
            and login is not None
            and login != self.current_login):
            # login changed
            self._reset_auth()

        if self.auth_key and (self.auth_state == auth_state):
            # already identified and state are consistent
            return True

        if self.auth_key:
            # already identified but state not compatible
            if auth_state == AuthState.KBDINT_CHECK:
                # Check State: return false
                return False
            # reset pending auth and continue
            self._reset_auth()

        if auth_state in self.removed_auth_state:
            # this auth method has been definitively banned
            Logger().debug(f">> PA method {auth_state} banned")
            return False

        Logger().debug(f">> PA init_identify current:{self.auth_key}  asked:{auth_state}")
        self.auth_state = auth_state
        self.auth_ident = IDENT_PROXY[auth_state]
        if auth_state in (
            AuthState.PASSWORD,
            AuthState.KERBEROS,
            AuthState.SSH_KEY,
            AuthState.MOBILE_DEVICE,
            AuthState.PASSTHROUGH,
            AuthState.URL_REDIRECT,
            AuthState.KBDINT_CHECK,
        ):
            self.current_login = login
            data = {
                'login': login,
            }
        elif auth_state == AuthState.OTP:
            data = {
                'token': login,
            }
        else:
            Logger().debug(f">> PA init_identify unknown state {auth_state}")
            self._reset_auth()
            return False

        auth_type = None
        try:
            Logger().debug(f"Call BEGIN identify {data}")
            self.auth_key = self.checker.identify(
                data=data,
                identification=self.auth_ident,
                ip_source=ip_source,
                server_ip=server_ip,
                client=client_name,
                no_delay=(AuthState.PASSWORD != self.auth_state),
            )
            # Logger().debug(f"Call END identify {self.auth_key}")
            Logger().debug("Call END identify")
            compatibility = COMPATIBILITY_PROXY[auth_state]
            Logger().debug(f"Call BEGIN compatibility ({compatibility})")
            self.auth_challenge = self.checker.compatibility(
                key=self.auth_key,
                compatibility=compatibility,
                priority=get_auth_priority(auth_state),
                no_delay=(AuthState.PASSWORD != self.auth_state),
            )
            Logger().debug(f"Call END compatibility {self.auth_challenge}")
            auth_type = self.auth_challenge.get('auth_type')
            if (auth_type not in EXPECTED_FIRST_COMPAT[auth_state]):
                Logger().debug(
                    f">> PA init_identify challenge auth type {auth_type}"
                    f" not expected for {auth_state}"
                )
                remove_state = None
                if auth_state in BANNABLE_STATES:
                    # if compatibility check on "automatic" method,
                    # do not try it again
                    remove_state = auth_state
                self._reset_auth(remove_auth_state=remove_state)
                return False
        except AuthenticationFailed as a:
            Logger().debug(f">> PA init_identify AuthenticationFailed {a}")
            remove_state = None
            if auth_state in BANNABLE_STATES:
                # if identification failed on "automatic" method,
                # do not try it again
                remove_state = auth_state
            self._reset_auth(remove_auth_state=remove_state,
                             cancel=(auth_state in (AuthState.SSH_KEY,
                                                    AuthState.URL_REDIRECT,
                                                    AuthState.KBDINT_CHECK)))
            return False
        except IdentificationFailed as a:
            Logger().debug(f">> PA init_identify IdentificationFailed {a}")
            remove_state = None
            if auth_state in BANNABLE_STATES:
                # if identification failed on "automatic" method,
                # do not try it again
                remove_state = auth_state
            self._reset_auth(remove_auth_state=remove_state, cancel=False)
            return False

        if (auth_type == AUTH.SSH_KEY
            and not self.auth_challenge.get('ssh_keys')):
            # user does not have any key configured
            # -> reject any other sshkey try
            Logger().debug(">> PA init_identify No stored keys")
            self._reset_auth(remove_auth_state=AuthState.SSH_KEY)
            return False

        return True

    def _authentify(self, enginei, data: Dict[str, Any], debug_str: str = "None", no_delay: bool = False) -> bool:
        try:
            Logger().debug(f"Call BEGIN authenticate ({debug_str})")
            enginei.wabengine = self.checker.authenticate(
                key=self.auth_key,
                data=data,
                no_delay=no_delay
            )
            Logger().debug(f"Call END authenticate ({debug_str})")
            self.challenge = None
            if enginei.wabengine is not None:
                enginei._post_authentication()
                return True
        except AuthenticationChallenge as ac:
            Logger().debug(f"AuthenticationChallenge {ac.__dict__}")
            self.set_challenge(ac.challenge)
            if self.auth_state == AuthState.MOBILE_DEVICE:
                return self._authentify(
                    enginei, {}, "mobile_device",
                    no_delay=True
                )
            # Logger().debug(f"Challenge {self.challenge.__dict__}")
            return False
        except AuthenticationFailed as exp:
            Logger().info(f"{exp}")
            self._reset_auth(cancel=False)
            return False
        except Exception:
            self._reset_auth(cancel=False)
            Logger().info(
                f"Engine authenticate ({debug_str}) failed: "
                f"((({traceback.format_exc()})))")
            raise
        self._reset_auth()
        return False

    def set_challenge(self, challenge: Dict[str, Union[int, str]], check_state: bool = False) -> None:
        auth_type = challenge.get("auth_type")
        self.auth_state = CHALLENGE_AUTH_STATE.get(auth_type)
        if self.auth_state is None:
            # Challenge only work for "PASSWORD", MOBILE_DEVICE and URL_REDIRECT
            self._reset_auth()
            return
        self.auth_challenge = challenge
        if auth_type == AUTH.URL_REDIRECT:
            self.challenge = ur_to_challenge(challenge)
        elif auth_type == AUTH.MOBILE_DEVICE:
            self.challenge = md_to_challenge(challenge)
        else:
            self.challenge = ac_to_challenge(challenge, check_state)
        return

    def get_challenge(self) -> Optional[Challenge]:
        return self.challenge

    def reset_challenge(self) -> None:
        self.challenge = None

    def is_x509_connected(self,
                          wab_login: str,
                          ip_client: str,
                          proxy_type: str,
                          target: str,
                          ip_server: str) -> bool:
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
            Logger().info("Engine is_x509_connected failed: "
                          f"((({traceback.format_exc()})))")
        return False

    def is_x509_validated(self) -> bool:
        try:
            result = False
            if self.auth_x509 is not None:
                result = self.auth_x509.is_validated()
        except Exception:
            Logger().info(f"Engine is_x509_validated failed: ((({traceback.format_exc()})))")
        return result

    def check_kbdint_authenticate(self, enginei, login: str, ip_client: str, ip_server: str) -> Union[bool, str, None]:
        if self.auth_key and login == self.current_login:
            return KBDINT_PENDING_STATUS.get(self.auth_state)
        if not self._init_identify(ip_client, ip_server, login=login,
                                   auth_state=AuthState.KBDINT_CHECK):
            return False
        self.set_challenge(self.auth_challenge, check_state=True)
        if self.auth_state == AuthState.MOBILE_DEVICE:
            return self._authentify(
                enginei, {}, "mobile_device",
                no_delay=True
            )
        return KBDINT_PENDING_STATUS.get(self.auth_state)

    def x509_authenticate(self, enginei, ip_client: Optional[str] = None, ip_server: Optional[str] = None) -> bool:
        try:
            enginei.wabengine = self.auth_x509.get_proxy()
            if enginei.wabengine is not None:
                enginei._post_authentication()
                return True
        except AuthenticationFailed:
            self.challenge = None
        except LicenseException:
            self.challenge = None
        except Exception:
            Logger().info("Engine x509_authenticate failed: "
                          f"((({traceback.format_exc()})))")
        return False

    def otp_authenticate(self, enginei, otp: str, ip_client: str, ip_server: str) -> bool:
        if not self._init_identify(ip_client, ip_server, login=otp,
                                   auth_state=AuthState.OTP):
            return False
        data = {}
        auth_type = self.auth_challenge.get('auth_type')
        if auth_type == AUTH.TOKEN:
            data['result'] = True
        if data:
            return self._authentify(enginei, data, "otp")
        self._reset_auth()
        return False

    def mobile_device_authenticate(self, enginei) -> bool:
        if self.auth_state != AuthState.MOBILE_DEVICE:
            self._reset_auth()
            return False
        try:
            auth_type = self.auth_challenge.get('auth_type')
            if auth_type == AUTH.MOBILE_DEVICE:
                return self._authentify(enginei, {}, "mobile_device",
                                        no_delay=True)
        except Exception:
            self._reset_auth(cancel=False)
            Logger().info("Engine mobile_device_authenticate failed: "
                          f"((({traceback.format_exc()})))")
            raise
        self._reset_auth()
        return False

    def url_redirect_authenticate(self, enginei) -> bool:
        if self.auth_state != AuthState.URL_REDIRECT:
            self._reset_auth()
            return False
        try:
            auth_type = self.auth_challenge.get('auth_type')
            if auth_type == AUTH.URL_REDIRECT:
                return self._authentify(enginei, {}, "url_redirect",
                                        no_delay=True)
        except Exception:
            self._reset_auth(cancel=False)
            Logger().info("Engine url_redirect_authenticate failed: "
                          f"((({traceback.format_exc()})))")
            raise
        self._reset_auth()
        return False

    def password_authenticate(self, enginei, wab_login: str, ip_client: str, password: str,
                              ip_server: str) -> bool:
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
            if auth_type == AUTH.MOBILE_DEVICE:
                return self._authentify(enginei, data, "mobile_device",
                                        no_delay=True)
            if data:
                return self._authentify(enginei, data, "password")
        except Exception:
            self._reset_auth(cancel=False)
            Logger().info("Engine password_authenticate failed: "
                          f"((({traceback.format_exc()})))")
            raise
        self._reset_auth()
        return False

    def passthrough_authenticate(self, enginei, wab_login: str, ip_client: str,
                                 ip_server: str) -> bool:
        if not self._init_identify(ip_client, ip_server, login=wab_login,
                                   auth_state=AuthState.PASSTHROUGH):
            return False
        try:
            data = {}
            auth_type = self.auth_challenge.get('auth_type')
            if auth_type == AUTH.PASSTHROUGH:
                data['result'] = True
            if data:
                return self._authentify(enginei, data, "passthrough",
                                        no_delay=True)
        except Exception as a:
            self._reset_auth(cancel=False)
            Logger().info("Engine passthrough_authenticate failed: "
                          f"{a} ({traceback.format_exc()})")
            raise
        self._reset_auth()
        return False

    def gssapi_authenticate(self, enginei, wab_login: str, ip_client: str, ip_server: str) -> bool:
        if not self._init_identify(ip_client, ip_server, login=wab_login,
                                   auth_state=AuthState.KERBEROS):
            return False
        try:
            data = {}
            auth_type = self.auth_challenge.get('auth_type')
            if auth_type == AUTH.KERBEROS:
                data['result'] = True
            if data:
                return self._authentify(enginei, data, "gssapi",
                                        no_delay=True)
        except Exception:
            self._reset_auth(cancel=False)
            Logger().info("Engine gssapi_authenticate failed: "
                          f"((({traceback.format_exc()})))")
            raise
        self._reset_auth()
        return False

    def pubkey_authenticate(self,
                            enginei,
                            wab_login: str,
                            ip_client: str,
                            ip_server: str,
                            pubkey: str,
                            ca_pubkey: str) -> bool:
        if not self._init_identify(ip_client, ip_server, login=wab_login,
                                   auth_state=AuthState.SSH_KEY):
            return False
        try:
            data = {}
            auth_type = self.auth_challenge.get('auth_type')
            if auth_type == AUTH.SSH_KEY:
                ssh_ca_keys = self.auth_challenge.get('ssh_ca_keys') or []
                for stored_ca_key in ssh_ca_keys:
                    if compare_pubkeys_str(stored_ca_key, ca_pubkey):
                        data['result'] = True
                        break
                else:
                    if ca_pubkey:
                        # no ca stored and ca signed pubkey at authentication
                        # force check against ca_pubkey
                        # (fallback to former behavior)
                        pubkey = ca_pubkey
                ssh_keys = self.auth_challenge.get('ssh_keys') or []
                for stored_key in ssh_keys:
                    if compare_pubkeys_str(stored_key, pubkey):
                        data['result'] = True
                        break
            if data:
                return self._authentify(enginei, data, "pubkey",
                                        no_delay=True)
        except Exception:
            self._reset_auth(cancel=False)
            Logger().info("Engine pubkey_authenticate failed: "
                          f"((({traceback.format_exc()})))")
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


def _ssh_public_key(pubkey):
    import base64
    import struct

    keydata = base64.b64decode(pubkey)

    i = 0
    parts = []
    while len(keydata) != i:
        # read the length of the data
        dlen = struct.unpack_from('>I', keydata, i)[0]
        i += 4
        # read in <length> bytes
        data = keydata[i:dlen + i]
        i += dlen
        # remove leading zeros
        parts.append(data.lstrip(b'\x00'))

    return parts


def compare_pubkeys(pubkey1, pubkey2):
    if not (pubkey1 and pubkey2):
        # failure if one of the key is empty
        return False

    return ((pubkey1 == pubkey2)
            or (_ssh_public_key(pubkey1) == _ssh_public_key(pubkey2)))
