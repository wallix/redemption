import unittest

from parsers import (parse_account,
                     replace_token,
                     parse_param,
                     parse_auth,
                     parse_app,
                     parse_duration,
                     )

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

    def test_parse_auth(self):
        self.assertEqual(parse_auth('secondaryuser@target:service:group:primaryuser'),
                         ('primaryuser', ('secondaryuser', 'target', 'service', 'group')))
        self.assertEqual(parse_auth('secondaryuser:service:group:primaryuser'),
                         ('secondaryuser:service:group:primaryuser', None))
        self.assertEqual(parse_auth('secondaryuser@target:service:primaryuser'),
                         ('primaryuser', ('secondaryuser', 'target', 'service', '')))
        self.assertEqual(parse_auth('secondaryuser@target:primaryuser'),
                         ('primaryuser', ('secondaryuser', 'target', '', '')))
        self.assertEqual(parse_auth('secondaryuser@target'),
                         ('secondaryuser@target', None))

        self.assertEqual(parse_auth('secondaryuser@target+service+group+primaryuser'),
                         ('primaryuser', ('secondaryuser', 'target', 'service', 'group')))
        self.assertEqual(parse_auth('secondaryuser+service+group+primaryuser'),
                         ('secondaryuser+service+group+primaryuser', None))
        self.assertEqual(parse_auth('secondaryuser@target+service+primaryuser'),
                         ('primaryuser', ('secondaryuser', 'target', 'service', '')))
        self.assertEqual(parse_auth('secondaryuser@target+primaryuser'),
                         ('primaryuser', ('secondaryuser', 'target', '', '')))
        self.assertEqual(parse_auth('secondaryuser@target'),
                         ('secondaryuser@target', None))

    def test_parse_app(self):
        self.assertEqual(parse_app('aaa@bbb@ccc'), ('aaa', 'bbb', 'ccc'))
        self.assertEqual(parse_app('aaa@bbb'), ('aaa', '', 'bbb'))
        self.assertEqual(parse_app('aaa'), ('', '', 'aaa'))

    def test_parse_duration(self):
        self.assertEqual(parse_duration(''), 3600)
        self.assertEqual(parse_duration('x'), 3600)  # invalid format
        self.assertEqual(parse_duration('0m'), 3600)  # 0 => 3600
        self.assertEqual(parse_duration('86m'), 86 * 60)
        self.assertEqual(parse_duration('17h'), 17 * 3600)
        self.assertEqual(parse_duration('17h86m'), 17 * 3600 + 86 * 60)
