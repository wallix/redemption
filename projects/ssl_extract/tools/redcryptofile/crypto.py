#!/usr/bin/python -O
# -*- coding: utf-8 -*-
#
""" The crypto module """

import os
import time
import base64
import struct
import random
import hashlib
import hmac
import pwd
import traceback
from pbkdf2 import PBKDF2
#from logger import Logger

from Crypto import Random
from Crypto.Cipher import AES
import zope.interface as zi

from redcrypto.interface import ICrypto
from redcrypto.disk import Disk
from redcrypto.padding import Padding
from redcrypto.error import CryptoError, WrongPassPhrase
from redcrypto.memory import Memory

IV_SIZE = 16
OLDCTRL_SIZE = 16 # size of md5 hash for old control scheme
OLDDATA_SIZE = 90 # len(base64.encodestring( 16*iv + ( 16*md5 + 32*key) ))
MKCTRL_SIZE = 32 # size of SHA256 and HMAC-SHA256 hash
MKSALT_SIZE = 8 # size of 64 bits salt
MK_OFFSET = MKCTRL_SIZE+MKSALT_SIZE # key offset in memory
MK_SIZE = 32 # size of 256 bits master key

CRYPT_DERIVATOR = '\xbb\x67\x0c\xff\x61\x95\x78\x92';
HASH_DERIVATOR = '\x95\x8b\xcb\xd4\xee\xa9\x89\x5b';


def touch(fname, times = None):
    """ Touch utility """
    with file(fname, 'a'):
        os.utime(fname, times)

def get_random_bytes(size):
    try:
        return Random.get_random_bytes(size)
    except AssertionError, ae:
        if len(ae.args) == 1 and ae.args[0] == 'PID check failed. RNG must be re-initialized after fork(). Hint: Try Random.atfork()':
            Random.atfork()
            return Random.get_random_bytes(size)

class Crypto:
    """ The Crypto class """
    zi.implements(ICrypto)

    def _encrypt(self, key, data):
        """ Encrypt data with key """
        # Generate Initialization Vectore
        initialization_vector = get_random_bytes(IV_SIZE)

        # Pad and encrypt data
        encrypted_data = AES.new(key, AES.MODE_CBC, initialization_vector).encrypt(self.pad(data))

        # Insert initialization_vector in encoded packet
        encrypted_data = initialization_vector + encrypted_data

        # Encode in base64 the whole payload
        return base64.b64encode(encrypted_data)

    def _decrypt(self, key, data):
        """ Decrypt data with key """
        # Decode base64
        data = base64.b64decode(data)

        # Extract initialization_vector
        initialization_vector = data[:IV_SIZE]
        data = data[IV_SIZE:]

        # Decrypt payload
        return AES.new(key, AES.MODE_CBC, initialization_vector).decrypt(data)

    def _is_ready(self):
        """ Raise an error if crypto module isn't ready to be used """
        if not self.k:
            raise CryptoError("Crypto is not initialized...")
        return True

    def _check_passphrase_strength(self, pass_phrase):
        if len(pass_phrase) < 4:
            raise CryptoError("Bad pass phrase, it is too short!")
        if len(pass_phrase) > 32:
            raise CryptoError("Bad pass phrase, it is too long!")
        return Padding(32).pad(pass_phrase)

############################################################################################################
# Initialization methods
    def __init__(self, security_level = 0, key_base_path = "/var/wab/etc", use_disk = True, use_memory = True):
        # UID check
        if os.geteuid() != pwd.getpwnam('wabuser').pw_uid:
