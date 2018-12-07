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
    tracelog = traceback.format_exc()
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
        self.pm_credentials = {}
        self.pm_rights = None

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
        # TODO: same as check_account_by_type with 'scenario' account_type
        #       with namedtuple result instead of dict
        # Logger().debug("CHECKOUTENGINE get_scenario_account_infos")
        account = (account_name, domain_name, device_name)
        res = self._update_creds_with_account_by_type(
            account_name=account_name,
            domain_name=domain_name,
            device_name=device_name,
            account_type='scenario'
        )
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

    def get_pm_account_infos(self, account_name,
                             domain_name, device_name):
        # TODO: same as check_account_by_type with 'pm' account_type
        #       with namedtuple result instead of dict
        # Logger().debug("CHECKOUTENGINE get_pm_account_infos")
        account = (account_name, domain_name, device_name)
        res = self._update_creds_with_account_by_type(
            account_name=account_name,
            domain_name=domain_name,
            device_name=device_name,
            account_type='pm'
        )
        if not res:
            return None
        right, creds = self.pm_credentials.get(account, (None, {}))
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
        """
        This function check a target:
        It checkout an account and returns the status and infos
        - If it require an approval, the status is APPROVAL_NONE
          and infos contains form to fill for a request ticket
        - If an approval is pending for validation,
          the status is APPROVAL_PENDING and infos contains
          target informations to show to user
        - If target is not allowed to access or there's an Internal error
          the status is APPROVAL_REJECTED and infos contains more information
        - If target is allowed, status is APPROVAL_ACCEPTED
          and infos contains available credentials.

        Once the access checking is accepted,
        the credentials are saved on cache and available with gets methods:
        get_target_login, get_target_passwords, get_target_privkeys
        and get_primary_password

        :param dict right: right to check access
        :param dict request_ticket: contains filled tickets
                                    if approval is needed
        :rtype: status, dict
        :return: status of the access checking, and more information
        """
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

    def check_account_by_type(self, account_name, domain_name, device_name,
                              account_type=None):
        """
        This function retrieve informations (credentials, login)
        of a scenario or password management account specified by its
        name, domain and device (or application)

        If the account exist and can be checkout, information is saved on cache
        If credentials are already on cache, just return them.

        :param str account_name: name of the account
        :param str domain_name: domain of the account
                                can be global or local
        :param str device_name: device of the account
                                if empty, domain_name is global
        :param str account_type: type of account to retrieve
                                 can be 'scenario' or 'pm'
        :rtype: dict { 'login', 'password' }
        :return: credentials
        """
        # Logger().debug("CHECKOUTENGINE check_account_by_type")
        account = (account_name, domain_name, device_name)
        res = self._update_creds_with_account_by_type(
            account_name=account_name,
            domain_name=domain_name,
            device_name=device_name,
            account_type=account_type
        )
        if not res:
            return None
        table_creds = (self.pm_credentials if account_type == 'pm' else
                       self.scenario_credentials)
        right, creds = table_creds.get(account, (None, {}))
        if not creds or not creds.get(CRED_TYPE_PASSWORD):
            Logger().debug("check_account_by_type: missing password")
            return None
        passwords = creds.get(CRED_TYPE_PASSWORD, [])
        a_infos = {
            'password': passwords[0] if passwords else None,
            'login': creds.get(CRED_DATA_LOGIN, None)
        }
        return a_infos

    def _update_creds_with_account_by_type(self, account_name, domain_name,
                                           device_name, account_type=None):
        """
        This function retrieve account credentials by type (scenario or pm)
        and save them on cache.
        If credentials are already on cache, just return them.

        :param str account_name: name of the account
        :param str domain_name: domain of the account
                                can be global or local
        :param str device_name: device of the account
                                if empty, domain_name is global
        :param str account_type: type of account to retrieve
                                 can be 'scenario' or 'pm'
        :rtype: dict
        :return: rights by type
        """
        if account_type is None:
            account_type = 'scenario'
        # Logger().debug("CHECKOUTENGINE check_account_by_type (%s)" %
        #                account_type)
        table_creds = None
        if account_type == 'scenario':
            table_creds = self.scenario_credentials
        if account_type == 'pm':
            table_creds = self.pm_credentials
        if table_creds is None:
            Logger().debug("_check_account_by_type: Invalid account_type %s" %
                           account_type)
            return False
        account = (account_name, domain_name, device_name)
        if account not in table_creds:
            right, creds = self._checkout_account_by_type(
                account_name=account_name,
                domain_name=domain_name,
                device_name=device_name,
                account_type=account_type
            )
            if right is None:
                Logger().info("No credentials in %s account" % account_type)
                return False
            table_creds[account] = (right, creds)
        return True

    def _update_rights_by_type(self, account_type=None):
        """
        This function retrieve rights by type (scenario or pm)
        and save them on cache.
        If rights are already on cache, just return them.

        :param str account_type: type of rights to retrieve
                                 can be 'scenario' or 'pm'
        :rtype: dict
        :return: rights by type
        """
        if account_type is None:
            account_type = 'scenario'
        table_rights = None
        if account_type == 'scenario':
            table_rights = self.scenario_rights
        elif account_type == 'pm':
            table_rights = self.pm_rights
        else:
            Logger().debug("_get_rights_by_type: Invalid account_type %s" %
                           account_type)
            return None
        if table_rights is None:
            try:
                Logger().debug("** CALL get_user_rights_by_type (%s)" %
                               account_type)
                rights = self.engine.get_user_rights_by_type(
                    'account',
                    scenario=(account_type == 'scenario')
                )
                Logger().debug("** END get_user_rights_by_type")
                if rights and (type(rights[0]) == str):
                    rights = map(json.loads, rights)
                table_rights = rights
                if account_type == 'scenario':
                    self.scenario_rights = rights
                elif account_type == 'pm':
                    self.pm_rights = rights
            except Exception as e:
                Logger().debug("Engine get_user_rights_by_type failed: %s" % e)
        return table_rights

    def _checkout_account_by_type(self, account_name, domain_name, device_name,
                                  account_type=None):
        """
        This function checkout an account by type (scenario or pm)
        It first retrieve the rights allowed for the session
        (or use the cached one)
        If a requested account match one in the lists of rights,
        do the checkout of it and return the result (right and credentials)
        :param str account_name: name of the account to checkout
        :param str domain_name: domain of the account to checkout
                                can be global or local
        :param str device_name: device of the account to checkout
                                if empty, domain_name is global
        :param str account_type: type of rights to retrieve
                                 can be 'scenario' or 'pm'
        :rtype: tuple (dict, dict)
        :return: rights of the matching account
                 and credentials of the checked out account
        """
        list_rights = self._update_rights_by_type(account_type)
        if list_rights is None:
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
            right for right in list_rights if (
                match_account_name(right)
                and match_domain_name(right)
                and (match_global_domain(right)
                     or match_local_domain(right))
            )
        )

        for right in matched_rights:
            try:
                Logger().debug("** CALL checkout_account (%s)" % account_type)
                status, infos = self.engine.checkout_account(
                    right,
                    session=True
                )
                Logger().debug("** END checkout_account (%s)" % account_type)
            except Exception as e:
                Logger().debug(
                    "Engine checkout_account (%s) failed: %s" %
                    (account_type, e)
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

    def release_pm_account(self, acc_name, dom_name, dev_name):
        # Logger().debug("CHECKOUTENGINE release_pm_account")
        account = (acc_name, dom_name, dev_name)
        if account in self.pm_credentials:
            sright, creds = self.pm_credentials.get(account)
            try:
                Logger().debug("** CALL checkin_account (pm)")
                self.engine.checkin_account(
                    right=sright,
                    session=True
                )
                Logger().debug("** END checkin_account (pm)")
            except Exception as e:
                Logger().debug(
                    "Engine checkin_pm_account failed: %s" % e
                )
            self.pm_credentials.pop(account, None)

    def release_account_by_type(self, acc_name, dom_name, dev_name,
                                account_type=None):
        # Logger().debug("CHECKOUTENGINE release_account_by_type")
        table_creds = (self.pm_credentials if account_type == 'pm' else
                       self.scenario_credentials)
        account = (acc_name, dom_name, dev_name)
        if account in table_creds:
            sright, creds = table_creds.get(account)
            try:
                Logger().debug("** CALL checkin_account (%s)" % account_type)
                self.engine.checkin_account(
                    right=sright,
                    session=True
                )
                Logger().debug("** END checkin_account (%s)" % account_type)
            except Exception as e:
                Logger().debug(
                    "Engine checkin_account failed: %s" % e
                )
            table_creds.pop(account, None)

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
        for account in self.pm_credentials:
            sright, creds = self.pm_credentials.get(account)
            try:
                Logger().debug("** CALL checkin_account (pm)")
                self.engine.checkin_account(
                    right=sright,
                    session=True
                )
                Logger().debug("** END checkin_account (pm)")
            except Exception as e:
                Logger().debug(
                    "Engine checkin_pm_account failed: %s" % e
                )
        self.pm_credentials.clear()
        self.pm_rights = None
