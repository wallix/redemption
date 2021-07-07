import ipaddress
from logger import Logger

def _get_adapted_device_iaddr_from_subnet(device, subnet_ivers):
    adapted_device_iaddr = None

    try:
        device_iaddr = ipaddress.ip_address(device)
        device_ivers = device_iaddr.version

        if device_ivers != subnet_ivers:
            if device_ivers == 4:
                # 6to4 mechanism from ipv4
                adapted_device_iaddr = ipaddress.IPv6Address(
                    "2002::" + device)
            else:
                # try "IPv4-mapped IPv6" mechanism
                adapted_device_iaddr = device_iaddr.ipv4_mapped
                if adapted_device_iaddr is not None:
                    return adapted_device_iaddr

                # try "6to4" mechanism from ipv6
                adapted_device_iaddr = device_iaddr.sixtofour
                if adapted_device_iaddr is not None:
                    return adapted_device_iaddr

                # try "teredo" mechanism by getting server IP address
                # from couple (server, client)
                if device_iaddr.teredo is not None:
                    adapted_device_iaddr = device_iaddr.teredo[0]

                    if adapted_device_iaddr is not None:
                        return adapted_device_iaddr

                Logger().error("Cannot adapt '%s' device to ipv4 format"
                               % device)

        else:
            adapted_device_iaddr = device_iaddr

    except (ValueError, ipaddress.AddressValueError) as e:
        Logger().error("Invalid IP address of device '%s' : %s"
                       % (device, str(e)))

    return adapted_device_iaddr

def is_device_in_subnet(device, subnet):
    if subnet is None:
        Logger().error("No value for subnet")
        return False

    try:
        subnet_iface = ipaddress.ip_interface(subnet)
    except ValueError as e:
        Logger().error("Invalid IP address of subnet '%s' : %s"
                       % (subnet, str(e)))

        return False

    adapted_device_iaddr = _get_adapted_device_iaddr_from_subnet(
        device, subnet_iface.ip.version)

    if adapted_device_iaddr is None:
        return False

    if '/' in subnet:
        try:
            adapted_device_iface_with_nmask = ipaddress.ip_interface(
                str(adapted_device_iaddr)
                + '/'
                + str(subnet_iface.network.prefixlen))
        except ValueError as e:
            Logger().error("Invalid IP address with "
                           "adapted IP version device "
                           "and subnet netmask : %s"
                           % str(e))
            return False

        result = (adapted_device_iface_with_nmask.network
                  == subnet_iface.network)
    else:
        result = adapted_device_iaddr == subnet_iface.ip

    Logger().debug("checking if device %s is in subnet %s -> %s" %
                   (device, subnet, ['No', 'Yes'][result]))

    return result
