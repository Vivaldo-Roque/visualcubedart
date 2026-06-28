/*
    image_export.cpp — VisualCube C++ Port
    SVG → PNG via NanoSVG + stb_image_write (header-only, zero system deps).

    Copyright (C) 2010 Conrad Rider (original PHP)
    C++ port: same LGPL v3 licence applies.
*/

// NanoSVG: include the implementation once here
#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvg.h"
#include "nanosvgrast.h"

// stb_image_write: include the implementation once here
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "visualcube/image_export.hpp"

#include <cstring>
#include <cstdio>
#include <algorithm>

namespace vc {

// =========================================================================
// svg_to_rgba
// Renders at OVERSAMPLE× then box-filters down for smooth edges.
// =========================================================================

std::vector<uint8_t> svg_to_rgba(const std::string& svg_str, int width, int height) {
    // NanoSVG modifies the buffer during parse
    std::string buf = svg_str;

    NSVGimage* image = nsvgParse(buf.data(), "px", 96.0f);
    if (!image) return {};

    NSVGrasterizer* rast = nsvgCreateRasterizer();
    if (!rast) { nsvgDelete(image); return {}; }

    // --- Oversampling for quality ---
    // Render at OVERSAMPLE× resolution, then box-filter down.
    // 4× gives very smooth edges while keeping reasonable memory usage.
    const int OVERSAMPLE = 4;
    int bw = width  * OVERSAMPLE;
    int bh = height * OVERSAMPLE;

    float scale_x = (image->width  > 0) ? (float)bw / image->width  : 1.0f;
    float scale_y = (image->height > 0) ? (float)bh / image->height : 1.0f;
    float scale   = std::min(scale_x, scale_y);

    std::vector<uint8_t> big(bw * bh * 4, 0);
    nsvgRasterize(rast, image, 0, 0, scale, big.data(), bw, bh, bw * 4);

    nsvgDeleteRasterizer(rast);
    nsvgDelete(image);

    // --- Box-filter downscale ---
    std::vector<uint8_t> out(width * height * 4, 0);
    const int S = OVERSAMPLE;
    const int S2 = S * S;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int r = 0, g = 0, b = 0, a = 0;
            for (int dy = 0; dy < S; ++dy) {
                for (int dx = 0; dx < S; ++dx) {
                    int px = (y * S + dy) * bw + (x * S + dx);
                    r += big[px * 4 + 0];
                    g += big[px * 4 + 1];
                    b += big[px * 4 + 2];
                    a += big[px * 4 + 3];
                }
            }
            int op = y * width + x;
            out[op * 4 + 0] = (uint8_t)(r / S2);
            out[op * 4 + 1] = (uint8_t)(g / S2);
            out[op * 4 + 2] = (uint8_t)(b / S2);
            out[op * 4 + 3] = (uint8_t)(a / S2);
        }
    }

    return out;
}

// =========================================================================
// rgba_to_png
// =========================================================================

std::vector<uint8_t> rgba_to_png(const std::vector<uint8_t>& rgba,
                                 int width, int height) {
    std::vector<uint8_t> png_buf;

    auto write_cb = [](void* context, void* data, int size) {
        auto* buf = reinterpret_cast<std::vector<uint8_t>*>(context);
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);
        buf->insert(buf->end(), bytes, bytes + size);
    };

    int stride = width * 4;
    int ok = stbi_write_png_to_func(write_cb, &png_buf,
                                    width, height, 4,
                                    rgba.data(), stride);
    if (!ok) return {};
    return png_buf;
}

// =========================================================================
// svg_to_png
// =========================================================================

std::vector<uint8_t> svg_to_png(const std::string& svg_str, int size) {
    auto rgba = svg_to_rgba(svg_str, size, size);
    if (rgba.empty()) return {};
    return rgba_to_png(rgba, size, size);
}

} // namespace vc
