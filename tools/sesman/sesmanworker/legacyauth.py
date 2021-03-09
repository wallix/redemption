from __future__ import absolute_import
from logger import Logger

try:
    from wabengine.common.exception import (
        AuthenticationFailed,
        AuthenticationChallenged,
        MultiFactorAuthentication,
        AuthenticationUpdatePwd,
        LicenseException,
    )
    from wabengine.client.sync_client import SynClient
    from wabx509 import AuthX509
except Exception:
    import traceback
    tracelog = traceback.format_exc()
    try:
        from .fake.proxyengine import *
        Logger().info("================================")
        Logger().info("==== Load Fake AUTH ENGINE =====")
        Logger().info("================================")
    except Exception:
        # Logger().info("FAKE LOADING FAILED>>>>>> %s" %
        #               traceback.format_exc())
        Logger().info("WABENGINE LOADING FAILED>>>>>> %s" % tracelog)


from .challenge import (
    Challenge,
    mfa_to_challenge,
    aup_to_challenge,
)


def wchallenge_to_challenge(challenge):
    """ Convert Challenge from bastion to internal Challenge

    param challenge: Challenge from bastion
    param previous_token: token from previous MFA if needed
    :rtype: Challenge
    :return: a converted Challenge
    """
    return Challenge(
        challenge_type="CHALLENGE",
        title="= Challenge =",
        message="",
        fields=[challenge.message],
        echos=[challenge.promptEcho],
        username=challenge.username,
        challenge=challenge,
        token=getattr(challenge, "mfa_token", None),
    )

class LegacyAuthenticator(object):
    __slots__ = (
        'synclient_port', 'client', 'auth_x509', 'challenge',
    )

    def __init__(self, synclient_port):
        self.synclient_port = synclient_port
        self.client = None
        self.challenge = None
        self.auth_x509 = None

    def _init_client(self):
        if self.client is None:
            self.client = SynClient(
                'localhost',
                self.synclient_port
            )

    def reset(self):
        self.client = None
        self.challenge = None
        self.auth_x509 = None

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
        return self.password_authenticate(
            enginei, otp,
            ip_client, "", ip_server
        )

    def check_mobile_device(self, wab_login, ip_client, ip_server):
        return False

    def mobile_device_authenticate(self, enginei):
        return False

    def password_authenticate(self, enginei, wab_login, ip_client, password,
                              ip_server):
        try:
            self._init_client()
            challenge = self.challenge.challenge if self.challenge else None
            token = self.challenge.token if self.challenge else None
            if self.challenge:
                wab_login = self.challenge.username or wab_login
            enginei.wabengine = self.client.authenticate(
                username=wab_login,
                password=password,
                ip_source=ip_client,
                challenge=challenge,
                server_ip=ip_server,
                mfa_token=token
            )
            self.challenge = None
            if enginei.wabengine is not None:
                enginei._post_authentication()
                return True
        except AuthenticationUpdatePwd as aup:
            self.challenge = aup_to_challenge(aup, wab_login)
        except AuthenticationChallenged as e:
            self.challenge = wchallenge_to_challenge(e.challenge)
        except MultiFactorAuthentication as mfa:
            self.challenge = mfa_to_challenge(mfa)
            if self.challenge and self.challenge.recall:
                return self.password_authenticate(
                    enginei,
                    wab_login, ip_client, "", ip_server)
        except AuthenticationFailed:
            self.challenge = None
        except LicenseException:
            self.challenge = None
        except Exception:
            self.challenge = None
            import traceback
            Logger().info("Engine password_authenticate failed: "
                          "(((%s)))" % traceback.format_exc())
            raise
        return False

    def passthrough_authenticate(self, enginei, wab_login, ip_client,
                                 ip_server):
        try:
            self._init_client()
            enginei.wabengine = self.client.authenticate_gssapi(
                username=wab_login,
                realm="realm",
                ip_source=ip_client,
                server_ip=ip_server
            )
            if enginei.wabengine is not None:
                enginei._post_authentication()
                return True
        except AuthenticationFailed:
            self.challenge = None
        except LicenseException:
            self.challenge = None
        except Exception:
            import traceback
            Logger().info("Engine passthrough_authenticate failed: "
                          "(((%s)))" % traceback.format_exc())
            raise
        return False

    def gssapi_authenticate(self, enginei, wab_login, ip_client, ip_server):
        try:
            self._init_client()
            enginei.wabengine = self.client.authenticate_gssapi(
                username=wab_login,
                realm="realm",
                ip_source=ip_client,
                server_ip=ip_server
            )
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
                    wab_login, ip_client, "", ip_server)
        except AuthenticationFailed:
            self.challenge = None
        except LicenseException:
            self.challenge = None
        except Exception:
            import traceback
            Logger().info("Engine passthrough_authenticate failed: "
                          "(((%s)))" % traceback.format_exc())
            raise
        return False

    def pubkey_authenticate(self, enginei, wab_login, ip_client, pubkey,
                            ip_server):
        try:
            self._init_client()
            enginei.wabengine = self.client.authenticate(
                username=wab_login,
                password=pubkey,
                ip_source=ip_client,
                challenge=self.challenge,
                publicKey=True,
                server_ip=ip_server
            )
            self.challenge = None
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
                    wab_login, ip_client, "", ip_server)
        except AuthenticationFailed:
            self.challenge = None
        except LicenseException:
            self.challenge = None
        except Exception:
            self.challenge = None
            import traceback
            Logger().info("Engine pubkey_authenticate failed: "
                          "(((%s)))" % traceback.format_exc())
            raise
        return False
