#pragma once

# include <cstdint>
# include <unordered_map>

// "sv-SE" locale 
namespace x0000041d
{
  // key : unicode, value : scancode + mod flags [8 + 8 bits]
  inline const std::unordered_map<std::uint16_t, std::pair<std::uint8_t, std::uint8_t>> SCANCODES_TABLE
  {
    {0x0000, {0x00, 0x00}},
    {0x001b, {0x01, 0x00}},
    {0x0031, {0x02, 0x00}},
    {0x0032, {0x03, 0x00}},
    {0x0033, {0x04, 0x00}},
    {0x0034, {0x05, 0x00}},
    {0x0035, {0x06, 0x00}},
    {0x0036, {0x07, 0x00}},
    {0x0037, {0x08, 0x00}},
    {0x0038, {0x09, 0x00}},
    {0x0039, {0x0a, 0x00}},
    {0x0030, {0x0b, 0x00}},
    {0x002b, {0x0c, 0x00}},
    {0x00b4, {0x0d, 0x00}},
    {0x0008, {0x0e, 0x00}},
    {0x0009, {0x0f, 0x00}},
    {0x0071, {0x10, 0x00}},
    {0x0077, {0x11, 0x00}},
    {0x0065, {0x12, 0x00}},
    {0x0072, {0x13, 0x00}},
    {0x0074, {0x14, 0x00}},
    {0x0079, {0x15, 0x00}},
    {0x0075, {0x16, 0x00}},
    {0x0069, {0x17, 0x00}},
    {0x006f, {0x18, 0x00}},
    {0x0070, {0x19, 0x00}},
    {0x00e5, {0x1a, 0x00}},
    {0x00a8, {0x1b, 0x00}},
    {0x000d, {0x1c, 0x00}},
    {0x0061, {0x1e, 0x00}},
    {0x0073, {0x1f, 0x00}},
    {0x0064, {0x20, 0x00}},
    {0x0066, {0x21, 0x00}},
    {0x0067, {0x22, 0x00}},
    {0x0068, {0x23, 0x00}},
    {0x006a, {0x24, 0x00}},
    {0x006b, {0x25, 0x00}},
    {0x006c, {0x26, 0x00}},
    {0x00f6, {0x27, 0x00}},
    {0x00e4, {0x28, 0x00}},
    {0x00a7, {0x29, 0x00}},
    {0x0027, {0x2b, 0x00}},
    {0x007a, {0x2c, 0x00}},
    {0x0078, {0x2d, 0x00}},
    {0x0063, {0x2e, 0x00}},
    {0x0076, {0x2f, 0x00}},
    {0x0062, {0x30, 0x00}},
    {0x006e, {0x31, 0x00}},
    {0x006d, {0x32, 0x00}},
    {0x002c, {0x33, 0x00}},
    {0x002e, {0x34, 0x00}},
    {0x002d, {0x35, 0x00}},
    {0x002a, {0x37, 0x00}},
    {0x0020, {0x39, 0x00}},
    {0x003c, {0x56, 0x00}},
    {0x002f, {0x62, 0x00}},
    {0x0021, {0x02, 0x01}},
    {0x0022, {0x03, 0x01}},
    {0x0023, {0x04, 0x01}},
    {0x00a4, {0x05, 0x01}},
    {0x0025, {0x06, 0x01}},
    {0x0026, {0x07, 0x01}},
    {0x0028, {0x09, 0x01}},
    {0x0029, {0x0a, 0x01}},
    {0x003d, {0x0b, 0x01}},
    {0x003f, {0x0c, 0x01}},
    {0x0060, {0x0d, 0x01}},
    {0x0051, {0x10, 0x01}},
    {0x0057, {0x11, 0x01}},
    {0x0045, {0x12, 0x01}},
    {0x0052, {0x13, 0x01}},
    {0x0054, {0x14, 0x01}},
    {0x0059, {0x15, 0x01}},
    {0x0055, {0x16, 0x01}},
    {0x0049, {0x17, 0x01}},
    {0x004f, {0x18, 0x01}},
    {0x0050, {0x19, 0x01}},
    {0x00c5, {0x1a, 0x01}},
    {0x005e, {0x1b, 0x01}},
    {0x0041, {0x1e, 0x01}},
    {0x0053, {0x1f, 0x01}},
    {0x0044, {0x20, 0x01}},
    {0x0046, {0x21, 0x01}},
    {0x0047, {0x22, 0x01}},
    {0x0048, {0x23, 0x01}},
    {0x004a, {0x24, 0x01}},
    {0x004b, {0x25, 0x01}},
    {0x004c, {0x26, 0x01}},
    {0x00d6, {0x27, 0x01}},
    {0x00c4, {0x28, 0x01}},
    {0x00bd, {0x29, 0x01}},
    {0x005a, {0x2c, 0x01}},
    {0x0058, {0x2d, 0x01}},
    {0x0043, {0x2e, 0x01}},
    {0x0056, {0x2f, 0x01}},
    {0x0042, {0x30, 0x01}},
    {0x004e, {0x31, 0x01}},
    {0x004d, {0x32, 0x01}},
    {0x003b, {0x33, 0x01}},
    {0x003a, {0x34, 0x01}},
    {0x005f, {0x35, 0x01}},
    {0x003e, {0x56, 0x01}},
    {0x007f, {0x63, 0x01}},
    {0x0040, {0x03, 0x02}},
    {0x00a3, {0x04, 0x02}},
    {0x0024, {0x05, 0x02}},
    {0x20ac, {0x06, 0x02}},
    {0x007b, {0x08, 0x02}},
    {0x005b, {0x09, 0x02}},
    {0x005d, {0x0a, 0x02}},
    {0x007d, {0x0b, 0x02}},
    {0x005c, {0x0c, 0x02}},
    {0x007e, {0x1b, 0x02}},
    {0x00b5, {0x32, 0x02}},
    {0x007c, {0x56, 0x02}},
    {0x001d, {0x1b, 0x08}},
    {0x001c, {0x29, 0x08}},
  };

