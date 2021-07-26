#include <unordered_map>

#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>

#include "scard/scard_pcsc_common.hpp"
#include "scard/scard_pcsc_context.hpp"


///////////////////////////////////////////////////////////////////////////////


LONG scard_pcsc_context::get(SCARDHANDLE hCard, SCARDCONTEXT &hContext) const
{
    LONG result;

    // look up context handle associated to the provided card handle
    const auto it = _contexts.find(hCard);

    // output
    if (it == _contexts.end())
    {
        result = SCARD_E_INVALID_VALUE;
    }
    else
    {
        hContext = it->second;
        result = SCARD_S_SUCCESS;
    }
    
    return result;
}

void scard_pcsc_context::set(SCARDHANDLE hCard, SCARDCONTEXT hContext)
{
    _contexts.emplace(hCard, hContext);
}

void scard_pcsc_context::unset(SCARDHANDLE hCard)
{
    _contexts.erase(hCard);
}