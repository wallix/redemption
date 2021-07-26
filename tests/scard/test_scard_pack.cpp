#include <limits>

#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "scard/scard_pack.hpp"
#include "utils/stream.hpp"
#include "utils/sugar/buffer_view.hpp"
#include "utils/sugar/bytes_view.hpp"


///////////////////////////////////////////////////////////////////////////////


static constexpr std::size_t padding_size(std::size_t data_size)
{
   return ((data_size % 8) ? (8 - (data_size % 8)) : 0);
}

static constexpr std::size_t padded_size(std::size_t data_size)
{
   return (data_size + padding_size(data_size));
}

static constexpr std::size_t packed_unique_array_size(std::size_t referent_size)
{
   return (4 + 4 + (referent_size ? (4 + referent_size) : 0));
}

static constexpr std::size_t packed_null_array_size(bool sized)
{
   std::size_t size(0);

   // size
   if (sized)
   {
      size += sizeof(uint32_t);
   }

   // pointer
   size += sizeof(uint32_t);

   return size;
}

static constexpr std::size_t packed_array_size(scard_pack_array_type type, bool sized,
   std::size_t element_size, std::size_t element_count)
{
   std::size_t size(0);

   // size
   if (sized)
   {
      size += sizeof(uint32_t);
   }

   // pointer
   size += sizeof(uint32_t);

   // array header
   switch (type)
   {
      case scard_pack_array_type_conformant:
      {
            size += sizeof(uint32_t);
            break;
      }
      case scard_pack_array_type_varying:
      {
            size += (2 * sizeof(uint32_t));
            break;
      }
      case scard_pack_array_type_conformant_varying:
      {
            size += (3 * sizeof(uint32_t));
            break;
      }
      case scard_pack_array_type_fixed:
      default:
            break;
   }

   const auto elements_size = (element_size * element_count);

   // array elements
   size += elements_size;

   // padding
   size += ((elements_size % 4) ? (4 - (elements_size % 4)) : 0);

   return size;
}

static constexpr std::size_t packed_fixed_array_size(
   std::size_t element_size, std::size_t element_count)
{
   return packed_array_size(scard_pack_array_type_fixed, false,
      element_size, element_count);
}

static constexpr std::size_t packed_conformant_array_size(bool sized,
   std::size_t element_size, std::size_t element_count)
{
   return packed_array_size(scard_pack_array_type_conformant, sized,
      element_size, element_count);
}

static constexpr std::size_t packed_varying_array_size(bool sized,
   std::size_t element_size, std::size_t element_count)
{
   return packed_array_size(scard_pack_array_type_varying, sized,
      element_size, element_count);
}

static constexpr std::size_t packed_conformant_varying_array_size(bool sized,
   std::size_t element_size, std::size_t element_count)
{
   return packed_array_size(scard_pack_array_type_conformant_varying, sized,
      element_size, element_count);
}

static const std::size_t native_context_size = sizeof(SCARDCONTEXT);
static const std::size_t packed_native_context_size = packed_array_size(
   scard_pack_array_type_conformant, true, sizeof(uint8_t), native_context_size);
static const std::size_t max_packed_context_size = packed_array_size(
   scard_pack_array_type_conformant, true, sizeof(uint8_t), 16);
static const std::size_t min_packed_context_size = packed_null_array_size(true);

static const std::size_t native_handle_size = sizeof(SCARDHANDLE);
static const std::size_t packed_native_handle_size = 4 + 4 + 4 + native_handle_size;


static const uint8_t establish_context_call_1[8] = {
   // dwScope
   0x01, 0x00, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};
