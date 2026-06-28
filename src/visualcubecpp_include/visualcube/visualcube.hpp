#pragma once
/*
    visualcube.hpp — VisualCube C++ Port
    Public API.

    Copyright (C) 2010 Conrad Rider (original PHP)
    C++ port: same LGPL v3 licence applies.
*/

#include "visualcube/config.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace vc {

// ---- String builders for Config -----------------------------------------

// Parse the rotation string "y45x-34" → Config::rotation
void parse_rotation(const std::string& r_str, Config& cfg);

// Parse a scheme string ("yrbwog" or "FEFE00,EE0000,...") → cfg.scheme
// and fill the scheme array (9 elements: U R F D L B N O T colors)
std::vector<std::string> resolve_scheme(const Config& cfg);

// ---- Main API -----------------------------------------------------------

// Generate an SVG string for the given configuration.
// This is the core function — everything else builds on it.
std::string generate_svg(const Config& cfg);

// Generate a PNG byte buffer for the given configuration.
// Returns empty vector on failure.
std::vector<uint8_t> generate_png(const Config& cfg);

// Write output to file. Format is determined by cfg.fmt.
// path "-" → write to stdout.
// Returns true on success.
bool render_to_file(const Config& cfg, const std::string& path);

} // namespace vc
