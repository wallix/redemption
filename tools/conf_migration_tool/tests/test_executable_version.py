import unittest

from conf_migrate import RedemptionVersion

class Test_RedemptionVersion(unittest.TestCase):
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

        v_invalid = RedemptionVersion()
        self.assertFalse(v_3_5_9 > v_invalid)
        self.assertFalse(v_invalid > v_invalid)
        self.assertTrue(v_invalid > v_3_5_9)

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

        v_invalid = RedemptionVersion()
        self.assertFalse(v_invalid < v_3_5_9)
        self.assertFalse(v_invalid < v_invalid)
        self.assertTrue(v_3_5_9 < v_invalid)

    def test_operator_init(self):
        v_invalid = RedemptionVersion()
        self.assertTrue(str(v_invalid), "(invalid)")

        v_invalid_2 = RedemptionVersion("TEST")
        self.assertTrue(str(v_invalid_2), "(invalid)")

    def test_operator_str(self):
        v_3_5_9 = RedemptionVersion("3.5.9")
        self.assertTrue(str(v_3_5_9), "3.5.9")

    def test_from_file(self):
        v_from_file = RedemptionVersion.fromfile(
            "./tests/fixtures/REDEMPTION_VERSION")
        self.assertTrue(str(v_from_file), "9.1.17")

        v_invalid_from_file = RedemptionVersion.fromfile(
            "./tests/fixtures/REDEMPTION_VERSION")
        self.assertTrue(str(v_invalid_from_file), "(invalid)")
