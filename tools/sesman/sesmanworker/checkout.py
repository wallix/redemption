from logger import Logger
try:
    from wabengine.common.const import (
        APPROVAL_PENDING as S_PENDING,
        APPROVAL_NONE as S_NONE,
        OK as S_OK,
        ERROR as S_ERROR,
        CRED_TYPE_PASSWORD,
        CRED_TYPE_SSH_KEY,
        CRED_DATA_PRIVATE_KEY,
        CRED_DATA_SSH_CERTIFICATE
    )
    CRED_DATA_LOGIN = "login"
    CRED_DATA_ACCOUNT_UID = "account_uid"
    CRED_INDEX = "credentials"
except Exception, e:
    import traceback
    tracelog = traceback.format_exc(e)
    try:
        from fake.const import (
            APPROVAL_PENDING as S_PENDING,
            APPROVAL_NONE as S_NONE,
            OK as S_OK,
            ERROR as S_ERROR,
            CRED_TYPE_PASSWORD,
            CRED_TYPE_SSH_KEY,
            CRED_DATA_PRIVATE_KEY,
            CRED_DATA_SSH_CERTIFICATE,
            CRED_DATA_LOGIN,
            CRED_DATA_ACCOUNT_UID,
            CRED_INDEX,
        )
    except Exception, e:
        Logger().info("Wabengine const LOADING FAILED %s" % tracelog)

APPROVAL_ACCEPTED = "APPROVAL_ACCEPTED"
APPROVAL_REJECTED = "APPROVAL_REJECTED"
APPROVAL_PENDING = "APPROVAL_PENDING"
APPROVAL_NONE = "APPROVAL_NONE"
STATUS_SUCCESS = [S_OK]
STATUS_PENDING = [S_PENDING]
STATUS_APPR_NEEDED = [S_NONE]

