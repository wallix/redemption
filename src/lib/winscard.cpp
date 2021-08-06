#include <cassert>
#include <cinttypes>
#include <vector>

#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>

#include "cxx/cxx.hpp"
#include "scard/scard_pcsc_client.hpp"
#include "scard/scard_pcsc_context.hpp"
#include "scard/scard_pcsc_environment.hpp"
#include "scard/scard_pack.hpp"
#include "scard/scard_utils.hpp"
#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/bytes_view.hpp"


///////////////////////////////////////////////////////////////////////////////


static scard_pcsc_context   _context;
static scard_pcsc_client    _client(scard_pcsc_environment::get_socket_path(), 0);
static const bool           _verbose = true;

static void transmit(const scard_pack_call &call, scard_pack_return &return_) noexcept
{
    {
        if (_verbose)
        {
            call.log(LOG_INFO);
        }

        std::vector<uint8_t> buffer;

        // allocate memory for packing
        buffer.reserve(call.packed_size());
        
        OutStream stream(writable_buffer_view(buffer.data(),
            buffer.capacity()));

        // pack
        const auto n_packed = call.pack(stream);

        SCARD_CHECK_PREDICATE(n_packed == buffer.capacity(),
            ERR_SCARD);

        // send
        const auto n_sent = _client.send(call.io_control_code(),
            bytes_view(buffer.data(), n_packed));

        SCARD_CHECK_PREDICATE(n_sent == n_packed,
            ERR_SCARD);
    }
    {
        // receive
        const auto buffer = _client.receive();

        InStream stream(buffer);
        
        // unpack
        const auto n_unpacked = return_.unpack(stream);

        SCARD_CHECK_PREDICATE(n_unpacked == buffer.size(),
            ERR_SCARD);

        if (_verbose)
        {
            return_.log(LOG_INFO);
        }
    }
}

