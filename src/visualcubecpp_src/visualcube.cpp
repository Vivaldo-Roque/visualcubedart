/*
    visualcube.cpp — VisualCube C++ Port
    Public API implementation: configuration processing + orchestration.

    Copyright (C) 2010 Conrad Rider (original PHP)
    C++ port: same LGPL v3 licence applies.
*/

#include "visualcube/visualcube.hpp"
#include "visualcube/colors.hpp"
#include "visualcube/facelet.hpp"
#include "visualcube/renderer.hpp"
#include "visualcube/image_export.hpp"

#include <regex>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <cctype>

namespace vc {

// =========================================================================
// Rotation string parser  "y45x-34z90" → Config::rotation
// =========================================================================

void parse_rotation(const std::string& r_str, Config& cfg) {
    cfg.rotation.clear();
    // Match [xyz] followed by optional '-' and digits
    std::regex re("([xyz])(-?[0-9]{1,3})");
    auto begin = std::sregex_iterator(r_str.begin(), r_str.end(), re);
    auto end   = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        std::smatch m = *it;
        int axis = (m[1].str()[0] == 'x') ? 0 : (m[1].str()[0] == 'y') ? 1 : 2;
        int deg  = std::stoi(m[2].str());
        cfg.rotation.push_back({ axis, deg });
    }
}

// =========================================================================
// Scheme resolver: returns 9-element hex array [U R F D L B N O T]
// =========================================================================

std::vector<std::string> resolve_scheme(const Config& cfg) {
    // Start from defaults
    std::vector<std::string> scheme(std::begin(DEF_SCHEME), std::end(DEF_SCHEME));

    if (cfg.scheme.empty()) return scheme;

    // 6-char abbreviation string e.g. "yrbwog"
    if (cfg.scheme.size() == 6) {
        bool all_abbr = true;
        for (char c : cfg.scheme)
            if (abbr_col_map().find(c) == abbr_col_map().end()) { all_abbr = false; break; }
        if (all_abbr) {
            for (int i = 0; i < 6; ++i)
                scheme[i] = abbr_col_map().at(cfg.scheme[i]);
            return scheme;
        }
    }

    // Comma-separated hex values e.g. "FEFE00,EE0000,0000F2,FEFEFE,FFA100,00D800"
    {
        std::vector<std::string> parts;
        std::string tok;
        for (char c : cfg.scheme) {
            if (c == ',') { parts.push_back(tok); tok.clear(); }
            else tok += c;
        }
        if (!tok.empty()) parts.push_back(tok);
        if (parts.size() == 6) {
            bool ok = true;
            for (int i = 0; i < 6; ++i) {
                std::string col = parse_col(parts[i]);
                if (col.empty()) { ok = false; break; }
                scheme[i] = col;
            }
            if (ok) return scheme;
        }
    }

    return scheme; // fall back to default
}

// =========================================================================
// Build initial facelet state from Config
// Returns: facelets_char, facelets_int, using_cols
// =========================================================================

struct FaceletState {
    std::vector<char> chars;   // color abbreviation per facelet
    std::vector<int>  ints;    // face index per facelet
    bool using_cols = false;
};

static FaceletState build_facelets(const Config& cfg,
                                   const std::vector<char>& schcode) {
    int dim = cfg.dim;
    int nf6 = dim * dim * 6;

    FaceletState fs;
    fs.chars.resize(nf6);
    fs.ints.resize(nf6);

    // Default: facelet i belongs to face (i / dim*dim)
    for (int fc = 0; fc < 6; ++fc)
        for (int i = 0; i < dim * dim; ++i) {
            int idx = fc * dim * dim + i;
            fs.ints[idx] = fc;
            fs.chars[idx] = schcode[fc];
        }

    // --- fc parameter (color abbreviations) ---
    if (!cfg.fc.empty()) {
        const std::string& uf = cfg.fc;
        // Validate: only color abbreviation chars
        static const std::string valid_fc = "ndlswyrobgmpt";
        bool ok = true;
        for (char c : uf) if (valid_fc.find(c) == std::string::npos) { ok=false; break; }
        if (ok) {
            fs.using_cols = true;
            int nf = (int)uf.size();
            for (int fc = 0; fc < 6; ++fc)
                for (int i = 0; i < dim * dim; ++i) {
                    int idx = fc * dim * dim + i;
                    fs.chars[idx] = (idx < nf) ? uf[idx] : schcode[fc];
                }
            return fs;
        }
    }

    // --- fd parameter (face letter definitions) ---
    if (!cfg.fd.empty()) {
        const std::string& uf = cfg.fd;
        static const std::string valid_fd = "udlrfbnot";
        bool ok = true;
        for (char c : uf) if (valid_fd.find(c) == std::string::npos) { ok=false; break; }
        if (ok) {
            // Map from face letters to face index
            auto fd_map = [](char c) -> int {
                switch (c) {
                    case 'u': return U; case 'r': return R; case 'f': return F;
                    case 'd': return D; case 'l': return L; case 'b': return B;
                    case 'n': return N; case 'o': return O; case 't': return T;
                }
                return N;
            };
            int nf = (int)uf.size();
            for (int fc = 0; fc < 6; ++fc)
                for (int i = 0; i < dim * dim; ++i) {
                    int idx = fc * dim * dim + i;
                    int fi = (idx < nf) ? fd_map(uf[idx])
                           : (cfg.view == View::Trans ? T : N);
                    fs.ints[idx] = fi;
                    fs.chars[idx] = (fi < 6) ? schcode[fi] : (fi == T ? 't' : 'l');
                }
        }
    }

    return fs;
}

