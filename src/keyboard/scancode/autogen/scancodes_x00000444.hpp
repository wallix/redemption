#pragma once

# include <cstdint>
# include <unordered_map>

// "tt-RU" locale 
namespace x00000444
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
    {0x002d, {0x0c, 0x00}},
    {0x003d, {0x0d, 0x00}},
    {0x0008, {0x0e, 0x00}},
    {0x0009, {0x0f, 0x00}},
    {0x0439, {0x10, 0x00}},
    {0x04e9, {0x11, 0x00}},
    {0x0443, {0x12, 0x00}},
    {0x043a, {0x13, 0x00}},
    {0x0435, {0x14, 0x00}},
    {0x043d, {0x15, 0x00}},
    {0x0433, {0x16, 0x00}},
    {0x0448, {0x17, 0x00}},
    {0x04d9, {0x18, 0x00}},
    {0x0437, {0x19, 0x00}},
    {0x0445, {0x1a, 0x00}},
    {0x04af, {0x1b, 0x00}},
    {0x000d, {0x1c, 0x00}},
    {0x0444, {0x1e, 0x00}},
    {0x044b, {0x1f, 0x00}},
    {0x0432, {0x20, 0x00}},
    {0x0430, {0x21, 0x00}},
    {0x043f, {0x22, 0x00}},
    {0x0440, {0x23, 0x00}},
    {0x043e, {0x24, 0x00}},
    {0x043b, {0x25, 0x00}},
    {0x0434, {0x26, 0x00}},
    {0x04a3, {0x27, 0x00}},
    {0x044d, {0x28, 0x00}},
    {0x04bb, {0x29, 0x00}},
    {0x005c, {0x2b, 0x00}},
    {0x044f, {0x2c, 0x00}},
    {0x0447, {0x2d, 0x00}},
    {0x0441, {0x2e, 0x00}},
    {0x043c, {0x2f, 0x00}},
    {0x0438, {0x30, 0x00}},
    {0x0442, {0x31, 0x00}},
    {0x0497, {0x32, 0x00}},
    {0x0431, {0x33, 0x00}},
    {0x044e, {0x34, 0x00}},
    {0x002e, {0x35, 0x00}},
    {0x002a, {0x37, 0x00}},
    {0x0020, {0x39, 0x00}},
    {0x0491, {0x56, 0x00}},
    {0x002f, {0x62, 0x00}},
    {0x0021, {0x02, 0x01}},
    {0x0022, {0x03, 0x01}},
    {0x2116, {0x04, 0x01}},
    {0x003b, {0x05, 0x01}},
    {0x0025, {0x06, 0x01}},
    {0x003a, {0x07, 0x01}},
    {0x003f, {0x08, 0x01}},
    {0x0028, {0x0a, 0x01}},
    {0x0029, {0x0b, 0x01}},
    {0x005f, {0x0c, 0x01}},
    {0x002b, {0x0d, 0x01}},
    {0x0419, {0x10, 0x01}},
    {0x04e8, {0x11, 0x01}},
    {0x0423, {0x12, 0x01}},
    {0x041a, {0x13, 0x01}},
    {0x0415, {0x14, 0x01}},
    {0x041d, {0x15, 0x01}},
    {0x0413, {0x16, 0x01}},
    {0x0428, {0x17, 0x01}},
    {0x04d8, {0x18, 0x01}},
    {0x0417, {0x19, 0x01}},
    {0x0425, {0x1a, 0x01}},
    {0x04ae, {0x1b, 0x01}},
    {0x0424, {0x1e, 0x01}},
    {0x042b, {0x1f, 0x01}},
    {0x0412, {0x20, 0x01}},
    {0x0410, {0x21, 0x01}},
    {0x041f, {0x22, 0x01}},
    {0x0420, {0x23, 0x01}},
    {0x041e, {0x24, 0x01}},
    {0x041b, {0x25, 0x01}},
    {0x0414, {0x26, 0x01}},
    {0x04a2, {0x27, 0x01}},
    {0x042d, {0x28, 0x01}},
    {0x04ba, {0x29, 0x01}},
    {0x042f, {0x2c, 0x01}},
    {0x0427, {0x2d, 0x01}},
    {0x0421, {0x2e, 0x01}},
    {0x041c, {0x2f, 0x01}},
    {0x0418, {0x30, 0x01}},
    {0x0422, {0x31, 0x01}},
    {0x0496, {0x32, 0x01}},
    {0x0411, {0x33, 0x01}},
    {0x042e, {0x34, 0x01}},
    {0x002c, {0x35, 0x01}},
    {0x0490, {0x56, 0x01}},
    {0x007f, {0x63, 0x01}},
    {0x0040, {0x03, 0x02}},
    {0x0023, {0x04, 0x02}},
    {0x0024, {0x05, 0x02}},
    {0x005b, {0x08, 0x02}},
    {0x005d, {0x09, 0x02}},
    {0x007b, {0x0a, 0x02}},
    {0x007d, {0x0b, 0x02}},
    {0x0446, {0x11, 0x02}},
    {0x0449, {0x18, 0x02}},
    {0x044a, {0x1b, 0x02}},
    {0x0436, {0x27, 0x02}},
    {0x0027, {0x28, 0x02}},
    {0x0451, {0x29, 0x02}},
    {0x044c, {0x32, 0x02}},
    {0x003c, {0x33, 0x02}},
    {0x003e, {0x34, 0x02}},
    {0x0426, {0x11, 0x03}},
    {0x0429, {0x18, 0x03}},
    {0x042a, {0x1b, 0x03}},
    {0x0416, {0x27, 0x03}},
    {0x0401, {0x29, 0x03}},
    {0x042c, {0x32, 0x03}},
    {0x001d, {0x27, 0x08}},
    {0x001c, {0x2b, 0x08}},
  };

  // key : extended unicode, value : extended scancode + unicode [8 + 16 bits]
  inline const std::unordered_map<std::uint16_t, std::pair<std::uint8_t, std::uint16_t>> EXTENDED_SCANCODES_TABLE
  {
  };
}
