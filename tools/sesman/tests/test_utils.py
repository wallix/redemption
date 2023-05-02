import unittest

from utils import collection_has_more, parse_duration


class Test_utils(unittest.TestCase):
    def test_collection_has_more_empty(self):
        it = collection_has_more([])
        with self.assertRaises(StopIteration):
            next(it)

    def test_collection_has_more(self):
        it = collection_has_more([1, 2, 3])
        self.assertEqual(next(it), (1, True))
        self.assertEqual(next(it), (2, True))
        self.assertEqual(next(it), (3, False))
        with self.assertRaises(StopIteration):
            next(it)

    def test_parse_duration(self):
        self.assertEqual(parse_duration(''), 3600)
        self.assertEqual(parse_duration('x'), 3600)  # invalid format
        self.assertEqual(parse_duration('0m'), 3600)  # 0 => 3600
        self.assertEqual(parse_duration('86m'), 86 * 60)
        self.assertEqual(parse_duration('17h'), 17 * 3600)
        self.assertEqual(parse_duration('17h86m'), 17 * 3600 + 86 * 60)
