#!/usr/bin/python
# -*- coding: utf-8 -*-
# flake8: noqa

from wabengine.common.exception import AuthenticationFailed
from wabengine.common.exception import AuthenticationChallenged
from wabengine.common.exception import MultiFactorAuthentication
from wabengine.common.exception import LicenseException
from wabengine.common.exception import MustChangePassword
from wabengine.common.exception import AccountLocked
from wabengine.common.exception import SessionAlreadyStopped
from wallixgenericnotifier import Notify, CX_EQUIPMENT, PATTERN_FOUND, \
    PRIMARY_CX_FAILED, SECONDARY_CX_FAILED, \
    RDP_PATTERN_FOUND, RDP_PROCESS_FOUND, RDP_OUTCXN_FOUND, FILESYSTEM_FULL
from wabconfig import Config
from wabengine.client.checker import Checker
from wallixconst.authentication import PASSWORD_VAULT, \
    PASSWORD_INTERACTIVE, PUBKEY_VAULT, PUBKEY_AGENT_FORWARDING, \
    KERBEROS_FORWARDING, PASSWORD_MAPPING, SUPPORTED_AUTHENTICATION_METHODS
from wallixconst.account import AM_IL_DOMAIN
from wallixconst.trace import LOCAL_TRACE_PATH_RDP
from wabx509 import AuthX509


def read_config_file(modulename="sesman"):
    return Config(modulename=modulename)


def read_config_value(section=None, key=None, default=None):
    config = read_config_file()
    section_conf = config
    value = default
    if section is not None:
        section_conf = config.get(section, {})
    if key is not None:
        value = section_conf.get(key, default)
    return value


def eval_bastion_license():
    """Upgrade eval license from L to L2 format"""
    from wabengine.client.utils.misc import get_ctrl

    ctrl = get_ctrl()
    try:
        # proc L2 instanciation and migration
        ctrl.begin_transaction()
        __ = ctrl.license  # noqa
    except Exception as exp:
        ctrl.rollback_transaction()
        raise exp
    else:
        ctrl.commit_transaction()
    finally:
        ctrl.close_session()
