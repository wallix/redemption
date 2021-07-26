#include <string>

#include <PCSC/pcsclite.h>

#include "scard/scard_pack_utils.hpp"


///////////////////////////////////////////////////////////////////////////////


#define SCARD_ATTR_VALUE(Class, Tag) (((static_cast<ULONG>(Class)) << 16) | (static_cast<ULONG>(Tag)))

#define SCARD_CLASS_VENDOR_INFO 1
#define SCARD_CLASS_COMMUNICATIONS 2
#define SCARD_CLASS_PROTOCOL 3
#define SCARD_CLASS_POWER_MGMT 4
#define SCARD_CLASS_SECURITY 5
#define SCARD_CLASS_MECHANICAL 6
#define SCARD_CLASS_VENDOR_DEFINED 7
#define SCARD_CLASS_IFD_PROTOCOL 8
#define SCARD_CLASS_ICC_STATE 9
#define SCARD_CLASS_SYSTEM 0x7FFF

#define SCARD_ATTR_VENDOR_NAME SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_INFO, 0x0100)
#define SCARD_ATTR_VENDOR_IFD_TYPE SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_INFO, 0x0101)
#define SCARD_ATTR_VENDOR_IFD_VERSION SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_INFO, 0x0102)
#define SCARD_ATTR_VENDOR_IFD_SERIAL_NO SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_INFO, 0x0103)
#define SCARD_ATTR_CHANNEL_ID SCARD_ATTR_VALUE(SCARD_CLASS_COMMUNICATIONS, 0x0110)
#define SCARD_ATTR_PROTOCOL_TYPES SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0120)
#define SCARD_ATTR_DEFAULT_CLK SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0121)
#define SCARD_ATTR_MAX_CLK SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0122)
#define SCARD_ATTR_DEFAULT_DATA_RATE SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0123)
#define SCARD_ATTR_MAX_DATA_RATE SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0124)
#define SCARD_ATTR_MAX_IFSD SCARD_ATTR_VALUE(SCARD_CLASS_PROTOCOL, 0x0125)
#define SCARD_ATTR_POWER_MGMT_SUPPORT SCARD_ATTR_VALUE(SCARD_CLASS_POWER_MGMT, 0x0131)
#define SCARD_ATTR_USER_TO_CARD_AUTH_DEVICE SCARD_ATTR_VALUE(SCARD_CLASS_SECURITY, 0x0140)
#define SCARD_ATTR_USER_AUTH_INPUT_DEVICE SCARD_ATTR_VALUE(SCARD_CLASS_SECURITY, 0x0142)
#define SCARD_ATTR_CHARACTERISTICS SCARD_ATTR_VALUE(SCARD_CLASS_MECHANICAL, 0x0150)

#define SCARD_ATTR_CURRENT_PROTOCOL_TYPE SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0201)
#define SCARD_ATTR_CURRENT_CLK SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0202)
#define SCARD_ATTR_CURRENT_F SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0203)
#define SCARD_ATTR_CURRENT_D SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0204)
#define SCARD_ATTR_CURRENT_N SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0205)
#define SCARD_ATTR_CURRENT_W SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0206)
#define SCARD_ATTR_CURRENT_IFSC SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0207)
#define SCARD_ATTR_CURRENT_IFSD SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0208)
#define SCARD_ATTR_CURRENT_BWT SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x0209)
#define SCARD_ATTR_CURRENT_CWT SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x020a)
#define SCARD_ATTR_CURRENT_EBC_ENCODING SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x020b)
#define SCARD_ATTR_EXTENDED_BWT SCARD_ATTR_VALUE(SCARD_CLASS_IFD_PROTOCOL, 0x020c)

#define SCARD_ATTR_ICC_PRESENCE SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0300)
#define SCARD_ATTR_ICC_INTERFACE_STATUS SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0301)
#define SCARD_ATTR_CURRENT_IO_STATE SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0302)
#define SCARD_ATTR_ATR_STRING SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0303)
#define SCARD_ATTR_ICC_TYPE_PER_ATR SCARD_ATTR_VALUE(SCARD_CLASS_ICC_STATE, 0x0304)

