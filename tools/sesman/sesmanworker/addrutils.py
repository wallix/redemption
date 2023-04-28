import ipaddress
import socket
from logger import Logger
from typing import Union, Optional, Tuple


IPInterface = Union[ipaddress.IPv4Interface, ipaddress.IPv6Interface]
IPAddress = Union[ipaddress.IPv4Address, ipaddress.IPv6Address]


def _get_adapted_device_ipaddr_from_subnet(device: str, subnet_ipaddr: IPAddress) -> Optional[IPAddress]:
    try:
        device_ipaddr = ipaddress.ip_address(device)

        if isinstance(subnet_ipaddr, ipaddress.IPv6Address):
            if isinstance(device_ipaddr, ipaddress.IPv4Address):
                if subnet_ipaddr.ipv4_mapped is not None:
                    # "IPv4-mapped IPv6" mechanism from ipv4
                    return ipaddress.IPv6Address("::ffff:" + device)
                else:
                    # "6to4" mechanism from ipv4
                    return ipaddress.IPv6Address("2002::" + device)

        elif isinstance(device_ipaddr, ipaddress.IPv6Address):
            # try "IPv4-mapped IPv6" mechanism or try "6to4" mechanism from IPv6
            adapted_device_ipaddr = device_ipaddr.ipv4_mapped or device_ipaddr.sixtofour
            if adapted_device_ipaddr is not None:
                return adapted_device_ipaddr

            # try "teredo" mechanism by getting server IP address
            # from couple (server, client)
            if device_ipaddr.teredo is not None:
                adapted_device_ipaddr = device_ipaddr.teredo[0]
                if adapted_device_ipaddr is not None:
                    return adapted_device_ipaddr

            Logger().debug(f"Cannot adapt '{device}' device to ipv4 format")
            return None

        return device_ipaddr

    except (ValueError, ipaddress.AddressValueError) as e:
        Logger().debug(f"Invalid IP address of device '{device}' : {e}")

    return None


def is_device_in_subnet(device: str, subnet: str) -> bool:
    try:
        subnet_ipface = ipaddress.ip_interface(subnet)
    except ValueError as e:
        Logger().debug(f"Invalid IP address of subnet '{subnet}' : {e}")
        return False

    adapted_device_ipaddr = _get_adapted_device_ipaddr_from_subnet(
        device, subnet_ipface.ip)

    if adapted_device_ipaddr is None:
        return False

    if '/' in subnet:
        try:
            adapted_device_ipface_with_nmask = ipaddress.ip_interface(
                f'{adapted_device_ipaddr}/{subnet_ipface.network.prefixlen}'
            )
        except ValueError as e:
            Logger().debug("Invalid IP address with "
                           "adapted IP version device "
                           f"and subnet netmask : {e}")
            return False
        result = (adapted_device_ipface_with_nmask.network
                  == subnet_ipface.network)
    else:
        result = adapted_device_ipaddr == subnet_ipface.ip

    Logger().debug(f"checking if device {device} is in subnet {subnet} -> {result}")
    return result


def is_ip_address(host: str) -> bool:
    try:
        ipaddress.ip_address(host)
    except ValueError:
        return False
    return True


def resolve_reverse_dns(ip: str) -> Optional[str]:
    found_fqdn = None
    try:
        found_fqdn = socket.gethostbyaddr(ip)[0]
        Logger().debug(f"Found fqdn {found_fqdn} for {ip}")
    except Exception:
        Logger().debug(f"Unable to reverse dns {ip}")
    return found_fqdn


def check_hostname_in_subnet(host: str, subnet: str) -> Tuple[bool, Optional[str]]:
    try:
        family = (socket.AF_INET6
                  if ipaddress.ip_network(subnet, strict=False).version == 6
                  else socket.AF_INET)
        host_ip = socket.getaddrinfo(host, None, family=family)[0][4][0]

        Logger().debug(f"Resolve DNS Hostname {host} -> {host_ip}")
    except Exception:
        return False, None
    return is_device_in_subnet(host_ip, subnet), host_ip
