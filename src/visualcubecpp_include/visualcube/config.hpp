#pragma once
/*
    config.hpp — VisualCube C++ Port
    Configuration struct replacing PHP $_REQUEST parameters.

    Copyright (C) 2010 Conrad Rider (original PHP)
    C++ port: same LGPL v3 licence applies.
*/

#include <string>
#include <vector>
#include <utility>

namespace vc {

// Face indices (mirrors PHP $U $R $F $D $L $B $N $O $T)
enum Face : int { U=0, R=1, F=2, D=3, L=4, B=5, N=6, O=7, T=8 };

// Output format
enum class Fmt { SVG, PNG };

// View mode
enum class View { Normal, Plan, Trans };

// A single rotation step: axis (0=x 1=y 2=z) + angle in degrees
struct Rotation { int axis; int degrees; };

// ---- Main config struct -------------------------------------------------
struct Config {
    // Puzzle dimension (2 = 2x2, 3 = 3x3, 4 = 4x4, …)
    int dim = 3;

    // Output image size in pixels
    int size = 128;

    // Projection distance (how close the eye is to the cube, 1–100)
    float dist = 5.0f;

    // Output format
    Fmt fmt = Fmt::SVG;

    // View mode
    View view = View::Normal;

    // Background color (6-digit hex, empty = transparent)
    std::string bg = "FFFFFF";

    // Cube body color (6-digit hex)
    std::string cc = "000000";

    // Cube body opacity 0–100
    int co = 100;

    // Face/sticker opacity 0–100
    int fo = 100;

    // Facelet color definition (fc parameter, e.g. "yyyrrrbbbwwwooogggrrrrrr...")
    // Uses single-char color abbreviations (y r b w o g n d l s m p t)
    std::string fc;

    // Facelet face definition (fd parameter, e.g. "uuurrr...")
    // Uses face letter codes (u d l r f b n o t)
    std::string fd;

    // Algorithm to apply (e.g. "R U R' U'")
    std::string alg;

    // Case (inverse alg applied before alg)
    std::string case_alg;

    // Stage mask name (e.g. "f2l", "oll", "pll")
    std::string stage;

    // Color scheme: 6-char abbreviation string (e.g. "yrbwog") or
    // 6 comma-separated hex values
    std::string scheme;

    // Arrow definitions (comma-separated, e.g. "U4-U6,U6-U8")
    std::string arrows;

    // Default arrow color (6-digit hex)
    std::string ac = "808080";

    // Rotation sequence applied to the cube before rendering
    // Default: y45, x-34 (standard 3D view)
    std::vector<Rotation> rotation = {{ 1, 45 }, { 0, -34 }};
};

} // namespace vc
