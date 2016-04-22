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

""" Crypto module interface library """

import zope.interface as zi

class ICrypto(zi.Interface):
    """ Handle data security """

############################################################################################################
# Initialization methods
    def __init__(security_level = 0, key_base_path = "/var/wab/etc", use_disk = True, use_memory = True):
        """ key_base_path is the storage place of the wab master key, will be passed to MasterKey
        class constructor
        """

    def initialize(pass_phrase = "", generate_key = False, key_data = None):
        """ Initialize the crypto engine.
        if generate_key == True:
            Generate and store a master Key, this must be done only once, at first boot,
            optional pass_phrase to encrypt the master key just after generation.
        else:
            Set cryptographic key, if no key_data the module will look for the hidden key with
            the MasterKey class.
            Key_data is used for Crypto object's tests without using MasterKey class

        Initialize must be called before using cypher and uncipher

        Throw a CryptoError if both generate_key and key_data exists
        Throw a CryptoError if pass_phrase defined and security level != 1
        Throw a CryptoError if already initialized (protection against master key overwrite)
        """

############################################################################################################
# Private methods
    def get_keys():
        """ Return a tuple (key, key_data) of the master key (low level function used for debug and HA mode)

        Note that key is the unprotected version, ready to be used by another Crypto() instance
        while key_data is the protected version written onto the disk
        """

    def set_keys(key, key_data):
        """ Define the master key as is (low level function used for debug and HA mode)

        Note that key is the unprotected version, ready to be used by this Crypto() instance
        while key_data is the protected version written onto the disk

        If security_level == 0, then key == key_data
        If security_level == 1, then key != key_data
        """


############################################################################################################
# Public methods
    def change_passphrase(old_pass_phrase, new_pass_phrase):
        """ To change the pass phrase
        """

    def get_security_level():
        """ return the security level stored with the key
        or the security level which is set before first initialization """

    def is_key_generated():
        """ return True if crypto key was generated, False otherwise. """

    def cipher(input_data):
        """ Encrypt and return the data encrypted.
        input_data must be python 'str' type, ie: a buffer of data whose __len__ method return its real length

        Use cipher_u for unicode type

        Throw a CryptoError if master key isn't initialized
        """

    def uncipher(encrypted_data):
        """ Decrypt and return the data decrypted
        It returns a buffer of python 'str' type.

        Use uncipher_u for unicode type

        Throw a CryptoError if master key isn't initialized
        """

    def cipher_u(input_udata):
        """ Encrypt and return the data encrypted.
        input_udata must be python 'unicode' type, ie: encodable in utf-8

        Throw a CryptoError if master key isn't initialized
        """

    def uncipher_u(encrypted_udata):
        """ Decrypt and return the data decrypted
        It returns an unicode string

        Throw a CryptoError if master key isn't initialized
        """
