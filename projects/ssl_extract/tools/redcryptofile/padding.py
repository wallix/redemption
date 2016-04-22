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
Simple padding utility to convert variable length data into fixed length (to ease crypto operation)
"""

class Padding:
    """ The padding class """

    def __init__(self, block_size = 32):
        """ Set the modulus size of returned data """
        self.size = block_size

    def pad(self, data, fill = ' '):
        """ Pad the data with fill byte to match block_size
        >>> Padding(16).pad("0123456789")
        "0123456789      "
        """
        num_block = len(data) / self.size
        if not len(data) or num_block * self.size < len(data):
            num_block += 1
        return format(data, "%s<%ds" % (fill, num_block * self.size))

    def unpad(self, data, fill = ' '):
        """ Unpad the data (remove trailling fill bytes)
        >>> Padding().unpad("mytest    ")
        "mytest"
        >>> Padding().unpad("mytest0000", fill = '0')
        "mytest"
        """
        return data.rstrip(fill)
