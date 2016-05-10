#!/usr/bin/python -O
# -*- coding: utf-8 -*-
#
# Copyright (c) 2011 WALLIX, SARL. All rights reserved.
#
# Licensed computer software. Property of WALLIX.
# Product Name : Sphinx
# Author(s)    : Tristan de Cacqueray <tdc@wallix.com>
# Id           : $Id$
# URL          : $URL$

"""
Crypto errors
"""

import traceback

class CryptoError(Exception):
    """ The CryptoError class """

    def __init__(self, message):
        """ Store in self.caller the parent context """
        Exception.__init__(self, message)
        self.message = message
#        self.caller = "%s:%d in %s" % (traceback.extract_stack()[-2][:-1])

    def __str__(self):
        return self.__repr__()

    def __repr__(self):
        return self.message #"%s: %s" % (self.caller, self.message)

class WrongPassPhrase(CryptoError):
    def __init__(self):
        CryptoError.__init__(self, u"Wrong pass phrase!")

