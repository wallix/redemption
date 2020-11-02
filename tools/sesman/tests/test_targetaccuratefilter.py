import unittest
import targetaccuratefilter as taf
from targetaccuratefilter import (FilterKeywordSyntaxError,
                                  FilterKeywordDuplicateError,
                                  ParsingError)

class Test_is_filterable(unittest.TestCase) :
    def test_is_filterable_with_several_filters(self) :
        def expect_is_filterable_true(filter_patterns, target_field_dict) :
            with self.subTest(filter_patterns = filter_patterns) :
                self.assertTrue(taf.is_filterable(filter_patterns,
                                                  target_field_dict))


        # with one filter
        
        expect_is_filterable_true(
            "$account:my_account",
            {"account" : "my_account",
             "device": "my_device"})
        
        expect_is_filterable_true(
            "$domain:my_domain",
            {"account" : "my_account",
             "domain" : "my_domain",
             "device" : "my_device"})

        expect_is_filterable_true(
            "$device:my_device",
            {"account" : "my_account",
             "device": "my_device"})

        expect_is_filterable_true(
            "$service:my_service",
            {"account" : "my_account",
             "device" : "my_device",
             "service" : "my_service"})


        # with two filters
        
        expect_is_filterable_true(
            ("$account:my_account,"
             "$domain:my_domain"),
            {"account" : "my_account",
             "domain" : "my_domain",
             "device" : "my_device"})

        expect_is_filterable_true(
            ("$device:my_device,"
             "$account:my_account"),
            {"account" : "my_account",
             "domain" : "my_domain",
             "device" : "my_device"})


        # with three filters
        
        expect_is_filterable_true(
            ("$account:my_account,"
             "$domain:my_domain,"
             "$device:my_device"),
            {"account" : "my_account",
             "domain" : "my_domain",
             "device" : "my_device"})

        expect_is_filterable_true(
            ("$domain:my_domain,"
             "$device:my_device,"
             "$account:my_account"),
            {"account" : "my_account",
             "domain" : "my_domain",
             "device" : "my_device"})


        # with four filters
        
        expect_is_filterable_true(
            ("$account:my_account,"
             "$domain:my_domain,"
             "$device:my_device,"
             "$service:my_service"),
            {"account" : "my_account",
             "domain" : "my_domain",
             "device" : "my_device",
             "service" : "my_service"})

        expect_is_filterable_true(
            ("$service:my_service,"
             "$device:my_device,"
             "$domain:my_domain,"
             "$account:my_account"),
            {"account" : "my_account",
             "domain" : "my_domain",
             "device" : "my_device",
             "service" : "my_service"})


    def test_is_filterable_with_filter_failure(self) :
        def expect_is_filterable_false(filter_patterns, target_field_dict) :
            with self.subTest(filter_patterns = filter_patterns) :
                self.assertFalse(taf.is_filterable(filter_patterns,
                                                   target_field_dict))

                                 
        expect_is_filterable_false(
            ("$account:my_account,"
             "$domain:my_domain,"
             "$device:my_device"),
            {"account" : "my_account",
             "device" : "my_device"})

        expect_is_filterable_false(
            "$domain:my_domain",
            {"account" : "my_account",
             "device" : "my_device",
             "service" : "my_service"})

        expect_is_filterable_false(
            "$device:my_device,$account:123456",
            {"account" : "my_account",
             "device" : "my_device",
             "service" : "my_service"})

        expect_is_filterable_false(
            "$service:42",
            {"account" : "my_account",
             "domain" : "my_domain",
             "device" : "my_device",
             "service" : "my_service"})

    def test_is_filterable_with_filter_keyword_duplicate_error_on_filter_patterns(self) :
        def expect_filter_keyword_duplicate_error(filter_patterns) :
            with self.subTest(filter_patterns = filter_patterns) :
                self.assertRaises(FilterKeywordDuplicateError,
                                  taf.is_filterable,
                                  filter_patterns,
                                  {})
                
        expect_filter_keyword_duplicate_error(
            "$account:my_account,$account:my_account")
        expect_filter_keyword_duplicate_error(
            "$device:my_device,$device:my_device2,$device:my_device3")

    def test_is_filterable_with_filter_keyword_syntax_error_on_filter_patterns(self) :
        def expect_filter_keyword_syntax_error(filter_patterns) :
            with self.subTest(filter_patterns = filter_patterns) :
                self.assertRaises(FilterKeywordSyntaxError,
                                  taf.is_filterable,
                                  filter_patterns,
                                  {})

        expect_filter_keyword_syntax_error(
            "$my_device:my_device")
        expect_filter_keyword_syntax_error(
            "$account$account:my_account")
        expect_filter_keyword_syntax_error(
            "$abcdefgh:my_account")
        expect_filter_keyword_syntax_error(
            "$accountaccount:my_account")

    def test_is_filterable_with_parsing_error_on_filter_patterns(self) :
        def expect_parsing_error(filter_patterns) :
            with self.subTest(filter_patterns = filter_patterns) :
                self.assertRaises(ParsingError,
                                  taf.is_filterable,
                                  filter_patterns,
                                  {})

        expect_parsing_error(
            "$domain:my_domain,")
        expect_parsing_error(
            "$service:")
        expect_parsing_error(
            "$service:,$device:my_device")
        expect_parsing_error(
            "$account$accountmy_account")
        expect_parsing_error(
            "$$$$$$$$$$$$$$$$$$$$")
        expect_parsing_error(
            "$service:::::::,my_service")
