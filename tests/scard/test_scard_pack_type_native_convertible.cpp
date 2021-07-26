#include <cstring>

#include <PCSC/wintypes.h>
#include <PCSC/winscard.h>

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "scard/scard_pack_type_native_convertible.hpp"


///////////////////////////////////////////////////////////////////////////////


static constexpr std::size_t packed_array_size(std::size_t referent_size)
{
   return (4 + 4 + (referent_size ? (4 + referent_size) : 0));
}


static const uint8_t reader_state_1[52] = {
   // szReader
   0x00, 0x00, 0x00, 0x00,
   // dwCurrentState
   0x00, 0x00, 0x00, 0x00,
   // dwEventState
   0x01, 0x00, 0x00, 0x00,
   // cbAtr
   0x00, 0x00, 0x00, 0x00,
   // rgbAtr
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t reader_state_2[72] = {
   // szReader
   0x00, 0x00, 0x02, 0x00,
   // dwCurrentState
   0x00, 0x00, 0x00, 0x00,
   // dwEventState
   0x01, 0x00, 0x00, 0x00,
   // cbAtr
   0x00, 0x00, 0x00, 0x00,
   // rgbAtr
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   // szReader (deferred)
   0x07, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x07, 0x00, 0x00, 0x00,
   'r', 'e', 'a', 'd', 'e', 'r', 0x00,
   0x00
};

static const uint8_t reader_state_3[72] = {
   // szReader
   0x00, 0x00, 0x02, 0x00,
   // dwCurrentState
   0x00, 0x00, 0x00, 0x00,
   // dwEventState
   0x01, 0x00, 0x00, 0x00,
   // cbAtr
   0x10, 0x00, 0x00, 0x00,
   // rgbAtr
   0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
   0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   // szReader (deferred)
   0x07, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x07, 0x00, 0x00, 0x00,
   'r', 'e', 'a', 'd', 'e', 'r', 0x00,
   0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_type_native_convertible_reader_state_1)
{
   SCARD_READERSTATE native_reader_state;
   {
      native_reader_state.szReader = nullptr;
      native_reader_state.pvUserData = nullptr;
      native_reader_state.dwCurrentState = SCARD_STATE_UNAWARE;
      native_reader_state.dwEventState = SCARD_STATE_IGNORE;
      native_reader_state.cbAtr = 0;
      std::memset(native_reader_state.rgbAtr, 0, 36);
   }

   scard_pack_reader_state reader_state(native_reader_state);

   {
      RED_CHECK_EQUAL((
            4              // szReader
            + 4            // dwCurrentState
            + 4            // dwEventState
            + 4            // cbAtr
            + 36           // rgbAtr
         ),
         reader_state.packed_size()
      );

      StaticOutStream<64> stream;
      uint32_t pointer_index(0);

      RED_CHECK_EQUAL((
            4              // szReader
            + 4            // dwCurrentState
            + 4            // dwEventState
            + 4            // cbAtr
            + 36           // rgbAtr 
         ),
         reader_state.pack(stream, pointer_index)
      );
      RED_CHECK_EQUAL(
         bytes_view(reader_state_1, sizeof(reader_state_1)),
         stream.get_produced_bytes()
      );
   }
   {
      StaticOutStream<64> stream;
      uint32_t pointer_index(0);

      RED_CHECK_EQUAL(0, reader_state.pack_deferred(stream, pointer_index));
      RED_CHECK_EQUAL(0, stream.get_offset());
   }
}

RED_AUTO_TEST_CASE(test_scard_pack_type_native_convertible_reader_state_2)
{
   SCARD_READERSTATE native_reader_state;
   {
      native_reader_state.szReader = "reader";
      native_reader_state.pvUserData = nullptr;
      native_reader_state.dwCurrentState = SCARD_STATE_UNAWARE;
      native_reader_state.dwEventState = SCARD_STATE_IGNORE;
      native_reader_state.cbAtr = 0;
      std::memset(native_reader_state.rgbAtr, 0, 36);
   }

   scard_pack_reader_state reader_state(native_reader_state);

   {
      RED_CHECK_EQUAL((
            4              // szReader
            + 4            // dwCurrentState
            + 4            // dwEventState
            + 4            // cbAtr
            + 36           // rgbAtr
            + (            // szReader (deferred)
               4 + 4 + 4   // header
               + 7         // value
               + 1         // padding
            )   
         ),
         reader_state.packed_size()
      );

      StaticOutStream<64> stream;
      uint32_t pointer_index(0);

      RED_CHECK_EQUAL((
            4        // szReader
            + 4      // dwCurrentState
            + 4      // dwEventState
            + 4      // cbAtr
            + 36     // rgbAtr 
         ),
         reader_state.pack(stream, pointer_index)
      );
      RED_CHECK_EQUAL(
         bytes_view(reader_state_2, 52),
         stream.get_produced_bytes()
      );
   }
   {
      StaticOutStream<64> stream;
      uint32_t pointer_index(0);

      RED_CHECK_EQUAL((
            (              // szReader (deferred)
               4 + 4 + 4   // header
               + 7         // value
               + 1         // padding
            ) 
         ),
         reader_state.pack_deferred(stream, pointer_index)
      );
      RED_CHECK_EQUAL(
         bytes_view(&reader_state_2[52], 20),
         stream.get_produced_bytes()
      );
   }
}

RED_AUTO_TEST_CASE(test_scard_pack_type_native_convertible_reader_state_3)
{
   SCARD_READERSTATE native_reader_state;
   {
      native_reader_state.szReader = "reader\0";
      native_reader_state.pvUserData = nullptr;
      native_reader_state.dwCurrentState = SCARD_STATE_UNAWARE;
      native_reader_state.dwEventState = SCARD_STATE_IGNORE;
      native_reader_state.cbAtr = 16;
      std::memcpy(native_reader_state.rgbAtr, &reader_state_3[16], 36);
   }

   scard_pack_reader_state reader_state(native_reader_state);

   {
      RED_CHECK_EQUAL((
            4              // szReader
            + 4            // dwCurrentState
            + 4            // dwEventState
            + 4            // cbAtr
            + 36           // rgbAtr
            + (            // szReader (deferred)
               4 + 4 + 4   // header
               + 7         // value
               + 1         // padding
            )   
         ),
         reader_state.packed_size()
      );

      StaticOutStream<64> stream;
      uint32_t pointer_index(0);

      RED_CHECK_EQUAL((
            4        // szReader
            + 4      // dwCurrentState
            + 4      // dwEventState
            + 4      // cbAtr
            + 36     // rgbAtr 
         ),
         reader_state.pack(stream, pointer_index)
      );
      RED_CHECK_EQUAL(
         bytes_view(reader_state_3, 52),
         stream.get_produced_bytes()
      );
   }
   {
      StaticOutStream<64> stream;
      uint32_t pointer_index(0);

      RED_CHECK_EQUAL((
            (              // szReader (deferred)
               4 + 4 + 4   // header
               + 7         // value
               + 1         // padding
            ) 
         ),
         reader_state.pack_deferred(stream, pointer_index)
      );
      RED_CHECK_EQUAL(
         bytes_view(&reader_state_2[52], 20),
         stream.get_produced_bytes()
      );
   }
}

static const uint8_t pci_1[12] = {
   // dwProtocol
   0x01, 0x00, 0x00, 0x00,
   // cbExtraBytes
   0x00, 0x00, 0x00, 0x00,
   // pbExtraBytes
   0x00, 0x00, 0x00, 0x00
};

static const uint8_t pci_2[48] = {
   // dwProtocol
   0x01, 0x00, 0x00, 0x00,
   // cbExtraBytes
   0x1E, 0x00, 0x00, 0x00,
   // pbExtraBytes
   0x00, 0x00, 0x02, 0x00,
   0x1E, 0x00, 0x00, 0x00,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
   0x00, 0x00
};

RED_AUTO_TEST_CASE(test_scard_pack_type_native_convertible_pci_1)
{
   std::array<uint8_t, sizeof(SCARD_IO_REQUEST)> native_pci_data;
   {
      SCARD_IO_REQUEST native_pci;
      {
         native_pci.dwProtocol = 0x01;
         native_pci.cbPciLength = sizeof(SCARD_IO_REQUEST);
      }

      // write PCI header
      std::memcpy(native_pci_data.data(), reinterpret_cast<const uint8_t *>(&native_pci),
        sizeof(SCARD_IO_REQUEST));
   }

   scard_pack_pci pci(reinterpret_cast<const SCARD_IO_REQUEST *>(native_pci_data.data()));

   RED_CHECK_EQUAL(0x01, pci.dwProtocol());
   RED_CHECK_EQUAL(
      bytes_view(),
      pci.ExtraBytes()
   );

   RED_CHECK_EQUAL((
         // dwProtocol
         4
         // ExtraBytes
         + 4 + 4
      ),
      pci.packed_size()
   );
   {
      StaticOutStream<64> stream;
      uint32_t pointer_index(0);

      RED_CHECK_EQUAL(12, pci.pack(stream, pointer_index));
      RED_CHECK_EQUAL(
         bytes_view(pci_1, 12),
         stream.get_produced_bytes()
      );
   }
   {
      StaticOutStream<64> stream;
      uint32_t pointer_index(0);

      RED_CHECK_EQUAL(0, pci.pack_deferred(stream, pointer_index));
      RED_CHECK_EQUAL(0, stream.get_offset());
   }
}

RED_AUTO_TEST_CASE(test_scard_pack_type_native_convertible_pci_2)
{
   std::array<uint8_t, sizeof(SCARD_IO_REQUEST) + 30> native_pci_data;
   {
      SCARD_IO_REQUEST native_pci;
      {
         native_pci.dwProtocol = 0x01;
         native_pci.cbPciLength = native_pci_data.size();
      }

      // write PCI header
      std::memcpy(native_pci_data.data(), reinterpret_cast<const uint8_t *>(&native_pci),
         sizeof(SCARD_IO_REQUEST));
      
      // write PCI data
      std::memset(native_pci_data.data() + sizeof(SCARD_IO_REQUEST), 0xFF,
         native_pci_data.size() - sizeof(SCARD_IO_REQUEST));
   }

   scard_pack_pci pci(reinterpret_cast<const SCARD_IO_REQUEST *>(native_pci_data.data()));

   RED_CHECK_EQUAL(0x01, pci.dwProtocol());
   RED_CHECK_EQUAL(
      bytes_view(native_pci_data.data() + sizeof(SCARD_IO_REQUEST),
         (native_pci_data.size() - sizeof(SCARD_IO_REQUEST))),
      pci.ExtraBytes()
   );

   RED_CHECK_EQUAL((
         // dwProtocol
         4
         // ExtraBytes
         + 4 + 4 + 4 + 30
         + 2
      ),
      pci.packed_size()
   );
   {
      StaticOutStream<64> stream;
      uint32_t pointer_index(0);

      RED_CHECK_EQUAL(12, pci.pack(stream, pointer_index));
      RED_CHECK_EQUAL(
         bytes_view(pci_2, 12),
         stream.get_produced_bytes()
      );
   }
   {
      StaticOutStream<64> stream;
      uint32_t pointer_index(0);

      RED_CHECK_EQUAL(4 + 32, pci.pack_deferred(stream, pointer_index));
      RED_CHECK_EQUAL(
         bytes_view(&pci_2[12], 36),
         stream.get_produced_bytes()
      );
   }
}