#define SCARD_ATTR_ESC_RESET SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED, 0xA000)
#define SCARD_ATTR_ESC_CANCEL SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED, 0xA003)
#define SCARD_ATTR_ESC_AUTHREQUEST SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED, 0xA005)
#define SCARD_ATTR_MAXINPUT SCARD_ATTR_VALUE(SCARD_CLASS_VENDOR_DEFINED, 0xA007)

#define SCARD_ATTR_DEVICE_UNIT SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0001)
#define SCARD_ATTR_DEVICE_IN_USE SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0002)
#define SCARD_ATTR_DEVICE_FRIENDLY_NAME_A SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0003)
#define SCARD_ATTR_DEVICE_SYSTEM_NAME_A SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0004)
#define SCARD_ATTR_DEVICE_FRIENDLY_NAME_W SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0005)
#define SCARD_ATTR_DEVICE_SYSTEM_NAME_W SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0006)
#define SCARD_ATTR_SUPRESS_T1_IFS_REQUEST SCARD_ATTR_VALUE(SCARD_CLASS_SYSTEM, 0x0007)

#define SCARD_PROTOCOL_Tx (SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1)
#define SCARD_PROTOCOL_DEFAULT 0x80000000u


///////////////////////////////////////////////////////////////////////////////


const char * return_code_to_string(SCARD_PACK_NATIVE_LONG code)
{
    switch (code)
    {
        case SCARD_S_SUCCESS:
            return "SCARD_S_SUCCESS";

        case SCARD_F_INTERNAL_ERROR:
            return "SCARD_F_INTERNAL_ERROR";

        case SCARD_E_CANCELLED:
            return "SCARD_E_CANCELLED";

        case SCARD_E_INVALID_HANDLE:
            return "SCARD_E_INVALID_HANDLE";

        case SCARD_E_INVALID_PARAMETER:
            return "SCARD_E_INVALID_PARAMETER";

        case SCARD_E_INVALID_TARGET:
            return "SCARD_E_INVALID_TARGET";

        case SCARD_E_NO_MEMORY:
            return "SCARD_E_NO_MEMORY";

        case SCARD_F_WAITED_TOO_LONG:
            return "SCARD_F_WAITED_TOO_LONG";

        case SCARD_E_INSUFFICIENT_BUFFER:
            return "SCARD_E_INSUFFICIENT_BUFFER";

        case SCARD_E_UNKNOWN_READER:
            return "SCARD_E_UNKNOWN_READER";

        case SCARD_E_TIMEOUT:
            return "SCARD_E_TIMEOUT";

        case SCARD_E_SHARING_VIOLATION:
            return "SCARD_E_SHARING_VIOLATION";

        case SCARD_E_NO_SMARTCARD:
            return "SCARD_E_NO_SMARTCARD";

        case SCARD_E_UNKNOWN_CARD:
            return "SCARD_E_UNKNOWN_CARD";

        case SCARD_E_CANT_DISPOSE:
            return "SCARD_E_CANT_DISPOSE";

        case SCARD_E_PROTO_MISMATCH:
            return "SCARD_E_PROTO_MISMATCH";

        case SCARD_E_NOT_READY:
            return "SCARD_E_NOT_READY";

        case SCARD_E_INVALID_VALUE:
            return "SCARD_E_INVALID_VALUE";

        case SCARD_E_SYSTEM_CANCELLED:
            return "SCARD_E_SYSTEM_CANCELLED";

        case SCARD_F_COMM_ERROR:
            return "SCARD_F_COMM_ERROR";

        case SCARD_F_UNKNOWN_ERROR:
            return "SCARD_F_UNKNOWN_ERROR";

        case SCARD_E_INVALID_ATR:
            return "SCARD_E_INVALID_ATR";

        case SCARD_E_NOT_TRANSACTED:
            return "SCARD_E_NOT_TRANSACTED";

        case SCARD_E_READER_UNAVAILABLE:
            return "SCARD_E_READER_UNAVAILABLE";

        case SCARD_P_SHUTDOWN:
            return "SCARD_P_SHUTDOWN";

        case SCARD_E_PCI_TOO_SMALL:
            return "SCARD_E_PCI_TOO_SMALL";

        case SCARD_E_READER_UNSUPPORTED:
            return "SCARD_E_READER_UNSUPPORTED";

        case SCARD_E_DUPLICATE_READER:
            return "SCARD_E_DUPLICATE_READER";

        case SCARD_E_CARD_UNSUPPORTED:
            return "SCARD_E_CARD_UNSUPPORTED";

        case SCARD_E_NO_SERVICE:
            return "SCARD_E_NO_SERVICE";

        case SCARD_E_SERVICE_STOPPED:
            return "SCARD_E_SERVICE_STOPPED";

        case SCARD_E_UNEXPECTED:
            return "SCARD_E_UNEXPECTED";

        case SCARD_E_ICC_INSTALLATION:
            return "SCARD_E_ICC_INSTALLATION";

        case SCARD_E_ICC_CREATEORDER:
            return "SCARD_E_ICC_CREATEORDER";

        /*
        case SCARD_E_UNSUPPORTED_FEATURE:
            return "SCARD_E_UNSUPPORTED_FEATURE";
        */

        case SCARD_E_DIR_NOT_FOUND:
            return "SCARD_E_DIR_NOT_FOUND";

        case SCARD_E_FILE_NOT_FOUND:
            return "SCARD_E_FILE_NOT_FOUND";

        case SCARD_E_NO_DIR:
            return "SCARD_E_NO_DIR";

        case SCARD_E_NO_FILE:
            return "SCARD_E_NO_FILE";

        case SCARD_E_NO_ACCESS:
            return "SCARD_E_NO_ACCESS";

        case SCARD_E_WRITE_TOO_MANY:
            return "SCARD_E_WRITE_TOO_MANY";

        case SCARD_E_BAD_SEEK:
            return "SCARD_E_BAD_SEEK";

        case SCARD_E_INVALID_CHV:
            return "SCARD_E_INVALID_CHV";

        case SCARD_E_UNKNOWN_RES_MNG:
            return "SCARD_E_UNKNOWN_RES_MNG";

        case SCARD_E_NO_SUCH_CERTIFICATE:
            return "SCARD_E_NO_SUCH_CERTIFICATE";

        case SCARD_E_CERTIFICATE_UNAVAILABLE:
            return "SCARD_E_CERTIFICATE_UNAVAILABLE";

        case SCARD_E_NO_READERS_AVAILABLE:
            return "SCARD_E_NO_READERS_AVAILABLE";

        case SCARD_E_COMM_DATA_LOST:
            return "SCARD_E_COMM_DATA_LOST";

        case SCARD_E_NO_KEY_CONTAINER:
            return "SCARD_E_NO_KEY_CONTAINER";

        case SCARD_E_SERVER_TOO_BUSY:
            return "SCARD_E_SERVER_TOO_BUSY";

        /*
        case SCARD_E_PIN_CACHE_EXPIRED:
            return "SCARD_E_PIN_CACHE_EXPIRED";
        */

        /*
        case SCARD_E_NO_PIN_CACHE:
            return "SCARD_E_NO_PIN_CACHE";
        */

        /*
        case SCARD_E_READ_ONLY_CARD:
            return "SCARD_E_READ_ONLY_CARD";
        */

        case SCARD_W_UNSUPPORTED_CARD:
            return "SCARD_W_UNSUPPORTED_CARD";

        case SCARD_W_UNRESPONSIVE_CARD:
            return "SCARD_W_UNRESPONSIVE_CARD";

        case SCARD_W_UNPOWERED_CARD:
            return "SCARD_W_UNPOWERED_CARD";

        case SCARD_W_RESET_CARD:
            return "SCARD_W_RESET_CARD";

        case SCARD_W_REMOVED_CARD:
            return "SCARD_W_REMOVED_CARD";

        case SCARD_W_SECURITY_VIOLATION:
            return "SCARD_W_SECURITY_VIOLATION";

        case SCARD_W_WRONG_CHV:
            return "SCARD_W_WRONG_CHV";

        case SCARD_W_CHV_BLOCKED:
            return "SCARD_W_CHV_BLOCKED";

        case SCARD_W_EOF:
            return "SCARD_W_EOF";

        case SCARD_W_CANCELLED_BY_USER:
            return "SCARD_W_CANCELLED_BY_USER";

        case SCARD_W_CARD_NOT_AUTHENTICATED:
            return "SCARD_W_CARD_NOT_AUTHENTICATED";

        /*
        case SCARD_W_CACHE_ITEM_NOT_FOUND:
            return "SCARD_W_CACHE_ITEM_NOT_FOUND";
        */

        /*
        case SCARD_W_CACHE_ITEM_STALE:
            return "SCARD_W_CACHE_ITEM_STALE";
        */

        /*
        case SCARD_W_CACHE_ITEM_TOO_BIG:
            return "SCARD_W_CACHE_ITEM_TOO_BIG";
        */

        default:
            return "SCARD_E_UNKNOWN";
    }
}