  // key : extended unicode, value : extended scancode + unicode [8 + 16 bits]
  inline const std::unordered_map<std::uint16_t, std::pair<std::uint8_t, std::uint16_t>> EXTENDED_SCANCODES_TABLE
  {
    {0x00f3, {0x0d, 0x006f}},
    {0x00cd, {0x0d, 0x0049}},
    {0x00dd, {0x0d, 0x0059}},
    {0x00c1, {0x0d, 0x0041}},
    {0x00c9, {0x0d, 0x0045}},
    {0x00da, {0x0d, 0x0055}},
    {0x00b4, {0x0d, 0x0020}},
    {0x00e9, {0x0d, 0x0065}},
    {0x00fa, {0x0d, 0x0075}},
    {0x00e1, {0x0d, 0x0061}},
    {0x00fd, {0x0d, 0x0079}},
    {0x00d3, {0x0d, 0x004f}},
    {0x00ed, {0x0d, 0x0069}},
    {0x00f2, {0x0d, 0x006f}},
    {0x00cc, {0x0d, 0x0049}},
    {0x0060, {0x0d, 0x0020}},
    {0x00c0, {0x0d, 0x0041}},
    {0x00c8, {0x0d, 0x0045}},
    {0x00d9, {0x0d, 0x0055}},
    {0x00e8, {0x0d, 0x0065}},
    {0x00f9, {0x0d, 0x0075}},
    {0x00e0, {0x0d, 0x0061}},
    {0x00d2, {0x0d, 0x004f}},
    {0x00ec, {0x0d, 0x0069}},
    {0x00f6, {0x1b, 0x006f}},
    {0x00cf, {0x1b, 0x0049}},
    {0x00a8, {0x1b, 0x0020}},
    {0x00c4, {0x1b, 0x0041}},
    {0x00cb, {0x1b, 0x0045}},
    {0x00dc, {0x1b, 0x0055}},
    {0x00eb, {0x1b, 0x0065}},
    {0x00fc, {0x1b, 0x0075}},
    {0x00e4, {0x1b, 0x0061}},
    {0x00ff, {0x1b, 0x0079}},
    {0x00d6, {0x1b, 0x004f}},
    {0x00ef, {0x1b, 0x0069}},
    {0x00f4, {0x1b, 0x006f}},
    {0x00ce, {0x1b, 0x0049}},
    {0x005e, {0x1b, 0x0020}},
    {0x00c2, {0x1b, 0x0041}},
    {0x00ca, {0x1b, 0x0045}},
    {0x00db, {0x1b, 0x0055}},
    {0x00ea, {0x1b, 0x0065}},
    {0x00fb, {0x1b, 0x0075}},
    {0x00e2, {0x1b, 0x0061}},
    {0x00d4, {0x1b, 0x004f}},
    {0x00ee, {0x1b, 0x0069}},
    {0x00f5, {0x1b, 0x006f}},
    {0x007e, {0x1b, 0x0020}},
    {0x00f1, {0x1b, 0x006e}},
    {0x00c3, {0x1b, 0x0041}},
    {0x00e3, {0x1b, 0x0061}},
    {0x00d1, {0x1b, 0x004e}},
    {0x00d5, {0x1b, 0x004f}},
  };
}