#            Logger().error("Unauthorized access to crypto from process %s" % (
#                open("/proc/self/cmdline").read().replace('\x00', ' ').strip()
#            ))
            raise CryptoError("Access Unauthorized")

        self.security_level = security_level
        self.pad = Padding(16).pad
        self.key = None # controlsum+salt+masterkey as a chunk of bytes
        self.key_data = None # encrypted form of the above chunk
        self.k = None # master key (don't use directly but through derivation)
        self.hk = None # global hash key
        self.ck = None # global cipher key (for database, don't use for traces)
        self.disk = None if not use_disk else Disk(key_base_path)
        self.memory = None if not use_memory else Memory()

        # Automatic initialization trial
        if self.memory:
            # Try to load crypto data from shared memory
            try:
                data = self.memory.load()
                if len(data) > 0:
                    self.security_level = int(data[0])
                    mem = data[1:]
                    if len(mem) == OLDCTRL_SIZE+MK_SIZE+OLDDATA_SIZE:
                        # old 3.1.x passphrase scheme data = base64(iv + aes(pwd, md5sum(mk) + mk, iv))
                        md5_key, key_data = mem[0:OLDCTRL_SIZE+MK_SIZE], mem[OLDCTRL_SIZE+MK_SIZE:]
                        # Create the key component (key_data decrypted)
                        ctrl = md5_key[:OLDCTRL_SIZE]
                        if ctrl == hashlib.md5(md5_key[OLDCTRL_SIZE:]).digest():
                            # convert to new scheme
                            salt = get_random_bytes(MKSALT_SIZE) # 64-bit salt
                            key = hashlib.sha256(salt + md5_key[OLDCTRL_SIZE:]).digest() + salt + md5_key[OLDCTRL_SIZE:] # in-memory we need to check without requiring the passphrase_key
                            if security_level == 0:
                                passphrase_key = PBKDF2(" " * 32, salt).read(32) # 256-bit derived key (temporary and in memory only)
                                hmac_key = hmac.new(passphrase_key, salt + md5_key[OLDCTRL_SIZE:], hashlib.sha256).digest() + md5_key[OLDCTRL_SIZE:]
                                key_data = salt + self._encrypt(passphrase_key, hmac_key)
                            #TODO: should we do something with key_data if we are in security level 1 ??
                            # Store the key_data component
                            self._set_keys(key, key_data)
                            self.memory.store(str(self.security_level) + self.key + self.key_data)
                        else:
                            raise WrongPassPhrase
                    else:
                        key, key_data = mem[0:MK_OFFSET+MK_SIZE], mem[MK_OFFSET+MK_SIZE:]
                        self._set_keys(key, key_data)
            except CryptoError, exc:
                #Logger().debug("Exception caught %s %s\n%s" % (type(exc), str(exc), traceback.format_exc()))
                # memory throw an exception if not ready when trying to read the shm
                self.memory.initialize()

