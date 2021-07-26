#include <cstdlib>

#include "scard/scard_pcsc_environment.hpp"

#define DEFAULT_SOCKET_PATH ""


///////////////////////////////////////////////////////////////////////////////

const char * const scard_pcsc_environment::variable_name = "RDPCSC_SOCKET";

void scard_pcsc_environment::set_socket_path(const char *path)
{
    if (!path)
    {
        path = DEFAULT_SOCKET_PATH;
    }

    ::setenv(variable_name, path, 1);
}

const char * scard_pcsc_environment::get_socket_path()
{
    const char *path = ::getenv(variable_name);
    
    if (!path)
    {
        path = DEFAULT_SOCKET_PATH;
    }

    return path;
}