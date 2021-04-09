#!/usr/bin/python3

import sys
import os
import timeit
import importlib
import inspect

sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
sys.path.append(os.path.join(os.path.dirname(__file__), "../sesmanworker"))

sys.modules["logger"] = importlib.import_module("empty_logger")

exec_nb = 10000


def _exec_fragment_code_bm(stmt):
    t = timeit.Timer(stmt, "from addrutils import is_device_in_subnet")
    res = t.timeit(exec_nb)

    return stmt, res

def _print_bm_func_results(bm_func):
    def inner():
        print("-------------------------------------------------"
              "-------------------------------------------------")
        print(f"benchmark test name : {bm_func.__name__}\n")


        stmt_res_list = bm_func()
        exec_average_time = 0

        for stmt, res in stmt_res_list:
            print(f"\"{stmt}\" duration time -> {res} seconds")
            exec_average_time += res

        print(f"\nexecution number by fragment code -> {exec_nb}")
        print(f"average of execution time -> {exec_average_time / len(stmt_res_list)} seconds")


        print("-------------------------------------------------"
              "-------------------------------------------------\n")

    return inner



@_print_bm_func_results
def _ipv4_matching_and_no_matching_bm():
    stmt_res_list = []

    # IPv4 matching
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('0.0.0.0', '0.0.0.0/0')"))
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('56.150.201.40', '56.150.128.99/17')"))
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('5.224.89.130', '5.224.32.89')"))

    # IPv4 no matching
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('10.10.10.10', '150.62.9.200/24')"))
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('129.5.9.200', '129.59.199')"))
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('192.168.1.15', '192.125.1.5/16')"))

    return stmt_res_list

@_print_bm_func_results
def _ipv6_matching_and_no_matching_bm():
    stmt_res_list = []

    # IPv6 matching
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('2001:0:1428:8f18:1480:1e5c:3f57:ffef', '2001:0000:3238:DFE1:63::FEFB/32')"))
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('2002:0620:0000:0000:0211:24FF:FE80:C12C', '2002:00A8:2102::1/20')"))
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('3FFE:0000:0000:0003:0200:F8FF:FE75:50DF', '3FFE:0000:0000:0003:5147:D1FA:3EBC:9FAB/50')"))

    # IPv6 no matching
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('2001:0000:3238:DFE1:63::FEFB', 'fe80::1ff:fe23:4567:890a/32')"))
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('::ffff:0:0', '2002:0620:0000:0000:0211:24FF:FE80:C12C/96')"))
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('2001:db8:a88:85a3::ac1f:8001', '2001:db8:a88:2390::ac1f:8001')"))

    return stmt_res_list

@_print_bm_func_results
def _ipv4_to_ipv6_matching_and_no_matching_bm():
    stmt_res_list = []

    # IPv4 to IPv6 matching
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('150.12.67.199', '2002::960c:43D2/40')"))
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('180.20.50.40', '2002::b414:3299/38')"))
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('180.20.50.40', '2002::b414:3299')"))

    # IPv4 to IPv6 no matching
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('150.12.67.199', 'fe80::4abc:6421:32FF/18')"))
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('180.20.50.40', '2001:db8:a88:85a3::ac1f:8001/75')"))
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('180.20.50.40', '2001:db8:a88:85a3::ac1f:8001')"))

    return stmt_res_list

@_print_bm_func_results
def _ipv6_to_ipv4_matching_and_no_matching_bm():
    stmt_res_list = []

    # IPv6 to IPv4 matching
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('2001:0:1428:8f18:1480:1e5c:3f57:ffef', '20.40.140.0/22')"))
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('2002:C0A8:2102::1', '192.168.32.9/21')"))
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('::ffff:130.180.0.2', '130.180.5.9/16')"))

    # IPv6 to IPv4 no matching
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('2001:0:1428:8f18:1480:1e5c:3f57:ffef', '134.90.170.60/20')"))
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('2001:0:1428:8f18:1480:1e5c:3f57:ffef', '134.90.199.80/30')"))
    stmt_res_list.append(
        _exec_fragment_code_bm(
            "is_device_in_subnet('::ffff:130.180.0.2', '130.180.99.200/19')"))

    return stmt_res_list



if __name__ == "__main__":
    _ipv4_matching_and_no_matching_bm()
    _ipv6_matching_and_no_matching_bm()
    _ipv4_to_ipv6_matching_and_no_matching_bm()
    _ipv6_to_ipv4_matching_and_no_matching_bm()
