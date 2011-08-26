# -*- coding: utf-8 -*-

#
# This is a grammar for extracting usefull docstrings
# for Redemption
#

import unittest

# Testing regexp which will just be copied because of crappy conception
# of yapps2
class TestTokenRe(object):
    def test_doc(self):
        #self.assertEqual()
        #self.assertTrue()
        pass

%%
parser redop_grammar:
    ignore:             r"\s"                               # Whitespaces and new lines
    ignore:             r"(?s)/\*.+?\*/"                    # C-style comments
    ignore:             r"#.+"                              # Any preprocessor directive
    ignore:             r"(typedef)?\s*enum[^}]+\};"        # Any enum
    ignore:             r"(public:|private:)"               # Hide public and private

    token BLURB:        r"[^{}]+"                           # Any brack-free string
    token DOC:          r"(?://.+)+"                        # Documentation-style string
    token CLASS:        r"class"                            # CLASS keyword
    token STRUCT:       r"struct"                           # STRUCT keyword
    token NAMESPACE:    r"namespace"                        # NAMESPACE keyword
    token METHID:       r"((\w|[&*<>])+\s+)*((\w|==|~)+\s*\([^)]*?\)\s*(:?[^{]+)?)" # Method ID (wide vision)
    token ID:           r"\w+\b"                            # ID is a word
    token STATEMENT:    r"[^{}'//']+?;"                     # Anything that ends with a semicolon
    token EOF:          r"$"                                # End of file

    rule top                   : documentableCodeItems EOF

    rule documentableCodeItems : (
                                 documentableCodeItem 
                                 )*
    
    rule documentableCodeItem  : class_ 
                               | statement
                               | method
                               | doc
    
    rule restrictedCodeItems   : '{' (restrictedCodeItems2) * '}'
    rule restrictedCodeItems2  : restrictedCodeItem
                               | restrictedCodeItems

    rule restrictedCodeItem    : BLURB 
    
    rule method                : METHID [ DOC ] restrictedCodeItems
                                 {{ print(".  method %s" % METHID) }}
    
    rule class_                : {{ print("Begin vv") }}
                                 classHead [ DOC ] '{' documentableCodeItems '}' [ ';' ]
                                 {{ print("End   ^^") }}
    
    rule classHead             : CLASS ID     {{ print(".  CLASS     named %s" % ID) }}
                               | STRUCT ID    {{ print(".  STRUCT    named %s" % ID) }}
                               | NAMESPACE ID {{ print(".  NAMESPACE named %s" % ID) }}

    rule statement             : STATEMENT
                                 {{ print(".  got %s" % STATEMENT) }}

    rule doc                   : DOC
                                 {{ print(".  doc %s" % DOC[2:]) }}
                               
%%
# Python code

