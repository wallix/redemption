##
# Copyright (c) 2017 WALLIX. All rights reserved.
# Licensed computer software. Property of WALLIX.
# Product Name: WALLIX Bastion v6.0
# Author(s): Olivier Hervieu, Christophe Grosjean, Raphael Zhou, Meng Tan
# Id: $Id$
# URL: $URL$
# Module description:  Sesman Worker
##
import random
import os
import signal
import traceback
import json
import re
import itertools
from logger import Logger

from struct import unpack_from, pack
from select import select
from time import time, ctime, mktime
from datetime import datetime
import socket
from socket import gethostname
from typing import Iterable, Any, Tuple, Optional, Generator, Dict, Union

from .addrutils import check_hostname_in_subnet
from .sesmanconf import TR, SESMANCONF, Sesmsg
from . import engine
from .sesmanconnpolicyspec import cp_spec
from .sesmanbacktoselector import (
    back_to_selector_default_reinit,
    back_to_selector_default_sent
)

from .engine import (LOCAL_TRACE_PATH_RDP,
                     SOCK_PATH_DIR,
                     APPROVAL_ACCEPTED,
                     APPROVAL_REJECTED,
                     APPROVAL_PENDING,
                     APPROVAL_NONE,
                     APPREQ_REQUIRED,
                     APPREQ_OPTIONAL,
                     PASSWORD_VAULT,
                     PASSWORD_INTERACTIVE,
                     PASSWORD_MAPPING,
                     TargetContext,
                     RDP,
                     VNC
                     )

import syslog

from .logtime import logtimer, Steps as LogSteps, logtime_function_pause


def collection_has_more(
        iterable: Iterable[Any]
) -> Generator[Tuple[Any, bool], None, None]:
    it = iter(iterable)
    try:
        cur_item = next(it)
    except StopIteration:
        return
    for item in it:
        yield cur_item, True
        cur_item = item
    yield cur_item, False


def parse_duration(duration: str) -> int:
    """
    duration format: {hours}h{min}m or {hours}h or {min}m
    """
    if duration:
        mres = re.search(r"(?:(\d+)h)?(?:(\d+)m)?", duration)
        if mres is not None:
            d = (
                60 * 60 * int(mres.group(1) or 0)
              + 60 * int(mres.group(2) or 0)
            )
            return d or 3600

    return 3600


_convert_to_int = lambda x: (int(x) if isinstance(x, str) else x)
_convert_to_bool = lambda x: (x.lower() == 'true' if isinstance(x, str) else x)
_identity = lambda x: x

KEYMAPPING = {
    # exchange key : (acl key, convert func)
    'height': ('video_height', _convert_to_int),
    'width': ('video_width', _convert_to_int),
    'rt_ready': ('rt', _convert_to_bool),
    'sharing_ready': ('sharing_allow', _convert_to_bool),
    'fdx_path': ('fdx_path', _identity),
    'smartcard_login': ('effective_login', _identity),
    'native_session_id': ('native_session_id', _identity),
}
EXPECTING_KEYS = list(KEYMAPPING.keys())


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


MAGICASK = ('UNLIKELYVALUEMAGICASPICONSTANTS'
            '3141592926ISUSEDTONOTIFYTHEVALUEMUSTBEASKED')

KEEPALIVE_INTERVAL = 30
KEEPALIVE_GRACEDELAY = 30
KEEPALIVE_TIMEOUT = KEEPALIVE_INTERVAL + KEEPALIVE_GRACEDELAY

WORKFLOW_POLL_INTERVAL = 5


def mundane(value):
    if value == MAGICASK:
        return 'Unknown'
    return value


def rvalue(value):
    if value == MAGICASK:
        return ''
    return value


DEBUG = False


def truncat_string(item, maxsize=20):
    return (item[:maxsize] + '..') if len(item) > maxsize else item


def parse_auth(username: str) -> Tuple[str, Optional[Tuple[str, str, str, str]]]:
    """
    Extract actual username and target if provided
    from authentication identity

    string format is <secondaryuser>@<target>:<service>:<group>:<primaryuser>
    always return primaryuser and either secondary target or None

    Note: primary user can be a path instead when this function
    is called to parse scp or sftp arguments.

    Because of compatibility issues with some ssh command line tools
    '+' can be used instead of ':'

    fields can be missing (typically service and group if there is
    no ambiguity)

    """
    user_dev_service_group, sep, primary = username.rpartition('+')
    if not sep:
        user_dev_service_group, sep, primary = username.rpartition(':')
    if sep:
        user_dev_service, sep, group = user_dev_service_group.rpartition(sep)
        if not sep:
            # service and group not provided
            user_at_dev, service, group = user_dev_service_group, '', ''
        else:
            user_at_dev, sep, service = user_dev_service.rpartition(sep)
            if not sep:
                # group not provided
                user_at_dev, service, group = user_dev_service, group, ''
        user, sep, dev = user_at_dev.rpartition('@')
        if sep:
            return primary, (user, dev, service, group)
    return username, None


# PM Function
def parse_param(param: str) -> Optional[Tuple[str, str, Optional[str]]]:
    """
    Extract account representation

    string format is <account_name>@<domain_name>[@<device_name>]

    If @<device_name> is absent, <domain_name> is a global domain name
    Else <domain_name> is a local domain name of the current_device

    if 'current_device' is not None
    <device_name> should be empty or equal to current_device
    Else <device_name> can be another device

    """
    parsed = param.rsplit("@", 2)
    if len(parsed) > 1:
        account_name = parsed[0]
        domain_name = parsed[1]
        device_name = parsed[2] if len(parsed) == 3 else None
        return account_name, domain_name, device_name
    return None


# PM Function
def pm_request(engine, request: str) -> Dict[str, Union[str, int]]:
    Logger().debug(f"pm_request: '{request}'")
    if request and request[0] == '/':
        request = request[1:]
    reql = request.split('/', 2)
    if len(reql) < 3:
        Logger().debug("pm_request: invalid request")
        return {
            'response_code': 202,
            'response_message': "Invalid request format",
        }
    req_name, req_cmd, req_param = reql
    if req_name != 'targetpasswords':
        Logger().debug("pm_request: invalid request name")
        return {
            'response_code': 203,
            'response_message': "Invalid request name",
        }
    res_param = parse_param(req_param)
    if res_param is None:
        Logger().debug("pm_request: invalid params")
        return {
            'response_code': 204,
            'response_message': "Invalid request parameters",
        }
    acc_n, domain_n, dev_n = res_param
    if req_cmd == 'checkout':
        res = engine.get_account_infos_by_type(
            account_name=acc_n,
            domain_name=domain_n,
            device_name=dev_n,
            with_ssh_key=False,
            account_type='pm'
        )
        if res is not None:
            res['response_code'] = 0
        else:
            res = {
                'response_code': 301,
                'response_message': "Account not found",
            }
        return res
    if req_cmd == 'checkin':
        engine.release_account_by_type(
            account_name=acc_n,
            domain_name=domain_n,
            device_name=dev_n,
            account_type='pm'
        )
        return {
            'response_code': 0,
        }
    return {
        'response_code': 205,
        'response_message': "Unknown request command",
    }


class AuthentifierSocketClosed(Exception):
    pass


class BastionSignal(Exception):
    def __init__(self, message="BastionSignal"):
        super().__init__(message)


class RTManager:
    __slots__ = ("sesman", "time_limit", "last_start")

    def __init__(self, sesman, time_limit: float):
        self.sesman = sesman
        self.time_limit = time_limit
        self.last_start = 0

    def reset(self) -> None:
        self.last_start = 0

    def start(self, current_time: float) -> None:
        Logger().debug(f"Start RT Manager at {current_time}")
        self.last_start = current_time
        if not self.sesman.shared.get("rt_display"):
            redis_addr = engine.redis.get_redis_master() or ('127.0.0.1', 6379)
            redis_config = engine.redis.get_redis_conf()
            use_tls = redis_config.get('ssl', False)
            data = {
                'rt_display': 1,
                'redis_address': redis_addr[0],
                'redis_port': redis_addr[1],
                'redis_password': redis_config.get('password', ''),
                'redis_db': redis_config.get('db', 0),
                'redis_use_tls': use_tls,
            }
            if use_tls:
                data['redis_tls_key'] = redis_config.get('ssl_keyfile')
                data['redis_tls_cert'] = redis_config.get('ssl_certfile')
                data['redis_tls_cacert'] = redis_config.get('ssl_ca_certs')
            self.sesman.send_data(data)

    def stop(self) -> None:
        self.last_start = 0
        self.send_rtdisplay(0)

    def check(self, current_time: float) -> bool:
        if (self.last_start
            and (current_time > self.last_start + self.time_limit)):
            Logger().debug(f"Check RT Manager at {current_time} STOP")
            # stop rt_display
            self.stop()
            return False
        return True

    def send_rtdisplay(self, rt_display: int) -> None:
        if self.sesman.shared.get("rt_display") != rt_display:
            Logger().debug(f"sending rt_display={rt_display}")
            self.sesman.send_data({"rt_display": rt_display})