// =========================================================================
// Apply stage mask to facelets
// =========================================================================

static void apply_stage(FaceletState& fs, const Config& cfg) {
    if (cfg.stage.empty()) return;

    // Check for rotation suffix: "f2l-y45"
    std::string stage_name = cfg.stage;
    std::string st_rtn;
    auto dash = cfg.stage.rfind('-');
    if (dash != std::string::npos && dash > 0) {
        st_rtn = cfg.stage.substr(dash + 1);
        stage_name = cfg.stage.substr(0, dash);
    }

    std::string mask_str = stage_mask(stage_name, cfg.dim);
    if (mask_str.empty()) return;

    // Apply algorithm to mask if needed
    std::vector<char> mask(mask_str.begin(), mask_str.end());
    if (!st_rtn.empty()) {
        std::string formatted = fcs_format_alg(st_rtn);
        mask = fcs_doperm(mask, formatted, cfg.dim);
    }

    int nf6 = cfg.dim * cfg.dim * 6;
    for (int i = 0; i < nf6; ++i) {
        if (mask[i] == '0') {
            if (fs.using_cols) {
                fs.chars[i] = (cfg.view == View::Trans) ? 't' : 'l';
            } else {
                fs.ints[i] = (cfg.view == View::Trans) ? T : N;
            }
        }
    }
}

// =========================================================================
// Apply algorithm / case
// =========================================================================

static void apply_alg(FaceletState& fs, const Config& cfg) {
    if (cfg.alg.empty() && cfg.case_alg.empty()) return;

    std::string case_fmt = cfg.case_alg.empty() ? ""
                         : invert_alg(fcs_format_alg(cfg.case_alg));
    std::string alg_fmt  = fcs_format_alg(cfg.alg);
    std::string combined = case_fmt + " " + alg_fmt;

    if (fs.using_cols)
        fs.chars = fcs_doperm(fs.chars, combined, cfg.dim);
    else
        fs.ints  = fcs_doperm(fs.ints,  combined, cfg.dim);
}

// =========================================================================
// generate_svg
// =========================================================================

std::string generate_svg(const Config& cfg) {
    // 1. Resolve color scheme
    std::vector<std::string> scheme = resolve_scheme(cfg);

    // Build schcode (abbreviation for each face's default color)
    std::vector<char> schcode(6);
    for (int i = 0; i < 6; ++i) schcode[i] = DEF_SCHCODE[i];
    // Override with custom scheme abbreviations if applicable
    if (cfg.scheme.size() == 6) {
        bool all_abbr = true;
        for (char c : cfg.scheme)
            if (abbr_col_map().find(c) == abbr_col_map().end()) { all_abbr=false; break; }
        if (all_abbr)
            for (int i = 0; i < 6; ++i) schcode[i] = cfg.scheme[i];
    }

    // 2. Override view-specific settings
    Config effective = cfg;
    if (cfg.view == View::Plan)
        effective.rotation = {{ 0, -90 }};
    if (cfg.view == View::Trans && effective.cc.empty())
        effective.cc = SILVER;
    if (effective.cc.empty()) effective.cc = BLACK;

    // 3. Build initial facelet state
    FaceletState fs = build_facelets(effective, schcode);

    // 4. Apply stage mask
    apply_stage(fs, effective);

    // 5. Apply alg/case
    apply_alg(fs, effective);

    // 6. Parse arrows
    std::string ac = effective.ac.empty() ? GREY : effective.ac;
    std::vector<Arrow> arrows = parse_arrows(effective.arrows, effective.dim, ac);

    // 7. Render SVG
    return render_svg(effective, fs.chars, fs.ints, fs.using_cols, scheme, arrows);
}

// =========================================================================
// generate_png
// =========================================================================

std::vector<uint8_t> generate_png(const Config& cfg) {
    std::string svg = generate_svg(cfg);
    return svg_to_png(svg, cfg.size);
}

// =========================================================================
// render_to_file
// =========================================================================

bool render_to_file(const Config& cfg, const std::string& path) {
    if (cfg.fmt == Fmt::PNG) {
        auto png = generate_png(cfg);
        if (png.empty()) return false;
        if (path == "-") {
            std::fwrite(png.data(), 1, png.size(), stdout);
            return true;
        }
        std::ofstream f(path, std::ios::binary);
        if (!f) return false;
        f.write(reinterpret_cast<const char*>(png.data()), (std::streamsize)png.size());
        return f.good();
    }

    // SVG (default)
    std::string svg = generate_svg(cfg);
    if (path == "-") {
        std::cout << svg;
        return true;
    }
    std::ofstream f(path);
    if (!f) return false;
    f << svg;
    return f.good();
}

} // namespace vc
