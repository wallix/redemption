import json

from logger import Logger


try:
    from wallixconst.approval import (
        APPROVAL_PENDING as S_PENDING,
        APPROVAL_NONE as S_NONE,
    )
    from wallixconst.misc import (
        OK as S_OK,
        ERROR as S_ERROR,
    )
    from wallixconst.chgpasswd import (
        CRED_TYPE_PASSWORD,
        CRED_TYPE_SSH_KEY,
        CRED_DATA_PRIVATE_KEY,
        CRED_DATA_SSH_CERTIFICATE,
    )
    CRED_DATA_LOGIN = "login"
    CRED_DATA_ACCOUNT_UID = "account_uid"
    CRED_INDEX = "credentials"
except Exception as e:
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
    except Exception as e:
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
        self.scenario_rights = None

    def get_target_login(self, right):
        # Logger().debug("CHECKOUTENGINE get_target_login")
        target_uid = right['target_uid']
        tright, credentials = self.session_credentials.get(target_uid,
                                                           ({}, {}))
        login = credentials.get(CRED_DATA_LOGIN)
        return login

    def get_target_passwords(self, right):
        # Logger().debug("CHECKOUTENGINE get_target_passwords")
        # Use for password vault or mapping
        target_uid = right['target_uid']
        tright, credentials = self.session_credentials.get(target_uid,
                                                           ({}, {}))
        passwords = credentials.get(CRED_TYPE_PASSWORD, [])
        return passwords

    def get_target_privkeys(self, right):
        # Logger().debug("CHECKOUTENGINE get_target_privkeys")
        target_uid = right['target_uid']
        tright, credentials = self.session_credentials.get(target_uid,
                                                           ({}, {}))
        privkeys = [(cred.get(CRED_DATA_PRIVATE_KEY),
                     cred.get("passphrase"),
                     cred.get(CRED_DATA_SSH_CERTIFICATE))
                    for cred in credentials.get(CRED_TYPE_SSH_KEY, [])]
        return privkeys

    def get_primary_password(self, right):
        # Logger().debug("CHECKOUTENGINE get_primary_password")
        if not right.get('is_am'):
            # if is_am, password in credentials is from primary account
            return None
        target_uid = right['target_uid']
        tright, credentials = self.session_credentials.get(target_uid,
                                                           ({}, {}))
        passwords = credentials.get(CRED_TYPE_PASSWORD, [])
        password = None
        if passwords:
            password = passwords[0]
        return password

    def get_scenario_account_infos(self, account_name,
                                   domain_name, device_name):
        # Logger().debug("CHECKOUTENGINE get_scenario_account_infos")
        account = (account_name, domain_name, device_name)
        res = self.check_scenario_account(account_name,
                                          domain_name,
                                          device_name)
        if not res:
            return None
        right, creds = self.scenario_credentials.get(account, (None, {}))
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
            right, creds = self.checkout_scenario_account(
                account_name=account_name,
                domain_name=domain_name,
                device_name=device_name
            )
            if right is None:
                Logger().info("No credentials in scenario account")
                return False
            self.scenario_credentials[account] = (right, creds)
        return True

    def update_scenario_rights(self):
        if self.scenario_rights is None:
            try:
                Logger().debug("** CALL get_user_rights_by_type")
                rights = self.engine.get_user_rights_by_type('account',
                                                             scenario=True)
                Logger().debug("** END get_user_rights_by_type")
                if rights and (type(rights[0]) == str):
                    rights = map(json.loads, rights)
                self.scenario_rights = rights
            except Exception as e:
                Logger().debug("Engine get_user_rights_by_type failed: %s" % e)

    def checkout_scenario_account(self, account_name,
                                  domain_name, device_name):
        self.update_scenario_rights()
        if self.scenario_rights is None:
            return None, {}

        def match_account_name(x):
            return x['account_name'] == account_name

        def match_domain_name(x):
            return x['domain_cn'] == domain_name

        def match_global_domain(x):
            return (not device_name
                    and x['device_cn'] is None
                    and x['application_cn'] is None)

        def match_local_domain(x):
            return (device_name is not None
                    and device_name in (x['device_cn'], x['application_cn']))

        matched_rights = (
            right for right in self.scenario_rights if (
                match_account_name(right)
                and match_domain_name(right)
                and (match_global_domain(right)
                     or match_local_domain(right))
            )
        )

        for right in matched_rights:
            try:
                Logger().debug("** CALL checkout_account (scenario)")
                status, infos = self.engine.checkout_account(right,
                                                             session=True)
                Logger().debug("** END checkout_account (scenario)")
            except Exception as e:
                Logger().debug(
                    "Engine checkout_account (scenario) failed: %s" % e
                )
                continue
            if status in STATUS_SUCCESS and CRED_INDEX in infos:
                return right, infos[CRED_INDEX]
        return None, {}

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
            sright, creds = self.scenario_credentials.get(account)
            try:
                Logger().debug("** CALL checkin_account (scenario)")
                self.engine.checkin_account(
                    right=sright,
                    session=True
                )
                Logger().debug("** END checkin_account (scenario)")
            except Exception as e:
                Logger().debug(
                    "Engine checkin_scenario_account failed: %s" % e
                )
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
            sright, creds = self.scenario_credentials.get(account)
            try:
                Logger().debug("** CALL checkin_account (scenario)")
                self.engine.checkin_account(
                    right=sright,
                    session=True
                )
                Logger().debug("** END checkin_account (scenario)")
            except Exception as e:
                Logger().debug(
                    "Engine checkin_scenario_account failed: %s" % e
                )
        self.scenario_credentials.clear()
        self.scenario_rights = None
