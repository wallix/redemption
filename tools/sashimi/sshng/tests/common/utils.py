
from common.utils import parse_ssh_auth

import unittest
class TestParseSSHAuth(unittest.TestCase):
    def test_parse_1(self):
        #ssh cgrosjean@dummy:SSH:cgrosjean@10.10.43.12
        #ssh -l cgrosjean@dummy:SSH:cgrosjean 10.10.43.12
        #ssh cgrosjean@10.10.43.12 cgrosjean@dummy:SSH
        self.assertEquals(
            parse_ssh_auth("cgrosjean@dummy:SSH:cgrosjean"),
            ('cgrosjean', ('cgrosjean', 'dummy', 'SSH')))

    def test_parse_2(self):
        #ssh  cgrosjean@dummy:SSH:cgrosjean@10.10.43.12
        #ssh -l cgrosjean@dummy:cgrosjean 10.10.43.12
        #ssh cgrosjean@10.10.43.12 cgrosjean@dummy
        self.assertEquals(
            parse_ssh_auth("cgrosjean@dummy:cgrosjean"),
            ('cgrosjean', ('cgrosjean', 'dummy', None)))

    def test_parse_3(self):
        #ssh cgrosjean@10.10.43.12 console
        self.assertEquals(
            parse_ssh_auth("cgrosjean"),
            ('cgrosjean', None))
        self.assertEquals(
            parse_ssh_auth("console"),
            ('console', None))

# scp cgrosjean@10.10.43.12:dummy:cgrosjean@dummy:SSH:WAB_31.ova .
