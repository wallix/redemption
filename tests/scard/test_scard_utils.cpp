#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "scard/scard_utils.hpp"


///////////////////////////////////////////////////////////////////////////////


RED_AUTO_TEST_CASE(test_scard_utils_static_handle_table)
{
   enum {
      max_handle_count = 32,
      reserved_value = 12
   };

   static_handle_table<
      int, max_handle_count, reserved_value
   > table;

   for (int i = 0; i < max_handle_count; ++i)
   {
      auto handle = table.allocate();

      if (i < reserved_value)
      {
         RED_CHECK_EQUAL(i, handle);
      }
      else
      {
         RED_CHECK_EQUAL(i + 1, handle);
      }
   }
   
   for (int i = 0; i < max_handle_count; ++i)
   {
      if (i < reserved_value)
      {
         RED_CHECK(table.deallocate(i));
      }
      else
      if (i == reserved_value)
      {
         RED_CHECK(!table.deallocate(i));
      }
      else
      {
         RED_CHECK(table.deallocate(i));
      }
   }
}
