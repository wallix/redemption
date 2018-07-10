#pragma once

#include "fonts.hpp"

namespace ocr {

    // Some parameters
    // *Binarization
    // \{
    static const int fuzzy = 45; // Hyper-connectivity, alpha flat zone (for color gradient)
    // \}
    // *Reconstruction
    // \{
    static const unsigned bbox_padding = 3u * 2u; // Padding between text and border (top and bottom)
    // Height of title bars
    static const unsigned bbox_max_height = std::max(fonts::max_height_font[0], fonts::max_height_font[1]) + 1u;  // Height of title bars
    static const unsigned bbox_min_height = std::max(fonts::min_height_font[0], fonts::min_height_font[1]) + 1u;
    static const unsigned bbox_treshold = 2; // Tolerance to title bar height
    static const unsigned bbox_min_width = 100; // Min width of title bar
    // \}
} // namespace ocr
