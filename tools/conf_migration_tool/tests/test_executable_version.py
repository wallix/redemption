#!/usr/bin/env python3
import unittest

from conf_migrate import RedemptionVersion, RedemptionVersionError

class Test_RedemptionVersion(unittest.TestCase):
    def test_invalid_version(self):
        with self.assertRaises(RedemptionVersionError):
            RedemptionVersion("3.5")
        with self.assertRaises(RedemptionVersionError):
            RedemptionVersion("TEST")

    def test_operator_greater_than(self):
        v_3_5_9 = RedemptionVersion("3.5.9")
        v_3_5_10 = RedemptionVersion("3.5.10")
        self.assertFalse(v_3_5_9 > v_3_5_9)
        self.assertFalse(v_3_5_9 > v_3_5_10)
        self.assertTrue(v_3_5_10 > v_3_5_9)

        v_3_5_9c = RedemptionVersion("3.5.9c")
        self.assertFalse(v_3_5_9 > v_3_5_9c)
        self.assertFalse(v_3_5_9c > v_3_5_9c)
        self.assertTrue(v_3_5_9c > v_3_5_9)

        v_3_5_9d = RedemptionVersion("3.5.9d")
        self.assertFalse(v_3_5_9c > v_3_5_9d)
        self.assertTrue(v_3_5_9d > v_3_5_9c)

    def test_operator_less_than(self):
        v_3_5_9 = RedemptionVersion("3.5.9")
        v_3_5_10 = RedemptionVersion("3.5.10")
        self.assertFalse(v_3_5_9 < v_3_5_9)
        self.assertFalse(v_3_5_10 < v_3_5_9)
        self.assertTrue(v_3_5_9 < v_3_5_10)

        v_3_5_9c = RedemptionVersion("3.5.9c")
        self.assertFalse(v_3_5_9c < v_3_5_9)
        self.assertFalse(v_3_5_9c < v_3_5_9c)
        self.assertTrue(v_3_5_9 < v_3_5_9c)

        v_3_5_9d = RedemptionVersion("3.5.9d")
        self.assertFalse(v_3_5_9d < v_3_5_9c)
        self.assertTrue(v_3_5_9c < v_3_5_9d)

    def test_operator_str(self):
        self.assertEqual(str(RedemptionVersion("3.5.9")), "3.5.9")

    def test_from_file(self):
        v_from_file = RedemptionVersion.fromfile(
            "./tests/fixtures/REDEMPTION_VERSION")
        self.assertEqual(str(v_from_file), "9.1.17")

        with self.assertRaises(Exception):
            RedemptionVersion.fromfile(
                "./tests/fixtures/REDEMPTION_VERSION_KO")