#        if self.disk and security_level == 0:
#            # Try to load crypto data from disk
#            try:
#                self.initialize()
#            except CryptoError:
#                pass

    def initialize(self, pass_phrase = "", generate_key = False, key_data = None):
        """ Generate and store a master Key, this must be done only once, at first boot,
        optional pass_phrase to encrypt the master key just after generation.

        Throw a CryptoError if pass_phrase defined and security level != 1
        Throw a CryptoError if already initialize (protection against master key overwrite)
        """
        if self.k:
            raise CryptoError("Crypto is already initialized!")

        if generate_key and key_data:
            raise CryptoError("key_data shall not be defined with generate_key")

        if self.disk and key_data:
            raise CryptoError("key_data isn't necessary because crypto already use disk")
        if not self.disk and not key_data:
            raise CryptoError("key_data is required when no disk storage is used")

        if generate_key:

            if self.security_level == 1:
                if pass_phrase == "":
                    raise CryptoError("Pass phrase is required to protect MasterKey in security_level 1")
                pass_phrase = self._check_passphrase_strength(pass_phrase)

            if self.security_level == 0:
                if pass_phrase != "":
                    raise CryptoError("Pass phrase is not required to protect MasterKey in security_level 0")
                pass_phrase = " " * 32


            # Generate the K
            self.k = get_random_bytes(MK_SIZE)
            self.hk = hashlib.sha256(HASH_DERIVATOR + self.k).digest()
            self.ck = hashlib.sha256(CRYPT_DERIVATOR + self.k).digest()
            # Create the key component (the K prefixed with a control sum and salt)
            salt = get_random_bytes(MKSALT_SIZE) # 64-bit salt
            self.key = hashlib.sha256(salt + self.k).digest() + salt + self.k  # in-memory we need to check without requiring the passphrase_key
            # Create the key_data component (the encrypted key element, prefixed by the salt)
            passphrase_key = PBKDF2(pass_phrase, salt).read(32) # 256-bit derived key
            hmac_key = hmac.new(passphrase_key, salt + self.k, hashlib.sha256).digest() + self.k
            self.key_data = salt + self._encrypt(passphrase_key, hmac_key)

            # Store the key_data component
            if self.disk:
                self.disk.initialize()
                self.disk.store("%s%s" % (self.security_level, self.key_data))

        else:
            if self.disk:
                # Load the key_data component (out of Disk class)
                data = self.disk.load()
                self.security_level = int(data[0])
                self.key_data = data[1:]
            else:
                # For debug purpose, to allow crypto test without disk storage involved
                self.key_data = key_data

            if self.security_level == 1:
                if pass_phrase == "":
                    raise CryptoError("Pass phrase is required to protect MasterKey in security_level 1")
                pass_phrase = self._check_passphrase_strength(pass_phrase)

            if self.security_level == 0:
                if pass_phrase != "":
                    raise CryptoError("Pass phrase is not required to protect MasterKey in security_level 0")
                pass_phrase = " " * 32

            # Validate the key component
            if len(self.key_data) == OLDDATA_SIZE:
                # old 3.1.x passphrase scheme data = base64(iv + aes(pwd, md5sum(mk) + mk, iv))
                # Create the key component (key_data decrypted)
                md5_key = self._decrypt(pass_phrase, self.key_data)
                ctrl = md5_key[:OLDCTRL_SIZE]
                if ctrl == hashlib.md5(md5_key[OLDCTRL_SIZE:]).digest():
                    # convert to new scheme
                    #self.security_level = 1
                    salt = get_random_bytes(MKSALT_SIZE) # 64-bit salt
                    self.key = hashlib.sha256(salt + md5_key[OLDCTRL_SIZE:]).digest() + salt + md5_key[OLDCTRL_SIZE:] # in-memory we need to check without requiring the passphrase_key
                    passphrase_key = PBKDF2(pass_phrase, salt).read(32) # 256-bit derived key
                    hmac_key = hmac.new(passphrase_key, salt + md5_key[OLDCTRL_SIZE:], hashlib.sha256).digest() + md5_key[OLDCTRL_SIZE:]
                    self.key_data = salt + self._encrypt(passphrase_key, hmac_key)
                    # Store the key_data component
                    if self.disk:
                        self.disk.store("%s%s" % (self.security_level, self.key_data))
                else:
                    raise WrongPassPhrase
            elif len(self.key_data) == 130:
                # key format used for cspn dev between commits 14457 and 14552 (not used in production)
                # Create the key component (key_data decrypted)
                sha256_key = self._decrypt(pass_phrase, self.key_data)
                ctrl = sha256_key[:MKCTRL_SIZE]
                salt = sha256_key[MKCTRL_SIZE:MK_OFFSET]
                if ctrl == hashlib.sha256(salt + sha256_key[MK_OFFSET:MK_OFFSET+MK_SIZE]).digest():
                    # convert to new scheme
                    #self.security_level = 1
                    self.key = sha256_key
                    passphrase_key = PBKDF2(pass_phrase, salt).read(32) # 256-bit derived key
                    hmac_key = hmac.new(passphrase_key, salt + sha256_key[MK_OFFSET:MK_OFFSET+MK_SIZE], hashlib.sha256).digest() + sha256_key[MK_OFFSET:MK_OFFSET+MK_SIZE]
                    self.key_data = salt + self._encrypt(passphrase_key, hmac_key)
                    # Store the key_data component
                    if self.disk:
                        self.disk.store("%s%s" % (self.security_level, self.key_data))
                else:
                    raise WrongPassPhrase
            else:
                # new 3.1.9 passphrase scheme data = [aes(kp, hmac-sha256(kp, salt + mk) + mk) + salt] where kp=pbkdf2(pwd,salt)
                # Create the key component (key_data decrypted)
                salt = self.key_data[:MKSALT_SIZE] # 64-bit salt
                passphrase_key = PBKDF2(pass_phrase, salt).read(32) # 256-bit derived key
                hmac_key = self._decrypt(passphrase_key, self.key_data[MKSALT_SIZE:])
                ctrl = hmac_key[:MKCTRL_SIZE]
                if ctrl != hmac.new(passphrase_key, salt + hmac_key[MKCTRL_SIZE:MKCTRL_SIZE+MK_SIZE], hashlib.sha256).digest():
                    raise WrongPassPhrase
                self.key = hashlib.sha256(salt + hmac_key[MKCTRL_SIZE:MKCTRL_SIZE+MK_SIZE]).digest() + salt + hmac_key[MKCTRL_SIZE:MKCTRL_SIZE+MK_SIZE] # in-memory we need to check without requiring the passphrase_key

            # Hold the K
            self.k = self.key[MK_OFFSET:MK_OFFSET+MK_SIZE]
            self.hk = hashlib.sha256(HASH_DERIVATOR + self.k).digest()
            self.ck = hashlib.sha256(CRYPT_DERIVATOR + self.k).digest()

        if self.memory:
            self.memory.store("%s%s" % (self.security_level, self.key + self.key_data))
        if os.path.exists("/etc/opt/wab/ha/config.pkl"):
            touch("/tmp/crypto_update")

    def change_passphrase(self, old_pass_phrase, new_pass_phrase):
        self._is_ready()

        if self.security_level != 1:
            raise CryptoError("Pass phrase is not required to protect MasterKey in security_level 0")

        new_pass_phrase = self._check_passphrase_strength(new_pass_phrase)
        old_pass_phrase = self._check_passphrase_strength(old_pass_phrase)

        #TODO: since __init__ doesn't convert if we are in security level 1, we should probably convert key_data here
        salt = self.key_data[:MKSALT_SIZE] # 64-bit salt
        passphrase_key = PBKDF2(old_pass_phrase, salt).read(32) # 256-bit derived key
        hmac_key = self._decrypt(passphrase_key, self.key_data[MKSALT_SIZE:])
        ctrl = hmac_key[:MKCTRL_SIZE]
        if ctrl != hmac.new(passphrase_key, salt + hmac_key[MKCTRL_SIZE:MKCTRL_SIZE+MK_SIZE], hashlib.sha256).digest():
            raise WrongPassPhrase

        salt = get_random_bytes(MKSALT_SIZE) # 64-bit salt
        self.key = hashlib.sha256(salt + self.k).digest() + salt + self.k  # in-memory we need to check without requiring the passphrase_key
        passphrase_key = PBKDF2(new_pass_phrase, salt).read(32) # 256-bit derived key
        hmac_key = hmac.new(passphrase_key, salt + self.k, hashlib.sha256).digest() + self.k
        self.key_data = salt + self._encrypt(passphrase_key, hmac_key)
        
        if self.disk:
            self.disk.store("%s%s" % (self.security_level, self.key_data))
        if self.memory:
            self.memory.store("%s%s" % (self.security_level, self.key + self.key_data))
        if os.path.exists("/etc/opt/wab/ha/config.pkl"):
            touch("/tmp/crypto_update")