const char * attribute_to_string(SCARD_PACK_NATIVE_LONG attribute)
{
	switch (attribute)
	{
		case SCARD_ATTR_VENDOR_NAME:
			return "SCARD_ATTR_VENDOR_NAME";

		case SCARD_ATTR_VENDOR_IFD_TYPE:
			return "SCARD_ATTR_VENDOR_IFD_TYPE";

		case SCARD_ATTR_VENDOR_IFD_VERSION:
			return "SCARD_ATTR_VENDOR_IFD_VERSION";

		case SCARD_ATTR_VENDOR_IFD_SERIAL_NO:
			return "SCARD_ATTR_VENDOR_IFD_SERIAL_NO";

		case SCARD_ATTR_CHANNEL_ID:
			return "SCARD_ATTR_CHANNEL_ID";

		case SCARD_ATTR_PROTOCOL_TYPES:
			return "SCARD_ATTR_PROTOCOL_TYPES";

		case SCARD_ATTR_DEFAULT_CLK:
			return "SCARD_ATTR_DEFAULT_CLK";

		case SCARD_ATTR_MAX_CLK:
			return "SCARD_ATTR_MAX_CLK";

		case SCARD_ATTR_DEFAULT_DATA_RATE:
			return "SCARD_ATTR_DEFAULT_DATA_RATE";

		case SCARD_ATTR_MAX_DATA_RATE:
			return "SCARD_ATTR_MAX_DATA_RATE";

		case SCARD_ATTR_MAX_IFSD:
			return "SCARD_ATTR_MAX_IFSD";

		case SCARD_ATTR_POWER_MGMT_SUPPORT:
			return "SCARD_ATTR_POWER_MGMT_SUPPORT";

		case SCARD_ATTR_USER_TO_CARD_AUTH_DEVICE:
			return "SCARD_ATTR_USER_TO_CARD_AUTH_DEVICE";

		case SCARD_ATTR_USER_AUTH_INPUT_DEVICE:
			return "SCARD_ATTR_USER_AUTH_INPUT_DEVICE";

		case SCARD_ATTR_CHARACTERISTICS:
			return "SCARD_ATTR_CHARACTERISTICS";

		case SCARD_ATTR_CURRENT_PROTOCOL_TYPE:
			return "SCARD_ATTR_CURRENT_PROTOCOL_TYPE";

		case SCARD_ATTR_CURRENT_CLK:
			return "SCARD_ATTR_CURRENT_CLK";

		case SCARD_ATTR_CURRENT_F:
			return "SCARD_ATTR_CURRENT_F";

		case SCARD_ATTR_CURRENT_D:
			return "SCARD_ATTR_CURRENT_D";

		case SCARD_ATTR_CURRENT_N:
			return "SCARD_ATTR_CURRENT_N";

		case SCARD_ATTR_CURRENT_W:
			return "SCARD_ATTR_CURRENT_W";

		case SCARD_ATTR_CURRENT_IFSC:
			return "SCARD_ATTR_CURRENT_IFSC";

		case SCARD_ATTR_CURRENT_IFSD:
			return "SCARD_ATTR_CURRENT_IFSD";

		case SCARD_ATTR_CURRENT_BWT:
			return "SCARD_ATTR_CURRENT_BWT";

		case SCARD_ATTR_CURRENT_CWT:
			return "SCARD_ATTR_CURRENT_CWT";

		case SCARD_ATTR_CURRENT_EBC_ENCODING:
			return "SCARD_ATTR_CURRENT_EBC_ENCODING";

		case SCARD_ATTR_EXTENDED_BWT:
			return "SCARD_ATTR_EXTENDED_BWT";

		case SCARD_ATTR_ICC_PRESENCE:
			return "SCARD_ATTR_ICC_PRESENCE";

		case SCARD_ATTR_ICC_INTERFACE_STATUS:
			return "SCARD_ATTR_ICC_INTERFACE_STATUS";

		case SCARD_ATTR_CURRENT_IO_STATE:
			return "SCARD_ATTR_CURRENT_IO_STATE";

		case SCARD_ATTR_ATR_STRING:
			return "SCARD_ATTR_ATR_STRING";

		case SCARD_ATTR_ICC_TYPE_PER_ATR:
			return "SCARD_ATTR_ICC_TYPE_PER_ATR";

		case SCARD_ATTR_ESC_RESET:
			return "SCARD_ATTR_ESC_RESET";

		case SCARD_ATTR_ESC_CANCEL:
			return "SCARD_ATTR_ESC_CANCEL";

		case SCARD_ATTR_ESC_AUTHREQUEST:
			return "SCARD_ATTR_ESC_AUTHREQUEST";

		case SCARD_ATTR_MAXINPUT:
			return "SCARD_ATTR_MAXINPUT";

		case SCARD_ATTR_DEVICE_UNIT:
			return "SCARD_ATTR_DEVICE_UNIT";

		case SCARD_ATTR_DEVICE_IN_USE:
			return "SCARD_ATTR_DEVICE_IN_USE";

		case SCARD_ATTR_DEVICE_FRIENDLY_NAME_A:
			return "SCARD_ATTR_DEVICE_FRIENDLY_NAME_A";

		case SCARD_ATTR_DEVICE_SYSTEM_NAME_A:
			return "SCARD_ATTR_DEVICE_SYSTEM_NAME_A";

		case SCARD_ATTR_DEVICE_FRIENDLY_NAME_W:
			return "SCARD_ATTR_DEVICE_FRIENDLY_NAME_W";

		case SCARD_ATTR_DEVICE_SYSTEM_NAME_W:
			return "SCARD_ATTR_DEVICE_SYSTEM_NAME_W";

		case SCARD_ATTR_SUPRESS_T1_IFS_REQUEST:
			return "SCARD_ATTR_SUPRESS_T1_IFS_REQUEST";

		default:
			return "SCARD_ATTR_UNKNOWN";
	}
}

