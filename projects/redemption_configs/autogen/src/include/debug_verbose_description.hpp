//
// DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN
//

namespace cfg_specs {

constexpr char const* CONFIG_DESC_AUTH = R"(- variable = 0x0002
- buffer   = 0x0040
- dump     = 0x1000)";

constexpr char const* CONFIG_DESC_FRONT = R"(- basic_trace     = 0x00000001
- basic_trace2    = 0x00000002
- basic_trace3    = 0x00000004
- basic_trace4    = 0x00000008
- basic_trace5    = 0x00000020
- graphic         = 0x00000040
- channel         = 0x00000080
- cache_from_disk = 0x00000400
- bmp_info        = 0x00000800
- global_channel  = 0x00002000
- sec_decrypted   = 0x00004000
- keymap          = 0x00008000
- keymap_and_basic_trace3 = keymap | basic_trace3

(Serializer)
- pointer             = 0x00040000
- primary_orders      = 0x00200000
- secondary_orders    = 0x00400000
- bitmap_update       = 0x00800000
- surface_commands    = 0x01000000
- bmp_cache           = 0x02000000
- internal_buffer     = 0x04000000
- sec_decrypted       = 0x10000000)";

constexpr char const* CONFIG_DESC_RDP = R"(- basic_trace         = 0x00000001
- connection          = 0x00000002
- security            = 0x00000004
- capabilities        = 0x00000008
- license             = 0x00000010
- asynchronous_task   = 0x00000020
- graphics_pointer    = 0x00000040
- graphics            = 0x00000080
- input               = 0x00000100
- rail_order          = 0x00000200
- credssp             = 0x00000400
- negotiation         = 0x00000800
- cache_persister     = 0x00001000
- fsdrvmgr            = 0x00002000
- sesprobe_launcher   = 0x00004000
- sesprobe_repetitive = 0x00008000
- drdynvc             = 0x00010000
- surfaceCmd          = 0x00020000
- cache_from_disk     = 0x00040000
- bmp_info            = 0x00080000
- drdynvc_dump        = 0x00100000
- printer             = 0x00200000
- rdpsnd              = 0x00400000
- channels            = 0x00800000
- rail                = 0x01000000
- sesprobe            = 0x02000000
- cliprdr             = 0x04000000
- rdpdr               = 0x08000000
- rail_dump           = 0x10000000
- sesprobe_dump       = 0x20000000
- cliprdr_dump        = 0x40000000
- rdpdr_dump          = 0x80000000)";

constexpr char const* CONFIG_DESC_VNC = R"(- basic_trace     = 0x00000001
- keymap_stack    = 0x00000002
- draw_event      = 0x00000004
- input           = 0x00000008
- connection      = 0x00000010
- hextile_encoder = 0x00000020
- cursor_encoder  = 0x00000040
- clipboard       = 0x00000080
- zrle_encoder    = 0x00000100
- zrle_trace      = 0x00000200
- hextile_trace   = 0x00000400
- cursor_trace    = 0x00001000
- rre_encoder     = 0x00002000
- rre_trace       = 0x00004000
- raw_encoder     = 0x00008000
- raw_trace       = 0x00010000
- copyrect_encoder= 0x00020000
- copyrect_trace  = 0x00040000
- keymap          = 0x00080000)";

constexpr char const* CONFIG_DESC_SCK = R"(- basic    = 0x0001
- dump     = 0x0002
- watchdog = 0x0004)";

constexpr char const* CONFIG_DESC_CAPTURE = R"(- kbd / ocr when != 0

(Wrm)
- pointer             = 0x0004
- primary_orders      = 0x0020
- secondary_orders    = 0x0040
- bitmap_update       = 0x0080
- surface_commands    = 0x0100
- bmp_cache           = 0x0200
- internal_buffer     = 0x0400
- sec_decrypted       = 0x1000)";

constexpr char const* CONFIG_DESC_SESSION = R"(- Event   = 0x02
- Acl     = 0x04
- Trace   = 0x08)";

constexpr char const* CONFIG_DESC_MOD_INTERNAL = R"(- copy_paste != 0
- client_execute = 0x01)";

constexpr char const* CONFIG_DESC_COMPRESSION = R"(- when != 0)";

constexpr char const* CONFIG_DESC_CACHE = R"(- life       = 0x0001
- persistent = 0x0200)";

constexpr char const* CONFIG_DESC_OCR = R"(- when != 0)";

}