############################################################################################################
# Private methods
    def get_keys(self):
        """ Return a tuple (key, key_data) of the master key (low level function used for debug and HA mode)

        Note that key is the unprotected version, ready to be used by another Crypto() instance
        while key_data is the protected version written onto the disk
        """
        self._is_ready()
        return str(self.security_level), self.key, self.key_data

    def set_keys(self, security_level, key, key_data):
        """ Define the master key as is (low level function used for debug and HA mode)

        Note that key is the unprotected version, ready to be used by this Crypto() instance
        while key_data is the protected version written onto the disk

        If security_level == 0, then key == key_data
        If security_level == 1, then key != key_data
        """
        self._set_keys(key, key_data)
        if self.memory:
            self.memory.store(security_level + self.key + self.key_data)

    def _set_keys(self, key, key_data):
        """ Simple method to check key component """
        ctrl = key[:MKCTRL_SIZE]
        salt = key[MKCTRL_SIZE:MK_OFFSET]
        if ctrl != hashlib.sha256(salt + key[MK_OFFSET:MK_OFFSET+MK_SIZE]).digest():
            raise WrongPassPhrase

        self.key = key
        self.key_data = key_data
        self.k = key[MK_OFFSET:MK_OFFSET+MK_SIZE]
        self.hk = hashlib.sha256(HASH_DERIVATOR + self.k).digest()
        self.ck = hashlib.sha256(CRYPT_DERIVATOR + self.k).digest()