class Sesman():

    def __init__(self, conn, addr):
        """
        =======================================================================
        __INIT__
        =======================================================================
        """
        try:
            confwab = engine.read_config_file(
                modulename='sesman',
                confdir='/var/wab/etc/sesman/config'
            )
            seswabconfig = confwab.get('sesman', {})
            SESMANCONF.conf['sesman'].update(seswabconfig)
            # Logger().info(f" WABCONFIG SESMANCONF = '{seswabconfig}'")
        except Exception:
            Logger().info("Failed to load Sesman WabConfig")
        # Logger().info(f" SESMANCONF = '{SESMANCONF['sesman']}'")
        if SESMANCONF['sesman'].get('debug', False):
            global DEBUG
            DEBUG = True

        self.rdplog = RdpProxyLog()

        self.cn = 'Unknown'

        self.proxy_conx = conn
        self.addr = addr
        self.full_path = None  # path + basename (without extension)
        self.record_filebase = None
        self.full_log_path = None

        self.engine = engine.Engine()

        self.effective_login = None
        self.source_connection_id = None

        # shared should be read from sesman but never written
        # except when sending
        self.shared = {}
        self._changed_keys = []

        self._full_user_device_account = 'Unknown'
        self.target_service_name = None
        self.target_group = None
        self.target_context = TargetContext()

        self.shared['module'] = 'login'
        self.shared['selector_group_filter'] = ''
        self.shared['selector_device_filter'] = ''
        self.shared['selector_proto_filter'] = ''
        self.shared['selector'] = 'False'
        self.shared['selector_current_page'] = '1'
        self.shared['selector_lines_per_page'] = '0'
        self.shared['real_target_device'] = MAGICASK
        self.shared['reporting'] = ''

        self._trace_type = self.engine.get_trace_type()
        self._selector_banner = self.engine.get_selector_banner()
        self.language = None
        self.pid = os.getpid()

        self.shared['target_login'] = MAGICASK
        self.shared['target_device'] = MAGICASK
        self.shared['target_host'] = MAGICASK
        self.shared['login'] = MAGICASK
        self.shared['ip_client'] = MAGICASK
        self.shared['target_protocol'] = MAGICASK
        self.shared['keyboard_layout'] = MAGICASK

        self.shared['auth_channel_answer'] = ''
        self.shared['auth_channel_target'] = ''

        self.shared['recording_started'] = 'False'

        self.shared['auth_notify'] = ''

        self.shared['login_language'] = MAGICASK

        self.internal_target = False
        self.check_session_parameters = False

        # Passthrough context
        self.passthrough_mode = SESMANCONF['sesman'].get(
            'auth_mode_passthrough', False
        )
        self.default_login = (
            SESMANCONF['sesman'].get('default_login', '').strip()
            or None
        )
        self.passthrough_target_login = None
        self.allow_back_selector = SESMANCONF['sesman'].get(
            'allow_back_to_selector', True
        )
        self.back_selector = False
        self.hide_approval_back_selector = False
        self.target_app_rights = {}

        self.login_message = ("Warning! Unauthorized access to this system "
                              "is forbidden and will be prosecuted by law.")

        self.shared['session_probe_launch_error_message'] = ''

        self.rtmanager = RTManager(
            self,
            self.engine.wabengine_conf.get("session_4eyes_timer", 60)
        )
        self.tun_process = None
        self.sharing_requests = set()

    def reset_session_var(self):
        """
        Proxy Session Vars reset
        """
        self._full_user_device_account = 'Unknown'
        self.target_service_name = None
        self.target_group = None
        self.internal_target = False
        self.passthrough_target_login = None
        self.target_context = TargetContext()

    def reset_target_session_vars(self):
        """
        Target Session Disconnection
        """
        self._full_user_device_account = 'Unknown'
        self.target_service_name = None
        self.target_group = None
        self.internal_target = False
        self.target_app_rights = {}
        self.sharing_requests = set()
        # Should set context values back to default
        for key, value in back_to_selector_default_reinit.items():
            self.shared[key] = value
        back_to_selector_default_sent["module"] = 'transitory'
        self.send_data(back_to_selector_default_sent)
        self.engine.reset_proxy_rights()
        self.rtmanager.reset()

    def load_login_message(self, language):
        try:
            self.login_message = self.engine.get_warning_message(language)
        except Exception:
            pass

    def set_language_from_keylayout(self):
        self.language = SESMANCONF.language
        french_layouts = [0x0000040C,  # French (France)
                          0x00000C0C,  # French (Canada) Canadian French
                                       #     (Legacy)
                          0x0000080C,  # French (Belgium)
                          0x0001080C,  # French (Belgium) Belgian (Comma)
                          0x0000100C]  # French (Switzerland)
        keylayout = 0
        if self.shared.get('keyboard_layout') != MAGICASK:
            try:
                keylayout = int(self.shared.get('keyboard_layout'))
            except Exception:
                pass
        if keylayout in french_layouts:
            self.language = 'fr'

        login_language = (self.shared.get('login_language').lower()
                          if (self.shared.get('login_language') != MAGICASK
                              and self.shared.get('login_language') != 'Auto')
                          else self.language)

        self.load_login_message(login_language)

    # TODO: is may be possible to delay sending data until the next
    #       input through receive_data
    def send_data(self, data):
        """ NB : Strings sent to the ReDemPtion proxy MUST be UTF-8 encoded
        * Packet format:
        uint16                   request_count
        request[request_count]   requests

        * request format:
        request := request_update | request_ask

        * request_update format:
        uint8                    "!"
        uint8                    key_size
        byte[key_size]           key
        uint32                   value_size
        byte[value_size]         value

        * request_ask format:
        uint8                    "?"
        uint8                    key_size
        byte[key_size]           key

        key and value are utf8 encoded
        uint are big-endian byte ordered
        """

        if DEBUG:
            Logger().info(f'=> send_data (update) = {data.keys()}')

        # if current language changed, send translations
        if self.language != SESMANCONF.language:
            if not self.language:
                self.set_language_from_keylayout()
            SESMANCONF.language = self.language

            data['language'] = SESMANCONF.language
            # if self.shared.get('password') == MAGICASK:
            #     data['password'] = ''

        self.shared.update(data)

        def _toval(key, value):
            key = key.encode()
            value = str(value).encode()
            key_len = len(key)
            value_len = len(value)
            return (
                True, key,
                pack(f'>1sB{key_len}sL{value_len}s',
                     b'!', key_len, key, value_len, value)
            )

        def _toask(key):
            key = key.encode()
            key_len = len(key)
            return (
                False, key,
                pack(f'>1sB{key_len}s', b'?', key_len, key)
            )

        _list = [(_toval(key, value) if value != MAGICASK else _toask(key))
                 for key, value in data.items()]
        _list.sort()

        if DEBUG:
            Logger().info(f'send_data (on the wire) length = {len(_list)}')

        _r_data = b''.join(t[2] for t in _list)
        self.proxy_conx.sendall(pack('>H', len(_list)))
        self.proxy_conx.sendall(_r_data)

    def wait_read_proxy_conx(self):
        self.proxy_conx.setblocking(False)
        r = []
        while True:
            r, w, x = select([self.proxy_conx], [], [], KEEPALIVE_INTERVAL)
            self.engine.keepalive(timeout=KEEPALIVE_TIMEOUT)
            if self.proxy_conx in r:
                break
        self.proxy_conx.setblocking(True)
        return r

    @logtime_function_pause
    def receive_data(self, expected_list=(), blocking_call=True):
        """ NB : Strings coming from the ReDemPtion proxy are UTF-8 encoded
        * Packet format:
        uint16                   request_count
        request[request_count]   requests

        * request format:
        request := request_update | request_ask

        * request_update format:
        uint8                    "!"
        uint8                    key_size
        byte[key_size]           key
        uint32                   value_size
        byte[value_size]         value

        * request_ask format:
        uint8                    "?"
        uint8                    key_size
        byte[key_size]           key

        key and value are utf8 encoded
        uint are big-endian byte ordered
        """

        if blocking_call:
            _ = self.wait_read_proxy_conx()

        self._changed_keys = []

        def read_sck():
            try:
                d = self.proxy_conx.recv(65536)
                if len(d):
                    if DEBUG:
                        Logger().debug(d)
                    return d

            except Exception:
                # Logger().info(
                #     "Failed to read data from rdpproxy authentifier socket"
                #     f"<<<{traceback.format_exc(e)}>>>"
                # )
                raise AuthentifierSocketClosed()

            if DEBUG:
                Logger().info("received_buffer (empty packet)")
            raise AuthentifierSocketClosed()

        class Buffer:
            def __init__(self):
                self._data = read_sck()
                self._offset = 0

            def reserve_data(self, n):
                while len(self._data) - self._offset < n:
                    if DEBUG:
                        Logger().info("received_buffer (big packet) "
                                      f"old = {self._offset} / {len(self._data)} ;"
                                      f"required = {n}")
                    self._data = self._data[self._offset:] + read_sck()
                    self._offset = 0

            def extract_string(self, n):
                self.reserve_data(n)
                _name = self._data[self._offset:self._offset + n]
                _name = _name.decode('utf8')
                self._offset += n
                return _name

            def unpack(self, fmt, n):
                self.reserve_data(n)
                r = unpack_from(fmt, self._data, self._offset)
                self._offset += n
                return r

            def is_empty(self):
                return len(self._data) == self._offset

        _buffer = Buffer()
        _data = {}

        while True:
            _nfield, = _buffer.unpack('>H', 2)

            if DEBUG:
                Logger().info(f"received_buffer (nfield) = {_nfield}")

            for _ in range(0, _nfield):
                _type, _n = _buffer.unpack("BB", 2)
                _key = _buffer.extract_string(_n)

                if DEBUG:
                    c = '?' if _type == 0x3f else '!'
                    Logger().info(f"received_buffer (key)   = {c}{_key}")

                if _type == 0x3f:  # b'?'
                    _data[_key] = MAGICASK
                else:
                    _n, = _buffer.unpack('>L', 4)
                    _data[_key] = _buffer.extract_string(_n)

                    if DEBUG:
                        if "password" in _key.lower():
                            Logger().info("received_buffer (value) = *****")
                        else:
                            Logger().info(
                                f"received_buffer (value) = {_data[_key]}"
                            )

            if _buffer.is_empty():
                break

            if DEBUG:
                Logger().info("received_buffer (several packet)")

        self._changed_keys += (key for key in expected_list
            if key in _data and _data[key] != self.shared.get(key))
        self.shared.update(_data)

        return True, ''

    def parse_username(self,
                       wab_login: str,
                       target_login: str,
                       target_device: str,
                       target_service: str,
                       target_group: str) -> Tuple[
                           bool,  # TODO Literal[True] for status
                           str,  # TODO Literal[''] for error message
                           str, str, str, str, str, Optional[str],
                        ]:
        effective_login = None
        wab_login, target_tuple = parse_auth(wab_login)
        if target_tuple is not None:
            (target_login, target_device,
             target_service, target_group) = target_tuple
        if self.passthrough_mode:
            Logger().info('Passthrough mode is enabled')
            if self.passthrough_target_login is None:
                self.passthrough_target_login = wab_login
            if not self.passthrough_target_login:
                Logger().info(
                    "Passthrough mode is enabled, "
                    "but login information is not provided"
                )
            if self.default_login:
                effective_login = self.passthrough_target_login
                wab_login = self.default_login
            Logger().info(f'ip_target="{self.shared.get("ip_target")}" '
                          f'real_target_device="{self.shared.get("real_target_device")}"')
        return (True, "", wab_login, target_login, target_device,
                target_service, target_group, effective_login)

    def interactive_ask_x509_connection(self):
        """
        Send a message to the proxy to prompt the user to validate x509
        in his browser.
        Wait until the user clicks Ok in Proxy prompt or until timeout
        """
        _status = False
        data_to_send = ({
            'message': TR(Sesmsg.VALID_AUTHORISATION),
            'password': 'x509',
            'module': 'confirm',
            'display_message': MAGICASK,
        })

        self.send_data(data_to_send)

        # Wait for the user to click Ok in proxy

        while self.shared.get('display_message') == MAGICASK:
            Logger().info('wait user grant or reject connection')
            _status, _error = self.receive_data()
            if not _status:
                break

            Logger().info('Data received')
            if self.shared.get('display_message').lower() != 'true':
                _status = False

        return _status

    def interactive_ask_url_redirect(self):
        """
        Send a message to the proxy to prompt the user copy to url redirection
        in his browser.
        Wait until the user clicks Ok in Proxy prompt or until timeout
        """
        _status = False

        challenge = self.engine.get_challenge()
        if not challenge:
            return _status

        message = challenge.message if challenge.message else ""
        link = challenge.link if challenge.link else False
        timeout = challenge.timeout

        data_to_send = ({
            'message': message,
            'password': 'url_redirect',
            "mod_timeout": timeout,
            'display_link': link,
            'module': 'link_confirm',
            'display_message': MAGICASK,
        })

        self.send_data(data_to_send)

        # Wait for the user to click Ok in proxy

        while self.shared.get('display_message') == MAGICASK:
            Logger().info('wait user grant or reject connection')
            _status, _error = self.receive_data()
            if not _status:
                break

            Logger().info('Data received')
            if self.shared.get('display_message').lower() != 'true':
                _status = False
                self.engine.reset_challenge()

        return _status

    def interactive_display_message(self, data_to_send):
        """ NB : Strings sent to the ReDemPtion proxy MUST be UTF-8 encoded """
        # TODO: we should not have to care about target login or device
        #       to display messages we should be able to send messages before
        #       or after defining target seamlessly
        data_to_send.update({'module': 'confirm'})

        self.send_data(data_to_send)
        _status, _error = self.receive_data()

        if self.shared.get('display_message') != 'True':
            _status, _error = False, TR(Sesmsg.NOT_DISPLAY_MESSAGE)

        return _status, _error

    def interactive_accept_message(self, data_to_send):
        data_to_send.update({'module': 'valid'})
        self.send_data(data_to_send)

        _status, _error = self.receive_data()
        if self.shared.get('accept_message') != 'True':
            _status, _error = False, TR(Sesmsg.NOT_ACCEPT_MESSAGE)

        return _status, _error

    def check_deconnection_time(self, selected_target):
        Logger().info("Checking timeframe")
        _status, _error = True, ""
        timeclose = None
        infinite_connection = False
        deconnection_time = self.engine.get_deconnection_time(selected_target)
        if not deconnection_time:
            Logger().error("No timeframe available, "
                           "Timeframe has not been checked !")
            _status = False
        if (deconnection_time == "-"
            or deconnection_time[0:4] >= "2034"):
            deconnection_time = "2034-12-31 23:59:59"
            infinite_connection = True

        now = datetime.strftime(datetime.now(), "%Y-%m-%d %H:%M:%S")
        if _status and not infinite_connection and now < deconnection_time:
            # deconnection time to epoch
            tt = datetime.strptime(
                deconnection_time, "%Y-%m-%d %H:%M:%S"
            ).timetuple()
            timeclose = int(mktime(tt))
            _status, _error = self.interactive_display_message(
                {'message': TR(Sesmsg.SESSION_CLOSED_S) % deconnection_time}
            )
        return timeclose, _status, _error

    def interactive_target(self, data_to_send):
        data_to_send.update({'module': 'interactive_target'})
        self.send_data(data_to_send)
        _status, _error = self.receive_data()
        if self.shared.get('display_message') != 'True':
            _status, _error = False, TR(Sesmsg.CONNECTION_CLOSED_BY_CLIENT)
        return _status, _error

    def complete_target_info(self, kv, allow_interactive_password,
                             allow_interactive_login):
        """
        This procedure show interactive screen to enter target host,
        target login and target password if needed:
        * Host is asked if host information is a subnet
        * Login is asked if it is a interactive login
        * password is asked if it is missing and it is allowed to ask
              for interactive password
        """
        keylist = ['target_password', 'target_login', 'target_host']
        extkv = {x: kv.get(x) for x in keylist if kv.get(x) is not None}
        tries = 3
        _status, _error = None, None
        while (tries > 0) and (_status is None):
            tries -= 1
            interactive_data = {}
            if (not extkv['target_password']
                and allow_interactive_password):
                interactive_data['target_password'] = MAGICASK
            if not extkv.get('target_login') and allow_interactive_login:
                interactive_data['target_login'] = MAGICASK
                if allow_interactive_password:
                    interactive_data['target_password'] = MAGICASK
            target_subnet = None
            if '/' in extkv.get('target_host'):  # target_host is a subnet
                target_subnet = extkv.get('target_host')
                interactive_data['target_host'] = MAGICASK
                if _error:
                    host_note = TR(Sesmsg.ERROR_S) % _error
                else:
                    host_note = TR(Sesmsg.IN_SUBNET_S) % target_subnet
                interactive_data['target_device'] = host_note
            if interactive_data:
                Logger().info("Interactive Target Info asking")
                if not target_subnet:
                    interactive_data['target_host'] = \
                        extkv.get('target_host')
                    interactive_data['target_device'] = (
                        kv.get('target_device') if self.target_context.host
                        else self.shared.get('target_device')
                    )
                if not interactive_data.get('target_password'):
                    interactive_data['target_password'] = ''
                if not interactive_data.get('target_login'):
                    interactive_data['target_login'] = \
                        extkv.get('target_login')
                _status, _error = self.interactive_target(interactive_data)
                if _status:
                    if interactive_data.get('target_password') == MAGICASK:
                        extkv['target_password'] = \
                            self.shared.get('target_password')
                    if interactive_data.get('target_login') == MAGICASK:
                        extkv['target_login'] = \
                            self.shared.get('target_login')
                    if interactive_data.get('target_host') == MAGICASK:
                        hostok, resolved_ip = check_hostname_in_subnet(
                            self.shared.get('target_host'),
                            target_subnet)

                        if hostok:
                            extkv['target_host'] = resolved_ip
                            extkv['target_device'] = \
                                self.shared.get('target_host')
                        else:
                            extkv['target_host'] = target_subnet
                            _status = None
                            _error = TR(Sesmsg.NO_MATCH_SUBNET_S_S) % (
                                truncat_string(
                                    self.shared.get('target_host')
                                ),
                                target_subnet
                            )
            else:
                _status, _error = True, "OK"
        return extkv, _status, _error

    def interactive_close(self, target, message):
        data_to_send = {
            'error_message': message,
            'trans_ok': 'OK',
            'module': 'close',
            'target_device': target,
            'target_login': self.shared.get('target_login'),
            'target_password': 'Default',
        }

        # If we send close we should expect authentifier socket will be
        # closed by the other end
        # No need to return some warning message if that happen
        self.send_data(data_to_send)
        _status, _error = self.receive_data()

        return _status, _error

    def authentify(self):
        """ Authentify the user through password engine and then retreive his rights
             The user preferred language will be set as the language to use in
             interactive messages
        return status: True: authenticated
                       None: retry (failed and continue or challenge)
                       False: failed and close connection
               err_msg: failure message in case of None or False
        """
        _status, _error = self.receive_data()
        if not _status:
            return False, _error

        login = self.shared.get('login')
        if login == MAGICASK:
            return None, TR(Sesmsg.EMPTY_USER)

        (_status, _error,
         wab_login, target_login, target_device,
         self.target_service_name, self.target_group,
         self.effective_login) = self.parse_username(
            login,
            self.shared.get('target_login'),
            self.shared.get('target_device'),
            self.target_service_name,
            self.target_group)
        if not _status:
            return None, TR(Sesmsg.INVALID_USER)

        self.rdplog.update_context(self.shared.get('psid'), wab_login)
        Logger().info(f"Continue with authentication ({login}) -> {wab_login}")

        method = "Password"

        try:
            target_info = None
            if (target_login
                and target_device
                and target_login != MAGICASK
                and target_device != MAGICASK):
                if (self.target_service_name
                    and self.target_service_name != MAGICASK):
                    target_info_str = f"{target_login}@{target_device}:{self.target_service_name}"
                else:
                    target_info_str = f"{target_login}@{target_device}"
                target_info = target_info_str.encode('utf8')

            # Check if we are using OTP
            # before trying any authentification method
            is_otp = wab_login.startswith('_OTP_')
            is_magic_password = self.shared.get('password') == MAGICASK
            is_empty_password = (
                is_magic_password
                or not self.shared.get('password')
            )

            authenticated = False

            # Check if X509 Authentication is active
            if (not is_otp and is_empty_password
                and self.engine.is_x509_connected(
                    wab_login,
                    self.shared.get('ip_client'),
                    "RDP",
                    target_info,
                    self.shared.get('ip_target'))):
                method = "X509"
                self.rdplog.log("AUTHENTICATION_TRY", method=method)
                # Prompt the user in proxy window
                # Wait for confirmation from GUI (or timeout)
                if not ((self.engine.is_x509_validated()
                         or self.interactive_ask_x509_connection())
                        and self.engine.x509_authenticate(
                            self.shared.get('ip_client'),
                            self.shared.get('ip_target'))):
                    self.rdplog.log("AUTHENTICATION_FAILURE", method=method)

                    return False, TR(Sesmsg.X509_AUTH_REFUSED_BY_USER)
                authenticated = True
            elif self.passthrough_mode:
                # Passthrough Authentification
                method = "Passthrough"
                self.rdplog.log("AUTHENTICATION_TRY", method=method)
                if not self.engine.passthrough_authenticate(
                        wab_login,
                        self.shared.get('ip_client'),
                        self.shared.get('ip_target')):
                    self.rdplog.log("AUTHENTICATION_FAILURE", method=method)
                    emsg = TR(Sesmsg.PASSTHROUGH_AUTH_FAILED_S) % wab_login
                    return False, emsg
                authenticated = True
            elif not authenticated:
                if is_otp:
                    # only try OTP
                    method = "OTP"
                    self.rdplog.log("AUTHENTICATION_TRY", method=method)
                    if not self.engine.password_authenticate(
                            wab_login,
                            self.shared.get('ip_client'),
                            "",
                            self.shared.get('ip_target')):
                        self.rdplog.log("AUTHENTICATION_FAILURE", method=method)
                        return None, TR(Sesmsg.AUTH_FAILED_WAB_S) % wab_login
                    authenticated = True
                else:
                    # check available authentication
                    check = self.engine.check_kbdint_auth(
                        wab_login,
                        self.shared.get('ip_client'),
                        self.shared.get('ip_target'),
                    )
                    if check == "url_redirect":
                        method = "URL_REDIRECT"
                        self.rdplog.log("AUTHENTICATION_TRY", method=method)
                        # Prompt the user in proxy window
                        # Wait for confirmation from GUI (or timeout)
                        if not (self.interactive_ask_url_redirect()
                                and self.engine.url_redirect_authenticate()):
                            self.rdplog.log("AUTHENTICATION_FAILURE", method=method)
                            return None, TR(Sesmsg.URL_AUTH_REFUSED_BY_USER)
                        authenticated = True
                    elif ((check == "password" and not is_empty_password)
                          or check is False):
                        # If password provided or unknown user,
                        # try password authenticate
                        method = "Password"
                        self.rdplog.log("AUTHENTICATION_TRY", method=method)
                        if ((is_magic_password and not is_otp)  # one-time pwd
                            or not self.engine.password_authenticate(
                                wab_login,
                                self.shared.get('ip_client'),
                                rvalue(self.shared.get('password')),
                                self.shared.get('ip_target'))):
                            if is_magic_password:
                                self.engine.reset_challenge()
                            self.rdplog.log("AUTHENTICATION_FAILURE", method=method)
                            return None, TR(Sesmsg.AUTH_FAILED_WAB_S) % wab_login
                        authenticated = True
                    elif (check is True
                          and self.engine.authenticated):
                        method = "MOBILE_DEVICE"
                        self.rdplog.log("AUTHENTICATION_TRY", method=method)
                        authenticated = True
                    else:
                        self.engine.reset_challenge()
                        return None, TR(Sesmsg.AUTH_FAILED_WAB_S) % wab_login

            if not authenticated:
                return None, TR(Sesmsg.AUTH_FAILED_WAB_S) % wab_login

            # At this point, User is authentified.
            if wab_login.startswith('_OTP_'):
                method = "OTP"
                real_wab_login = self.engine.get_username()
                # Use OTP as source connection_id
                self.source_connection_id = wab_login
                self.rdplog.update_context(self.shared.get('psid'),
                                           real_wab_login)
                if re.search('WALLIX Access Manager',
                             self.engine.get_otp_client(), re.IGNORECASE):
                    self.send_data({
                        'module': 'transitory',
                        'is_wabam': 'True'
                    })
                self.shared['login'] = self.shared.get('login').replace(
                    wab_login, real_wab_login
                )

            self.language = self.engine.get_language()
            self.load_login_message(self.language)
            self._load_selector_banner()

            self.rdplog.log("AUTHENTICATION_SUCCESS", method=method)
            Logger().info(f'lang={self.language}')

        except Exception:
            if DEBUG:
                import traceback
                Logger().info(f"<<<{traceback.format_exc()}>>>")
            _status, _error = None, TR(Sesmsg.AUTH_FAILED_WAB_S) % wab_login
            self.rdplog.log("AUTHENTICATION_FAILURE", method=method)

        return _status, _error

    def get_service(self):
        """ Send service pages to proxy until the selected service is returned.
        """

        Logger().info("get_service")
        self.back_selector = False
        self.hide_approval_back_selector = True
        (_status, _error,
         wab_login, target_login, target_device,
         self.target_service_name, self.target_group,
         self.effective_login) = self.parse_username(
            self.shared.get('login'),
            self.shared.get('target_login'),
            self.shared.get('target_device'),
            self.target_service_name,
            self.target_group)

        if not _status:
            Logger().info(f"Invalid user {self.shared.get('login')}, try again")
            return None, TR(Sesmsg.INVALID_USER)
        _status, _error = None, TR(Sesmsg.NO_ERROR)

        (target_device,
         self.target_context) = self.engine.resolve_target_host(
             rvalue(target_device), rvalue(target_login),
             self.target_service_name, self.target_group,
             rvalue(self.shared.get('real_target_device')),
             self.target_context,
             self.passthrough_mode, ['RDP', 'VNC'])

        while _status is None:
            if (target_device and target_device != MAGICASK
                and (target_login or self.passthrough_mode)
                and target_login != MAGICASK):
                # Target is provided at login
                self._full_user_device_account = f"{target_login}@{target_device}:{wab_login}"
                data_to_send = {
                    'login': wab_login,
                    'target_login': target_login,
                    'target_device': target_device,
                    'module': 'transitory',
                    'target_service': self.target_service_name,
                }
                if not self.internal_target:
                    self.internal_target = (
                        True if self.target_service_name == 'INTERNAL'
                        else False
                    )
                self.send_data(data_to_send)
                _status = True
            elif self.shared.get('selector') == MAGICASK:
                # filters ("Group" and "Account/Device") entered by user
                # in selector are applied to raw services list
                self.engine.get_proxy_rights(
                    ['RDP', 'VNC'],
                    target_context=self.target_context
                )
                selector_filters_case_sensitive = SESMANCONF['sesman'].get(
                    'selector_filters_case_sensitive', False
                )
                services, item_filtered = self.engine.get_targets_list(
                    group_filter=self.shared.get('selector_group_filter'),
                    device_filter=self.shared.get('selector_device_filter'),
                    protocol_filter=self.shared.get('selector_proto_filter'),
                    case_sensitive=selector_filters_case_sensitive
                )
                if (len(services) > 1) or item_filtered:
                    try:
                        _current_page = \
                            int(self.shared.get('selector_current_page')) - 1
                        _lines_per_page = \
                            int(self.shared.get('selector_lines_per_page'))

                        if not _lines_per_page:
                            target_login = ""
                            target_device = ""
                            proto_dest = ""

                            data_to_send = {
                                'login': wab_login,
                                'target_login': target_login,
                                'target_device': target_device,
                                'proto_dest': proto_dest,
                                # , 'selector'                : "True"
                                'ip_client': self.shared.get('ip_client'),
                                'selector_number_of_pages': "0",
                                # No lines sent, reset filters
                                'selector_group_filter': "",
                                'selector_device_filter': "",
                                'selector_proto_filter': "",
                                'module': 'selector',
                            }

                        else:
                            _number_of_pages = (
                                1 + (len(services) - 1) // _lines_per_page
                            )
                            if _current_page >= _number_of_pages:
                                _current_page = _number_of_pages - 1
                            if _current_page < 0:
                                _current_page = 0
                            _start_of_page = _current_page * _lines_per_page
                            _end_of_page = _start_of_page + _lines_per_page

                            services = sorted(
                                services,
                                key=lambda x: x[1]
                            )[_start_of_page:_end_of_page]

                            all_target_login = [s[0] for s in services]
                            all_target_device = [s[1] for s in services]
                            all_proto_dest = [s[2] for s in services]

                            target_login = "\x01".join(all_target_login)
                            target_device = "\x01".join(all_target_device)
                            proto_dest = "\x01".join(all_proto_dest)

                            data_to_send = {
                                'login': wab_login,
                                'target_login': target_login,
                                'target_device': target_device,
                                'proto_dest': proto_dest,
                                'ip_client': self.shared.get('ip_client'),
                                'selector_number_of_pages': (
                                    str(max(_number_of_pages, _current_page + 1))
                                ),
                                'selector_current_page': (
                                    str(_current_page + 1)
                                ),
                                'selector_group_filter':
                                self.shared.get('selector_group_filter'),
                                'selector_device_filter':
                                self.shared.get('selector_device_filter'),
                                'selector_proto_filter':
                                self.shared.get('selector_proto_filter'),
                                'opt_message': '',
                                'module': 'selector',
                            }

                        self.send_data(data_to_send)

                        _status, _error = self.receive_data()

                        if self.shared.get('login') == MAGICASK:
                            self.language = self.engine.get_language()
                            self.load_login_message(self.language)
                            self.send_data({
                                'login': MAGICASK,
                                'selector_lines_per_page': '0',
                                'login_message': self.login_message,
                                'language': self.language,
                                'module': 'login',
                            })
                            Logger().info("Logout")
                            return None, "Logout"

                        target_login = MAGICASK
                        target_device = MAGICASK
                        # proto_dest = MAGICASK
                        (_status, _error,
                         wab_login, target_login, target_device,
                         self.target_service_name, self.target_group,
                         self.effective_login) = self.parse_username(
                            self.shared.get('login'), target_login,
                             target_device, self.target_service_name,
                             self.target_group)
                        if not _status:
                            Logger().info(f"Invalid user {self.shared.get('login')}, try again")
                            return None, TR(Sesmsg.INVALID_USER)

                        _status = None  # One more loop
                    except Exception:
                        _emsg = "Unexpected error in selector pagination"
                        if DEBUG:
                            import traceback
                            Logger().info(f"{_emsg} {traceback.format_exc()}")

                        return False, _emsg
                    if self.allow_back_selector:
                        self.back_selector = True
                    self.hide_approval_back_selector = False
                elif len(services) == 1:
                    Logger().info(f"service len = 1 {services}")
                    s = services[0]
                    data_to_send = {}
                    data_to_send['login'] = wab_login
                    data_to_send['module'] = 'transitory'
                    # service_login (s[1]) format:
                    # target_login@device_name:service_name
                    # target_login can contains '@'
                    # device_name and service_name can not contain ':', nor '@'

                    # target_split = [**target_login, device_name:service_name]
                    target_split = s[1].split('@')
                    target_login = '@'.join(target_split[:-1])
                    # device_service_split = [ device_name, service_name ]
                    device_service_split = target_split[-1].split(':')
                    device_name = device_service_split[0]
                    service_name = device_service_split[-1]

                    data_to_send['target_login'] = target_login
                    data_to_send['target_device'] = device_name
                    data_to_send['target_service'] = service_name
                    self._full_user_device_account = f"{target_login}@{device_name}:{wab_login}"
                    if not self.internal_target:
                        self.internal_target = (True if s[2] == 'INTERNAL'
                                                else False)
                    self.send_data(data_to_send)
                    self.target_service_name = service_name
                    self.target_group = s[0]
                    # Logger().info(f"Only one target : service name {self.target_service_name}")
                    _status = True
                else:
                    _status, _error = False, TR(Sesmsg.TARGET_UNREACHABLE)

            else:
                self.send_data({
                    'login': MAGICASK,
                    'login_message': self.login_message,
                    'module': 'login',
                })
                return None, "Logout"

        return _status, _error
    # END METHOD - GET_SERVICE

    def check_password_expiration_date(self):
        _status, _error = True, ''
        try:
            notify, days = self.engine.password_expiration_date()
            if notify:
                if days == 0:
                    message = TR(Sesmsg.PASSWORD_EXPIRE_SOON)
                else:
                    message = TR(Sesmsg.PASSWORD_EXPIRE_IN_S_DAYS) % days
                _status, _error = self.interactive_display_message({
                    'message': message
                })
        except Exception:
            if DEBUG:
                import traceback
                Logger().info(f"<<<<{traceback.format_exc()}>>>>")
        return _status, _error

    def create_record_path_directory(self, rec_path):
        try:
            os.stat(rec_path)
        except OSError:
            try:
                os.mkdir(rec_path)
            except Exception:
                Logger().info(f"Failed creating recording path ({rec_path})")
                self.send_data({'rejected': TR(Sesmsg.ERROR_RECORD_PATH)})
                return False, TR(Sesmsg.ERROR_RECORD_PATH_S) % rec_path
        return True, ''

    def generate_record_filebase(self, session_id, user, account, start_time):
        """
        Naming convention :
        {session_id},
        YYYYMMDD-HHMMSS,{wabhostname},{uid}

        NB :  backslashes are replaced by pipes for IE compatibility
        """
        random.seed(self.pid)
        timestamp = start_time.strftime("%Y%m%d-%H%M%S")
        num = random.randint(1000, 9999)
        basename = f"{session_id},{timestamp},{gethostname()},{num}"
        basename = re.sub(r'[^-A-Za-z0-9_@,.]', "", basename)
        return basename

    def get_trace_keys(self):
        derivator = self.record_filebase + ".mwrm"
        encryption_key = self.engine.get_trace_encryption_key(derivator, False)
        formated_encryption_key = encryption_key.hex()
        sign_key = self.engine.get_trace_sign_key()
        formated_sign_key = sign_key.hex()
        return formated_encryption_key, formated_sign_key

    def interactive_ask_recording(self, user):
        message = ("Warning! Your remote session may be recorded and"
                   "kept in electronic format.")
        try:
            message = self.engine.get_banner(self.language)
            _status, _error = self.interactive_accept_message(
                {'message': message}
            )
        except Exception:
            if DEBUG:
                import traceback
                Logger().debug(traceback.format_exc())
            _status, _error = False, TR(Sesmsg.CONNECTION_CLOSED_BY_CLIENT)
        target_login = self.shared.get('target_login')
        target_device = self.shared.get('target_device')
        Logger().info(f"Recording agreement of {user} to {target_login}@{target_device}"
                      f": {['NO', 'YES'][_status]}")
        if _status is False:
            reason = "Session recording rejected by user"
            self.engine.set_session_status(result=False, diag=reason)
        return _status, _error

    def load_video_recording(self, rec_path, user):
        Logger().info("Checking video")

        _status, _error = True, ''
        data_to_send = {
            'is_rec': 'False',
            'trace_type': "0",
            'module': 'transitory',
        }

        data_to_send['is_rec'] = True
        if self._trace_type == "localfile":
            data_to_send["trace_type"] = '0'
        elif self._trace_type == "cryptofile":
            data_to_send["trace_type"] = '2'
        else:   # localfile_hashed
            data_to_send["trace_type"] = '1'

        self.full_path = os.path.join(rec_path, self.record_filebase)

        self.send_data(data_to_send)

        return _status, _error

    def load_session_log_redirection(self, rec_path):
        Logger().info("Checking session log redirection")

        data_to_send = {
            'module': 'transitory'
        }

        self.full_log_path = os.path.join(
            rec_path,
            self.record_filebase + '.log'
        )

        Logger().info(f"Session log will be redirected to {self.full_log_path}")
        self.send_data(data_to_send)

        return True, ''

    def select_target(self):
        """
        FIND TARGET

        The purpose of the snippet below is electing the first right that match
        the login AND device AND service that have been passed in the
        connection string.
        If service is blank take the first right that match login AND device
        (may happen with a command line or a mstsc '.rdp' file connections ;
        never happens if the selector is used).
        NB : service names are supposed to be in alphabetical ascending order.
        """
        selected_target = None
        target_device = self.shared.get('target_device')
        target_login = self.shared.get('target_login')
        target_service = (
            self.target_service_name if self.target_service_name != 'INTERNAL'
            else 'RDP'
        )
        target_group = self.target_group

        Logger().info(f"selected target ==> {target_login} {target_device} {target_service}")
        selected_target = self.engine.get_selected_target(target_login,
                                                          target_device,
                                                          target_service,
                                                          target_group,
                                                          self.target_context)
        if not selected_target:
            _target = f"{target_login}@{target_device}:{target_service} ({target_group})"
            _error_log = TR(Sesmsg.TARGET_S_NOT_FOUND) % _target
            _status, _error = False, _error_log
            self.rdplog.log("TARGET_ERROR",
                            target=target_login,
                            reason="Target not found in user rights")
            Logger().info(_error)
            return None, _status, _error
        return selected_target, True, ""

    def check_target(self, selected_target):
        """ Checking selected target validity
        """
        ticket = None
        status = None
        info_message = None
        got_signal = False
        while True:
            Logger().info(f"Begin check_target ticket = {ticket}...")
            previous_status = status
            previous_info_message = info_message
            status, infos = self.engine.check_target(selected_target, ticket)
            ticket = None
            info_message = infos.get('message')
            refresh_page = (got_signal
                            or (status != previous_status)
                            or (previous_info_message != info_message)
                            or (status == APPROVAL_NONE))
            Logger().info(f"End check_target ... refresh : {refresh_page}")
