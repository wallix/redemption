#!/usr/bin/python -O
#
# Copyright (c) 2014 WALLIX, SAS. All rights reserved.
#
# Licensed computer software. Property of WALLIX.
# Author(s) : Christophe Grosjean <cgrosjean@wallix.com>
#

import re
from logger import Logger
from time import sleep

class SSHException(Exception):
    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return "SSH error: "+self.msg


try:
    from sshng.pysshct import (SSH_ERROR, SSH_OK, SSH_AGAIN)
except Exception, e:
    SSH_OK = 0
    SSH_ERROR = -1
    SSH_AGAIN = -2

def mdecode(item):
    if not item:
        return ""
    try:
        item = item.decode('utf8')
    except:
        item = item.decode('latin1')
    return item


def parse_ssh_auth(username):
    """
    Extract actual username and target if provided
    from SSH authentication identity

    string format is <secondaryuser>@<target>:<service>:<primaryuser>
    always return primaryuser and either secondary target or None

    Note: primary user can be a path instead when this function
    is called to parse scp or sftp arguments.

    Because of compatibility issues with some ssh command line tools
    '#' can be used instead of '@' and '+' instead of ':'

    #TODO: escape sequences by doubling characters,
    fields can be missing (typically service if there is no ambiguity)
    # but never be empty

    """
    l1 = re.split('[:+]', username)
    if len(l1) in [2, 3]:
        if len(l1) == 3:
            user_at_dev, service, primary = l1
        else:
            service = None
            user_at_dev, primary = l1
        l2 = re.split('[@#]', user_at_dev)
        if len(l2) == 2:
            user, dev = l2
        # for new console syntax in ssh user
        # elif (service is None) and (len(l2) == 1):
        #     if l2[0].lower() in [ 'console', 'wabconsole' ]:
        #         return primary, 'console'
            return primary, (user, dev, service)

    return username, None


def dotries(func, *args):
    res = SSH_ERROR
    # try func until too many tries (double waiting time at each try)
    # first waiting time is 1 ms and 11th will be 1 sec
    wait_again = 0.001
    for i in range(11):
        res = func(*args)
        # Logger().info("%s done res = %s" % (func.__name__, res))
        if res != SSH_AGAIN:
            break
        sleep(wait_again)
        wait_again = wait_again * 2
    return res

def _binary_ip(network, bits):
    # TODO need Ipv6 support
    # This is a bit too resilient, add check for obviously bad values
    a,b,c,d = [ int(x)&0xFF for x in network.split('.')]
    mask = (0xFFFFFFFF >> bits) ^ 0xFFFFFFFF
    return ((a << 24) + (b << 16) + (c << 8) + d) & mask

def is_device_in_subnet(device, subnet):

    if '/' in subnet:
        try:
            network, bits = subnet.rsplit('/')
            network_bits = _binary_ip(network, int(bits))
            device_bits = _binary_ip(device, int(bits))
            result = network_bits == device_bits
        except Exception, e:
            Logger().error("Bad host definition device '%s' subnet '%s': %s" % (device, subnet, str(e)))
            result = False
    else:
        result = device == subnet
    Logger().info("checking if device %s is in subnet %s -> %s" % (device, subnet, ['No', 'Yes'][result]))
    return result
