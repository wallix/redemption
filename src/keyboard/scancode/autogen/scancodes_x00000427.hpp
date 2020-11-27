#pragma once

# include <cstdint>
# include <unordered_map>

// "lt-LT.ibm" locale 
namespace x00000427
{
  // key : unicode, value : scancode + mod flags [8 + 8 bits]
  inline const std::unordered_map<std::uint16_t, std::pair<std::uint8_t, std::uint8_t>> SCANCODES_TABLE
  {
    {0x0000, {0x00, 0x00}},
    {0x001b, {0x01, 0x00}},
    {0x0021, {0x02, 0x00}},
    {0x0022, {0x03, 0x00}},
    {0x002f, {0x04, 0x00}},
    {0x003b, {0x05, 0x00}},
    {0x003a, {0x06, 0x00}},
    {0x002c, {0x07, 0x00}},
    {0x002e, {0x08, 0x00}},
    {0x003f, {0x09, 0x00}},
    {0x0028, {0x0a, 0x00}},
    {0x0029, {0x0b, 0x00}},
    {0x005f, {0x0c, 0x00}},
    {0x002b, {0x0d, 0x00}},
    {0x0008, {0x0e, 0x00}},
    {0x0009, {0x0f, 0x00}},
    {0x0105, {0x10, 0x00}},
    {0x017e, {0x11, 0x00}},
    {0x0065, {0x12, 0x00}},
    {0x0072, {0x13, 0x00}},
    {0x0074, {0x14, 0x00}},
    {0x0079, {0x15, 0x00}},
    {0x0075, {0x16, 0x00}},
    {0x0069, {0x17, 0x00}},
    {0x006f, {0x18, 0x00}},
    {0x0070, {0x19, 0x00}},
    {0x012f, {0x1a, 0x00}},
    {0x201c, {0x1b, 0x00}},
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
    {0x0173, {0x27, 0x00}},
    {0x0117, {0x28, 0x00}},
    {0x0060, {0x29, 0x00}},
    {0x007c, {0x2b, 0x00}},
    {0x007a, {0x2c, 0x00}},
    {0x016b, {0x2d, 0x00}},
    {0x0063, {0x2e, 0x00}},
    {0x0076, {0x2f, 0x00}},
    {0x0062, {0x30, 0x00}},
    {0x006e, {0x31, 0x00}},
    {0x006d, {0x32, 0x00}},
    {0x010d, {0x33, 0x00}},
    {0x0161, {0x34, 0x00}},
    {0x0119, {0x35, 0x00}},
    {0x002a, {0x37, 0x00}},
    {0x0020, {0x39, 0x00}},
    {0x005c, {0x56, 0x00}},
    {0x0031, {0x02, 0x01}},
    {0x0032, {0x03, 0x01}},
    {0x0033, {0x04, 0x01}},
    {0x0034, {0x05, 0x01}},
    {0x0035, {0x06, 0x01}},
    {0x0036, {0x07, 0x01}},
    {0x0037, {0x08, 0x01}},
    {0x0038, {0x09, 0x01}},
    {0x0039, {0x0a, 0x01}},
    {0x0030, {0x0b, 0x01}},
    {0x002d, {0x0c, 0x01}},
    {0x003d, {0x0d, 0x01}},
    {0x0104, {0x10, 0x01}},
    {0x017d, {0x11, 0x01}},
    {0x0045, {0x12, 0x01}},
    {0x0052, {0x13, 0x01}},
    {0x0054, {0x14, 0x01}},
    {0x0059, {0x15, 0x01}},
    {0x0055, {0x16, 0x01}},
    {0x0049, {0x17, 0x01}},
    {0x004f, {0x18, 0x01}},
    {0x0050, {0x19, 0x01}},
    {0x012e, {0x1a, 0x01}},
    {0x201d, {0x1b, 0x01}},
    {0x0041, {0x1e, 0x01}},
    {0x0053, {0x1f, 0x01}},
    {0x0044, {0x20, 0x01}},
    {0x0046, {0x21, 0x01}},
    {0x0047, {0x22, 0x01}},
    {0x0048, {0x23, 0x01}},
    {0x004a, {0x24, 0x01}},
    {0x004b, {0x25, 0x01}},
    {0x004c, {0x26, 0x01}},
    {0x0172, {0x27, 0x01}},
    {0x0116, {0x28, 0x01}},
    {0x007e, {0x29, 0x01}},
    {0x005a, {0x2c, 0x01}},
    {0x016a, {0x2d, 0x01}},
    {0x0043, {0x2e, 0x01}},
    {0x0056, {0x2f, 0x01}},
    {0x0042, {0x30, 0x01}},
    {0x004e, {0x31, 0x01}},
    {0x004d, {0x32, 0x01}},
    {0x010c, {0x33, 0x01}},
    {0x0160, {0x34, 0x01}},
    {0x0118, {0x35, 0x01}},
    {0x007f, {0x63, 0x01}},
    {0x007b, {0x08, 0x02}},
    {0x005b, {0x09, 0x02}},
    {0x005d, {0x0a, 0x02}},
    {0x007d, {0x0b, 0x02}},
    {0x20ac, {0x12, 0x02}},
    {0x001d, {0x1b, 0x08}},
    {0x001c, {0x2b, 0x08}},
  };

  // key : extended unicode, value : extended scancode + unicode [8 + 16 bits]
  inline const std::unordered_map<std::uint16_t, std::pair<std::uint8_t, std::uint16_t>> EXTENDED_SCANCODES_TABLE
  {
  };
}
