# -*- coding: utf-8 -*-

#
# This (small) program is called ReDoP
# for Redemption Documentation Parser
#
# It now uses YAPPS2 (instead of LELR which has a lousy REGEX Parser)

import redop_grammar 
import sys

class Redop:

    def __init__(self):
        # Import file to string
        self.source_to_parse = ''
        file_to_parse = ''
        if sys.argv[1]:
            file_to_parse = open(sys.argv[1])
            print("Opened %s" % sys.argv[1])
        else:
            print("HORROR!! Error")

        # Load the whole file in a string
        for line in file_to_parse:
            self.source_to_parse += line

    def parse(self):
        redop_grammar.parse('top', self.source_to_parse)
 
if __name__ == "__main__":
    redop = Redop()
    redop.parse()
# class docParserTest(unittest.TestCase):
#     
#     def setUp(self):
#         pass
#     
#     def test_tokenIgnored(self):
#         self.assertTrue(
#                 docParser.IGNORED.parse(
#                 ''.join(u"""/* La
#                 Salade c'est bon
#                 mais point
#                 trop n'en fô
#                 */""".split("\n")))
#         )
# 
#     def test_tokenStatement(self):
#         self.assertTrue(
#                 docParser.STATEMENT.parse(
#                 ''.join(
#                 u"""long int var;
#                 long long var2 = blah();
#                 */""".split("\n")))
#         )
