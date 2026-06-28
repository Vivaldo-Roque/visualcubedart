#pragma once
/*
    image_export.hpp — VisualCube C++ Port
    PNG export from SVG strings via NanoSVG + stb_image_write.
    Header-only, cross-platform (Linux/Windows/macOS/Android).

    Copyright (C) 2010 Conrad Rider (original PHP)
    C++ port: same LGPL v3 licence applies.
*/

#include <string>
#include <vector>
#include <cstdint>

namespace vc {

// Rasterizes an SVG string into raw RGBA pixels at the given output size.
// Returns an empty vector on failure.
std::vector<uint8_t> svg_to_rgba(const std::string& svg_str, int width, int height);

// Encodes RGBA pixels into a PNG byte buffer (ready to write to file or send over network).
// Returns an empty vector on failure.
std::vector<uint8_t> rgba_to_png(const std::vector<uint8_t>& rgba,
                                 int width, int height);

// Convenience: SVG string → PNG bytes in one call.
std::vector<uint8_t> svg_to_png(const std::string& svg_str, int size);

} // namespace vc
