#pragma once
/*
    colors.hpp — VisualCube C++ Port
    Color constants, name/abbreviation maps, and parse_col().

    Copyright (C) 2010 Conrad Rider (original PHP)
    C++ port: same LGPL v3 licence applies.
*/

#include <string>
#include <unordered_map>

namespace vc {

// ---- Hex color constants ------------------------------------------------
inline constexpr const char* BLACK  = "000000";
inline constexpr const char* DGREY  = "404040";
inline constexpr const char* GREY   = "808080";
inline constexpr const char* SILVER = "BFBFBF";
inline constexpr const char* WHITE  = "FFFFFF";
inline constexpr const char* YELLOW = "FEFE00";
inline constexpr const char* RED    = "EE0000";
inline constexpr const char* ORANGE = "FFA100";
inline constexpr const char* BLUE   = "0000F2";
inline constexpr const char* GREEN  = "00D800";
inline constexpr const char* PURPLE = "A83DD9";
inline constexpr const char* PINK   = "F33D7B";

// Transparent sentinel (not a real hex color)
inline constexpr const char* TRANSPARENT = "t";

// Default color scheme: U=Y R=R F=B D=W L=O B=G + N=DGrey O=Grey T=transparent
// Indices 0-5 are face colors, 6=N(blank), 7=O(other), 8=T(transparent)
inline const std::string DEF_SCHEME[] = {
    YELLOW, RED, BLUE, WHITE, ORANGE, GREEN, DGREY, GREY, "t"
};

// Default scheme color codes (single-char abbreviation per face)
inline const char DEF_SCHCODE[] = { 'y', 'r', 'b', 'w', 'o', 'g' };

// ---- Maps ---------------------------------------------------------------

// Returns the map from color abbreviation char → hex string
inline const std::unordered_map<char, std::string>& abbr_col_map() {
    static const std::unordered_map<char, std::string> m = {
        {'n', BLACK},  {'d', DGREY},  {'l', GREY},   {'s', SILVER},
        {'w', WHITE},  {'y', YELLOW}, {'r', RED},    {'o', ORANGE},
        {'b', BLUE},   {'g', GREEN},  {'m', PURPLE}, {'p', PINK},
        {'t', "t"}
    };
    return m;
}

// Returns the map from color name string → hex string
inline const std::unordered_map<std::string, std::string>& name_col_map() {
    static const std::unordered_map<std::string, std::string> m = {
        {"black",  BLACK},  {"dgrey",  DGREY},  {"grey",   GREY},
        {"silver", SILVER}, {"white",  WHITE},  {"yellow", YELLOW},
        {"red",    RED},    {"orange", ORANGE}, {"blue",   BLUE},
        {"green",  GREEN},  {"purple", PURPLE}, {"pink",   PINK}
    };
    return m;
}

// ---- parse_col ----------------------------------------------------------
// Returns a 6-char hex string, "t" for transparent, or "" on failure.
// Accepts: single abbreviation char, color name, 3-digit hex, 6-digit hex.
inline std::string parse_col(const std::string& col) {
    if (col.empty()) return "";

    // Single abbreviation character
    if (col.size() == 1) {
        auto& m = abbr_col_map();
        auto it = m.find(col[0]);
        if (it != m.end()) return it->second;
    }

    // Color name
    {
        auto& m = name_col_map();
        auto it = m.find(col);
        if (it != m.end()) return it->second;
    }

    // Helper: is hex digit?
    auto is_hex = [](char c) {
        return (c >= '0' && c <= '9') ||
               (c >= 'a' && c <= 'f') ||
               (c >= 'A' && c <= 'F');
    };

    // 3-digit hex → expand to 6
    if (col.size() == 3 && is_hex(col[0]) && is_hex(col[1]) && is_hex(col[2])) {
        return { col[0], col[0], col[1], col[1], col[2], col[2] };
    }

    // 6-digit hex
    if (col.size() == 6) {
        bool ok = true;
        for (char c : col) ok = ok && is_hex(c);
        if (ok) return col;
    }

    return "";
}

// Parse a color from its abbreviation char (used when iterating fc strings)
inline std::string abbr_to_hex(char c) {
    auto& m = abbr_col_map();
    auto it = m.find(c);
    return it != m.end() ? it->second : "";
}

} // namespace vc
