import unittest

from addrutils import is_device_in_subnet

class Test_addrutils(unittest.TestCase):
    def test_is_device_in_subnet_failure(self):
        def expect_false_result(device, subnet):
            with self.subTest(device = device, subnet = subnet):
                self.assertFalse(is_device_in_subnet(device, subnet))


        # with subnet param badly formatted
        expect_false_result("0.0.0.0", None)
        expect_false_result("0.0.0.0", "")
        expect_false_result("0.0.0.0", "0123456789")
        expect_false_result("0.0.0.0", "ABCDEFGHIJ")
        expect_false_result("0.0.0.0", "A.B.C.D")
        expect_false_result("0.0.0.0", "127..0..0..1")
        expect_false_result("0.0.0.0", "127.0.0.1/64")
        expect_false_result("0.0.0.0", "::::1")
        expect_false_result("0.0.0.0", "::1/256")

        # with device param badly formatted
        expect_false_result(None, "0.0.0.0/0")
        expect_false_result("", "0.0.0.0/0")
        expect_false_result("0123456789", "0.0.0.0/0")
        expect_false_result("ABCDEFGHIJ", "0.0.0.0/0")
        expect_false_result("A.B.C.D", "0.0.0.0/0")
        expect_false_result("127..0..0..1", "0.0.0.0/0")
        expect_false_result("127.0.0.1/64", "0.0.0.0/0")
        expect_false_result("::::1", "0.0.0.0/0")
        expect_false_result("::1/9999", "0.0.0.0/0")

        # with subnet no matching (with IPv4 only)
        expect_false_result("172.16.254.1", "0.0.0.0/24")
        expect_false_result("192.168.1.15", "192.125.1.5/16")
        expect_false_result("10.10.10.10", "150.62.9.200/24")
        expect_false_result("0.0.0.0", "180.0.150.9/20")
        expect_false_result("129.5.9.200", "129.5.9.199")

        # with subnet no matching (with IPv6 only)
        expect_false_result("2001:0000:3238:DFE1:63::FEFB",
                            "fe80::1ff:fe23:4567:890a/32")
        expect_false_result("::ffff:0:0",
                            "2002:0620:0000:0000:0211:24FF:FE80:C12C/96")
        expect_false_result("1543:0000:3238:AAAA:63::FFFF",
                            "::1/8")
        expect_false_result("0000:0000:0000:0000:0000:0000:0000:0000",
                            "febf:ffff:ffff:ffff:ffff:ffff:ffff:ffff/58")
        expect_false_result("2001:db8:a88:85a3::ac1f:8001",
                            "2001:db8:a88:2390::ac1f:8001")

        # with subnet no matching (both IPv4 and Ipv6)
        expect_false_result("2001:0:1428:8f18:1480:1e5c:3f57:ffef",
                            "134.90.170.60/20")
        expect_false_result("150.12.67.199",
                            "fe80::4abc:6421:32FF/18")
        expect_false_result("::ffff:130.180.0.2",
                            "130.180.99.200/19")
        expect_false_result("2002:C0A8:2102::1",
                            "0.0.0.0/14")
        expect_false_result("180.20.50.40",
                            "2001:db8:a88:85a3::ac1f:8001")

    def test_is_device_in_subnet_success(self):
        def expect_true_result(device, subnet):
            with self.subTest(device = device, subnet = subnet):
                self.assertTrue(is_device_in_subnet(device, subnet))


        # with subnet matching (with IPv4 only)
        expect_true_result("0.0.0.0", "0.0.0.0/0")
        expect_true_result("56.150.201.40", "56.150.128.99/17")
        expect_true_result("169.3.97.240", "169.3.97.2/24")
        expect_true_result("5.224.89.130", "5.224.32.89/11")
        expect_true_result("200.134.79.1", "200.134.79.1")

        # with subnet matching (with IPv6 only)
        expect_true_result("2001:0:1428:8f18:1480:1e5c:3f57:ffef",
                           "2001:0000:3238:DFE1:63::FEFB/32")
        expect_true_result("2002:0620:0000:0000:0211:24FF:FE80:C12C",
                           "2002:00A8:2102::1/20")
        expect_true_result("2001:db8:a88:85a3::ac1f:8001",
                           "2001:db8:a88:85a3::ac1f:8099/88")
        expect_true_result("3FFE:0000:0000:0003:0200:F8FF:FE75:50DF",
                           "3FFE:0000:0000:0003:5147:D1FA:3EBC:9FAB/50")
        expect_true_result("2001:db8:a88:85a3::ac1f:8001",
                           "2001:db8:a88:85a3::ac1f:8001")

        # with subnet matching (both IPv4 and IPv6)
        expect_true_result("2001:0:1428:8f18:1480:1e5c:3f57:ffef",
                           "20.40.140.0/22")
        expect_true_result("150.12.67.199",
                           "2002::960c:43D2/40")
        expect_true_result("::ffff:130.180.0.2",
                           "130.180.5.9/16")
        expect_true_result("2002:C0A8:2102::1",
                           "192.168.32.9/21")
        expect_true_result("180.20.50.40",
                           "2002::b414:3299/38")