static const uint8_t establish_context_return_1[24] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01
};
static const uint8_t establish_context_return_2[8] = {
   // ReturnCode
   0x01, 0x01, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_establish_context)
{
   {
      scard_pack_call_establish_context call(establish_context_call_1[0]);

      RED_CHECK_EQUAL(sizeof(establish_context_call_1), call.packed_size());
      /*
      RED_CHECK_EQUAL(sizeof(establish_context_call_1), call.max_packed_size());
      */

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(establish_context_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(establish_context_call_1, sizeof(establish_context_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_return_establish_context return_;

      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_THROW(return_.hContext(), Error);
      RED_CHECK_EQUAL(padded_size(
            // ReturnCode
            4
            // Context
            + packed_null_array_size(true)    
         ),
         return_.packed_size()
      );

      InStream stream(establish_context_return_1);

      RED_CHECK_EQUAL(sizeof(establish_context_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(establish_context_return_1, sizeof(establish_context_return_1)),
         stream.get_consumed_bytes()
      );
      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_EQUAL(0x0102030405060708, return_.hContext());
   }
   {
      scard_pack_return_establish_context return_;

      InStream stream(establish_context_return_2);

      RED_CHECK_EQUAL(sizeof(establish_context_return_2), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(establish_context_return_2, sizeof(establish_context_return_2)),
         stream.get_consumed_bytes()
      );
      RED_CHECK_EQUAL(0x00000101, return_.ReturnCode());
      RED_CHECK_THROW(return_.hContext(), Error);
   }
}


static const uint8_t release_context_call_1[24] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // padding
   0x00, 0x00, 0x00, 0x00
};
static const uint8_t release_context_return_1[8] = {
   // ReturnCode
   0x04, 0x03, 0x02, 0x01,
   // padding
   0x00, 0x00, 0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_release_context)
{
   {
      scard_pack_call_release_context call(0x0102030405060708);

      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
         ),
         call.packed_size());

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(release_context_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(release_context_call_1, sizeof(release_context_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_return_release_context return_;

      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_EQUAL(8, return_.packed_size());

      InStream stream(release_context_return_1);

      RED_CHECK_EQUAL(sizeof(release_context_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(release_context_return_1, sizeof(release_context_return_1)),
         stream.get_consumed_bytes()
      );
      RED_CHECK_EQUAL(0x01020304, return_.ReturnCode());
   }
}


static const uint8_t is_valid_context_call_1[24] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t is_valid_context_return_1[8] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_is_valid_context)
{
   {
      scard_pack_call_is_valid_context call(0x0102030405060708);

      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(is_valid_context_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(is_valid_context_call_1, sizeof(is_valid_context_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_return_is_valid_context return_;

      InStream stream(is_valid_context_return_1);

      RED_CHECK_EQUAL(sizeof(is_valid_context_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(is_valid_context_return_1, sizeof(is_valid_context_return_1)),
         stream.get_consumed_bytes()
      );

      RED_CHECK_EQUAL(0, return_.ReturnCode());
   }
}


static const uint8_t connect_call_1[32] = {
   // szReader
   0x00, 0x00, 0x00, 0x00,
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // dwShareMode
   0x02, 0x00, 0x00, 0x00,
   // dwPreferredProtocols
   0x01, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01
};

static const uint8_t connect_call_2[56] = {
   // szReader
   0x00, 0x00, 0x02, 0x00,
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // dwShareMode
   0x02, 0x00, 0x00, 0x00,
   // dwPreferredProtocols
   0x01, 0x00, 0x00, 0x00,
   // szReader (deferred)
   0x07, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x07, 0x00, 0x00, 0x00,
   'r', 'e', 'a', 'd', 'e', 'r', 0x00,
   0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t connect_return_1[48] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // dwActiveProtocol
   0x02, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09
};

static const uint8_t connect_return_2[8] = {
   // ReturnCode
   0x01, 0x00, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_connect)
{
   {
      scard_pack_call_connect call(0x0102030405060708,
         nullptr, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, false);
      
      RED_CHECK_EQUAL(padded_size(
            // szReader
            packed_null_array_size(false)
            // Context
            + packed_native_context_size
            // dwShareMode
            + 4
            // dwPreferredProtocols
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(connect_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(connect_call_1, sizeof(connect_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_call_connect call(0x0102030405060708,
         "reader", SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0, false);
      
      RED_CHECK_EQUAL(padded_size(
            // szReader
            packed_conformant_varying_array_size(
               true, sizeof(char), 7
            )
            // Context
            + packed_native_context_size
            // dwShareMode
            + 4
            // dwPreferredProtocols
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(connect_call_2), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(connect_call_2, sizeof(connect_call_2)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_return_connect return_;

      InStream stream(connect_return_1);

      RED_CHECK_EQUAL(sizeof(connect_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(connect_return_1, sizeof(connect_return_1)),
         stream.get_consumed_bytes()
      );

      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_EQUAL(0x0102030405060708, return_.hContext());
      RED_CHECK_EQUAL(0x090A0B0C0D0E0F10, return_.hCard());
      RED_CHECK_EQUAL(2, return_.dwActiveProtocol());
   }
   {
      scard_pack_return_connect return_;

      InStream stream(connect_return_2);

      RED_CHECK_EQUAL(sizeof(connect_return_2), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(connect_return_2, sizeof(connect_return_2)),
         stream.get_consumed_bytes()
      );

      RED_CHECK_EQUAL(1, return_.ReturnCode());
   }
}


static const uint8_t reconnect_call_1[56] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // dwShareMode
   0x02, 0x00, 0x00, 0x00,
   // dwPreferredProtocols
   0x01, 0x00, 0x00, 0x00,
   // dwInitialization
   0x01, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t reconnect_return_1[8] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // dwActiveProtocol
   0x02, 0x00, 0x00, 0x00
};

static const uint8_t reconnect_return_2[8] = {
   // ReturnCode
   0x01, 0x00, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_reconnect)
{
   {
      scard_pack_call_reconnect call(0x0102030405060708,
         0x090A0B0C0D0E0F10, SCARD_SHARE_SHARED,
         SCARD_PROTOCOL_T0, SCARD_RESET_CARD);
      
      RED_CHECK_EQUAL(padded_size(
            // Context
            + packed_native_context_size
            // Card
            + packed_native_handle_size
            // dwShareMode
            + 4
            // dwPreferredProtocols
            + 4
            // dwInitialization
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(reconnect_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(reconnect_call_1, sizeof(reconnect_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_return_reconnect return_;

      InStream stream(reconnect_return_1);

      RED_CHECK_EQUAL(sizeof(reconnect_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(reconnect_return_1, sizeof(reconnect_return_1)),
         stream.get_consumed_bytes()
      );

      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_EQUAL(2, return_.dwActiveProtocol());
   }
   {
      scard_pack_return_reconnect return_;

      InStream stream(reconnect_return_2);

      RED_CHECK_EQUAL(sizeof(reconnect_return_2), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(reconnect_return_2, sizeof(reconnect_return_2)),
         stream.get_consumed_bytes()
      );

      RED_CHECK_EQUAL(1, return_.ReturnCode());
   }
}


static const uint8_t disconnect_call_1[48] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // dwDisposition
   0x01, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t disconnect_return_1[8] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t disconnect_return_2[8] = {
   // ReturnCode
   0x01, 0x00, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_disconnect)
{
   {
      scard_pack_call_disconnect call(0x0102030405060708,
         0x090A0B0C0D0E0F10, SCARD_RESET_CARD);
      
      RED_CHECK_EQUAL(padded_size(
            // Context
            + packed_native_context_size
            // Card
            + packed_native_handle_size
            // dwDisposition
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(disconnect_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(disconnect_call_1, sizeof(disconnect_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_return_disconnect return_;

      InStream stream(disconnect_return_1);

      RED_CHECK_EQUAL(sizeof(disconnect_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(disconnect_return_1, sizeof(disconnect_return_1)),
         stream.get_consumed_bytes()
      );

      RED_CHECK_EQUAL(0, return_.ReturnCode());
   }
   {
      scard_pack_return_reconnect return_;

      InStream stream(disconnect_return_2);

      RED_CHECK_EQUAL(sizeof(disconnect_return_2), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(disconnect_return_2, sizeof(disconnect_return_2)),
         stream.get_consumed_bytes()
      );

      RED_CHECK_EQUAL(1, return_.ReturnCode());
   }
}


static const uint8_t begin_transaction_call_1[48] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // dwDisposition (ignored)
   0x00, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t begin_transaction_return_1[8] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t begin_transaction_return_2[8] = {
   // ReturnCode
   0x01, 0x00, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_begin_transaction)
{
   {
      scard_pack_call_begin_transaction call(0x0102030405060708,
         0x090A0B0C0D0E0F10);
      
      RED_CHECK_EQUAL(padded_size(
            // Context
            + packed_native_context_size
            // Card
            + packed_native_handle_size
            // dwDisposition
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(begin_transaction_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(begin_transaction_call_1, sizeof(begin_transaction_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_return_begin_transaction return_;

      InStream stream(begin_transaction_return_1);

      RED_CHECK_EQUAL(sizeof(begin_transaction_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(disconnect_return_1, sizeof(begin_transaction_return_1)),
         stream.get_consumed_bytes()
      );

      RED_CHECK_EQUAL(0, return_.ReturnCode());
   }
   {
      scard_pack_return_reconnect return_;

      InStream stream(begin_transaction_return_2);

      RED_CHECK_EQUAL(sizeof(begin_transaction_return_2), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(begin_transaction_return_2, sizeof(begin_transaction_return_2)),
         stream.get_consumed_bytes()
      );

      RED_CHECK_EQUAL(1, return_.ReturnCode());
   }
}


static const uint8_t end_transaction_call_1[48] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // dwDisposition
   0x01, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t end_transaction_return_1[8] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t end_transaction_return_2[8] = {
   // ReturnCode
   0x01, 0x00, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_end_transaction)
{
   {
      scard_pack_call_end_transaction call(0x0102030405060708,
         0x090A0B0C0D0E0F10, SCARD_RESET_CARD);
      
      RED_CHECK_EQUAL(padded_size(
            // Context
            + packed_native_context_size
            // Card
            + packed_native_handle_size
            // dwDisposition
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(end_transaction_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(end_transaction_call_1, sizeof(end_transaction_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_return_end_transaction return_;

      InStream stream(end_transaction_return_1);

      RED_CHECK_EQUAL(sizeof(end_transaction_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(disconnect_return_1, sizeof(end_transaction_return_1)),
         stream.get_consumed_bytes()
      );

      RED_CHECK_EQUAL(0, return_.ReturnCode());
   }
   {
      scard_pack_return_reconnect return_;

      InStream stream(end_transaction_return_2);

      RED_CHECK_EQUAL(sizeof(end_transaction_return_2), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(end_transaction_return_2, sizeof(end_transaction_return_2)),
         stream.get_consumed_bytes()
      );

      RED_CHECK_EQUAL(1, return_.ReturnCode());
   }
}


static const uint8_t status_call_1[56] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // fmszReaderNamesIsNULL
   0x01, 0x00, 0x00, 0x00,
   // cchReaderLen
   0x00, 0x00, 0x00, 0x00,
   // cbAtrLen
   0x00, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t status_call_2[56] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // fmszReaderNamesIsNULL
   0x00, 0x00, 0x00, 0x00,
   // cchReaderLen
   0x00, 0x00, 0x00, 0x00,
   // cbAtrLen
   0x00, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t status_call_3[56] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // fmszReaderNamesIsNULL
   0x01, 0x00, 0x00, 0x00,
   // cchReaderLen
   0x0A, 0x00, 0x00, 0x00,
   // cbAtrLen
   0x00, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t status_call_4[56] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // fmszReaderNamesIsNULL
   0x00, 0x00, 0x00, 0x00,
   // cchReaderLen
   0x0A, 0x00, 0x00, 0x00,
   // cbAtrLen
   0x00, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t status_return_1[72] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // ReaderNames
   0x07, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // dwState
   0x01, 0x00, 0x00, 0x00,
   // dwProtocol
   0x02, 0x00, 0x00, 0x00,
   // pbAtr
   0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08,
   0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   // cbAtrLen
   0x10, 0x00, 0x00, 0x00,
   // ReaderNames (deferred)
   0x07, 0x00, 0x00, 0x00,
   'r', 'e', 'a', 'd', 'e', 'r', 0x00,
   0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t status_return_2[8] = {
   // ReturnCode
   0x01, 0x00, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_status)
{
   {
      scard_pack_call_status call(0x0102030405060708,
         0x090A0B0C0D0E0F10, nullptr, nullptr, false);
      
      RED_CHECK_EQUAL(padded_size(
            // Context
            + packed_native_context_size
            // Card
            + packed_native_handle_size
            // fmszReaderNamesIsNULL
            + 4
            // cchReaderLen
            + 4
            // cbAtrLen
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(status_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(status_call_1, sizeof(status_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      const char *mszReaderNames = "reader";

      scard_pack_call_status call(0x0102030405060708,
         0x090A0B0C0D0E0F10, mszReaderNames, nullptr, false);
      
      RED_CHECK_EQUAL(padded_size(
            // Context
            + packed_native_context_size
            // Card
            + packed_native_handle_size
            // fmszReaderNamesIsNULL
            + 4
            // cchReaderLen
            + 4
            // cbAtrLen
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(status_call_2), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(status_call_2, sizeof(status_call_2)),
         stream.get_produced_bytes()
      );
   }
   {
      unsigned long cchReaderLen = 10;

      scard_pack_call_status call(0x0102030405060708,
         0x090A0B0C0D0E0F10, nullptr, &cchReaderLen, false);
      
      RED_CHECK_EQUAL(padded_size(
            // Context
            + packed_native_context_size
            // Card
            + packed_native_handle_size
            // fmszReaderNamesIsNULL
            + 4
            // cchReaderLen
            + 4
            // cbAtrLen
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(status_call_3), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(status_call_3, sizeof(status_call_3)),
         stream.get_produced_bytes()
      );
   }
   {
      const char *mszReaderNames = "reader";
      unsigned long cchReaderLen = 10;

      scard_pack_call_status call(0x0102030405060708,
         0x090A0B0C0D0E0F10, mszReaderNames, &cchReaderLen, false);
      
      RED_CHECK_EQUAL(padded_size(
            // Context
            + packed_native_context_size
            // Card
            + packed_native_handle_size
            // fmszReaderNamesIsNULL
            + 4
            // cchReaderLen
            + 4
            // cbAtrLen
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(status_call_4), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(status_call_4, sizeof(status_call_4)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_return_status return_;

      InStream stream(status_return_1);

      RED_CHECK_EQUAL(sizeof(status_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(status_return_1, sizeof(status_return_1)),
         stream.get_consumed_bytes()
      );

      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_EQUAL(7, return_.cBytes());
      RED_CHECK_EQUAL(
         bytes_view(&status_return_1[60], 7),
         return_.mszReaderNames()
      );
      RED_CHECK_EQUAL(1, return_.dwState());
      RED_CHECK_EQUAL(2, return_.dwProtocol());
      RED_CHECK(return_.pbAtr());
      RED_CHECK_EQUAL(16, return_.cbAtrLen());
      RED_CHECK_EQUAL(
         bytes_view(&status_return_1[20], 16),
         bytes_view(return_.pbAtr(), return_.cbAtrLen())
      );
   }
   {
      scard_pack_return_status return_;

      InStream stream(status_return_2);

      RED_CHECK_EQUAL(sizeof(status_return_2), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(status_return_2, sizeof(status_return_2)),
         stream.get_consumed_bytes()
      );

      RED_CHECK_EQUAL(1, return_.ReturnCode());
      RED_CHECK_EQUAL(0, return_.cBytes());
      RED_CHECK_EQUAL(bytes_view(), return_.mszReaderNames());
      RED_CHECK_EQUAL(0, return_.dwState());
      RED_CHECK_EQUAL(0, return_.dwProtocol());
      RED_CHECK(return_.pbAtr());
      RED_CHECK_EQUAL(0, return_.cbAtrLen());
   }
}


static const uint8_t get_status_change_call_1[112] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // dwTimeout
   0x01, 0x00, 0x00, 0x00,
   // ReaderStates
   0x01, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // ReaderStates (deferred)
   0x01, 0x00, 0x00, 0x00,
   // ReaderState #1
   // szReader
   0x08, 0x00, 0x02, 0x00,
   // dwCurrentState
   0x00, 0x00, 0x00, 0x00,
   // dwEventState
   0x01, 0x00, 0x00, 0x00,
   // cbAtr
   0x08, 0x00, 0x00, 0x00,
   // rgbAtr
   0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   // szReader (deferred)
   0x07, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x07, 0x00, 0x00, 0x00,
   'r', 'e', 'a', 'd', 'e', 'r', 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t get_status_change_return_1[72] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // ReaderStates
   0x01, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // ReaderStates (deferred)
   0x01, 0x00, 0x00, 0x00,
   // ReaderState #1
   // dwCurrentState
   0x00, 0x00, 0x00, 0x00,
   // dwEventState
   0x01, 0x00, 0x00, 0x00,
   // cbAtr
   0x08, 0x00, 0x00, 0x00,
   // rgbAtr
   0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   // paddding
   0x00, 0x00, 0x00, 0x00,
   // paddding
   0x00, 0x00, 0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_get_status_change)
{
   {
      SCARD_READERSTATE reader_state;

      reader_state.szReader = "reader";
      reader_state.pvUserData = nullptr;
      reader_state.dwCurrentState = SCARD_STATE_UNAWARE;
      reader_state.dwEventState = SCARD_STATE_IGNORE;
      reader_state.cbAtr = 8;
      std::memcpy(reader_state.rgbAtr, &get_status_change_call_1[52], 36);

      scard_pack_call_get_status_change call(0x0102030405060708,
         1, &reader_state, 1, false);
      
      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // dwTimeout
            + 4
            // ReaderStates
            + packed_conformant_array_size(
               true,
               (
                  // szReader
                  packed_conformant_varying_array_size(
                     true, sizeof(char), 7
                  )
                  // dwCurrentState
                  + 4
                  // dwEventState
                  + 4
                  // cbAtr
                  + 4
                  // rgbAtr
                  + 36
               ),
               1
            )
         ),
         call.packed_size()
      );

      StaticOutStream<128> stream;

      RED_CHECK_EQUAL(sizeof(get_status_change_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(get_status_change_call_1, sizeof(get_status_change_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_return_get_status_change return_;

      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_EQUAL(0, return_.cReaders());
      RED_CHECK(return_.rgReaderStates().empty());

      InStream stream(get_status_change_return_1);

      RED_CHECK_EQUAL(sizeof(get_status_change_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(get_status_change_return_1, sizeof(get_status_change_return_1)),
         stream.get_consumed_bytes()
      );

      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_EQUAL(1, return_.cReaders());
      RED_CHECK_EQUAL(1, return_.rgReaderStates().size());
   }
}


static const uint8_t control_call_1[80] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // dwControlCode
   0x01, 0x00, 0x00, 0x00,
   // InBuffer
   0x10, 0x00, 0x00, 0x00,
   0x08, 0x00, 0x02, 0x00,
   // fpvOutBufferIsNULL
   0x01, 0x00, 0x00, 0x00,
   // cbOutBufferSize
   0x00, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // InBuffer (deferred)
   0x10, 0x00, 0x00, 0x00,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static const uint8_t control_call_2[64] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // dwControlCode
   0x01, 0x00, 0x00, 0x00,
   // InBuffer
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   // fpvOutBufferIsNULL
   0x00, 0x00, 0x00, 0x00,
   // cbOutBufferSize
   0xFF, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t control_call_3[80] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // dwControlCode
   0x01, 0x00, 0x00, 0x00,
   // InBuffer
   0x10, 0x00, 0x00, 0x00,
   0x08, 0x00, 0x02, 0x00,
   // fpvOutBufferIsNULL
   0x00, 0x00, 0x00, 0x00,
   // cbOutBufferSize
   0xFF, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // InBuffer (deferred)
   0x10, 0x00, 0x00, 0x00,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static const uint8_t control_return_1[32] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // OutBuffer
   0x10, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // OutBuffer (deferred)
   0x10, 0x00, 0x00, 0x00,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static const uint8_t control_return_2[16] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // OutBuffer
   0x10, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_control)
{
   {
      scard_pack_call_control call(0x0102030405060708,
         0x090A0B0C0D0E0F10, 1, &control_call_1[64], 16,
         nullptr, 0xFF);
      
      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // Card
            + packed_native_handle_size
            // dwControlCode
            + 4
            // InBuffer
            + packed_conformant_array_size(
               true, 16, 1
            )
            // fpvOutBufferIsNULL
            + 4
            // cbOutBufferSize
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<128> stream;

      RED_CHECK_EQUAL(sizeof(control_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(control_call_1, sizeof(control_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      uint8_t OutBuffer[0xFF];

      scard_pack_call_control call(0x0102030405060708,
         0x090A0B0C0D0E0F10, 1, nullptr, 0,
         &OutBuffer, sizeof(OutBuffer));
      
      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // Card
            + packed_native_handle_size
            // dwControlCode
            + 4
            // InBuffer
            + packed_null_array_size(true)
            // fpvOutBufferIsNULL
            + 4
            // cbOutBufferSize
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<128> stream;

      RED_CHECK_EQUAL(sizeof(control_call_2), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(control_call_2, sizeof(control_call_2)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_call_control call(0x0102030405060708,
         0x090A0B0C0D0E0F10, 1, &control_call_3[64], 16,
         control_call_3, 0xFF);
      
      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // Card
            + packed_native_handle_size
            // dwControlCode
            + 4
            // InBuffer
            + packed_conformant_array_size(
               true, 16, 1
            )
            // fpvOutBufferIsNULL
            + 4
            // cbOutBufferSize
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<128> stream;

      RED_CHECK_EQUAL(sizeof(control_call_3), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(control_call_3, sizeof(control_call_3)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_return_control return_;

      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_EQUAL(0, return_.cbOutBufferSize());
      RED_CHECK(!return_.pvOutBuffer());

      InStream stream(control_return_1);

      RED_CHECK_EQUAL(sizeof(control_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(control_return_1, sizeof(control_return_1)),
         stream.get_consumed_bytes()
      );

      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_EQUAL(16, return_.cbOutBufferSize());
      RED_CHECK(return_.pvOutBuffer());
      RED_CHECK_EQUAL(
         bytes_view(&control_return_1[16], 16),
         bytes_view(return_.pvOutBuffer(), return_.cbOutBufferSize())
      );
   }
   {
      scard_pack_return_control return_;

      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_EQUAL(0, return_.cbOutBufferSize());
      RED_CHECK(!return_.pvOutBuffer());

      InStream stream(control_return_2);

      RED_CHECK_EQUAL(sizeof(control_return_2), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(control_return_2, sizeof(control_return_2)),
         stream.get_consumed_bytes()
      );

      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_EQUAL(16, return_.cbOutBufferSize());
      // FIXME
      //RED_CHECK(!return_.pvOutBuffer());
   }
}


static const uint8_t transmit_call_1[144] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // ioSendPci
   0x01, 0x00, 0x00, 0x00,
   0x05, 0x00, 0x00, 0x00,
   0x08, 0x00, 0x02, 0x00,
   // SendBuffer
   0x20, 0x00, 0x00, 0x00,
   0x0C, 0x00, 0x02, 0x00,
   // pioRecvPci
   0x10, 0x00, 0x02, 0x00,
   // fpbRecvBufferIsNULL
   0x00, 0x00, 0x00, 0x00,
   // cbRecvLength
   0x20, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // ioSendPci (deferred)
   0x05, 0x00, 0x00, 0x00,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0x00, 0x00, 0x00,
   // SendBuffer (deferred)
   0x20, 0x00, 0x00, 0x00,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   // pioRecvPci (deferred)
   0x01, 0x00, 0x00, 0x00,
   0x05, 0x00, 0x00, 0x00,
   0x14, 0x00, 0x02, 0x00,
   0x05, 0x00, 0x00, 0x00,
   0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
   // padding
   0x00, 0x00, 0x00,
};

static const uint8_t transmit_return_1[] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // pioRecvPci
   0x00, 0x00, 0x02, 0x00,
   // cbRecvLength
   0x20, 0x00, 0x00, 0x00,
   // pbRecvBuffer
   0x04, 0x00, 0x02, 0x00,
   // pioRecvPci (deferred)
   0x01, 0x00, 0x00, 0x00,
   0x05, 0x00, 0x00, 0x00,
   0x08, 0x00, 0x02, 0x00,
   0x05, 0x00, 0x00, 0x00,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0x00, 0x00, 0x00,
   // pbRecvBuffer (deferred)
   0x20, 0x00, 0x00, 0x00,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   // padding
   0x00, 0x00, 0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_transmit)
{
   {
      std::array<uint8_t, sizeof(SCARD_IO_REQUEST) + 5> ioSendPci;
      {
         SCARD_IO_REQUEST header;
         {
            header.dwProtocol = 0x01;
            header.cbPciLength = ioSendPci.size();
         }

         // write PCI header
         std::memcpy(ioSendPci.data(), reinterpret_cast<const uint8_t *>(&header),
            sizeof(SCARD_IO_REQUEST));
         
         // write PCI data
         std::memset(ioSendPci.data() + sizeof(SCARD_IO_REQUEST), 0xFF, 5);
      }

      const SCARD_IO_REQUEST *pioSendPci = reinterpret_cast<const SCARD_IO_REQUEST *>(&ioSendPci);

      uint8_t pbSendBuffer[32];
      {
         std::memset(pbSendBuffer, 0xFF, sizeof(pbSendBuffer));
      }

      unsigned long cbSendLength = sizeof(pbSendBuffer);

      std::array<uint8_t, sizeof(SCARD_IO_REQUEST) + 5> ioRecvPci;
      {
         SCARD_IO_REQUEST header;
         {
            header.dwProtocol = 0x01;
            header.cbPciLength = ioRecvPci.size();
         }

         // write PCI header
         std::memcpy(ioRecvPci.data(), reinterpret_cast<const uint8_t *>(&header),
            sizeof(SCARD_IO_REQUEST));
         
         // write PCI data
         std::memset(ioRecvPci.data() + sizeof(SCARD_IO_REQUEST), 0xCC, 5);
      }

      SCARD_IO_REQUEST *pioRecvPci = reinterpret_cast<SCARD_IO_REQUEST *>(&ioRecvPci);

      uint8_t pbRecvBuffer[32];
      {
         std::memset(pbRecvBuffer, 0x00, sizeof(pbRecvBuffer));
      }

      unsigned long pcbRecvLength = sizeof(pbRecvBuffer);

      scard_pack_call_transmit call(0x0102030405060708,
         0x090A0B0C0D0E0F10, pioSendPci, pbSendBuffer, cbSendLength,
         pioRecvPci, pbRecvBuffer, &pcbRecvLength);

      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // Card
            + packed_native_handle_size
            // ioSendPci
            + (
               // dwProtocol
               4
               // ExtraBytes
               + packed_conformant_array_size(
                  true, 1, 5
               )
            )
            // SendBuffer
            + packed_conformant_array_size(
               true, 1, 32
            )
            // pioRecvPci
            + (
               + packed_fixed_array_size(
                  (
                     // dwProtocol
                     4
                     // ExtraBytes
                     + packed_conformant_array_size(
                        true, 1, 5
                     )
                  ),
                  1
               )
            )
            // fpbRecvBufferIsNULL
            + 4
            // cbRecvLength
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<256> stream;

      RED_CHECK_EQUAL(sizeof(transmit_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(transmit_call_1, sizeof(transmit_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_return_transmit return_;

      RED_CHECK_EQUAL(0, return_.ReturnCode());
      //RED_CHECK_EQUAL(0, return_.ioRecvPci());
      RED_CHECK_EQUAL(0, return_.cbRecvLength());
      RED_CHECK(!return_.pbRecvBuffer());

      InStream stream(transmit_return_1);

      RED_CHECK_EQUAL(sizeof(transmit_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(transmit_return_1, sizeof(transmit_return_1)),
         stream.get_consumed_bytes()
      );
      RED_CHECK_EQUAL(0, return_.ReturnCode());
      //
      RED_CHECK_EQUAL(32, return_.cbRecvLength());
      RED_CHECK(return_.pbRecvBuffer());
      /*
      RED_CHECK_EQUAL(
         bytes_view(&list_reader_groups_return_1[16], 10),
         bytes_view(return_.mszGroups(), return_.cBytes())
      );
      */
   }
}


static const uint8_t list_reader_groups_call_1[32] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // fmszGroupsIsNULL
   0x01, 0x00, 0x00, 0x00,
   // cchGroups
   0x00, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t list_reader_groups_call_2[32] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // fmszGroupsIsNULL
   0x00, 0x00, 0x00, 0x00,
   // cchGroups
   0xFF, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t list_reader_groups_return_1[32] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // Groups
   0x0A, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   0x0A, 0x00, 0x00, 0x00,
   'T', 'e', 's', 't', 'G', 'r', 'o', 'u', 'p', 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const uint8_t list_reader_groups_return_2[8] = {
   // ReturnCode
   0x01, 0x00, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_list_reader_groups)
{
   {
      scard_pack_call_list_reader_groups call(0x0102030405060708,
         nullptr, 0, false);
      
      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // fmszGroupsIsNULL
            + 4
            // cchGroups
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(list_reader_groups_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(list_reader_groups_call_1, sizeof(list_reader_groups_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_call_list_reader_groups call(0x0102030405060708,
         "groups", 0xFF, false);
      
      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // fmszGroupsIsNULL
            + 4
            // cchGroups
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(list_reader_groups_call_2), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(list_reader_groups_call_2, sizeof(list_reader_groups_call_2)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_return_list_reader_groups return_(false);

      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_EQUAL(0, return_.cBytes());
      RED_CHECK(!return_.mszGroups());
      RED_CHECK_EQUAL(padded_size(
            // ReturnCode
            4
            // Groups
            + packed_null_array_size(true)
         ),
         return_.packed_size()
      );

      InStream stream(list_reader_groups_return_1);

      RED_CHECK_EQUAL(sizeof(list_reader_groups_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(list_reader_groups_return_1, sizeof(list_reader_groups_return_1)),
         stream.get_consumed_bytes()
      );
      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_EQUAL(10, return_.cBytes());
      RED_CHECK(return_.mszGroups());
      RED_CHECK_EQUAL(
         bytes_view(&list_reader_groups_return_1[16], 10),
         bytes_view(return_.mszGroups(), return_.cBytes())
      );
   }
   {
      scard_pack_return_list_reader_groups return_(false);

      InStream stream(list_reader_groups_return_2);

      RED_CHECK_EQUAL(sizeof(list_reader_groups_return_2), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(list_reader_groups_return_2, sizeof(list_reader_groups_return_2)),
         stream.get_consumed_bytes()
      );
      RED_CHECK_EQUAL(1, return_.ReturnCode());
      RED_CHECK_EQUAL(0, return_.cBytes());
      RED_CHECK(!return_.mszGroups());
   }
}


static const uint8_t list_readers_call_1[40] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Groups
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   // fmszReadersIsNULL
   0x01, 0x00, 0x00, 0x00,
   // cchReaders
   0x00, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
};
static const uint8_t list_readers_call_2[64] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Groups
   0x11, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // fmszReadersIsNULL
   0x01, 0x00, 0x00, 0x00,
   // cchReaders
   0x00, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Groups (deferred)
   0x11, 0x00, 0x00, 0x00,
   'S', 'C', 'a', 'r', 'd', '$', 'A', 'l', 'l', 'R', 'e', 'a', 'd', 'e', 'r', 's', 0x00,
};
static const uint8_t list_readers_call_3[40] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Groups
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   // fmszReadersIsNULL
   0x00, 0x00, 0x00, 0x00,
   // cchReaders
   0x00, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // padding
   0x00, 0x00, 0x00, 0x00
};
static const uint8_t list_readers_call_4[40] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Groups
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   // fmszReadersIsNULL
   0x00, 0x00, 0x00, 0x00,
   // cchReaders
   0x04, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // padding
   0x00, 0x00, 0x00, 0x00
};
static const uint8_t list_readers_return_1[32] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // Readers
   0x0B, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   0x0B, 0x00, 0x00, 0x00,
   'T', 'e', 's', 't', 'R', 'e', 'a', 'd', 'e', 'r', 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};
static const uint8_t list_readers_return_2[8] = {
   // ReturnCode
   0x01, 0x00, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_list_readers)
{
   {
      scard_pack_call_list_readers call(0x0102030405060708,
         nullptr, nullptr, 0, false);

      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // Groups
            + packed_null_array_size(true)
            // fmszReadersIsNULL
            + 4
            // cchReaders
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(list_readers_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(list_readers_call_1, sizeof(list_readers_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      const char groups[17] = "SCard$AllReaders";

      scard_pack_call_list_readers call(0x0102030405060708,
         groups, nullptr, 0, false);

      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // Groups
            + packed_unique_array_size(sizeof(groups))
            // fmszReadersIsNULL
            + 4
            // cchReaders
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(list_readers_call_2), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(list_readers_call_2, sizeof(list_readers_call_2)),
         stream.get_produced_bytes()
      );
   }
   {
      char readers[4] = {
         0x11, 0x22, 0x33, 0x44
      };

      scard_pack_call_list_readers call(0x0102030405060708,
         nullptr, readers, 0, false);

      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // Groups
            + packed_null_array_size(true)
            // fmszReadersIsNULL
            + 4
            // cchReaders
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(list_readers_call_3), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(list_readers_call_3, sizeof(list_readers_call_3)),
         stream.get_produced_bytes()
      );
   }
   {
      char readers[4] = {
         0x11, 0x22, 0x33, 0x44
      };

      scard_pack_call_list_readers call(0x0102030405060708,
         nullptr, readers, sizeof(readers), false);

      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // Groups
            + packed_null_array_size(true)
            // fmszReadersIsNULL
            + 4
            // cchReaders
            + 4                                    
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(list_readers_call_4), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(list_readers_call_4, sizeof(list_readers_call_4)),
         stream.get_produced_bytes()
      );
   }
   
   {
      scard_pack_return_list_readers return_(false);

      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_EQUAL(0, return_.cBytes());
      RED_CHECK(!return_.mszReaders());
      RED_CHECK_EQUAL(padded_size(
            // ReturnCode
            4
            // Readers
            + packed_null_array_size(true)
         ),
         return_.packed_size()
      );

      InStream stream(list_readers_return_1);

      RED_CHECK_EQUAL(sizeof(list_readers_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(list_readers_return_1, sizeof(list_readers_return_1)),
         stream.get_consumed_bytes()
      );
      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_EQUAL(11, return_.cBytes());
      RED_CHECK(return_.mszReaders());
      RED_CHECK_EQUAL(
         bytes_view(&list_readers_return_1[16], 11),
         bytes_view(return_.mszReaders(), return_.cBytes())
      );
   }
   {
      scard_pack_return_list_readers return_(false);

      InStream stream(list_readers_return_2);

      RED_CHECK_EQUAL(sizeof(list_readers_return_2), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(list_readers_return_2, sizeof(list_readers_return_2)),
         stream.get_consumed_bytes()
      );
      RED_CHECK_EQUAL(1, return_.ReturnCode());
      RED_CHECK_EQUAL(0, return_.cBytes());
      RED_CHECK(!return_.mszReaders());
   }
}


static const uint8_t cancel_call_1[24] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t cancel_return_1[8] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_cancel)
{
   {
      scard_pack_call_cancel call(0x0102030405060708);

      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(cancel_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(cancel_call_1, sizeof(cancel_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_return_cancel return_;

      InStream stream(cancel_return_1);

      RED_CHECK_EQUAL(sizeof(cancel_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(cancel_return_1, sizeof(cancel_return_1)),
         stream.get_consumed_bytes()
      );
      RED_CHECK_EQUAL(0, return_.ReturnCode());
   }
}


static const uint8_t get_attrib_call_1[56] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // dwAttrId
   0x01, 0x00, 0x00, 0x00,
   // fpbAttrIsNULL
   0x01, 0x00, 0x00, 0x00,
   // cbAttrLen
   0x00, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t get_attrib_call_2[56] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // dwAttrId
   0x01, 0x00, 0x00, 0x00,
   // fpbAttrIsNULL
   0x00, 0x00, 0x00, 0x00,
   // cbAttrLen
   0x00, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t get_attrib_call_3[56] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // dwAttrId
   0x01, 0x00, 0x00, 0x00,
   // fpbAttrIsNULL
   0x01, 0x00, 0x00, 0x00,
   // cbAttrLen
   0x00, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t get_attrib_call_4[56] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // dwAttrId
   0x01, 0x00, 0x00, 0x00,
   // fpbAttrIsNULL
   0x00, 0x00, 0x00, 0x00,
   // cbAttrLen
   0xFF, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t get_attrib_return_1[32] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // Attr
   0x10, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Attr (deferred)
   0x10, 0x00, 0x00, 0x00,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

RED_AUTO_TEST_CASE(test_scard_pack_get_attrib)
{
   {
      scard_pack_call_get_attrib call(0x0102030405060708,
         0x090A0B0C0D0E0F10, 1, nullptr, nullptr);

      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // Card
            + packed_native_handle_size
            // dwAttrId
            + 4
            // fpbAttrIsNULL
            + 4
            // cbAttrLen
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(get_attrib_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(get_attrib_call_1, sizeof(get_attrib_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      unsigned char pbAttr;

      scard_pack_call_get_attrib call(0x0102030405060708,
         0x090A0B0C0D0E0F10, 1, &pbAttr, nullptr);

      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // Card
            + packed_native_handle_size
            // dwAttrId
            + 4
            // fpbAttrIsNULL
            + 4
            // cbAttrLen
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(get_attrib_call_2), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(get_attrib_call_2, sizeof(get_attrib_call_2)),
         stream.get_produced_bytes()
      );
   }
   {
      unsigned long cbAttrLen = 0xFF;

      scard_pack_call_get_attrib call(0x0102030405060708,
         0x090A0B0C0D0E0F10, 1, nullptr, &cbAttrLen);

      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // Card
            + packed_native_handle_size
            // dwAttrId
            + 4
            // fpbAttrIsNULL
            + 4
            // cbAttrLen
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(get_attrib_call_3), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(get_attrib_call_3, sizeof(get_attrib_call_3)),
         stream.get_produced_bytes()
      );
   }
   {
      unsigned char pbAttr;
      unsigned long cbAttrLen = 0xFF;

      scard_pack_call_get_attrib call(0x0102030405060708,
         0x090A0B0C0D0E0F10, 1, &pbAttr, &cbAttrLen);

      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // Card
            + packed_native_handle_size
            // dwAttrId
            + 4
            // fpbAttrIsNULL
            + 4
            // cbAttrLen
            + 4
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(get_attrib_call_4), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(get_attrib_call_4, sizeof(get_attrib_call_4)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_return_get_attrib return_;

      InStream stream(get_attrib_return_1);

      RED_CHECK_EQUAL(sizeof(get_attrib_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(get_attrib_return_1, sizeof(get_attrib_return_1)),
         stream.get_consumed_bytes()
      );
      RED_CHECK_EQUAL(0, return_.ReturnCode());
      RED_CHECK_EQUAL(16, return_.cbAttrLen());
      RED_CHECK(return_.pbAttr());
      RED_CHECK_EQUAL(
         bytes_view(&get_attrib_return_1[16], 16),
         bytes_view(return_.pbAttr(), return_.cbAttrLen())
      );
   }
}


static const uint8_t set_attrib_call_1[56] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // dwAttrId
   0x01, 0x00, 0x00, 0x00,
   // Attr
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // padding
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t set_attrib_call_2[72] = {
   // Context
   0x08, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x02, 0x00,
   // Card
   0x08, 0x00, 0x00, 0x00,
   0x04, 0x00, 0x02, 0x00,
   // dwAttrId
   0x01, 0x00, 0x00, 0x00,
   // Attr
   0x10, 0x00, 0x00, 0x00,
   0x08, 0x00, 0x02, 0x00,
   // Context (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01,
   // Card (deferred)
   0x08, 0x00, 0x00, 0x00,
   0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
   // Attr (deferred)
   0x10, 0x00, 0x00, 0x00,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static const uint8_t set_attrib_return_1[8] = {
   // ReturnCode
   0x00, 0x00, 0x00, 0x00,
   // padding
   0x00, 0x00, 0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_set_attrib)
{
   {
      scard_pack_call_set_attrib call(0x0102030405060708,
         0x090A0B0C0D0E0F10, 1, nullptr, 0);

      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // Card
            + packed_native_handle_size
            // dwAttrId
            + 4
            // Attr
            + packed_null_array_size(true)
         ),
         call.packed_size()
      );

      StaticOutStream<64> stream;

      RED_CHECK_EQUAL(sizeof(set_attrib_call_1), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(set_attrib_call_1, sizeof(set_attrib_call_1)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_call_set_attrib call(0x0102030405060708,
         0x090A0B0C0D0E0F10, 1, &set_attrib_call_2[56], 16);

      RED_CHECK_EQUAL(padded_size(
            // Context
            packed_native_context_size
            // Card
            + packed_native_handle_size
            // dwAttrId
            + 4
            // Attr
            + packed_conformant_array_size(
               true, 1, 16
            )
         ),
         call.packed_size()
      );

      StaticOutStream<128> stream;

      RED_CHECK_EQUAL(sizeof(set_attrib_call_2), call.pack(stream));
      RED_CHECK_EQUAL(
         bytes_view(set_attrib_call_2, sizeof(set_attrib_call_2)),
         stream.get_produced_bytes()
      );
   }
   {
      scard_pack_return_set_attrib return_;

      InStream stream(set_attrib_return_1);

      RED_CHECK_EQUAL(sizeof(set_attrib_return_1), return_.unpack(stream));
      RED_CHECK_EQUAL(
         bytes_view(set_attrib_return_1, sizeof(set_attrib_return_1)),
         stream.get_consumed_bytes()
      );
      RED_CHECK_EQUAL(0, return_.ReturnCode());
   }
}