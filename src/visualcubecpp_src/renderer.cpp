/*
    renderer.cpp — VisualCube C++ Port
    SVG generation: 3D projection, face rendering, arrows.

    Copyright (C) 2010 Conrad Rider (original PHP)
    C++ port: same LGPL v3 licence applies.
*/

#include "visualcube/renderer.hpp"
#include "visualcube/colors.hpp"
#include "visualcube/geometry.hpp"

#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <regex>

namespace vc {

// =========================================================================
// Arrow parsing
// =========================================================================

static int face_code(char c) {
    switch (c) {
        case 'U': return 0; case 'R': return 1; case 'F': return 2;
        case 'D': return 3; case 'L': return 4; case 'B': return 5;
    }
    return -1;
}

std::vector<Arrow> parse_arrows(const std::string& arw_str, int dim,
                                const std::string& default_color) {
    std::vector<Arrow> result;
    if (arw_str.empty()) return result;

    // Split by comma
    std::vector<std::string> parts;
    {
        std::string tok;
        for (char c : arw_str) {
            if (c == ',') { parts.push_back(tok); tok.clear(); }
            else tok += c;
        }
        if (!tok.empty()) parts.push_back(tok);
    }

    for (const auto& part : parts) {
        // Split by '-'
        std::vector<std::string> segs;
        {
            std::string tok;
            for (char c : part) {
                if (c == '-') { segs.push_back(tok); tok.clear(); }
                else tok += c;
            }
            if (!tok.empty()) segs.push_back(tok);
        }
        if (segs.empty()) continue;

        // PHP format: first segment contains ALL face+number pairs concatenated
        // e.g. "U0U2", "R6R2R0" → extract [face,number] pairs via scanning
        struct FNPair { int face; int num; };
        std::vector<FNPair> fn_pairs;
        {
            const std::string& s = segs[0];
            for (size_t si = 0; si < s.size(); ) {
                int fc = face_code(s[si]);
                if (fc >= 0) {
                    ++si;
                    // Read following digits
                    std::string digits;
                    while (si < s.size() && std::isdigit((unsigned char)s[si]))
                        digits += s[si++];
                    if (!digits.empty())
                        fn_pairs.push_back({fc, std::stoi(digits)});
                } else {
                    ++si;
                }
            }
        }
        if (fn_pairs.size() < 2) continue;

        auto make_facelet = [&](const FNPair& p) -> Arrow::Facelet {
            int fn = std::min(p.num, dim * dim - 1);
            return { p.face, fn % dim, fn / dim };
        };

        Arrow a;
        a.color = default_color;
        a.from = make_facelet(fn_pairs[0]);
        a.to   = make_facelet(fn_pairs[1]);

        // Optional via point (3rd face+number in first segment)
        if (fn_pairs.size() >= 3) {
            a.has_via = true;
            a.via = make_facelet(fn_pairs[2]);
            a.via_scale = 2.0f;
        }

        // Remaining dash-separated segments: i<N>=influence, s<N>=scale, color
        for (size_t si = 1; si < segs.size(); ++si) {
            const auto& seg = segs[si];
            if (seg.empty()) continue;
            if (seg[0] == 'i' && seg.size() > 1) {
                a.via_scale = std::stof(seg.substr(1)) / 5.0f;
                a.via_scale = std::min(a.via_scale, 10.0f);
            } else if (seg[0] == 's' && seg.size() > 1) {
                a.scale = std::stof(seg.substr(1)) / 10.0f;
                a.scale = std::min(a.scale, 2.0f);
            } else {
                std::string col = parse_col(seg);
                if (!col.empty() && col != "t") a.color = col;
            }
        }

        result.push_back(a);
    }
    return result;
}

// =========================================================================
// SVG helpers
// =========================================================================

static std::string fmt_f(float v) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(4) << v;
    // Trim trailing zeros after decimal
    std::string s = os.str();
    auto dot = s.find('.');
    if (dot != std::string::npos) {
        size_t last = s.size() - 1;
        while (last > dot + 1 && s[last] == '0') --last;
        s = s.substr(0, last + 1);
    }
    return s;
}

static std::string fmt_pt(const Vec3& p) {
    return fmt_f(p.x) + "," + fmt_f(p.y);
}