const char * protocols_to_string(SCARD_PACK_NATIVE_LONG protocols)
{
	if (protocols == SCARD_PROTOCOL_UNDEFINED)
		return "SCARD_PROTOCOL_UNDEFINED";

	if (protocols == SCARD_PROTOCOL_T0)
		return "SCARD_PROTOCOL_T0";

	if (protocols == SCARD_PROTOCOL_T1)
		return "SCARD_PROTOCOL_T1";

	if (protocols == SCARD_PROTOCOL_Tx)
		return "SCARD_PROTOCOL_Tx";

	if (protocols == SCARD_PROTOCOL_RAW)
		return "SCARD_PROTOCOL_RAW";

	if (protocols == SCARD_PROTOCOL_DEFAULT)
		return "SCARD_PROTOCOL_DEFAULT";

	if (protocols == (SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_RAW))
		return "SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_RAW";

	if (protocols == (SCARD_PROTOCOL_T1 | SCARD_PROTOCOL_RAW))
		return "SCARD_PROTOCOL_T1 | SCARD_PROTOCOL_RAW";

	if (protocols == (SCARD_PROTOCOL_Tx | SCARD_PROTOCOL_RAW))
		return "SCARD_PROTOCOL_Tx | SCARD_PROTOCOL_RAW";

	return "SCARD_PROTOCOL_UNKNOWN";
}

