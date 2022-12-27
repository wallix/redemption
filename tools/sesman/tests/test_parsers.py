import unittest

from parsers import parse_account, replace_token, parse_param

class Test_parsers(unittest.TestCase):
    def test_param(self):
        self.assertEqual(parse_param('my_account'), None)
        self.assertEqual(parse_param('my_account@my_domain'),
                         ('my_account', 'my_domain', None))
        self.assertEqual(parse_param('my_account@my_domain', ''),
                         ('my_account', 'my_domain', None))
        self.assertEqual(parse_param('my_account@my_domain', ''),
                         ('my_account', 'my_domain', None))
        self.assertEqual(parse_param('my_account@my_domain', 'bad_device'),
                         ('my_account', 'my_domain', None))
        self.assertEqual(parse_param('my_account@my_domain@my_device'),
                         ('my_account', 'my_domain', 'my_device'))
        self.assertEqual(parse_param('my_account@my_domain@my_device', ''),
                         ('my_account', 'my_domain', 'my_device'))
        self.assertEqual(parse_param('my_account@my_domain@my_device', 'bad_device'),
                         None)
        self.assertEqual(parse_param('my_account@my_domain@my_device', 'my_device'),
                         ('my_account', 'my_domain', 'my_device'))

    def test_replace_token(self):
        self.assertEqual(replace_token('abcd', {}), 'abcd')
        self.assertEqual(replace_token('ab<c>d', {}), 'abd')
        self.assertEqual(replace_token('a<bc>d', {}), 'ad')

        replace = {'c': 'xx', 'bc': 'yy'}
        self.assertEqual(replace_token('abcd', replace), 'abcd')
        self.assertEqual(replace_token('ab<c>d', replace), 'abxxd')
        self.assertEqual(replace_token('a<bc>d', replace), 'ayyd')

    def test_parse_account(self):
        replace = {'domain': 'DOMAIN', 'device': 'DEVICE'}
        self.assertEqual(parse_account('my_account@my_<domain>@my_device', replace, False),
                         ('my_account', 'my_DOMAIN', 'my_device'))
        self.assertEqual(parse_account('my_account@my_<domain>@my_device', replace, True),
                         None)
        self.assertEqual(parse_account('my_account@my_<domain>@DEVICE', replace, True),
                         ('my_account', 'my_DOMAIN', 'DEVICE'))
