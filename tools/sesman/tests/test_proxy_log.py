import unittest
import syslog

from unittest.mock import patch
from proxy_log import RdpProxyLog


class Test_RdpProxyLog(unittest.TestCase):
    @patch('syslog.syslog')
    def test_log(self, mock):
        logger = RdpProxyLog()
        logger.update_context(1234, 'myname')

        logger.log('TEST', myvalue='first "value"', data="ab\\ds\"a\\'")
        mock.assert_called_with(syslog.LOG_INFO, '[rdpproxy] psid="1234" user="myname" type="TEST" myvalue="first \\"value\\"" data="ab\\\\ds\\\"a\\\\\'"')

        logger.log('CONN', vvv='abc', target="computer")  # target moved before vvv
        mock.assert_called_with(syslog.LOG_INFO, r'[rdpproxy] psid="1234" user="myname" type="CONN" target="computer" vvv="abc"')
