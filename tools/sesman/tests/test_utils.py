import unittest

from utils import collection_has_more


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
