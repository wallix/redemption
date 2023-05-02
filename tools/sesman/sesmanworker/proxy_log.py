##
# Copyright (c) 2023 WALLIX. All rights reserved.
# Licensed computer software. Property of WALLIX.
##
import syslog


class RdpProxyLog:
    def __init__(self):
        syslog.openlog('rdpproxy')
        self._context = '[rdpproxy] '

    def update_context(self, psid, user) -> None:
        self._context = f'[rdpproxy] psid="{psid}" user="{user}"'

    def log(self, type, **kwargs) -> None:
        target = kwargs.pop('target', None)
        arg_list = list(kwargs.items())
        if target:
            arg_list[:0] = [('target', target)]
        arg_list[:0] = [('type', type)]
        args = ' '.join(f'{k}="{self.escape_bs_dq(v)}"'
                        for (k, v) in arg_list if v)
        syslog.syslog(syslog.LOG_INFO, f'{self._context} {args}')

    @staticmethod
    def escape_bs_dq(string):
        if isinstance(string, str):
            return string.replace('\\', '\\\\').replace('"', '\\"')
        return string
