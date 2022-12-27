#!/usr/bin/python
# -*- coding: utf-8 -*-

import ipaddress
import socket
from logger import Logger


def _get_adapted_device_ipaddr_from_subnet(device, subnet_ipaddr):
    adapted_device_ipaddr = None

    try:
        device_ipaddr = ipaddress.ip_address(device)
        device_ipversion = device_ipaddr.version

        if device_ipversion != subnet_ipaddr.version:
            if device_ipversion == 4:
                if subnet_ipaddr.ipv4_mapped is not None:
                    # "IPv4-mapped IPv6" mechanism from ipv4
                    adapted_device_ipaddr = ipaddress.IPv6Address(
                        "::ffff:" + device)
                else:
                    # "6to4" mechanism from ipv4
                    adapted_device_ipaddr = ipaddress.IPv6Address(
                        "2002::" + device)
            else:
                # try "IPv4-mapped IPv6" mechanism
                adapted_device_ipaddr = device_ipaddr.ipv4_mapped
                if adapted_device_ipaddr is not None:
                    return adapted_device_ipaddr

                # try "6to4" mechanism from IPv6
                adapted_device_ipaddr = device_ipaddr.sixtofour
                if adapted_device_ipaddr is not None:
                    return adapted_device_ipaddr

                # try "teredo" mechanism by getting server IP address
                # from couple (server, client)
                if device_ipaddr.teredo is not None:
                    adapted_device_ipaddr = device_ipaddr.teredo[0]

                    if adapted_device_ipaddr is not None:
                        return adapted_device_ipaddr
                Logger().debug("Cannot adapt '%s' device to ipv4 format"
                               % device)
        else:
            adapted_device_ipaddr = device_ipaddr

    except (ValueError, ipaddress.AddressValueError) as e:
        Logger().debug("Invalid IP address of device '%s' : %s"
                       % (device, str(e)))

    return adapted_device_ipaddr


def is_device_in_subnet(device, subnet):
    if subnet is None:
        Logger().debug("No value for subnet")
        return False

    try:
        subnet_ipface = ipaddress.ip_interface(subnet)
    except ValueError as e:
        Logger().debug("Invalid IP address of subnet '%s' : %s"
                       % (subnet, str(e)))

        return False

    adapted_device_ipaddr = _get_adapted_device_ipaddr_from_subnet(
        device, subnet_ipface.ip)

    if adapted_device_ipaddr is None:
        return False

    if '/' in subnet:
        try:
            adapted_device_ipface_with_nmask = ipaddress.ip_interface(
                str(adapted_device_ipaddr)
                + '/'
                + str(subnet_ipface.network.prefixlen))
        except ValueError as e:
            Logger().debug("Invalid IP address with "
                           "adapted IP version device "
                           "and subnet netmask : %s"
                           % str(e))
            return False
        result = (adapted_device_ipface_with_nmask.network
                  == subnet_ipface.network)
    else:
        result = adapted_device_ipaddr == subnet_ipface.ip

    Logger().debug("checking if device %s is in subnet %s -> %s" %
                   (device, subnet, ['No', 'Yes'][result]))
    return result


def is_ip_address(host):
    try:
        ipaddress.ip_address(host)
    except ValueError:
        return False
    return True


def resolve_reverse_dns(ip_str):
    found_fqdn = None
    try:
        found_fqdn = socket.gethostbyaddr(ip_str)[0]
        Logger().debug("Found fqdn %s for %s" % (found_fqdn, ip_str))
    except Exception:
        Logger().debug("Unable to reverse dns %s" % ip_str)
    return found_fqdn


def check_hostname_in_subnet(host, subnet):
    try:
        family = (socket.AF_INET6
                  if ipaddress.ip_network(subnet, strict=False).version == 6
                  else socket.AF_INET)
        host_ip = socket.getaddrinfo(host, None, family=family)[0][4][0]

        Logger().debug("Resolve DNS Hostname %s -> %s" % (host,
                                                          host_ip))
    except Exception:
        return False, None
    return is_device_in_subnet(host_ip, subnet), host_ip