const char * share_mode_to_string(SCARD_PACK_NATIVE_LONG share_mode)
{
	switch (share_mode)
	{
		case SCARD_SHARE_EXCLUSIVE:
			return "SCARD_SHARE_EXCLUSIVE";

		case SCARD_SHARE_SHARED:
			return "SCARD_SHARE_SHARED";

		case SCARD_SHARE_DIRECT:
			return "SCARD_SHARE_DIRECT";

		default:
			return "SCARD_SHARE_UNKNOWN";
	}
}

const char * disposition_to_string(SCARD_PACK_NATIVE_LONG disposition)
{
	switch (disposition)
	{
		case SCARD_LEAVE_CARD:
			return "SCARD_LEAVE_CARD";

		case SCARD_RESET_CARD:
			return "SCARD_RESET_CARD";

		case SCARD_UNPOWER_CARD:
			return "SCARD_UNPOWER_CARD";

		default:
			return "SCARD_UNKNOWN_CARD";
	}
}

const char * scope_to_string(SCARD_PACK_NATIVE_LONG scope)
{
	switch (scope)
	{
		case SCARD_SCOPE_USER:
			return "SCARD_SCOPE_USER";

		case SCARD_SCOPE_TERMINAL:
			return "SCARD_SCOPE_TERMINAL";

		case SCARD_SCOPE_SYSTEM:
			return "SCARD_SCOPE_SYSTEM";

		default:
			return "SCARD_SCOPE_UNKNOWN";
	}
}

