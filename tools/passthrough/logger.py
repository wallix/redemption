#!/usr/bin/python

import logging

class Logger():

    def info(self, s):
        print("Info: %s" % s)

    def error(self, s):
        print("Error: %s" % s)

    def warning(self, s):
        print("Warning: %s" % s)

    def debug(self, s):
        print("Debug: %s" % s)

    def exception(self, s):
        print("Exception: %s" % s)

    def trace(self, s):
        print("Trace: %s" % s)
