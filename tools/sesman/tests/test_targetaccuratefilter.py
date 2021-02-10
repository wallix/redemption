import unittest

import targetaccuratefilter as taf
from targetaccuratefilter import (FilterKeywordSyntaxError,
                                  FilterKeywordDuplicateError,
                                  ParsingError)

class Test_targetaccuratefilter(unittest.TestCase):
    def test_get_selector_filter_mode_with_several_patterns(self):
        def expect_is_selector_filter_mode_equal(
                pattern, expected_selector_filter_mode):
            with self.subTest(pattern = pattern):
                self.assertEqual(
                    taf.get_selector_filter_mode(pattern),
                    expected_selector_filter_mode)


        expect_is_selector_filter_mode_equal(
            None,
            taf.SelectorFilterMode.NONE)
        expect_is_selector_filter_mode_equal(
            "",
            taf.SelectorFilterMode.NONE)
        expect_is_selector_filter_mode_equal(
            "abcdefgh",
            taf.SelectorFilterMode.NORMAL)
        expect_is_selector_filter_mode_equal(
            "12345678",
            taf.SelectorFilterMode.NORMAL)
        expect_is_selector_filter_mode_equal(
            "?",
            taf.SelectorFilterMode.ADVANCED)
        expect_is_selector_filter_mode_equal(
            "?device=my_device&?account=my_account",
            taf.SelectorFilterMode.ADVANCED)


    def test_get_filter_pattern_dict_with_several_filter_patterns(self):
        def expect_is_filter_pattern_dict_equal(filter_patterns,
                                                expected_filter_pattern_dict):
            with self.subTest(filter_patterns = filter_patterns):
                self.assertDictEqual(
                    taf.get_filter_pattern_dict(filter_patterns),
                    expected_filter_pattern_dict)


        # with one filter

        expect_is_filter_pattern_dict_equal(
            "?account=toto",
            {"account" : "toto"})

        expect_is_filter_pattern_dict_equal(
            "?domain=&&&tata&",
            {"domain" : "&&&tata&"})

        expect_is_filter_pattern_dict_equal(
            "?device=ti=ti",
            {"device" : "ti=ti"})

        expect_is_filter_pattern_dict_equal(
            "?service=ab&=cd",
            {"service" : "ab&=cd"})


        # with two filters

        expect_is_filter_pattern_dict_equal(
            "?account=toto&?domain=tata",
            {"account" : "toto",
             "domain" : "tata"})

        expect_is_filter_pattern_dict_equal(
            "?device=toto=tata&?account=titi&",
            {"device" : "toto=tata",
             "account" : "titi&"})


        # with three filters

        expect_is_filter_pattern_dict_equal(
            "?account=toto&?domain==tata=&?device=titi&=",
            {"account" : "toto",
             "domain" : "=tata=",
             "device" : "titi&="})

        expect_is_filter_pattern_dict_equal(
            "?domain=toto&?device=tata&?account=titi",
            {"domain" : "toto",
             "device" : "tata",
             "account" : "titi"})


        # with four filters

        expect_is_filter_pattern_dict_equal(
            "?account=&to=to&&?domain=tata&?device=&=&titi&=&&?service=abcd",
            {"account" : "&to=to&",
             "domain" : "tata",
             "device" : "&=&titi&=&",
             "service" : "abcd"})

        expect_is_filter_pattern_dict_equal(
            "?service=toto&?device=tata&?domain=titi&?account=abcd",
            {"service" : "toto",
             "device" : "tata",
             "domain" : "titi",
             "account" : "abcd"})


    def test_is_filterable_with_several_filter_pattern_dicts(self):
        def expect_is_filterable_true(filter_pattern_dict,
                                      target_field_dict):
            with self.subTest(filter_pattern_dict = filter_pattern_dict):
                self.assertTrue(taf.is_filterable(filter_pattern_dict,
                                                  target_field_dict))


        # with one filter

        expect_is_filterable_true(
            # filter_patterns :
            #  "?account=toto"
            {"account" : "toto"},

            {"account" : "toto",
             "device" : "my_device"})

        expect_is_filterable_true(
            # filter_patterns :
            #  "?domain=toto"
            {"domain" : "toto"},

            {"domain" : "toto",
             "device" : "my_device"})

        expect_is_filterable_true(
            # filter_patterns :
            #  "?device=toto"
            {"device" : "toto"},

            {"account" : "my_account",
             "device" : "toto"})

        expect_is_filterable_true(
            # filter_patterns :
            #  "?service=toto"
            {"service" : "toto"},

            {"account" : "my_account",
             "device" : "my_device",
             "service" : "toto"})


        # with two filters

        expect_is_filterable_true(
            # filter_patterns :
            #  "?account=toto&?domain=tata"
            {"account" : "toto",
             "domain" : "tata"},

            {"account" : "toto",
             "domain" : "tata",
             "device" : "my_device"})

        expect_is_filterable_true(
            # filter_patterns :
            #  "?device=toto&?account=tata"
            {"device" : "toto",
             "account" : "tata"},

            {"account" : "tata",
             "domain" : "my_domain",
             "device" : "toto"})


        # with three filters

        expect_is_filterable_true(
            # filter_patterns :
            #  "?account=toto&?domain=tata&?device=titi"
            {"account" : "toto",
             "domain" : "tata",
             "device" : "titi"},

            {"account" : "toto",
             "domain" : "tata",
             "device" : "titi"})

        expect_is_filterable_true(
            # filter_patterns :
            #  "?domain=toto&?device=tata&?account=titi"
            {"domain" : "toto",
             "device" : "tata",
             "account" : "titi"},

            {"account" : "titi",
             "domain" : "toto",
             "device" : "tata"})


        # with four filters

        expect_is_filterable_true(
            # filter_patterns :
            #  "?account=toto&?domain=tata&?device=titi&?service=abcd"
            {"account" : "toto",
             "domain" : "tata",
             "device" : "titi",
             "service" : "abcd"},

            {"account" : "toto",
             "domain" : "tata",
             "device" : "titi",
             "service" : "abcd"})

        expect_is_filterable_true(
            # filter_patterns :
            #  "?service=toto&?device=tata&?domain=titi&?account=abcd"
            {"service" : "toto",
             "device" : "tata",
             "domain" : "titi",
             "account" : "abcd"},

            {"account" : "abcd",
             "domain" : "titi",
             "device" : "tata",
             "service" : "toto"})


    def test_is_filterable_with_filter_failure(self):
        def expect_is_filterable_false(filter_pattern_dict,
                                       target_field_dict):
            with self.subTest(filter_pattern_dict = filter_pattern_dict):
                self.assertFalse(taf.is_filterable(filter_pattern_dict,
                                                   target_field_dict))


        expect_is_filterable_false(
            # filter_patterns :
            #  "?account=toto&?domain=tata&?device=titi"
            {"account" : "toto",
             "domain" : "tata",
             "device" : "titi"},

            {"account" : "toto",
             "device" : "titi"})

        expect_is_filterable_false(
            # filter_patterns :
            #  "?domain=toto"
            {"domain" : "toto"},

            {"account" : "my_account",
             "device" : "my_device",
             "service" : "my_service"})

        expect_is_filterable_false(
            # filter_patterns :
            #  "?device=toto&?account=123456"
            {"device" : "toto",
             "account" : "123456"},

            {"account" : "my_account",
             "device" : "toto",
             "service" : "my_service"})

        expect_is_filterable_false(
            # filter_patterns :
            #  "?service=42"
            {"service" : "42"},

            {"account" : "my_account",
             "domain" : "my_domain",
             "device" : "my_device",
             "service" : "my_service"})


    def test_get_filter_pattern_dict_with_filter_keyword_duplicate_error_on_filter_patterns(self):
        def expect_filter_keyword_duplicate_error(filter_patterns,
                                                  expected_filter_keyword_err):
            with self.subTest(filter_patterns = filter_patterns):
                with self.assertRaises(FilterKeywordDuplicateError) as cm:
                    taf.get_filter_pattern_dict(filter_patterns)
                self.assertEqual(cm.exception.args[1],
                                 ("duplicated filter keyword with '%s'"
                                  % expected_filter_keyword_err))


        expect_filter_keyword_duplicate_error(
            "?account=my_account&?account=my_account",
            "?account")
        expect_filter_keyword_duplicate_error(
            "?device=my_device&?device=my_device2&?device=my_device3",
            "?device")


    def test_get_filter_pattern_dict_with_filter_keyword_syntax_error_on_filter_patterns(self):
        def expect_filter_keyword_syntax_error(filter_patterns,
                                               expected_filter_keyword_err):
            with self.subTest(filter_patterns = filter_patterns):
                with self.assertRaises(FilterKeywordSyntaxError) as cm:
                    taf.get_filter_pattern_dict(filter_patterns)
                self.assertEqual(cm.exception.args[1],
                                 ("unknown filter keyword with '%s'"
                                  % expected_filter_keyword_err))


        expect_filter_keyword_syntax_error(
            "?my_device=my_device",
            "?my_device")
        expect_filter_keyword_syntax_error(
            "?account?account=my_account",
            "?account?account")
        expect_filter_keyword_syntax_error(
            "?abcdefgh=my_account",
            "?abcdefgh")
        expect_filter_keyword_syntax_error(
            "?accountaccount=my_account",
            "?accountaccount")
        expect_filter_keyword_syntax_error(
            "?device=my_device&??service=my_service",
            "??service")
        expect_filter_keyword_syntax_error(
            "?domain=my_domain&??&account=my_account",
            "??&account")


    def test_get_filter_pattern_dict_with_parsing_error_on_filter_patterns(self):
        def expect_parsing_error(filter_patterns, expected_bad_string_err):
            with self.subTest(filter_patterns = filter_patterns):
                with self.assertRaises(ParsingError) as cm:
                    taf.get_filter_pattern_dict(filter_patterns)
                self.assertEqual(cm.exception.args[1],
                                 ("'keyword"
                                  + taf.FILTER_KV_SEPARATOR
                                  + "value' format parsing error with '%s'"
                                  % expected_bad_string_err))


        expect_parsing_error(
            "?domain=my_domain&?",
            "?")
        expect_parsing_error(
            "?service=&?",
            "?service=")
        expect_parsing_error(
            "?service=&?device=my_device",
            "?service=")
        expect_parsing_error(
            "?account?accountmy_account",
            "?account?accountmy_account")
        expect_parsing_error(
            "????????????????????",
            "????????????????????")
        expect_parsing_error(
            "&&&&&&&&&&&&&&&&&&&&",
            "&&&&&&&&&&&&&&&&&&&&")
        expect_parsing_error(
            "?service=======&?my_service",
            "?my_service")
        expect_parsing_error(
            "?&?&?&?&?&?&?&",
            "?")
        expect_parsing_error(
            "?device=my_device&?&?&?&?domain=my_domain",
            "?")
