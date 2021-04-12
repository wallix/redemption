#!/usr/bin/python -O
# -*- coding: iso-8859-1 -*-
#
# Copyright (c) 2021 WALLIX, SAS. All rights reserved.
#
# Licensed computer software. Property of WALLIX.
# Author(s) : Meng Tan <dev@wallix.com>


import re

from logger import Logger


def parse_param(param, forced_device=None):
    """
    Extract account representation

    string format is <account_name>@<domain_name>[@<device_name>]

    If @<device_name> is absent, <domain_name> is a global domain name
    Else <domain_name> is a local domain name of the forced_device

    if 'forced_device' is not None
    <device_name> should be empty or equal to forced_device
    Else <device_name> can be another device

    """
    parsed = param.rsplit("@", 2)
    if len(parsed) > 1:
        account_name = parsed[0]
        domain_name = parsed[1]
        if (len(parsed) == 3
            and parsed[2]
            and forced_device
            and (parsed[2] != forced_device)):
            return None
        device_name = (forced_device or parsed[2]) if len(parsed) == 3 \
            else None
        return account_name, domain_name, device_name
    else:
        return None


def replace_token(param, replace_dict):
    """
    token = match.group(0) # <..>
    field = match.group(1) # field
    """
    if param is None:
        return None
    field_token = re.compile(r"\<(\w*)\>")
    matches = field_token.finditer(param)
    for res in matches:
        token = res.group(0)
        field = res.group(1)
        param = param.replace(token, replace_dict.get(field.lower(), ""))
    return param


def parse_account(param, replace_dict, force_device=False):
    param = replace_token(param, replace_dict)
    device_name = replace_dict.get("device") if force_device else None
    acc_tuple = parse_param(param, device_name)
    return acc_tuple


def resolve_scenario_account(enginei, field, param, force_device=True, default=None):
    """
    Get password or login field from scenario account

    :param enginei: engine interface
        must implement:
        - get_target_login_info
        - get_username
        - get_account_infos_by_type
    :param field: str
        requested field of scenario account, must be "password" or "login"
    :param param: str
        string representing scenario account:
        string format is account_name@domain_name[@device_name]
        each part can contain placeholder <attribute>
    :param force_device: bool
        force device_name part in param to match current device
        True by default
    :param default: bool
        default value to be returned in case resolution fails;
        if set to None, param will be returned
        True by default

    :return: requested field (password or login) of scenario account
             if param is not a scenario account, returns its value
    :rtype: str
    """
    if default is None:
        default = param
    session_infos = enginei.get_target_login_info().get_target_dict()
    session_infos["user"] = enginei.get_username()
    acc_tuple = parse_account(param, session_infos, force_device)
    if acc_tuple is None:
        return default
    acc_infos = enginei.get_account_infos_by_type(
        *acc_tuple,
        account_type="scenario"
    )
    if acc_infos is None:
        Logger().debug(
            "Error: Unable to retrieve account info from '%s'" %
            param
        )
        return default
    return acc_infos.get(field.lower())