#            if refresh_page:
#                self.send_data({'forcemodule': True})
            if refresh_page:
                self.send_data({'module': 'transitory'})

            if status == APPROVAL_ACCEPTED:
                return True, ""
            if refresh_page:
                self.interactive_display_waitinfo(status, infos)
            got_signal = False
            r = []
            try:
                Logger().info("Start Select ...")
                logtimer.pause()
                if status == APPROVAL_PENDING:
                    # waiting for status update
                    r, w, x = select([self.proxy_conx], [], [],
                                     WORKFLOW_POLL_INTERVAL)
                else:
                    r = self.wait_read_proxy_conx()
                logtimer.resume()
            except BastionSignal as e:
                Logger().info(f"Got Signal {e}")
                got_signal = True
            except Exception as e:
                logtimer.resume()
                if DEBUG:
                    Logger().info(f"exception: '{e}'")
                    import traceback
                    Logger().info(f"<<<<{traceback.format_exc()}>>>>")
                raise
            if self.proxy_conx in r:
                _status, _error = self.receive_data(blocking_call=False)
                if self.shared.get('waitinforeturn') == "backselector":
                    # received back to selector
                    self.send_data({
                        'module': 'selector', 'target_login': '',
                        'target_device': ''
                    })
                    return None, ""
                if self.shared.get('waitinforeturn') == "exit":
                    # received exit
                    self.send_data({'module': 'close'})
                    return False, ""
                if self.shared.get('waitinforeturn') == "confirm":
                    # should parse the ticket info
                    desc = self.shared.get('comment')
                    ticketno = self.shared.get('ticket')
                    duration = parse_duration(self.shared.get('duration'))
                    ticket = {"description": desc if desc else None,
                              "ticket": ticketno if ticketno else None,
                              "duration": duration}
        return False, ""

    @staticmethod
    def _get_tf_flags(ticketfields):
        flag = 0

        field = ticketfields.get("description")
        if field is not None:
            flag += 0x01
            if field == APPREQ_REQUIRED:
                flag += 0x02

        field = ticketfields.get("ticket")
        if field is not None:
            flag += 0x04
            if field == APPREQ_REQUIRED:
                flag += 0x08

        field = ticketfields.get("duration")
        if field is not None:
            flag += 0x10
            if field == APPREQ_REQUIRED:
                flag += 0x20

        return flag

    # 'request_fields':{
    #     '<name>': {
    #         'label': str, # translated field to be displayed by the clients
    #         'type': 'str'|'int'|'datetime'|'duration',
    #         'mandatory': bool,
    #         'min': int,
    #         'max': int
    # }, ...}

    MAP_FIELD_FLAG = {
        'description': 0x01,
        'ticket': 0x04,
        'duration': 0x10
    }

    @staticmethod
    def _get_rf_flags(request_fields):
        flag = 0
        if not request_fields:
            return flag
        for key, value in request_fields.items():
            bitset = Sesman.MAP_FIELD_FLAG.get(key)
            if bitset is not None:
                flag += bitset
                flag += (bitset << 1) if value.get('mandatory') else 0
        return flag

    @staticmethod
    def _get_rf_duration_max(request_fields):
        return request_fields.get('duration', {}).get('max', 0)

    def interactive_display_waitinfo(self, status, infos):
        show_message = infos.get('message') or ''
        target = infos.get('target')
        if target:
            show_message = f"{TR(Sesmsg.SELECTED_TARGET)}: {target}\n{show_message}"
        tosend = {
            'module': 'waitinfo',
            'message': show_message,
            'display_message': MAGICASK,
            'waitinforeturn': MAGICASK
        }
        flag = 0
        duration_max = infos.get("duration_max") or 0
        ticketfields = infos.get("ticket_fields")
        if ticketfields:
            flag = self._get_tf_flags(ticketfields)
        request_fields = infos.get('request_fields')
        if request_fields:
            flag = self._get_rf_flags(request_fields)
            # duration_max is in minutes
            duration_max = self._get_rf_duration_max(request_fields) // 60
        if self.hide_approval_back_selector:
            flag |= 0x10000
        if status == APPROVAL_NONE:
            tosend["showform"] = True
            tosend["duration_max"] = duration_max
        else:
            tosend["showform"] = False
        tosend["formflag"] = flag
        self.send_data(tosend)

    def start(self):
        _status, tries = None, 5
        while _status is None and tries > 0:
            self.reset_session_var()

            # AUTHENTIFY
            # [ LOGIN ]
            logtimer.start(LogSteps.PRIMARY_AUTH)
            _status, _error = self.authentify()

            if _status is None and self.engine.get_challenge():
                challenge = self.engine.get_challenge()
                if not challenge.first_password:
                    # on first password, let the login page
                    # submit challenge:
                    message = challenge.fields[0] if challenge.fields else ""
                    echo = challenge.echos[0] if challenge.echos else False
                    if not message:
                        message = challenge.message
                    elif challenge.challenge_type == "MFA":
                        message = f"{message}:"
                        if challenge.message:
                            message = f"{challenge.message}\n{message}"
                    data_to_send = {
                        'authentication_challenge': echo,
                        'message': message,
                        'module': 'challenge'
                    }
                    self.send_data(data_to_send)
                    continue

            tries = tries - 1
            if _status is None and tries > 0:
                login = mundane(self.shared.get('login'))
                ip_client = mundane(self.shared.get('ip_client'))
                Logger().info(
                    f"Bastion user '{login}' authentication "
                    f"from {ip_client} failed [{tries} tries remains]"
                )

                (current_status, current_error,
                 current_wab_login, current_target_login,
                 current_target_device, self.target_service_name,
                 self.target_group,
                 self.effective_login) = self.parse_username(
                     self.shared.get('login'),
                     self.shared.get('target_login'),
                     self.shared.get('target_device'),
                     self.target_service_name,
                     self.target_group)

                if self.language != SESMANCONF.language:
                    if not self.language:
                        self.set_language_from_keylayout()
                    SESMANCONF.language = self.language

                data_to_send = {
                    'login': (
                        self.shared.get('login')
                        if not current_wab_login.startswith('_OTP_')
                        else MAGICASK
                    ),
                    'password': MAGICASK,
                    'module': 'login',
                    'login_message': self.login_message,
                    'language': SESMANCONF.language,
                    'opt_message': (
                        TR(Sesmsg.AUTHENTICATION_FAILED)
                        if self.shared.get('password') != MAGICASK
                        else ''
                    )
                }
                self.send_data(data_to_send)
                continue

            if _status:
                tries = 5
                login = mundane(self.shared.get('login'))
                Logger().info(
                    f"Bastion user '{login}' authentication succeeded"
                )
                if not self.engine.check_license():
                    _status, _error = False, "License 'sm' not available"
                    break
                # Warn password will expire soon for user
                _status, _error = self.check_password_expiration_date()

                # Get services for identified user
                _status = None
                while _status is None:
                    # [ SELECTOR ]
                    logtimer.start(LogSteps.FETCH_RIGHTS)
                    _status, _error = self.get_service()
                    Logger().info(f"get_service end: {_status}")
                    if not _status:
                        # logout or error in selector
                        self.engine.reset_proxy_rights()
                        logtimer.stop(LogSteps.FETCH_RIGHTS)
                        break
                    logtimer.start(LogSteps.CHECKOUT_TARGET)
                    selected_target, _status, _error = self.select_target()
                    Logger().info(f"select_target end: {_status}")
                    if not _status:
                        # target not available
                        self.engine.reset_proxy_rights()
                        logtimer.stop(LogSteps.CHECKOUT_TARGET)
                        break
                    # [ WAIT INFO ]
                    _status, _error = self.check_target(selected_target)
                    Logger().info(f"check_target end: {_status}")
                    if not _status:
                        logtimer.stop(LogSteps.CHECKOUT_TARGET)
                        if _status is None:
                            continue
                        self.engine.reset_proxy_rights()
                        break
                    # [ CONNECT TO TARGET ]
                    try:
                        _status, _error = self.connect_to_target(
                            selected_target
                        )
                    except Exception as e:
                        Logger().info(
                            f"Unexpected Error on target connection ({e})"
                        )
                        _status, _error = False, "End of Session"
                        logtimer.stop()
                        self.engine.release_all_target()
                        self.engine.stop_session()
                    self.reset_target_session_vars()
                self.engine.proxy_session_logout()
                self.rdplog.log("LOGOUT")
                self.engine.close_client()

        if tries <= 0:
            Logger().info("Too many login failures")
            _status = False

        if _status:
            target_login = self.shared.get('target_login')
            target_device = self.shared.get('target_device')
            Logger().info(f"Asking service {target_login}@{target_device}")

    # END METHOD - START

    def connect_to_target(self, selected_target):
        """
        #####################
        ### START_SESSION ###
        #####################
        """
        extra_info = self.engine.get_target_extra_info()
        _status, _error = True, ''

        Logger().info("Fetching protocol")
        kv = {}
        if _status:
            target_login_info = self.engine.get_target_login_info(
                selected_target
            )
            proto_info = self.engine.get_target_protocols(selected_target)
            kv['proto_dest'] = proto_info.protocol
            kv['target_str'] = target_login_info.get_target_str()

        if _status:
            kv['password'] = 'pass'

            # id to recognize primary user for session probe
            kv['primary_user_id'] = self.engine.get_username()

            # register signal
            signal.signal(signal.SIGUSR1, self.kill_handler)
            signal.signal(signal.SIGUSR2, self.check_handler)

            Logger().info(f"Starting Session, effective login='{self.effective_login}'")

            user = self.engine.get_username()
            uname = self.effective_login or target_login_info.account_login

            # Add connection to the observer
            session_id, start_time, error_msg = self.engine.start_session(
                selected_target,
                self.pid,
                self.effective_login,
                source_connection_id=self.source_connection_id
            )
            if session_id is None:
                _status, _error = False, \
                    TR(Sesmsg.START_SESSION_FAILED) + " (" + error_msg + ")"

        if _status:
            record_warning = SESMANCONF['sesman'].get('record_warning', True)
            if record_warning and extra_info.is_recorded:
                _status, _error = self.interactive_ask_recording(
                    user
                )

        if _status:
            timeclose, _status, _error = self.check_deconnection_time(
                selected_target
            )
            if timeclose is not None:
                kv['timeclose'] = timeclose

        if _status:
            # add "Year-Month-Day" subdirectory to record path
            date_path = start_time.strftime("%Y-%m-%d")
            rec_path = os.path.join(LOCAL_TRACE_PATH_RDP, date_path)
            self.record_filebase = self.generate_record_filebase(
                session_id,
                user,
                uname,
                start_time
            )
            kv['record_filebase'] = self.record_filebase
            kv['record_subdirectory'] = date_path
            # TODO kv['record_path'] = LOCAL_TRACE_PATH_RDP
            Logger().info(f"Session will be recorded in {self.record_filebase}")
            try:
                _status, _error = self.create_record_path_directory(
                    rec_path
                )
                if _status and extra_info.is_recorded:
                    _status, _error = self.load_video_recording(
                        rec_path, user
                    )
                if _status:
                    _status, _error = self.load_session_log_redirection(
                        rec_path
                    )
                if _status:
                    encryption_key, sign_key = self.get_trace_keys()
                    kv['encryption_key'] = encryption_key
                    kv['sign_key'] = sign_key
            except Exception:
                import traceback
                Logger().debug(traceback.format_exc())
                _status, _error = False, TR(Sesmsg.CONNECTION_CLOSED_BY_CLIENT)

        if not _status:
            self.send_data({'rejected': _error})

        if _status:
            kv['session_id'] = session_id
            trace_written = False  # reminder to write_trace later
            pattern_kill, pattern_notify = self.engine.get_restrictions(
                selected_target, "RDP"
            )
            if pattern_kill:
                self.send_data({'module': 'transitory',
                                'pattern_kill': pattern_kill})
            if pattern_notify:
                self.send_data({'module': 'transitory',
                                'pattern_notify': pattern_notify})

        if _status:
            module = kv.get('proto_dest')
            if module not in ['RDP', 'VNC', 'INTERNAL']:
                module = 'RDP'
            if self.internal_target:
                module = 'INTERNAL'
            kv['module'] = module

        self.shared['recording_started'] = 'False'

        self.reporting_reason = None
        self.reporting_target = None
        self.reporting_message = None

        try_next = False
        close_box = False

        if _status:
            for physical_target, has_more_physical_target in collection_has_more(
                self.engine.get_effective_target(selected_target)
            ):
                kv['try_alternate_target'] = "True" if try_next else "False"
                kv['has_more_target'] = "True" if has_more_physical_target else "False"

                try_next = False
                close_box = False
                kv['recording_started'] = "False"

                cstatus, infos = self.engine.check_target(physical_target)
                if cstatus != APPROVAL_ACCEPTED:
                    Logger().info(f"Jump server unavailable ({infos.get('message')})")
                    _status = False
                    continue

                physical_info = self.engine.get_physical_target_info(
                    physical_target
                )
                if not _status:
                    physical_target = None
                    break

                physical_proto_info = self.engine.get_target_protocols(
                    physical_target
                )
                application = self.engine.get_application(selected_target)
                conn_opts = self.engine.get_target_conn_options(
                    physical_target
                )
                if physical_proto_info.protocol in (RDP, VNC):
                    if physical_proto_info.protocol == RDP:
                        kv['proxy_opt'] = ",".join(
                            physical_proto_info.subprotocols
                        )

                    conn_type = self.engine.get_target_conn_type(physical_target)
                    conn_spec = cp_spec[conn_type.lower()]

                    kv.update(self._fetch_connectionpolicy(conn_spec[0], conn_opts))
                    kv.update(conn_spec[1])

                kv.update(self._load_kerberos_armoring_options(conn_opts))

                kv['disable_tsk_switch_shortcuts'] = 'no'
                if application:
                    app_params = self.engine.get_app_params(
                        selected_target,
                        physical_target
                    )
                    if not app_params:
                        continue
                    kv['alternate_shell'] = app_params.program
                    if app_params.params is not None:
                        kv['shell_arguments'] = app_params.params
                    kv['shell_working_directory'] = app_params.workingdir

                    kv['target_application'] = \
                        f"{target_login_info.account_name}@{target_login_info.target_name}"
                    if app_params.params is not None:
                        if '${USER}' in app_params.params:
                            kv['target_application_account'] = \
                                target_login_info.account_login or \
                                self.target_context.login or ""
                        if '${PASSWORD}' in app_params.params:
                            kv['target_application_password'] = (
                                self.engine.get_target_password(
                                    selected_target
                                )
                                or self.engine.get_primary_password(
                                    selected_target
                                )
                                or ''
                            )
                    self.target_app_rights[kv['target_application']] = (
                        selected_target, app_params
                    )

                    if not self.engine.is_sharing_session(selected_target):
                        kv['disable_tsk_switch_shortcuts'] = 'yes'

                if self.engine.is_sharing_session(selected_target):
                    kv['enable_osd_display_remote_target'] = '0'

                self.cn = target_login_info.target_name

                if self.target_context.host:
                    kv['target_host'] = self.target_context.host
                    kv['target_device'] = self.target_context.showname()
                else:
                    kv['target_host'] = physical_info.device_host
                    if physical_info.sharing_host:
                        _phost = physical_info.sharing_host
                        if _phost.startswith("sock://"):
                            # session sharing
                            kv["tunneling_target_host"] = \
                                _phost[len("sock://"):]
                        else:
                            # session shadow
                            kv['target_host'] = _phost

                kv['target_login'] = physical_info.account_login
                if (not kv.get('target_login')
                    and self.target_context.login
                    and not application):
                    # on application,
                    # login in target_context is the login of application
                    kv['target_login'] = self.target_context.login

                kv['target_port'] = physical_info.service_port
                kv['device_id'] = physical_info.device_id

                kv['target_device'] = target_login_info.target_name
                kv['target_service'] = target_login_info.service_name

                try:
                    auth_policy_methods = self.engine.get_target_auth_methods(
                        physical_target)
                    Logger().info(f"auth_mode_passthrough={self.passthrough_mode}")

                    target_password = ''
                    if self.passthrough_mode:
                        kv['target_login'] = self.passthrough_target_login
                        if self.shared.get('password') == MAGICASK:
                            target_password = ''
                        else:
                            target_password = self.shared.get('password')
                        # Logger().info(
                        #     f"auth_mode_passthrough {target_password=}"
                        # )
                        kv['password'] = 'password'
                    elif PASSWORD_VAULT in auth_policy_methods:
                        target_passwords = self.engine.get_target_passwords(
                            physical_target
                        )
                        target_password = '\x01'.join(target_passwords)

                    if (not target_password
                        and PASSWORD_MAPPING in auth_policy_methods):
                        target_password = (
                            self.engine.get_primary_password(physical_target)
                            or ''
                        )

                    force_sc_auth = conn_opts.get(
                        'rdp', {}
                    ).get(
                        'force_smartcard_authentication', False
                    )
                    Logger().info(f"force_smartcard_authentication = {force_sc_auth}")
                    allow_interactive_password = (
                        (PASSWORD_INTERACTIVE in auth_policy_methods
                         or self.passthrough_mode)
                        and not force_sc_auth
                    )
                    allow_interactive_login = not force_sc_auth

                    kv['target_password'] = target_password
                    is_interactive_login = (
                        not bool(kv.get('target_login'))
                        and not force_sc_auth
                    )
                    extra_kv, _status, _error = self.complete_target_info(
                        kv, allow_interactive_password,
                        allow_interactive_login)
                    kv.update(extra_kv)

                    if self.target_context.host:
                        self._physical_target_host = self.target_context.host
                    elif ('/' in physical_info.device_host
                          and extra_kv.get('target_host') != MAGICASK):
                        self._physical_target_host = \
                            extra_kv.get('target_host')
                    else:
                        self._physical_target_host = physical_info.device_host

                    Logger().info(
                        "Send critic notification "
                        "(every attempt to connect to some physical node)"
                    )
                    if extra_info.is_critical:
                        Logger().info("CRITICAL CONNECTION")
                        self.engine.NotifyConnectionToCriticalEquipment(
                            ('APP' if application else proto_info.protocol),
                            self.shared.get('login'),
                            socket.getfqdn(self.shared.get('ip_client')),
                            self.shared.get('ip_client'),
                            self.shared.get('target_login'),
                            self.shared.get('target_device'),
                            self._physical_target_host,
                            ctime(),
                            None
                        )

                    update_args = {
                        "is_application": bool(application),
                        "target_host": self._physical_target_host,
                        "session_log_path": self.full_log_path
                    }
                    if is_interactive_login:
                        update_args["effective_login"] = kv.get('target_login')
                    if self.shared.get('width'):
                        update_args["video_width"] = \
                            int(self.shared.get('width'))
                    if self.shared.get('height'):
                        update_args["video_height"] = \
                            int(self.shared.get('height'))

                    self.engine.update_session_target(physical_target,
                                                      **update_args)

                    if (_status
                        and 'vnc_over_ssh' in conn_opts
                        and conn_opts.get("vnc_over_ssh").get("enable")):
                        _status, _error = \
                            self._load_vnc_over_ssh_options(kv, conn_opts.get("vnc_over_ssh"))

                    if not _status:
                        login = mundane(self.shared.get('login'))
                        ip_client = mundane(self.shared.get('ip_client'))
                        Logger().info(
                            f"({ip_client}):{login}:REJECTED : User message: \"{_error}\""
                        )

                        kv = {
                            "login": "",
                            'password': "",
                            'target_login': "",
                            'target_password': "",
                            'target_device': "",
                            'target_host': "",
                            'rejected': _error,
                        }

                    try_next = True
                    self.shared['module'] = ''
                    logtimer.stop(LogSteps.CHECKOUT_TARGET)
                    try:
                        ###########
                        # SEND KV #
                        ###########
                        self.send_data(kv)

                        Logger().info(
                            "Added connection to active Bastion services"
                        )

                        # Looping on keepalived socket
                        while True:
                            r = []
                            got_signal = False
                            try:
                                r, w, x = select([self.proxy_conx], [], [],
                                                 KEEPALIVE_TIMEOUT)
                            except BastionSignal as e:
                                Logger().info(f"Got Signal {e}")
                                got_signal = True
                            except Exception as e:
                                if DEBUG:
                                    Logger().info(f"exception: '{e}'")
                                    import traceback
                                    Logger().info(f"<<<<{traceback.format_exc()}>>>>")
                                raise
                            self.engine.keepalive(timeout=KEEPALIVE_TIMEOUT)
                            current_time = time()
                            if self.check_session_parameters:
                                self.update_session_parameters(current_time)
                                self.check_session_parameters = False
                            self.rtmanager.check(current_time)
                            if self.proxy_conx in r:
                                _status, _error = self.receive_data(
                                    EXPECTING_KEYS,
                                    blocking_call=False
                                )

                                if self._changed_keys:
                                    self.update_session_data(
                                        self._changed_keys
                                    )

                                self.process_target_connection_time()

                                if self.shared.get('auth_notify'):
                                    self.handle_auth_notify(physical_target)
                                    self.shared['auth_notify'] = ''

                                if (not trace_written
                                    and (self.shared.get('recording_started')
                                         == 'True')):
                                    _status, _error = \
                                        self.handle_recording_started()
                                    trace_written = True

                                if self.shared.get("pm_request"):
                                    self._manage_pm()

                                self.handle_shadowing()
                                self.handle_session_sharing()

                                if self.shared.get('reporting'):
                                    report_status = self.handle_reporting()
                                    if report_status:
                                        try_next = False

                                if self.shared.get('disconnect_reason_ack'):
                                    break

                                if self.shared.get('auth_channel_target'):
                                    self.handle_auth_channel_target(
                                        selected_target
                                    )
                                    self.shared['auth_channel_target'] = ''
                                if self.shared.get('module') == "close":
                                    break
                                if self.shared.get('keepalive') == MAGICASK:
                                    self.send_data({'keepalive': 'True'})
                            # r can be empty
                            elif not self.internal_target and not got_signal:
                                Logger().info('Missing Keepalive')
                                Logger().error('break connection')
                                break
                        if self.shared.get('module') == "close":
                            close_box = True
                        Logger().debug("End Of Keep Alive")

                    except AuthentifierSocketClosed:
                        if DEBUG:
                            import traceback
                            Logger().info(
                                "RDP/VNC connection terminated by client"
                            )
                            Logger().info(f"<<<{traceback.format_exc()}>>>")
                    except Exception:
                        if DEBUG:
                            import traceback
                            Logger().info(
                                "RDP/VNC connection terminated by client"
                            )
                            Logger().info(f"<<<{traceback.format_exc()}>>>")
                    if not try_next:
                        break
                    elif not has_more_physical_target:
                        current_result, _ = self.engine.get_session_status()
                        if current_result:
                            self.engine.set_session_status(
                                result=False,
                                diag="Connection failed"
                            )
                finally:
                    self.engine.release_target(physical_target)

        self.engine.release_all_target()
        Logger().info("Stop session ...")
        # Notify WabEngine to stop connection if it has been
        # launched successfully
        self.engine.stop_session(title="End session")
        if self.tun_process:
            self.tun_process.stop()
            self.tun_process = None
        Logger().info("Stop session done.")
        if self.shared.get("module") == "close":
            if close_box and self.back_selector:
                try:
                    self.send_data({'module': 'close_back',
                                    'selector': 'False'})
                    while True:
                        _status, _error = self.receive_data()
                        if (_status
                            and self.shared.get('selector') == MAGICASK):
                            return None, "Go back to selector"
                except Exception:
                    _status, _error = False, "End of Session"
            else:
                self.send_data({'module': 'close'})
        # Error
        if try_next:
            _status, _error = self.interactive_close(self.reporting_target,
                                                     self.reporting_message)

        try:
            Logger().info("Close connection ...")

            self.proxy_conx.close()

            Logger().info("Close connection done.")
        except IOError:
            if DEBUG:
                Logger().info("Close connection: Exception")
                Logger().info(f"<<<<{traceback.format_exc()}>>>>")
        return False, "End of Session"

    def handle_reporting(self):
        _reporting = self.shared.get('reporting')
        _reporting_reason, _, _remains = \
            _reporting.partition(':')
        _reporting_target, _, _reporting_message = \
            _remains.partition(':')
        self.shared['reporting'] = ''

        Logger().info(f'Reporting: reason="{_reporting_reason}" '
                      f'target="{_reporting_target}" message="{_reporting_message}"')

        self.process_report(_reporting_reason,
                            _reporting_target,
                            _reporting_message)

        if _reporting_reason == 'CONNECTION_FAILED':
            self.reporting_reason = _reporting_reason
            self.reporting_target = _reporting_target
            self.reporting_message = _reporting_message

            release_reason = 'Connection failed'
            self.engine.set_session_status(
                result=False, diag=release_reason)
        elif _reporting_reason == 'FINDPATTERN_KILL':
            Logger().info(
                "RDP connection terminated. Reason: Kill pattern detected"
            )
            release_reason = 'Kill pattern detected'
            self.engine.set_session_status(
                result=False, diag=release_reason)
            self.send_data({'disconnect_reason': TR(Sesmsg.PATTERN_KILL)})
        elif _reporting_reason == 'SERVER_REDIRECTION':
            (redir_login, _, redir_host) = \
                _reporting_message.rpartition('@')
            update_args = {}
            if redir_host:
                update_args["target_host"] = redir_host
            if redir_login:
                update_args["target_account"] = redir_login
            self.engine.update_session(**update_args)
        elif _reporting_reason == 'SESSION_EXCEPTION':
            Logger().info(
                "RDP connection terminated. Reason: Session exception"
            )
            release_reason = 'Session exception: ' + _reporting_message
            self.engine.set_session_status(
                diag=release_reason)
        elif _reporting_reason == 'SESSION_EXCEPTION_NO_RECORD':
            Logger().info(
                "RDP connection terminated. Reason: Session exception "
                "(no record)"
            )
            release_reason = 'Session exception: ' + _reporting_message
            self.engine.set_session_status(
                result=False, diag=release_reason)
        elif _reporting_reason == 'SESSION_PROBE_LAUNCH_FAILED':
            Logger().info(
                "RDP connection terminated. Reason: Session Probe "
                "launch failed"
            )
            release_reason = 'Interrupt: Session Probe launch failed'
            self.engine.set_session_status(
                result=False, diag=release_reason)
            if self.shared.get('session_probe_launch_error_message'):
                self.send_data({
                    'disconnect_reason':
                    self.shared.get('session_probe_launch_error_message')
                })
                self.shared['session_probe_launch_error_message'] = ''
            else:
                self.send_data({
                    'disconnect_reason': TR(Sesmsg.SESPROBE_LAUNCH_FAILED)
                })
        elif _reporting_reason == 'SESSION_PROBE_KEEPALIVE_MISSED':
            Logger().info(
                'RDP connection terminated. Reason: Session Probe '
                'keepalive missed'
            )
            release_reason = 'Interrupt: Session Probe keepalive missed'
            self.engine.set_session_status(
                result=False, diag=release_reason)
            self.send_data({
                'disconnect_reason': TR(Sesmsg.SESPROBE_KEEPALIVE_MISSED)
            })
        elif (_reporting_reason
              == 'SESSION_PROBE_OUTBOUND_CONNECTION_BLOCKING_FAILED'):
            Logger().info(
                'RDP connection terminated. Reason: Session Probe failed '
                'to block outbound connection'
            )
            release_reason = (
                'Interrupt: Session Probe failed to block outbound connection'
            )
            self.engine.set_session_status(
                result=False, diag=release_reason)
            self.send_data({
                'disconnect_reason':
                TR(Sesmsg.SESPROBE_OUTBOUND_CONNECTION_BLOCKING_FAILED)
            })
        elif _reporting_reason == 'SESSION_PROBE_PROCESS_BLOCKING_FAILED':
            Logger().info(
                'RDP connection terminated. Reason: Session Probe failed '
                'to block process'
            )
            release_reason = (
                'Interrupt: Session Probe failed to block process'
            )
            self.engine.set_session_status(
                result=False, diag=release_reason
            )
            self.send_data({
                'disconnect_reason':
                TR(Sesmsg.SESPROBE_PROCESS_BLOCKING_FAILED)
            })
        elif (_reporting_reason
              == 'SESSION_PROBE_RUN_STARTUP_APPLICATION_FAILED'):
            Logger().info(
                'RDP connection terminated. Reason: Session Probe failed '
                'to run startup application'
            )
            release_reason = (
                'Interrupt: Session Probe failed to run startup application'
            )
            self.engine.set_session_status(
                result=False, diag=release_reason
            )
            self.send_data({
                'disconnect_reason':
                TR(Sesmsg.SESPROBE_FAILED_TO_RUN_STARTUP_APPLICATION)
            })
        elif _reporting_reason == 'SESSION_PROBE_RECONNECTION':
            Logger().info(
                'RDP connection terminated. Reason: Session Probe '
                'reconnection without disconnection'
            )
            release_reason = (
                'Interrupt: Session Probe reconnection without disconnection'
            )
            self.engine.set_session_status(
                result=False, diag=release_reason
            )
            self.send_data({
                'disconnect_reason': TR(Sesmsg.SESPROBE_RECONNECTION)
            })
        elif _reporting_reason == 'SESSION_EVENT':
            (event_level, event_id, event_details) = \
                _reporting_message.split(" : ", 2)
            self.engine.update_session(
                event_level=event_level,
                event_id=event_id,
                event_details=event_details,
            )

            if 'FATAL' == event_level:
                Logger().info(
                    'RDP connection terminated. Reason: Application '
                    'fatal error'
                )
                release_reason = (
                    'Interrupt: Application fatal error'
                )
                self.engine.set_session_status(
                    result=False, diag=release_reason
                )
                self.send_data({
                    'disconnect_reason': TR(Sesmsg.APPLICATION_FATAL_ERROR)
                })
        elif (_reporting_reason == 'OPEN_SESSION_SUCCESSFUL'
              or _reporting_reason == 'CONNECT_DEVICE_SUCCESSFUL'):
            return True

        return False

    def process_target_connection_time(self):
        if self.shared.get("target_connection_time"):
            try:
                tct = int(self.shared.get("target_connection_time", 0))
                fct = int(self.shared.get("front_connection_time", 0))
                logtimer.add_step_time(LogSteps.TARGET_CONN, tct / 1000.0)
                logtimer.add_step_time(LogSteps.PRIMARY_CONN, fct / 1000.0)
                metrics = logtimer.report_metrics()
                self.rdplog.log("TIME_METRICS",
                                session_id=self.shared.get('session_id'),
                                **metrics)
            except Exception:
                pass
            self.shared["target_connection_time"] = None

    def process_report(self, reason, target, message):
        if reason == 'CLOSE_SESSION_SUCCESSFUL':
            pass
        elif reason == 'CONNECTION_FAILED':
            self.engine.NotifySecondaryConnectionFailed(
                self.shared.get('login'),
                self.shared.get('ip_client'),
                self.shared.get('target_login'),
                self._physical_target_host)
        # elif reason == 'CONNECTION_SUCCESSFUL':
        #     pass
        elif reason == 'CONNECT_DEVICE_SUCCESSFUL':
            pass
        elif reason == 'OPEN_SESSION_FAILED':
            self.engine.NotifySecondaryConnectionFailed(
                self.shared.get('login'),
                self.shared.get('ip_client'),
                self.shared.get('target_login'),
                self._physical_target_host)
        elif reason == 'OPEN_SESSION_SUCCESSFUL':
            pass
        elif reason == 'FILESYSTEM_FULL':
            data = message.split('|')
            used = data[0]
            filesystem = data[1]

            self.engine.NotifyFilesystemIsFullOrUsedAtXPercent(filesystem,
                                                               used)
        elif reason == 'SESSION_EXCEPTION':
            pass
        elif reason == 'SESSION_EXCEPTION_NO_RECORD':
            pass
        elif reason == 'SESSION_PROBE_LAUNCH_FAILED':
            pass
        elif reason == 'SESSION_PROBE_KEEPALIVE_MISSED':
            pass
        elif reason == 'SESSION_PROBE_OUTBOUND_CONNECTION_BLOCKING_FAILED':
            pass
        elif reason == 'SESSION_PROBE_PROCESS_BLOCKING_FAILED':
            pass
        elif reason == 'SESSION_PROBE_RUN_STARTUP_APPLICATION_FAILED':
            pass
        elif reason == 'SERVER_REDIRECTION':
            (nlogin, _, nhost) = message.rpartition('@')
            Logger().info(f"Server Redirection: login='{nlogin}', host='{nhost}'")
        elif (reason == 'FINDPATTERN_KILL'
              or reason == 'FINDPATTERN_NOTIFY'):
            pattern = message.split('|')
            regexp = pattern[0]
            string = pattern[1]
            self.engine.NotifyFindPatternInRDPFlow(
                regexp, string, self.shared.get('login'),
                self.shared.get('target_login'),
                self.shared.get('target_device'),
                self.cn, self.target_service_name
            )
        elif (reason == 'FINDCONNECTION_DENY'
              or reason == 'FINDCONNECTION_NOTIFY'):
            pattern = message.split('|')
            self.engine.notify_find_connection_rdp(
                rule=pattern[0],
                deny=(reason == 'FINDCONNECTION_DENY'),
                app_name=pattern[1],
                app_cmd_line=pattern[2],
                dst_addr=pattern[3],
                dst_port=pattern[4],
                user_login=self.shared.get('login'),
                user=self.shared.get('target_login'),
                host=self.shared.get('target_device'),
                cn=self.cn,
                service=self.target_service_name
            )
        elif (reason == 'FINDPROCESS_DENY'
              or reason == 'FINDPROCESS_NOTIFY'):
            pattern = message.split('|')
            self.engine.notify_find_process_rdp(
                regex=pattern[0],
                deny=(reason == 'FINDPROCESS_DENY'),
                app_name=pattern[1],
                app_cmd_line=pattern[2],
                user_login=self.shared.get('login'),
                user=self.shared.get('target_login'),
                host=self.shared.get('target_device'),
                cn=self.cn,
                service=self.target_service_name
            )
        elif reason == 'SESSION_EVENT':
            pass
        else:
            Logger().info(
                "Unexpected reporting reason: "
                f'"{reason}" "{target}" "{message}"')

    def handle_auth_channel_target(self, selected_target):
        Logger().info(
            f"Auth channel target=\"{self.shared.get('auth_channel_target')}\""
        )
        if self.shared.get('auth_channel_target').startswith(
                'GetWabSessionParameters'
        ):
            app_target = selected_target
            _prefix, _sep, _val = self.shared.get(
                'auth_channel_target'
            ).partition(':')
            if _sep:
                app_right_params = self.target_app_rights.get(_val)
                if app_right_params is not None:
                    app_target, _app_param = app_right_params
            app_info = self.engine.get_target_login_info(app_target)
            account_login = app_info.account_login
            application_password = \
                self.engine.get_target_password(app_target) \
                or self.engine.get_primary_password(app_target) \
                or ''
            _message = {
                'user': account_login,
                'password': application_password
            }

            self.send_data({'auth_channel_answer': json.dumps(_message)})
            Logger().info(
                "Sending of auth channel "
                "answer ok "
                "(GetWabSessionParameters)"
            )

    def handle_shadowing(self):
        if self.shared.get("rd_shadow_available") == 'True':
            self.engine.update_session(shadow_allow=True)
            self.shared["rd_shadow_available"] = 'False'

        if self.shared.get("rd_shadow_invitation_error_code"):
            shadow_token = {
                "shadow_id":
                self.shared.get("rd_shadow_invitation_id"),
                "shadow_ip":
                self.shared.get("rd_shadow_invitation_addr"),
                "shadow_port":
                self.shared.get("rd_shadow_invitation_port"),
            }
            self.engine.sharing_response(
                errcode=self.shared.get("rd_shadow_invitation_error_code"),
                errmsg=self.shared.get("rd_shadow_invitation_error_message"),
                token=shadow_token,
                request_id=self.shared.get("rd_shadow_userdata")
            )
            self.shared["rd_shadow_available"] = 'False'

            self.shared["rd_shadow_userdata"] = None
            self.shared["rd_shadow_invitation_error_code"] = 0
            self.shared["rd_shadow_invitation_error_message"] = None
            self.shared["rd_shadow_invitation_id"] = None
            self.shared["rd_shadow_invitation_addr"] = None
            self.shared["rd_shadow_invitation_port"] = None

    def handle_session_sharing(self):
        if self.shared.get("session_sharing_invitation_error_code"):
            sharing_addr = self.shared.get("session_sharing_invitation_addr")
            if not sharing_addr.startswith("sock://"):
                sharing_addr = "sock://" + sharing_addr
            session_sharing_token = {
                "native_session_sharing": True,
                "sharing_pass":
                    self.shared.get("session_sharing_invitation_id"),
                "shadow_ip": sharing_addr,
                "shadow_port": 0,  # force 0 to use Unix Socket,
                "host_target_ip":
                    self.shared.get("session_sharing_target_ip"),
                "host_target_login":
                    self.shared.get("session_sharing_target_login"),
            }
            self.engine.sharing_response(
                errcode=self.shared.get("session_sharing_invitation_error_code"),
                errmsg=self.shared.get("session_sharing_invitation_error_message"),
                token=session_sharing_token,
                request_id=self.shared.get("session_sharing_userdata")
            )
            self.shared["session_sharing_userdata"] = None
            self.shared["session_sharing_invitation_error_code"] = 0
            self.shared["session_sharing_invitation_error_message"] = None
            self.shared["session_sharing_invitation_id"] = None
            self.shared["session_sharing_invitation_addr"] = None

    def handle_auth_notify(self, physical_target):
        if self.shared.get('auth_notify') == 'rail_exec':
            flags = self.shared.get('auth_notify_rail_exec_flags')
            exe_or_file = self.shared.get('auth_notify_rail_exec_exe_or_file')
            Logger().info(
                f"rail_exec flags=\"{flags}\" exe_or_file=\"{exe_or_file}\""
            )
            auth_command_kv = self.check_application(
                physical_target, flags, exe_or_file
            )
            self.send_data(auth_command_kv)

            self.shared['auth_notify_rail_exec_flags'] = ''
            self.shared['auth_notify_rail_exec_exe_or_file'] = ''

    def handle_recording_started(self):
        # write mwrm path to rdptrc (allow real time display)
        Logger().info("Call write trace")
        _status, _error = self.engine.write_trace(self.full_path)
        if not _status:
            _error = TR(Sesmsg.TRACE_WRITER_FAILED_S) % self.full_path
            Logger().info(f"Failed accessing recording path ({self.full_path})")
            self.send_data({'rejected': TR(Sesmsg.ERROR_RECORD_PATH)})
        return _status, _error

    def update_session_data(self, changed_keys):
        data_to_update = {
            acl_key: convert(val) for (acl_key, convert), val in (
                (KEYMAPPING[key], self.shared[key])
                for key in changed_keys if (
                    self.shared.get(key) is not None
                )
            )
        }
        self.engine.update_session(**data_to_update)

    def _fetch_connectionpolicy(self, conn_spec, conn_opts):
        # Logger().info(f"{conn_opts}")
        def get_values(section_name, value_infos):
            values = conn_opts.get(section_name, {})
            return ((config_key, values.get(cp_key, default_value))
                    for config_key, cp_key, default_value in value_infos)
        return itertools.chain.from_iterable(
            get_values(section_name, value_infos)
            for section_name, value_infos in conn_spec.items())

    def kill_handler(self, signum, frame):
        # Logger().info(f"KILL_HANDLER = {signum}")
        if signum == signal.SIGUSR1:
            self.kill()
            raise BastionSignal()

    def check_handler(self, signum, frame):
        # Logger().info(f"CHECK_HANDLER = {signum}")
        if signum == signal.SIGUSR2:
            self.check_session_parameters = True
            raise BastionSignal()

    def kill(self):
        try:
            Logger().info("Closing a RDP/VNC connection")
            self.proxy_conx.close()
        except Exception:
            pass

    def update_session_parameters(self, current_time: float) -> None:
        params = self.engine.read_session_parameters()
        res = params.get("rt_display")
        Logger().debug(f"rt_display={res}")
        if res:
            self.rtmanager.start(current_time)

        res = params.get("shadow_type")
        Logger().debug(f"shadow_type={res}")
        if res:
            # TODO remove once session sharing interface unified
            # Former Session Shadowing Trigger interface
            userdata = params.get("shadow_userdata")
            Logger().debug(f"sending rd_shadow_type={res}")
            self.send_data({
                'rd_shadow_type': res,
                'rd_shadow_userdata': userdata
            })
        sharing_request_id = params.get("sharing_request_id")
        if (sharing_request_id
            and sharing_request_id not in self.sharing_requests):
            self.sharing_requests.add(sharing_request_id)
            sharing_mode = params.get("sharing_mode")
            sharing_type = params.get("sharing_type")
            sharing_ttl = params.get("sharing_request_ttl")
            Logger().debug(f"sending sharing_mode={sharing_mode}")
            Logger().debug(f"sending sharing_type={sharing_type}")
            if sharing_type == "SHADOWING":
                self.send_data({
                    'rd_shadow_type': sharing_mode,
                    'rd_shadow_userdata': sharing_request_id,
                })
            elif sharing_mode:
                sharing_mode = sharing_mode.lower()
                enable_control = ("control" in sharing_mode
                                  or "write" in sharing_mode)
                self.send_data({
                    'session_sharing_enable_control': enable_control,
                    'session_sharing_userdata': sharing_request_id,
                    'session_sharing_ttl': sharing_ttl,
                })

    def parse_app(self, value):
        acc_name, sep, app_name = value.rpartition('@')
        if acc_name:
            acc, sep, dom = acc_name.rpartition('@')
            if sep:
                return acc, dom, app_name
        return acc_name, '', app_name

    def check_application(self, effective_target, flags, exe_or_file):
        kv = {
            'auth_command_rail_exec_flags': flags,
            'auth_command_rail_exec_original_exe_or_file': exe_or_file,
            'auth_command_rail_exec_exec_result': '3',
            # RAIL_EXEC_E_NOT_IN_ALLOWLIST
            'auth_command': 'rail_exec'
        }
        app_right_params = self.target_app_rights.get(exe_or_file)
        if app_right_params is not None:
            app_right, app_params = app_right_params
            kv = self._complete_app_infos(kv, app_right, app_params)
            return kv
        acc_name, dom_name, app_name = self.parse_app(exe_or_file)
        if not app_name or not acc_name:
            Logger().debug("check_application: Parsing failed")
            return kv
        app_rights = self.engine.get_proxy_user_rights(['RDP'], app_name)
        Logger().debug(f"check_application: app rights len = {len(app_rights)}")
        app_rights = self.engine.filter_app_rights(
            app_rights, acc_name, dom_name, app_name
        )
        app_params = None
        app_right = None
        Logger().debug(f"check_application: after filter app rights len = {len(app_rights)}")
        for ar in app_rights:
            if not self.engine.check_effective_target(ar, effective_target):
                Logger().debug("check_application: jump server not compatible")
                continue
            _status, _infos = self.engine.check_target(ar)
            if _status != APPROVAL_ACCEPTED:
                Logger().debug("check_application: approval not accepted")
                continue
            _deconnection_time = _infos.get('deconnection_time')
            if (_deconnection_time != "-"
                and _deconnection_time[0:4] < "2034"):
                _tt = datetime.strptime(_deconnection_time,
                                        "%Y-%m-%d %H:%M:%S").timetuple()
                _timeclose = int(mktime(_tt))
                if _timeclose != self.shared.get('timeclose'):
                    Logger().debug("check_application: timeclose different")
                    self.engine.release_target(ar)
                    continue
            app_params = self.engine.get_app_params(ar, effective_target)
            if app_params is None:
                self.engine.release_target(ar)
                Logger().debug("check_application: Get app params failed")
                continue
            app_right = ar
            break
        if app_params is None:
            return kv
        self.target_app_rights[exe_or_file] = (app_right, app_params)
        kv = self._complete_app_infos(kv, app_right, app_params)
        return kv

    def _complete_app_infos(self, kv, app_right, app_params):
        app_login_info = self.engine.get_target_login_info(app_right)

        kv['auth_command_rail_exec_exe_or_file'] = app_params.program
        kv['auth_command_rail_exec_arguments'] = app_params.params or ''
        kv['auth_command_rail_exec_working_dir'] = app_params.workingdir
        kv['auth_command_rail_exec_exec_result'] = '0'  # RAIL_EXEC_S_OK
        kv['auth_command_rail_exec_account'] = ''
        kv['auth_command_rail_exec_password'] = ''
        if app_params.params is not None:
            if '${USER}' in app_params.params:
                kv['auth_command_rail_exec_account'] = \
                    app_login_info.account_login
            if '${PASSWORD}' in app_params.params:
                kv['auth_command_rail_exec_password'] = \
                    self.engine.get_target_password(app_right) \
                    or self.engine.get_primary_password(app_right) \
                    or ''
        return kv

    def _manage_pm(self):
        response = pm_request(self.engine, self.shared.get("pm_request"))
        self.shared["pm_request"] = ""
        self.send_data({'pm_response': json.dumps(response)})

    def _load_selector_banner(self):
        if not self._selector_banner:
            return

        banner_message = self._selector_banner.get("message", "")
        banner_type = self._selector_banner.get("type", "")
        banner_enable = self._selector_banner.get("enable", False)

        if banner_enable is True and banner_message:
            if banner_type == "alert":
                banner_type = 2
            elif banner_type == "warn":
                banner_type = 1
            else:  # banner_type == "info"
                banner_type = 0

            data_to_send = {
                "banner_message": banner_message,
                "banner_type": banner_type
            }

            self.send_data(data_to_send)

    def _load_kerberos_armoring_options(self, conn_opts):
        krb_data = {}
        if 'rdp' in conn_opts:
            rdp_opts = conn_opts.get('rdp', {})
            krb_armoring_account = rdp_opts.get('krb_armoring_account')
            krb_armoring_realm = rdp_opts.get('krb_armoring_realm')
            effective_krb_armoring_user = rdp_opts.get('krb_armoring_fallback_user') or ''
            effective_krb_armoring_password = rdp_opts.get('krb_armoring_fallback_password') or ''

            if krb_armoring_account:
                acc_infos = self.engine.get_scenario_account(
                    krb_armoring_account,
                    force_device=False,
                ) or {}
                effective_krb_armoring_user = acc_infos.get('login', effective_krb_armoring_user)
                effective_krb_armoring_password = acc_infos.get('password', effective_krb_armoring_password)

            if krb_armoring_realm:
                effective_krb_armoring_user += '@' + krb_armoring_realm

            krb_data['effective_krb_armoring_user'] = \
                effective_krb_armoring_user
            krb_data['effective_krb_armoring_password'] = \
                effective_krb_armoring_password
        return krb_data

    def _load_vnc_over_ssh_options(self, kv: Dict[str, Any], opts: Dict[str, Any]) -> Tuple[bool, str]:
        from .tunneling_process import check_tunneling
        Logger().debug("VNC over SSH Tunneling")
        try:
            self.tun_process = check_tunneling(
                self.engine,
                opts,
                self._physical_target_host,
                kv['target_port'],
                sock_path_dir=SOCK_PATH_DIR
            )

            status = self.tun_process.start()
            if status:
                kv['tunneling_target_host'] = self.tun_process.sock_path
                status = self.tun_process.pre_connect()
                if status:
                    return status, "No Error"

        except Exception:
            import traceback
            Logger().debug(traceback.format_exc())

        return False, "VNC over SSH Tunneling Error"


# END CLASS - Sesman


# This little main permets to run the Sesman Server Alone for one connection
# if __name__ == '__main__':
#    sck = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#    sck.bind(('', 3350))
#    sck.listen(100)
#    connection, address = sck.accept()

#    Sesman(connection, address)

# EOF