// Polygon SVG element from 4 points
static std::string polygon_svg(const Vec3& p1, const Vec3& p2,
                               const Vec3& p3, const Vec3& p4,
                               const std::string& fill, const std::string& stroke,
                               bool transparent) {
    std::string s = "\t\t<polygon fill='#";
    s += (transparent ? "000000" : fill);
    s += "' stroke='#";
    s += stroke;
    s += "' ";
    if (transparent) s += "opacity='0' ";
    s += "points='";
    s += fmt_pt(p1) + " " + fmt_pt(p2) + " " + fmt_pt(p3) + " " + fmt_pt(p4);
    s += "'/>\n";
    return s;
}

// =========================================================================
// Main render function
// =========================================================================

std::string render_svg(
    const Config& cfg,
    const std::vector<char>& facelets_char,
    const std::vector<int>&  facelets_int,
    bool using_cols,
    const std::vector<std::string>& scheme,
    const std::vector<Arrow>& arrows)
{
    const int dim = cfg.dim;
    const float dist = cfg.dist;
    const float OUTLINE_WIDTH = 0.94f;
    const float sw = 0.0f; // stroke-width for facelets

    // Viewport
    const float ox = -0.9f, oy = -0.9f, vw = 1.8f, vh = 1.8f;

    // Translation to centre the cube
    Vec3 t{ -(float)dim/2, -(float)dim/2, -(float)dim/2 };
    Vec3 zpos{ 0, 0, dist };

    // Build all cube face points: p[face][i][j]
    // i,j range: 0..dim (inclusive — corner grid, not facelet grid)
    FacePoints p(6, std::vector<std::vector<Vec3>>(dim+1, std::vector<Vec3>(dim+1)));

    // Rotation normal vectors for each face (for visibility/depth sort)
    std::array<Vec3, 6> rv = {{
        {0,-1,0}, {1,0,0}, {0,0,-1}, {0,1,0}, {-1,0,0}, {0,0,1}
    }};

    for (int fc = 0; fc < 6; ++fc) {
        for (int i = 0; i <= dim; ++i) {
            for (int j = 0; j <= dim; ++j) {
                Vec3 pt;
                switch (fc) {
                    case U: pt = { (float)i,    0.0f,      (float)(dim-j) }; break;
                    case R: pt = { (float)dim,  (float)j,  (float)i       }; break;
                    case F: pt = { (float)i,    (float)j,  0.0f           }; break;
                    case D: pt = { (float)i,    (float)dim,(float)j       }; break;
                    case L: pt = { 0.0f,        (float)j,  (float)(dim-i) }; break;
                    case B: pt = { (float)(dim-i),(float)j,(float)dim     }; break;
                }
                pt = translate(pt, t);
                pt = scale(pt, 1.0f / dim);
                for (const auto& rn : cfg.rotation)
                    pt = rotate(pt, rn.axis, (float)M_PI * rn.degrees / 180.0f);
                pt = translate(pt, zpos);
                pt = project(pt, zpos.z);
                p[fc][i][j] = pt;
            }
        }
        // Rotate the face normal vector too
        for (const auto& rn : cfg.rotation)
            rv[fc] = rotate(rv[fc], rn.axis, (float)M_PI * rn.degrees / 180.0f);
    }

    // Depth-sort faces (bubble sort on rv[fc].z, descending = front-first)
    std::array<int,6> ro = {0,1,2,3,4,5};
    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 5; ++j)
            if (rv[ro[j]].z < rv[ro[j+1]].z) std::swap(ro[j], ro[j+1]);

    auto face_visible = [&](int fc) {
        return rv[fc].z < -0.105f;
    };

    // ---- Helpers to get facelet color string ----------------------------
    auto facelet_color = [&](int seq) -> std::string {
        if (using_cols) {
            char ch = facelets_char[seq];
            if (ch == 't') return "t";
            return abbr_to_hex(ch);
        } else {
            int fi = facelets_int[seq];
            if (fi == T) return "t";
            if (fi < (int)scheme.size()) return scheme[fi];
            return "000000";
        }
    };

    // ---- SVG generators -------------------------------------------------
    auto outline_svg = [&](int fc) -> std::string {
        return "\t\t<polygon fill='#" + cfg.cc + "' stroke='#" + cfg.cc + "' points='" +
            fmt_pt({p[fc][0][0].x*OUTLINE_WIDTH,   p[fc][0][0].y*OUTLINE_WIDTH,   0}) + " " +
            fmt_pt({p[fc][dim][0].x*OUTLINE_WIDTH,  p[fc][dim][0].y*OUTLINE_WIDTH, 0}) + " " +
            fmt_pt({p[fc][dim][dim].x*OUTLINE_WIDTH,p[fc][dim][dim].y*OUTLINE_WIDTH,0}) + " " +
            fmt_pt({p[fc][0][dim].x*OUTLINE_WIDTH,  p[fc][0][dim].y*OUTLINE_WIDTH, 0}) +
            "'/>\n";
    };

    auto gen_facelet_svg = [&](const Vec3& p1, const Vec3& p2,
                               const Vec3& p3, const Vec3& p4, int seq) -> std::string {
        std::string col = facelet_color(seq);
        bool transparent = (col == "t");
        if (transparent) col = "000000";
        return polygon_svg(p1, p2, p3, p4, col, cfg.cc, transparent);
    };

    auto facelet_svg = [&](int fc) -> std::string {
        std::string svg;
        for (int i = 0; i < dim; ++i) {
            for (int j = 0; j < dim; ++j) {
                Vec3 cf{
                    (p[fc][j][i].x + p[fc][j+1][i+1].x) / 2.0f,
                    (p[fc][j][i].y + p[fc][j+1][i+1].y) / 2.0f,
                    0
                };
                Vec3 p1 = trans_scale(p[fc][j  ][i  ], cf, 0.85f);
                Vec3 p2 = trans_scale(p[fc][j+1][i  ], cf, 0.85f);
                Vec3 p3 = trans_scale(p[fc][j+1][i+1], cf, 0.85f);
                Vec3 p4 = trans_scale(p[fc][j  ][i+1], cf, 0.85f);
                svg += gen_facelet_svg(p1, p2, p3, p4, fc * dim * dim + i * dim + j);
            }
        }
        return svg;
    };

    auto oll_svg = [&](int fc) -> std::string {
        std::string svg;
        Vec3 tv1 = scale(rv[fc], 0.00f);
        Vec3 tv2 = scale(rv[fc], 0.20f);
        int i = 0;
        for (int j = 0; j < dim; ++j) {
            Vec3 cf{
                (p[fc][j][i].x + p[fc][j+1][i+1].x) / 2.0f,
                (p[fc][j][i].y + p[fc][j+1][i+1].y) / 2.0f,
                0
            };
            Vec3 p1 = translate(trans_scale(p[fc][j  ][i  ], cf, 0.94f), tv1);
            Vec3 p2 = translate(trans_scale(p[fc][j+1][i  ], cf, 0.94f), tv1);
            Vec3 p3 = translate(trans_scale(p[fc][j+1][i+1], cf, 0.94f), tv2);
            Vec3 p4 = translate(trans_scale(p[fc][j  ][i+1], cf, 0.94f), tv2);
            svg += gen_facelet_svg(p1, p2, p3, p4, fc * dim * dim + i * dim + j);
        }
        return svg;
    };

    auto gen_arrow_svg = [&](const Arrow& a) -> std::string {
        if (a.color == "t" || a.color.empty()) return "";

        // Centre of from-facelet
        Vec3 p1{
            (p[a.from.face][a.from.col][a.from.row].x + p[a.from.face][a.from.col+1][a.from.row+1].x) / 2.0f,
            (p[a.from.face][a.from.col][a.from.row].y + p[a.from.face][a.from.col+1][a.from.row+1].y) / 2.0f,
            0
        };
        Vec3 p2{
            (p[a.to.face][a.to.col][a.to.row].x + p[a.to.face][a.to.col+1][a.to.row+1].x) / 2.0f,
            (p[a.to.face][a.to.col][a.to.row].y + p[a.to.face][a.to.col+1][a.to.row+1].y) / 2.0f,
            0
        };
        Vec3 cp{ (p1.x + p2.x)/2.0f, (p1.y + p2.y)/2.0f, 0 };

        p1 = trans_scale(p1, cp, a.scale);
        p2 = trans_scale(p2, cp, a.scale);

        // Via point
        bool has_pv = false;
        Vec3 pv{};
        if (a.has_via) {
            pv = {
                (p[a.via.face][a.via.col][a.via.row].x + p[a.via.face][a.via.col+1][a.via.row+1].x) / 2.0f,
                (p[a.via.face][a.via.col][a.via.row].y + p[a.via.face][a.via.col+1][a.via.row+1].y) / 2.0f,
                0
            };
            pv = trans_scale(pv, cp, a.via_scale);
            has_pv = true;
        }

        // Rotation angle of arrowhead
        Vec3 ref = has_pv ? pv : p1;
        float rt;
        if (std::abs(p2.x - ref.x) < 1e-6f) {
            rt = ref.y > p2.y ? 270.0f : 90.0f;
        } else {
            rt = (float)(180.0 / M_PI) * std::atan((p2.y - ref.y) / (p2.x - ref.x));
            if (ref.x > p2.x) rt += 180.0f;
        }

        std::string arrow_color = a.color;

        std::string s = "\t\t<path d=\"M " + fmt_f(p1.x) + "," + fmt_f(p1.y) + " ";
        if (has_pv)
            s += "Q " + fmt_f(pv.x) + "," + fmt_f(pv.y) + " ";
        else
            s += "L ";
        s += fmt_f(p2.x) + "," + fmt_f(p2.y) + "\"\n";
        s += "\t\t\tstyle=\"fill:none;stroke:#" + arrow_color + ";stroke-opacity:1\" />\n";
        s += "\t\t<path transform=\" translate(" + fmt_f(p2.x) + "," + fmt_f(p2.y) + ")"
             " scale(" + fmt_f(0.033f / dim) + ") rotate(" + fmt_f(rt) + ")\"\n";
        s += "\t\t\td=\"M 5.77,0.0 L -2.88,5.0 L -2.88,-5.0 L 5.77,0.0 z\"\n";
        s += "\t\t\tstyle=\"fill:#" + arrow_color + ";stroke-width:0;stroke-linejoin:round\"/>\n";
        return s;
    };

    // =========================================================================
    // Compose SVG
    // =========================================================================
    std::ostringstream svg;

    svg << "<?xml version='1.0' standalone='no'?>\n"
        << "<!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN'\n"
        << "'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'>\n\n"
        << "<svg version='1.1' xmlns='http://www.w3.org/2000/svg'\n"
        << "\twidth='" << cfg.size << "' height='" << cfg.size << "'\n"
        << "\tviewBox='" << fmt_f(ox) << " " << fmt_f(oy) << " "
                         << fmt_f(vw) << " " << fmt_f(vh) << "'>\n";

    // Background
    if (!cfg.bg.empty())
        svg << "\t<rect fill='#" << cfg.bg << "' x='" << fmt_f(ox) << "' y='" << fmt_f(oy)
            << "' width='" << fmt_f(vw) << "' height='" << fmt_f(vh) << "'/>\n";

    float co_frac = cfg.co / 100.0f;
    float fo_frac = cfg.fo / 100.0f;

    // Transparency background rendering (back 3 faces)
    if (cfg.co < 100) {
        svg << "\t<g style='opacity:" << fmt_f(fo_frac)
            << ";stroke-opacity:0.5;stroke-width:" << fmt_f(sw) << ";stroke-linejoin:round'>\n";
        for (int ri = 0; ri < 3; ++ri) svg << facelet_svg(ro[ri]);
        svg << "\t</g>\n";

        svg << "\t<g style='stroke-width:0.1;stroke-linejoin:round;opacity:"
            << fmt_f(co_frac) << "'>\n";
        for (int ri = 0; ri < 3; ++ri) svg << outline_svg(ro[ri]);
        svg << "\t</g>\n";
    }

    // Outlines for visible faces
    svg << "\t<g style='stroke-width:0.1;stroke-linejoin:round;opacity:"
        << fmt_f(co_frac) << "'>\n";
    for (int ri = 3; ri < 6; ++ri)
        if (face_visible(ro[ri]) || cfg.co < 100) svg << outline_svg(ro[ri]);
    svg << "\t</g>\n";

    // Facelets for visible faces
    svg << "\t<g style='opacity:" << fmt_f(fo_frac)
        << ";stroke-opacity:0.5;stroke-width:" << fmt_f(sw) << ";stroke-linejoin:round'>\n";
    for (int ri = 3; ri < 6; ++ri)
        if (face_visible(ro[ri]) || cfg.co < 100) svg << facelet_svg(ro[ri]);
    svg << "\t</g>\n";

    // OLL/plan view guides
    if (cfg.view == View::Plan) {
        svg << "\t<g style='opacity:" << fmt_f(fo_frac)
            << ";stroke-opacity:1;stroke-width:0.02;stroke-linejoin:round'>\n";
        for (int fc : {F, L, B, R}) svg << oll_svg(fc);
        svg << "\t</g>\n";
    }

    // Arrows
    if (!arrows.empty()) {
        float awidth = 0.12f / dim;
        svg << "\t<g style='opacity:1;stroke-opacity:1;stroke-width:"
            << fmt_f(awidth) << ";stroke-linecap:round'>\n";
        for (const auto& a : arrows) svg << gen_arrow_svg(a);
        svg << "\t</g>\n";
    }

    svg << "</svg>\n";
    return svg.str();
}

} // namespace vc