############################################################################################################
# Public methods
    def is_ready(self):
        """ Return True if redcrypto is ready to be used"""
        try:
            return self._is_ready()
        except CryptoError:
            return False

    def get_security_level(self):
        sec = None
        if self.memory:
            try:
                data = self.memory.load()
                sec = int(data[0])
            except CryptoError:
                pass

        if self.disk:
            try:
                data = self.disk.load()
                sec = int(data[0])
            except CryptoError:
                pass

        return sec if sec else self.security_level

    def is_key_generated(self):
        mkey, dkey = True, True
        if self.memory:
            try:
                data = self.memory.load()
                if not data:
                    mkey = False
            except CryptoError:
                mkey = False

        if self.disk:
            try:
                data = self.disk.load()
            except CryptoError:
                dkey = False

        return (self.memory and mkey) or (self.disk and dkey)

    def destroy(self, confirmation = False):
        """ Completely destroy current crypto """
        if confirmation:
            if self.memory:
                try:
                    self.memory.destroy()
                except CryptoError:
                    pass
            if self.disk:
                self.disk.destroy()
        else:
            raise RuntimeError("Please confirm")

    def detach(self):
        if self.memory:
            self.memory.detach()

    def cipher_u(self, input_udata):
        """ Encrypt and return the data encrypted."""
        return self.cipher(input_udata.encode('utf-8'))

    def uncipher_u(self, encrypted_udata):
        """ Decrypt and return the data decrypted."""
        return self.uncipher(encrypted_udata).decode('utf-8')

    def cipher(self, input_data, key=None):
        """ Encrypt and return the data encrypted."""
        if key is None:
            key = self.ck

        # Check if crypto is ready
        self._is_ready()

        # Check data len
        if input_data == None or len(input_data) > 4294967295:
            raise CryptoError("Could not cipher: invalid data size (len(data) not in 0 < %d <= 4294967295)" % 
                0 if input_data == None else len(input_data)
            )

        # Insert a size header
        data = struct.pack('I', len(input_data)) + input_data

        # Encrypt the payload
        return self._encrypt(key, data)

    def uncipher(self, encrypted_data, key=None):
        """ Decrypt and return the data decrypted."""
        if key is None:
            key = self.ck

        # Check if crypto is ready
        self._is_ready()

        # Check input data
        try:
            if encrypted_data == None or len(encrypted_data) < 20:
                raise CryptoError("Invalid encrypted data")

            # Decrypt input buffer
            data = self._decrypt(key, encrypted_data)

            # Check the size header
            try:
                data_size = struct.unpack('I', data[:4])[0]
                if data_size > len(data) - 4:
                    raise ValueError("Size header is invalid (%d > %d)" % (data_size, len(data)-4))
            except Exception, exc:
                raise CryptoError("Invalid decrypted chunk, excepted a size header, got %s" % repr(exc))
        except:
            if key != self.k:
                # try uncipher with the old crypto scheme using the master key
                try:
                    return self.uncipher(encrypted_data, key=self.k)
                except:
                    pass
            raise

        # Extract the data
        return data[4:4+data_size]

    def hmac(self, key=None):
        if key is None:
            key = self.hk
        md = hmac.new(key, digestmod=hashlib.sha256)
        return md
