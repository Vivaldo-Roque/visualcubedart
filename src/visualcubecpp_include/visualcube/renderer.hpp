#pragma once
/*
    renderer.hpp — VisualCube C++ Port
    SVG generation for the 3D cube diagram.

    Copyright (C) 2010 Conrad Rider (original PHP)
    C++ port: same LGPL v3 licence applies.
*/

#include "visualcube/config.hpp"
#include "visualcube/geometry.hpp"
#include <string>
#include <vector>
#include <array>

namespace vc {

// Parsed arrow: from-facelet, to-facelet, optional via-facelet, scale, color
struct Arrow {
    struct Facelet { int face, col, row; };
    Facelet from, to;
    bool has_via = false;
    Facelet via{};
    float via_scale = 2.0f; // influence of via-point (default PHP: 2)
    float scale = 1.0f;     // arrow length scale
    std::string color;      // "" = use default arrow color
};

// Parse the raw "arw" parameter string into a vector of Arrow structs.
// e.g. "U4-U6,F0-F2-red"
std::vector<Arrow> parse_arrows(const std::string& arw_str, int dim,
                                const std::string& default_color);

// ---- Main renderer -------------------------------------------------------

// Holds all the projected 2D points for a cube of dimension dim.
// p[face][i][j] is a Vec3 (x,y already projected; z kept for depth sort).
using FacePoints = std::vector<std::vector<std::vector<Vec3>>>; // [6][dim+1][dim+1]

// Generate SVG string for the given config + pre-computed facelets.
// facelets_char: 6*dim*dim chars (color abbreviations), used when fc/fd set
// facelets_int : 6*dim*dim ints (face indices), used for face-based coloring
// using_cols   : true → use facelets_char + DEF_SCHEME abbr lookup
//                false → use facelets_int + scheme[] hex colors
std::string render_svg(
    const Config& cfg,
    const std::vector<char>& facelets_char,
    const std::vector<int>&  facelets_int,
    bool using_cols,
    const std::vector<std::string>& scheme,   // 9-element hex array
    const std::vector<Arrow>& arrows);

} // namespace vc
