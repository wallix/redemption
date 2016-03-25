#!/usr/bin/python -O
# -*- coding: utf-8 -*-
#
"""
Memory module (to load and store data in a shared memory segment)
"""

import ctypes
import os
import base64

from redcrypto.error import CryptoError

IPC_CREAT = 01000
IPC_RMID = 0

class Memory:
    """ The memory class """

    def __init__(self, key = 2242, size = 512):
        """ Constructor with shm key value """
        # Class configuration
        self.key = key
        self.size = size

        # Ctypes binding
        self.librt = ctypes.CDLL("librt.so.1", use_errno=True)
        self.librt.shmat.restype = ctypes.c_void_p

        # Shm states
        self.shmid = self.librt.shmget(self.key, self.size, 0600)
        self.shm = None

    def _is_ready(self):
        """ Raise an error if memory module isn't ready to be used """
        if self.shmid == -1:
            raise CryptoError("Memory isn't initialized!")

    def _check_shm_value(self):
        """ Raise an error if the shm address value isn't valid
        (to be used after every call to shmat) """
        if self.shm == None or self.shm == ctypes.c_void_p(-1).value:
            raise CryptoError("Could not get shared memory address: %s (%s)" % (
                os.strerror(ctypes.get_errno()),
                str(self.shm)
            ))

    def initialize(self):
        """ Create the shared memory """
        if self.shmid != -1:
            raise CryptoError("Memory is already initialized!")
        self.shmid = self.librt.shmget(self.key, self.size, 0600 | IPC_CREAT)
        if self.shmid == -1:
            raise CryptoError("Could not initialize memory!")

    def detach(self):
        """ Detach crypto memory """
        self._is_ready()
        if not self.shm:
            self.shm = self.librt.shmat(self.shmid, 0, 0)
        if self.librt.shmdt(ctypes.c_void_p(self.shm)):
            raise CryptoError("Could not detatch from shared memory: %s" % os.strerror(ctypes.get_errno()))
        self.shm = None

    def destroy(self):
        """ Destroy the shared memory """
        self._is_ready()
        if not self.shm:
            self.shm = self.librt.shmat(self.shmid, 0, 0)
        self._check_shm_value()
        ctypes.memmove(self.shm, '\x00' * self.size, self.size)
        if self.librt.shmdt(ctypes.c_void_p(self.shm)):
            raise CryptoError("Could not detatch from shared memory: %s" % os.strerror(ctypes.get_errno()))

        if self.librt.shmctl(self.shmid, IPC_RMID, 0):
            raise CryptoError("Could not delete shm: %s" % os.strerror(ctypes.get_errno()))

        self.shm = None
        self.shmid = -1

    def load(self):
        """ Read data from the shared memory and return a pointer """
        self._is_ready()
        if not self.shm:
            self.shm = self.librt.shmat(self.shmid, 0, 0)
        self._check_shm_value()
        return base64.decodestring(ctypes.c_char_p(self.shm).value)

    def store(self, data):
        """ Store data to the shared memory """
        self._is_ready()
        if not self.shm:
            self.shm = self.librt.shmat(self.shmid, 0, 0)
        self._check_shm_value()
        ctypes.memmove(self.shm, base64.encodestring(data), self.size)

def clear_memory():
    """ Convenient procedure to clear the memory storage """
    try:
        Memory().destroy()
    except CryptoError:
        pass

def get_memory():
    """ Admin procedure to get memory content (used in HA) """
    import base64
    try:
        return base64.encodestring(Memory().load()).replace('\n', '')
    except CryptoError:
        pass

def set_memory(buffer_b64):
    """ Admin procedure to set memory content (used in HA) """
    import base64
    m = Memory()
    try:
        m.initialize()
    except CryptoError:
        pass
    try:
        m.store(base64.decodestring(buffer_b64))
        return 0
    except CryptoError:
        return 1
