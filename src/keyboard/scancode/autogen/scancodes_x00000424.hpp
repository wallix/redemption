#pragma once

# include <cstdint>
# include <unordered_map>

// "sl-SI" locale 
namespace x00000424
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
    {0x0027, {0x0c, 0x00}},
    {0x002b, {0x0d, 0x00}},
    {0x0008, {0x0e, 0x00}},
    {0x0009, {0x0f, 0x00}},
    {0x0071, {0x10, 0x00}},
    {0x0077, {0x11, 0x00}},
    {0x0065, {0x12, 0x00}},
    {0x0072, {0x13, 0x00}},
    {0x0074, {0x14, 0x00}},
    {0x007a, {0x15, 0x00}},
    {0x0075, {0x16, 0x00}},
    {0x0069, {0x17, 0x00}},
    {0x006f, {0x18, 0x00}},
    {0x0070, {0x19, 0x00}},
    {0x0161, {0x1a, 0x00}},
    {0x0111, {0x1b, 0x00}},
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
    {0x010d, {0x27, 0x00}},
    {0x0107, {0x28, 0x00}},
    {0x00b8, {0x29, 0x00}},
    {0x017e, {0x2b, 0x00}},
    {0x0079, {0x2c, 0x00}},
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
    {0x0024, {0x05, 0x01}},
    {0x0025, {0x06, 0x01}},
    {0x0026, {0x07, 0x01}},
    {0x0028, {0x09, 0x01}},
    {0x0029, {0x0a, 0x01}},
    {0x003d, {0x0b, 0x01}},
    {0x003f, {0x0c, 0x01}},
    {0x0051, {0x10, 0x01}},
    {0x0057, {0x11, 0x01}},
    {0x0045, {0x12, 0x01}},
    {0x0052, {0x13, 0x01}},
    {0x0054, {0x14, 0x01}},
    {0x005a, {0x15, 0x01}},
    {0x0055, {0x16, 0x01}},
    {0x0049, {0x17, 0x01}},
    {0x004f, {0x18, 0x01}},
    {0x0050, {0x19, 0x01}},
    {0x0160, {0x1a, 0x01}},
    {0x0110, {0x1b, 0x01}},
    {0x0041, {0x1e, 0x01}},
    {0x0053, {0x1f, 0x01}},
    {0x0044, {0x20, 0x01}},
    {0x0046, {0x21, 0x01}},
    {0x0047, {0x22, 0x01}},
    {0x0048, {0x23, 0x01}},
    {0x004a, {0x24, 0x01}},
    {0x004b, {0x25, 0x01}},
    {0x004c, {0x26, 0x01}},
    {0x010c, {0x27, 0x01}},
    {0x0106, {0x28, 0x01}},
    {0x00a8, {0x29, 0x01}},
    {0x017d, {0x2b, 0x01}},
    {0x0059, {0x2c, 0x01}},
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
    {0x007e, {0x02, 0x02}},
    {0x02c7, {0x03, 0x02}},
    {0x005e, {0x04, 0x02}},
    {0x02d8, {0x05, 0x02}},
    {0x00b0, {0x06, 0x02}},
    {0x02db, {0x07, 0x02}},
    {0x0060, {0x08, 0x02}},
    {0x02d9, {0x09, 0x02}},
    {0x00b4, {0x0a, 0x02}},
    {0x02dd, {0x0b, 0x02}},
    {0x005c, {0x10, 0x02}},
    {0x007c, {0x11, 0x02}},
    {0x20ac, {0x12, 0x02}},
    {0x00f7, {0x1a, 0x02}},
    {0x00d7, {0x1b, 0x02}},
    {0x005b, {0x21, 0x02}},
    {0x005d, {0x22, 0x02}},
    {0x0142, {0x25, 0x02}},
    {0x0141, {0x26, 0x02}},
    {0x00df, {0x28, 0x02}},
    {0x00a4, {0x2b, 0x02}},
    {0x0040, {0x2f, 0x02}},
    {0x007b, {0x30, 0x02}},
    {0x007d, {0x31, 0x02}},
    {0x00a7, {0x32, 0x02}},
    {0x001d, {0x27, 0x08}},
    {0x001c, {0x2b, 0x08}},
  };

  // key : extended unicode, value : extended scancode + unicode [8 + 16 bits]
  inline const std::unordered_map<std::uint16_t, std::pair<std::uint8_t, std::uint16_t>> EXTENDED_SCANCODES_TABLE
  {
    {0x017e, {0x03, 0x007a}},
    {0x010c, {0x03, 0x0043}},
    {0x0148, {0x03, 0x006e}},
    {0x013e, {0x03, 0x006c}},
    {0x0160, {0x03, 0x0053}},
    {0x0158, {0x03, 0x0052}},
    {0x0164, {0x03, 0x0054}},
    {0x0159, {0x03, 0x0072}},
    {0x010e, {0x03, 0x0044}},
    {0x0161, {0x03, 0x0073}},
    {0x011a, {0x03, 0x0045}},
    {0x02c7, {0x03, 0x0020}},
    {0x010f, {0x03, 0x0064}},
    {0x011b, {0x03, 0x0065}},
    {0x010d, {0x03, 0x0063}},
    {0x017d, {0x03, 0x005a}},
    {0x013d, {0x03, 0x004c}},
    {0x0147, {0x03, 0x004e}},
    {0x0165, {0x03, 0x0074}},
    {0x00f4, {0x04, 0x006f}},
    {0x00ce, {0x04, 0x0049}},
    {0x005e, {0x04, 0x0020}},
    {0x00c2, {0x04, 0x0041}},
    {0x00e2, {0x04, 0x0061}},
    {0x00d4, {0x04, 0x004f}},
    {0x00ee, {0x04, 0x0069}},
    {0x0102, {0x05, 0x0041}},
    {0x02d8, {0x05, 0x0020}},
    {0x0103, {0x05, 0x0061}},
    {0x00b0, {0x06, 0x0020}},
    {0x016f, {0x06, 0x0075}},
    {0x016e, {0x06, 0x0055}},
    {0x0104, {0x07, 0x0041}},
    {0x02db, {0x07, 0x0020}},
    {0x0119, {0x07, 0x0065}},
    {0x0118, {0x07, 0x0045}},
    {0x0105, {0x07, 0x0061}},
    {0x017c, {0x09, 0x007a}},
    {0x017b, {0x09, 0x005a}},
    {0x02d9, {0x09, 0x0020}},
    {0x00dd, {0x0a, 0x0059}},
    {0x015a, {0x0a, 0x0053}},
    {0x0154, {0x0a, 0x0052}},
    {0x00da, {0x0a, 0x0055}},
    {0x0139, {0x0a, 0x004c}},
    {0x0143, {0x0a, 0x004e}},
    {0x00d3, {0x0a, 0x004f}},
    {0x017a, {0x0a, 0x007a}},
    {0x00e9, {0x0a, 0x0065}},
    {0x0107, {0x0a, 0x0063}},
    {0x00e1, {0x0a, 0x0061}},
    {0x00ed, {0x0a, 0x0069}},
    {0x00cd, {0x0a, 0x0049}},
    {0x00b4, {0x0a, 0x0020}},
    {0x00c1, {0x0a, 0x0041}},
    {0x0106, {0x0a, 0x0043}},
    {0x00c9, {0x0a, 0x0045}},
    {0x0179, {0x0a, 0x005a}},
    {0x00f3, {0x0a, 0x006f}},
    {0x0144, {0x0a, 0x006e}},
    {0x013a, {0x0a, 0x006c}},
    {0x015b, {0x0a, 0x0073}},
    {0x0155, {0x0a, 0x0072}},
    {0x00fa, {0x0a, 0x0075}},
    {0x00fd, {0x0a, 0x0079}},
    {0x0151, {0x0b, 0x006f}},
    {0x02dd, {0x0b, 0x0020}},
    {0x0171, {0x0b, 0x0075}},
    {0x0150, {0x0b, 0x004f}},
    {0x0170, {0x0b, 0x0055}},
    {0x00f6, {0x0c, 0x006f}},
    {0x00a8, {0x0c, 0x0020}},
    {0x00c4, {0x0c, 0x0041}},
    {0x00cb, {0x0c, 0x0045}},
    {0x00dc, {0x0c, 0x0055}},
    {0x00eb, {0x0c, 0x0065}},
    {0x00fc, {0x0c, 0x0075}},
    {0x00e4, {0x0c, 0x0061}},
    {0x00d6, {0x0c, 0x004f}},
    {0x015e, {0x0d, 0x0053}},
    {0x015f, {0x0d, 0x0073}},
    {0x00c7, {0x0d, 0x0043}},
    {0x00e7, {0x0d, 0x0063}},
    {0x00b8, {0x0d, 0x0020}},
  };
}