const char * card_state_to_string(SCARD_PACK_NATIVE_LONG card_state)
{
	switch (card_state)
	{
		case SCARD_UNKNOWN:
			return "SCARD_UNKNOWN";

		case SCARD_ABSENT:
			return "SCARD_ABSENT";

		case SCARD_PRESENT:
			return "SCARD_PRESENT";

		case SCARD_SWALLOWED:
			return "SCARD_SWALLOWED";

		case SCARD_POWERED:
			return "SCARD_POWERED";

		case SCARD_NEGOTIABLE:
			return "SCARD_NEGOTIABLE";

		case SCARD_SPECIFIC:
			return "SCARD_SPECIFIC";

		default:
			return "SCARD_UNKNOWN";
	}
}

std::string reader_state_to_string(SCARD_PACK_NATIVE_DWORD reader_state)
{
	std::string string;

	if (reader_state & SCARD_STATE_IGNORE)
	{
		if (!string.empty())
        {
			string.append(" | ");
        }

		string.append("SCARD_STATE_IGNORE");
	}

	if (reader_state & SCARD_STATE_CHANGED)
	{
		if (!string.empty())
        {
			string.append(" | ");
        }

		string.append("SCARD_STATE_CHANGED");
	}

	if (reader_state & SCARD_STATE_UNKNOWN)
	{
		if (!string.empty())
        {
			string.append(" | ");
        }

		string.append("SCARD_STATE_UNKNOWN");
	}

	if (reader_state & SCARD_STATE_UNAVAILABLE)
	{
		if (!string.empty())
        {
			string.append(" | ");
        }

		string.append("SCARD_STATE_UNAVAILABLE");
	}

	if (reader_state & SCARD_STATE_EMPTY)
	{
		if (!string.empty())
        {
			string.append(" | ");
        }

		string.append("SCARD_STATE_EMPTY");
	}

	if (reader_state & SCARD_STATE_PRESENT)
	{
		if (!string.empty())
        {
			string.append(" | ");
        }

		string.append("SCARD_STATE_PRESENT");
	}

	if (reader_state & SCARD_STATE_ATRMATCH)
	{
		if (!string.empty())
        {
			string.append(" | ");
        }

		string.append("SCARD_STATE_ATRMATCH");
	}

	if (reader_state & SCARD_STATE_EXCLUSIVE)
	{
		if (!string.empty())
        {
			string.append(" | ");
        }

		string.append("SCARD_STATE_EXCLUSIVE");
	}

	if (reader_state & SCARD_STATE_INUSE)
	{
		if (!string.empty())
        {
			string.append(" | ");
        }

		string.append("SCARD_STATE_INUSE");
	}

	if (reader_state & SCARD_STATE_MUTE)
	{
		if (!string.empty())
        {
			string.append(" | ");
        }

		string.append("SCARD_STATE_MUTE");
	}

	if (reader_state & SCARD_STATE_UNPOWERED)
	{
		if (!string.empty())
        {
			string.append(" | ");
        }

		string.append("SCARD_STATE_UNPOWERED");
	}

    if (string.empty())
    {
        string.append("SCARD_STATE_UNPOWERED");
    }

	return string;
}