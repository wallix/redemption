import hashlib
import os
import unittest

from conf_migrate import load_configuration_file, save_configuration_file

class Test_ConfigurationFile(unittest.TestCase):
    def test_load_save(self):
        target_file = './rdpproxy.sav'

        try:
            os.remove(target_file)
        except:
            pass

        file_content = load_configuration_file('./tests/fixtures/rdpproxy.ini')

        save_configuration_file(target_file, file_content)

        f = open(target_file, 'rb')

        self.assertEqual(hashlib.md5(f.read()).hexdigest(),
            'ffb503380ff480ced4c26ba4ce3b42bb')

        f.close()

        os.remove(target_file)