class CheckoutEngine(object):
    def __init__(self, engine):
        self.engine = engine
        self.session_credentials = {}
        self.scenario_credentials = {}

    def get_target_login(self, right):
        # Logger().debug("CHECKOUTENGINE get_target_login")
        target_uid = right['target_uid']
        tright, credentials = self.session_credentials.get(target_uid, ({}, {}))
        login = credentials.get(CRED_DATA_LOGIN)
        return login

    def get_target_passwords(self, right):
        # Logger().debug("CHECKOUTENGINE get_target_passwords")
        target_uid = right['target_uid']
        tright, credentials = self.session_credentials.get(target_uid, ({}, {}))
        passwords = credentials.get(CRED_TYPE_PASSWORD, [])
        return passwords

    def get_target_privkeys(self, right):
        # Logger().debug("CHECKOUTENGINE get_target_privkeys")
        target_uid = right['target_uid']
        tright, credentials = self.session_credentials.get(target_uid, ({}, {}))
        privkeys = [ (cred.get(CRED_DATA_PRIVATE_KEY),
                      cred.get("passphrase"),
                      cred.get(CRED_DATA_SSH_CERTIFICATE)) \
                     for cred in credentials.get(CRED_TYPE_SSH_KEY, []) ]
        return privkeys

    def get_scenario_account_infos(self, account_name, domain_name, device_name):
        # Logger().debug("CHECKOUTENGINE get_scenario_account_infos")
        account = (account_name, domain_name, device_name)
        res = self.check_scenario_account(account_name,
                                          domain_name,
                                          device_name)
        if not res:
            return None
        acc_uid, creds = self.scenario_credentials.get(account, (0, {}))
        if not creds:
            return None
        from collections import namedtuple
        account_infos = namedtuple('account_infos', 'passwords login')
        a_infos = account_infos(
            creds.get(CRED_TYPE_PASSWORD, []),
            creds.get(CRED_DATA_LOGIN, None)
        )
        return a_infos

    def check_target(self, right, request_ticket=None):
        # Logger().debug("CHECKOUTENGINE check_target")
        if request_ticket:
            try:
                Logger().debug("** CALL request_approval")
                status, infos = self.engine.request_approval(
                    right=right,
                    approval_fields=request_ticket
                )
                Logger().debug("** END request_approval")
            except Exception as e:
                Logger().debug("Engine request_approval failed: %s" % e)
        try:
            Logger().debug("** CALL checkout_account")
            status, infos = self.engine.checkout_account(
                right=right,
                session=True
            )
            Logger().debug("** END checkout_account")
        except Exception as e:
            Logger().debug("Engine checkout_account failed: %s" % e)
            status = S_ERROR
            infos = {'message': u'An internal error has occured.'}
        return_status = APPROVAL_REJECTED
        if status in STATUS_SUCCESS:
            target_uid = right['target_uid']
            if self.session_credentials.get(target_uid) is None:
                creds = infos.get(CRED_INDEX, {})
                self.session_credentials[target_uid] = (right, creds)
            return_status = APPROVAL_ACCEPTED
        if status in STATUS_PENDING:
            return_status = APPROVAL_PENDING
        if status in STATUS_APPR_NEEDED:
            return_status = APPROVAL_NONE
        return return_status, infos

    def check_scenario_account(self, account_name, domain_name, device_name):
        # Logger().debug("CHECKOUTENGINE check_scenario_account")
        account = (account_name, domain_name, device_name)
        if account not in self.scenario_credentials:
            try:
                Logger().debug("** CALL checkout_scenario_account")
                status, infos = self.engine.checkout_scenario_account(
                    account_name=account_name,
                    domain_name=domain_name,
                    device_name=device_name
                )
                Logger().debug("** END checkout_scenario_account")
            except Exception as e:
                Logger().debug("Engine checkin_scenario_account failed: %s" % e)
            if CRED_INDEX not in infos or CRED_DATA_ACCOUNT_UID not in infos:
                Logger().info("No credentials in scenario account")
                return False
            scenario_creds = (infos[CRED_DATA_ACCOUNT_UID], infos[CRED_INDEX])
            self.scenario_credentials[account] = scenario_creds
        return True

    def release_target(self, right):
        # Logger().debug("CHECKOUTENGINE release_target")
        target_uid = right['target_uid']
        if target_uid in self.session_credentials:
            tright, creds = self.session_credentials.get(target_uid, ({}, {}))
            try:
                Logger().debug("** CALL checkin_account")
                self.engine.checkin_account(
                    right=tright,
                    session=True
                )
                Logger().debug("** END checkin_account")
            except Exception as e:
                Logger().debug("Engine checkin_account failed: %s" % e)
            self.session_credentials.pop(target_uid, None)

    def release_scenario_account(self, acc_name, dom_name, dev_name):
        # Logger().debug("CHECKOUTENGINE release_scenario_account")
        account = (acc_name, dom_name, dev_name)
        if account in self.scenario_credentials:
            acc_uid, creds = self.scenario_credentials.get(account)
            try:
                Logger().debug("** CALL checkin_scenario_account")
                self.engine.checkin_scenario_account(
                    account_uid=acc_uid
                )
                Logger().debug("** END checkin_scenario_account")
            except Exception as e:
                Logger().debug("Engine checkin_scenario_account failed: %s" % e)
            self.scenario_credentials.pop(account, None)

    def release_all(self):
        # Logger().debug("CHECKOUTENGINE release_all")
        for target_uid in self.session_credentials:
            tright, creds = self.session_credentials.get(target_uid, ({}, {}))
            try:
                Logger().debug("** CALL checkin_account")
                self.engine.checkin_account(
                    right=tright,
                    session=True
                )
                Logger().debug("** END checkin_account")
            except Exception as e:
                Logger().debug("Engine checkin_account failed: %s" % e)
        self.session_credentials.clear()
        for account in self.scenario_credentials:
            acc_uid, creds = self.scenario_credentials.get(account)
            try:
                Logger().debug("** CALL checkin_scenario_account")
                self.engine.checkin_scenario_account(
                    account_uid=acc_uid
                )
                Logger().debug("** END checkin_scenario_account")
            except Exception as e:
                Logger().debug("Engine checkin_scenario_account failed: %s" % e)
        self.scenario_credentials.clear()
