#pragma once

#include <sys/types.h>

struct sockaddr;

[[nodiscard]]
bool is_ipv4_mapped_ipv6(const char *ipv6_address) noexcept;

void get_ipv4_address(const char *ipv6_address,
                      char *dest_ip,
                      std::size_t dest_ip_size) noexcept;

const char *get_underlying_ip_port(const sockaddr& sa,
                                   socklen_t socklen,
                                   char *dest_ip,
                                   std::size_t dest_ip_size,
                                   char *dest_port,
                                   std::size_t dest_port_size) noexcept;