#ifdef __cplusplus
extern "C"
{
#endif  // __cplusplus

REDEMPTION_LIB_EXPORT
LONG SCardEstablishContext(
    DWORD dwScope,
    /*@null@*/ LPCVOID /*pvReserved1*/,
    /*@null@*/ LPCVOID /*pvReserved2*/,
    /*@out@*/ LPSCARDCONTEXT phContext)
{
    SCARD_CHECK_PREDICATE(phContext, ERR_SCARD);

    LONG result;

    scard_pack_call_establish_context call(dwScope);
    scard_pack_return_establish_context return_;

    // send call and receive return
    transmit(call, return_);

    // output
    result = return_.ReturnCode();
    if (result == SCARD_S_SUCCESS)
    {
        *phContext = return_.hContext();
    }

    return result;
}

REDEMPTION_LIB_EXPORT
LONG SCardReleaseContext(
    SCARDCONTEXT hContext)
{
    LONG result;

    scard_pack_call_release_context call(hContext);
    scard_pack_return_release_context return_;

    // send call and receive return
    transmit(call, return_);

    // output
    result = return_.ReturnCode();

    return result;
}

REDEMPTION_LIB_EXPORT
LONG SCardIsValidContext(
    SCARDCONTEXT hContext)
{
    LONG result;

    scard_pack_call_is_valid_context call(hContext);
    scard_pack_return_is_valid_context return_;

    // send call and receive return
    transmit(call, return_);

    // output
    result = return_.ReturnCode();

    return result;
}

REDEMPTION_LIB_EXPORT
LONG SCardConnect(
    SCARDCONTEXT hContext,
    LPCSTR szReader,
    DWORD dwShareMode,
    DWORD dwPreferredProtocols,
    /*@out@*/ LPSCARDHANDLE phCard,
    /*@out@*/ LPDWORD pdwActiveProtocol)
{
    SCARD_CHECK_PREDICATE(phCard, ERR_SCARD);
    SCARD_CHECK_PREDICATE(pdwActiveProtocol, ERR_SCARD);

    LONG result;

    scard_pack_call_connect call(hContext, szReader,
        dwShareMode, dwPreferredProtocols, false);
    scard_pack_return_connect return_;

    // send call and receive return
    transmit(call, return_);

    // output
    result = return_.ReturnCode();
    if (result == SCARD_S_SUCCESS)
    {
        *phCard = return_.hCard();
        *pdwActiveProtocol = return_.dwActiveProtocol();

        // update context
        _context.set(*phCard, hContext);
    }

    return result;
}

REDEMPTION_LIB_EXPORT
LONG SCardReconnect(
    SCARDHANDLE hCard,
    DWORD dwShareMode,
    DWORD dwPreferredProtocols,
    DWORD dwInitialization,
    /*@out@*/ LPDWORD pdwActiveProtocol)
{
    SCARD_CHECK_PREDICATE(pdwActiveProtocol, ERR_SCARD);

    LONG result;
    SCARDCONTEXT hContext;

    // look up context
    result = _context.get(hCard, hContext);
    if (result != SCARD_S_SUCCESS)
    {
        return result;
    }

    scard_pack_call_reconnect call(hContext, hCard, dwShareMode,
        dwPreferredProtocols, dwInitialization);
    scard_pack_return_reconnect return_;

    // send call and receive return
    transmit(call, return_);

    // output
    result = return_.ReturnCode();
    *pdwActiveProtocol = return_.dwActiveProtocol();

    return result;
}

REDEMPTION_LIB_EXPORT
LONG SCardDisconnect(
    SCARDHANDLE hCard,
    DWORD dwDisposition)
{
    LONG result;
    SCARDCONTEXT hContext;

    // look up context
    result = _context.get(hCard, hContext);
    if (result != SCARD_S_SUCCESS)
    {
        return result;
    }

    scard_pack_call_disconnect call(hContext, hCard, dwDisposition);
    scard_pack_return_disconnect return_;

    // send call and receive return
    transmit(call, return_);

    // output
    result = return_.ReturnCode();

    // update context
    _context.unset(hCard);

    return result;
}

REDEMPTION_LIB_EXPORT
LONG SCardBeginTransaction(
    SCARDHANDLE hCard)
{
    LONG result;
    SCARDCONTEXT hContext;

    // look up context
    result = _context.get(hCard, hContext);
    if (result != SCARD_S_SUCCESS)
    {
        return result;
    }

    scard_pack_call_begin_transaction call(hContext, hCard);
    scard_pack_return_begin_transaction return_;

    // send call and receive return
    transmit(call, return_);

    // output
    result = return_.ReturnCode();

    return result;
}

REDEMPTION_LIB_EXPORT
LONG SCardEndTransaction(
    SCARDHANDLE hCard,
    DWORD dwDisposition)
{
    LONG result;
    SCARDCONTEXT hContext;

    // look up context
    result = _context.get(hCard, hContext);
    if (result != SCARD_S_SUCCESS)
    {
        return result;
    }

    scard_pack_call_end_transaction call(hContext, hCard, dwDisposition);
    scard_pack_return_end_transaction return_;

    // send call and receive return
    transmit(call, return_);

    // output
    result = return_.ReturnCode();

    return result;
}

REDEMPTION_LIB_EXPORT
LONG SCardStatus(
    SCARDHANDLE hCard,
    /*@null@*/ /*@out@*/ LPSTR mszReaderNames,
    /*@null@*/ /*@out@*/ LPDWORD pcchReaderLen,
    /*@null@*/ /*@out@*/ LPDWORD pdwState,
    /*@null@*/ /*@out@*/ LPDWORD pdwProtocol,
    /*@null@*/ /*@out@*/ LPBYTE pbAtr,
    /*@null@*/ /*@out@*/ LPDWORD pcbAtrLen)
{
    LONG result;
    SCARDCONTEXT hContext;

    // look up context
    result = _context.get(hCard, hContext);
    if (result != SCARD_S_SUCCESS)
    {
        return result;
    }

    scard_pack_call_status call(hContext, hCard, mszReaderNames,
        pcchReaderLen, false);
    scard_pack_return_status return_;

    // send call and receive return
    transmit(call, return_);
    
    // output
    result = return_.ReturnCode();
    if (mszReaderNames)
    {
        std::memcpy(mszReaderNames, return_.mszReaderNames().data(),
            return_.mszReaderNames().size());
    }
    if (pcchReaderLen)
    {
        *pcchReaderLen = return_.cBytes();
    }
    if (pdwState)
    {
        *pdwState = return_.dwState();
    }
    if (pdwProtocol)
    {
        *pdwProtocol = return_.dwProtocol();
    }
    if (pbAtr)
    {
        std::memcpy(pbAtr, return_.pbAtr(), return_.cbAtrLen());
    }
    if (pcbAtrLen)
    {
        *pcbAtrLen = return_.cbAtrLen();
    }

    return result;
}

REDEMPTION_LIB_EXPORT
LONG SCardGetStatusChange(
    SCARDCONTEXT hContext,
    DWORD dwTimeout,
    SCARD_READERSTATE *rgReaderStates,
    DWORD cReaders)
{
    LONG result;

    scard_pack_call_get_status_change call(
        hContext, dwTimeout, rgReaderStates, cReaders, false);
    scard_pack_return_get_status_change return_;

    // send call and receive return
    transmit(call, return_);

    // output
    result = return_.ReturnCode();
    for (DWORD i = 0; i < return_.cReaders(); ++i)
    {
        return_.rgReaderStates(i).to_native(rgReaderStates[i]);
    }

    return result;
}

REDEMPTION_LIB_EXPORT
LONG SCardControl(
    SCARDHANDLE hCard,
    DWORD dwControlCode,
    LPCVOID pbSendBuffer,
    DWORD cbSendLength,
    /*@out@*/ LPVOID pbRecvBuffer,
    DWORD cbRecvLength,
    LPDWORD lpBytesReturned)
{
    LONG result;
    SCARDCONTEXT hContext;

    // look up context
    result = _context.get(hCard, hContext);
    if (result != SCARD_S_SUCCESS)
    {
        return result;
    }

    scard_pack_call_control call(hContext, hCard,
        dwControlCode, pbSendBuffer, cbSendLength,
        pbRecvBuffer, cbRecvLength);
    scard_pack_return_control return_;

    // send call and receive return
    transmit(call, return_);

    // output
    result = return_.ReturnCode();
    if (pbRecvBuffer)
    {
        std::memcpy(pbRecvBuffer, return_.pvOutBuffer(),
            return_.cbOutBufferSize());
    }
    if (lpBytesReturned)
    {
        *lpBytesReturned = return_.cbOutBufferSize();
    }

    return result;
}

REDEMPTION_LIB_EXPORT
LONG SCardTransmit(
    SCARDHANDLE hCard,
    const SCARD_IO_REQUEST *pioSendPci,
    LPCBYTE pbSendBuffer,
    DWORD cbSendLength,
    /*@out@*/ SCARD_IO_REQUEST *pioRecvPci,
    /*@out@*/ LPBYTE pbRecvBuffer,
    LPDWORD pcbRecvLength)
{
    LONG result;
    SCARDCONTEXT hContext;

    // look up context
    result = _context.get(hCard, hContext);
    if (result != SCARD_S_SUCCESS)
    {
        return result;
    }

    scard_pack_call_transmit call(hContext, hCard,
        pioSendPci, pbSendBuffer, cbSendLength,
        pioRecvPci, pbRecvBuffer, pcbRecvLength);
    scard_pack_return_transmit return_;

    // send call and receive return
    transmit(call, return_);

    // output
    result = return_.ReturnCode();
    if (pioRecvPci)
    {
        const auto pioRecvPci_ = return_.pioRecvPci();

        if (pioRecvPci_)
        {
            pioRecvPci_->to_native(pioRecvPci);
        }
    }
    if (pbRecvBuffer)
    {
        const auto pbRecvBuffer_ = return_.pbRecvBuffer();
        const auto cbRecvLength_ = return_.cbRecvLength();

        if (pbRecvBuffer_)
        {
            std::memcpy(pbRecvBuffer, pbRecvBuffer_, cbRecvLength_);
        }
    }
    if (pcbRecvLength)
    {
        *pcbRecvLength = return_.cbRecvLength();
    }

    return result;
}

REDEMPTION_LIB_EXPORT
LONG SCardListReaderGroups(
    SCARDCONTEXT hContext,
    /*@out@*/ LPSTR mszGroups,
    LPDWORD pcchGroups)
{
    SCARD_CHECK_PREDICATE(pcchGroups, ERR_SCARD);

    LONG result;

    scard_pack_call_list_reader_groups call(hContext, mszGroups,
        *pcchGroups, false);
    scard_pack_return_list_reader_groups return_(false);

    // send call and receive return
    transmit(call, return_);

    // output
    result = return_.ReturnCode();
    if (mszGroups)
    {
        std::memcpy(mszGroups, return_.mszGroups(),
            return_.cBytes());
    }
    *pcchGroups = return_.cBytes();

    return result;
}

REDEMPTION_LIB_EXPORT
LONG SCardListReaders(
    SCARDCONTEXT hContext,
    /*@null@*/ /*@out@*/ LPCSTR mszGroups,
    /*@null@*/ /*@out@*/ LPSTR mszReaders,
    /*@out@*/ LPDWORD pcchReaders)
{
    SCARD_CHECK_PREDICATE(pcchReaders, ERR_SCARD);

    LONG result;

    scard_pack_call_list_readers call(hContext, mszGroups,
        mszReaders, *pcchReaders, false);
    scard_pack_return_list_readers return_(false);

    // send call and receive return
    transmit(call, return_);

    // output
    result = return_.ReturnCode();
    if (mszReaders)
    {
        std::memcpy(mszReaders, return_.mszReaders(),
            return_.cBytes());
    }
    *pcchReaders = return_.cBytes();

    return result;
}

REDEMPTION_LIB_EXPORT
LONG SCardFreeMemory(
    SCARDCONTEXT /*hContext*/,
    LPCVOID /*pvMem*/)
{
    // FIXME Deallocate memory

    return SCARD_S_SUCCESS;
}

REDEMPTION_LIB_EXPORT
LONG SCardCancel(
    SCARDCONTEXT hContext)
{
    LONG result;

    scard_pack_call_cancel call(hContext);
    scard_pack_return_cancel return_;

    // send call and receive return
    transmit(call, return_);

    // output
    result = return_.ReturnCode();

    return result;
}

REDEMPTION_LIB_EXPORT
LONG SCardGetAttrib(
    SCARDHANDLE hCard,
    DWORD dwAttrId,
    /*@out@*/ LPBYTE pbAttr,
    LPDWORD pcbAttrLen)
{
    LONG result;
    SCARDCONTEXT hContext;

    // look up context
    result = _context.get(hCard, hContext);
    if (result != SCARD_S_SUCCESS)
    {
        return result;
    }

    scard_pack_call_get_attrib call(hContext, hCard,
        dwAttrId, pbAttr, pcbAttrLen);
    scard_pack_return_get_attrib return_;

    // send call and receive return
    transmit(call, return_);

    // output
    result = return_.ReturnCode();
    if (pbAttr)
    {
        std::memcpy(pbAttr, return_.pbAttr(),
            return_.cbAttrLen());
    }
    if (pcbAttrLen)
    {
        *pcbAttrLen = return_.cbAttrLen();
    }

    return result;
}

REDEMPTION_LIB_EXPORT
LONG SCardSetAttrib(
    SCARDHANDLE hCard,
    DWORD dwAttrId,
    LPCBYTE pbAttr,
    DWORD cbAttrLen)
{
    LONG result;
    SCARDCONTEXT hContext;

    // look up context
    result = _context.get(hCard, hContext);
    if (result != SCARD_S_SUCCESS)
    {
        return result;
    }

    scard_pack_call_set_attrib call(hContext, hCard,
        dwAttrId, pbAttr, cbAttrLen);
    scard_pack_return_set_attrib return_;

    // send call and receive return
    transmit(call, return_);

    // output
    result = return_.ReturnCode();

    return result;
}

#ifdef __cplusplus
}
#endif  // __cplusplus