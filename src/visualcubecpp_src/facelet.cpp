/*
    facelet.cpp — VisualCube C++ Port
    NxNxN facelet permutation system.

    Copyright (C) 2010 Conrad Rider (original PHP)
    C++ port: same LGPL v3 licence applies.
*/

#include "visualcube/facelet.hpp"
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <cmath>

namespace vc {

// =========================================================================
// Move ID / power helpers
// =========================================================================

int fcs_move_id(char c) {
    switch (c) {
        case 'y': return 0;  case 'x': return 1;  case 'z': return 2;
        case 'E': return 3;  case 'M': return 4;  case 'S': return 5;
        case 'U': return 6;  case 'R': return 7;  case 'F': return 8;
        case 'D': return 9;  case 'L': return 10; case 'B': return 11;
        case 'u': return 12; case 'r': return 13; case 'f': return 14;
        case 'd': return 15; case 'l': return 16; case 'b': return 17;
    }
    return -1;
}

int move_pow(char c) {
    if (c == '2') return 2;
    if (c == '\'' || c == '3' || c == '`') return 3;
    return 1;
}

// =========================================================================
// Algorithm string processing
// =========================================================================

std::string fcs_format_alg(const std::string& alg) {
    std::string r;
    r.reserve(alg.size());

    // 1. URL-decode %27 → ' and %20 → space
    for (size_t i = 0; i < alg.size(); ++i) {
        if (alg[i] == '%' && i + 2 < alg.size()) {
            if (alg.substr(i, 3) == "%27") { r += '\''; i += 2; continue; }
            if (alg.substr(i, 3) == "%20") { r += ' ';  i += 2; continue; }
        }
        r += alg[i];
    }

    // 2. Keep only legal chars
    {
        std::string filtered;
        filtered.reserve(r.size());
        const std::string legal = "UDLRFBudlrfbMESxyzw'`234567890 ";
        for (char c : r)
            if (legal.find(c) != std::string::npos)
                filtered += c;
        r = std::move(filtered);
    }

    // 3. Normalise: ` → '
    for (char& c : r) if (c == '`') c = '\'';

    // 4. Normalise: 2' or '2 → 2
    {
        std::string tmp;
        tmp.reserve(r.size());
        for (size_t i = 0; i < r.size(); ++i) {
            if (i + 1 < r.size()) {
                if ((r[i] == '2' && r[i+1] == '\'') ||
                    (r[i] == '\'' && r[i+1] == '2')) {
                    tmp += '2'; ++i; continue;
                }
            }
            tmp += r[i];
        }
        r = std::move(tmp);
    }

    // 5. Wide notation: Rw → r, etc.
    if (r.find('w') != std::string::npos) {
        std::string tmp;
        tmp.reserve(r.size());
        for (size_t i = 0; i < r.size(); ++i) {
            if (i + 1 < r.size() && r[i+1] == 'w') {
                char wide = (char)std::tolower((unsigned char)r[i]);
                tmp += wide; ++i;
            } else if (r[i] != 'w') {
                tmp += r[i];
            }
        }
        r = std::move(tmp);
    }

    return r;
}

// =========================================================================
// Parse algorithm into move-ID sequence (with power expansion)
// =========================================================================

std::vector<int> fcs_parse_alg(const std::string& alg, int dim) {
    std::vector<int> moves;
    moves.reserve(alg.size());

    int pre = 0;
    for (size_t i = 0; i < alg.size(); ++i) {
        char c = alg[i];

        if (std::isdigit((unsigned char)c)) {
            pre = c - '0';
            continue;
        }

        int mv = fcs_move_id(c);
        if (mv >= 0) {
            int pow = 1;
            if (i + 1 < alg.size()) {
                int p = move_pow(alg[i + 1]);
                if (p != 1) { pow = p; ++i; }
            }

            // Clamp prefix
            if (pre < 1) pre = 1;
            if (pre > dim - 1) pre = dim - 1;

            // Wide moves (lowercase urfdlb) need pre >= 2
            if (mv >= 12) { mv -= 6; if (pre < 2) pre = 2; }

            // Compute final move index
            int final_mv = (mv < 6)
                ? mv
                : 6 + (mv - 6) * (dim - 1) + (pre - 1);

            for (int k = 0; k < pow; ++k)
                moves.push_back(final_mv);

            pre = 1;
        } else {
            pre = 1;
        }
    }
    return moves;
}

// =========================================================================
// Invert algorithm (mirrors PHP invert_alg exactly)
// =========================================================================

std::string invert_alg(const std::string& alg) {
    static const char* ALG_POW[] = {"", "2", "'"};

    std::string inv;
    int pow = 1;
    int i = (int)alg.size() - 1;

    while (i >= 0) {
        char c = alg[i];
        int mv = fcs_move_id(c);
        if (mv != -1) {
            // Check for numeric layer prefix before the move letter
            std::string pre;
            if (i > 0) {
                char prev = alg[i - 1];
                if (std::isdigit((unsigned char)prev)) {
                    // Is it a prefix, or a power suffix of the previous move?
                    if (i > 1 && fcs_move_id(alg[i - 2]) != -1) {
                        // Char before digit is a move → digit is that move's power, not our prefix
                        pre = "";
                    } else {
                        pre = std::string(1, prev);
                        --i;
                    }
                }
            }
            // Invert power: 1→'(idx2), 2→2(idx1), 3→nothing(idx0)
            inv += pre;
            inv += c;
            inv += ALG_POW[3 - pow];
            inv += ' ';
            pow = 1;
        } else {
            pow = move_pow(c);
        }
        --i;
    }
    return inv;
}

// =========================================================================
// Internal: facelet position helper (mirrors PHP fcs_pos exactly)
// =========================================================================

static int fcs_pos(int r, int c, int o, int d) {
    int n = d * d;
    switch (o) {
        case 1: return d * r + c;
        case 2: return d * (r + 1) - 1 - c;
        case 3: return n - d * (r + 1) + c;
        case 4: return n - d * r - 1 - c;
        case 5: return d * c + r;
        case 6: return d * (c + 1) - 1 - r;
        case 7: return n - d * (c + 1) + r;
        case 8: return n - d * c - 1 - r;
    }
    return 0;
}

// =========================================================================
// fcs_union helpers
//
// The PHP version uses associative arrays (only set keys are iterated).
// We use two overloads:
//   - dense: for fc_twist / fc_twist2 (every element is valid)
//   - sparse: for sl_twist (unordered_map, only set entries)
// =========================================================================

using SparseMap = std::unordered_map<int, int>;

// Dense: t1[k + offset] = patch[k] + offset, for all k in patch
static void fcs_union_dense(std::vector<int>& t1,
                            const std::vector<int>& patch, int offset) {
    for (int k = 0; k < (int)patch.size(); ++k)
        t1[k + offset] = patch[k] + offset;
}

// Sparse: t1[k + offset] = v + offset, only for entries that exist in map
static void fcs_union_sparse(std::vector<int>& t1,
                             const SparseMap& patch, int offset) {
    for (const auto& [k, v] : patch)
        t1[k + offset] = v + offset;
}

// =========================================================================
// Core: build move tables and execute permutation
// =========================================================================

template<typename T>
static std::vector<T> fcs_doperm_impl(std::vector<T> fcs, const std::string& alg, int dim) {
    int nf = dim * dim;
    int total = nf * 6;

    // ---- Face twist tables (clockwise and counter-clockwise) ----
    std::vector<int> fc_twist(nf), fc_twist2(nf);
    for (int row = 0; row < dim; ++row) {
        for (int col = 0; col < dim; ++col) {
            fc_twist [row * dim + col] = (dim - col - 1) * dim + row;
            fc_twist2[row * dim + col] = col * dim + (dim - row - 1);
        }
    }

    // ---- Slice twist tables (SPARSE — using unordered_map) ----

    // Face order for slice turns (6 directions × 6 faces; 6 = null/skip)
    static const int sl_fo[6][6] = {
        {6, 5, 1, 6, 2, 4}, // x
        {2, 6, 3, 5, 6, 0}, // y
        {4, 0, 6, 1, 3, 6}, // z
        {6, 2, 4, 6, 5, 1}, // x'
        {5, 6, 0, 2, 6, 3}, // y'
        {1, 3, 6, 4, 0, 6}, // z'
    };
    // Sticker orientation for slice turns (0 = face not involved)
    static const int sl_so[3][6] = {
        {0, 1, 1, 0, 1, 1}, // x
        {8, 0, 8, 8, 0, 5}, // y
        {4, 7, 0, 1, 6, 0}, // z
    };

    // sl_twist[lr][sl] → SparseMap { src_pos → dst_pos }
    std::vector<std::vector<SparseMap>> sl_twist(6, std::vector<SparseMap>(dim));

    for (int lr = 0; lr < 6; ++lr) {
        for (int sl = 0; sl < dim; ++sl) {
            auto& t = sl_twist[lr][sl];
            for (int fc = 0; fc < 6; ++fc) {
                int so = sl_so[lr % 3][fc];
                if (so == 0) continue;
                int dst_fc = sl_fo[lr][fc];
                if (dst_fc == 6) continue; // null face
                int dst_so = sl_so[lr % 3][dst_fc];
                for (int ci = 0; ci < dim; ++ci) {
                    int src_pos = fc * nf + fcs_pos(sl, ci, so, dim);
                    int dst_pos = dst_fc * nf + fcs_pos(sl, ci, dst_so, dim);
                    t[src_pos] = dst_pos;
                }
            }
        }
    }

    // ---- Build per-move complete permutation tables ----
    int n_moves = 6 + 6 * (dim - 1);
    std::vector<std::vector<int>> fc_moves(n_moves, std::vector<int>(total));
    // Initialise to identity
    for (auto& mv : fc_moves)
        for (int j = 0; j < total; ++j) mv[j] = j;

    // Rotations (0=y, 1=x, 2=z)
    for (int i = 0; i < 3; ++i) {
        // All slice moves for this axis
        for (int j = 0; j < dim; ++j)
            fcs_union_sparse(fc_moves[i], sl_twist[i][j], 0);
        // Twist the two end faces
        fcs_union_dense(fc_moves[i], fc_twist,  i * nf);
        fcs_union_dense(fc_moves[i], fc_twist2, (i + 3) * nf);
    }

    // Centre-slice moves (E=3, M=4, S=5) — only for odd-dim cubes
    if (dim % 2 == 1) {
        int half = dim / 2;
        fcs_union_sparse(fc_moves[3], sl_twist[3][half], 0); // E
        fcs_union_sparse(fc_moves[4], sl_twist[4][half], 0); // M
        fcs_union_sparse(fc_moves[5], sl_twist[2][half], 0); // S
    }

    // Normal face moves
    for (int ax = 0; ax < 3; ++ax) {
        for (int dp = 0; dp < dim - 1; ++dp) {
            // Primary axis end (U/R/F for ax=0/1/2)
            int idx_p = 6 + ax * (dim - 1) + dp;
            fcs_union_dense(fc_moves[idx_p], fc_twist, ax * nf);
            for (int sl = 0; sl <= dp; ++sl)
                fcs_union_sparse(fc_moves[idx_p], sl_twist[ax][sl], 0);

            // Secondary axis end (D/L/B for ax=0/1/2)
            int idx_s = 6 + (3 + ax) * (dim - 1) + dp;
            fcs_union_dense(fc_moves[idx_s], fc_twist, (3 + ax) * nf);
            for (int sl = 0; sl <= dp; ++sl)
                fcs_union_sparse(fc_moves[idx_s], sl_twist[3 + ax][dim - sl - 1], 0);
        }
    }

    // ---- Execute moves ----
    std::vector<int> move_seq = fcs_parse_alg(alg, dim);

    std::vector<T> fcs2(fcs.size());
    for (int mv : move_seq) {
        if (mv < 0 || mv >= n_moves) continue;
        const auto& perm = fc_moves[mv];
        for (size_t idx = 0; idx < fcs.size(); ++idx)
            fcs2[idx] = fcs[perm[idx]];
        std::swap(fcs, fcs2);
    }

    return fcs;
}

std::vector<char> fcs_doperm(std::vector<char> fcs, const std::string& alg, int dim) {
    return fcs_doperm_impl(std::move(fcs), alg, dim);
}

std::vector<int> fcs_doperm(std::vector<int> fcs, const std::string& alg, int dim) {
    return fcs_doperm_impl(std::move(fcs), alg, dim);
}

// =========================================================================
// Stage masks
// =========================================================================

std::string stage_mask(const std::string& stage, int dim) {
    if (dim == 3) {
        if (stage == "fl")     return "000000000000000111000000111111111111000000111000000111";
        if (stage == "f2l")    return "000000000000111111000111111111111111000111111000111111";
        if (stage == "ll")     return "111111111111000000111000000000000000111000000111000000";
        if (stage == "cll")    return "101010101101000000101000000000000000101000000101000000";
        if (stage == "ell")    return "010111010010000000010000000000000000010000000010000000";
        if (stage == "oll")    return "111111111000000000000000000000000000000000000000000000";
        if (stage == "ocll")   return "101010101000000000000000000000000000000000000000000000";
        if (stage == "oell")   return "010111010000000000000000000000000000000000000000000000";
        if (stage == "coll")   return "111111111101000000101000000000000000101000000101000000";
        if (stage == "ocell")  return "111111111010000000010000000000000000010000000010000000";
        if (stage == "wv")     return "111111111000111111000111111111111111000111111000111111";
        if (stage == "vh")     return "010111010000111111000111111111111111000111111000111111";
        if (stage == "els")    return "010111010000111011000111110110111111000111111000111111";
        if (stage == "cls")    return "111111111000111111000111111111111111000111111000111111";
        if (stage == "cmll")   return "101000101101111111101101101101101101101111111101101101";
        if (stage == "cross")  return "000000000000010010000010010010111010000010010000010010";
        if (stage == "f2l_3")  return "000000000000110110000011011011111010000010010000010010";
        if (stage == "f2l_2")  return "000000000000011011000010010010111111000110110000111111";
        if (stage == "f2l_sm") return "000000000000110110000011011011111110000110110000011011";
        if (stage == "f2l_1")  return "000000000000011011000110110110111111000111111000111111";
        if (stage == "f2b")    return "000000000000111111000101101101101101000111111000101101";
        if (stage == "line")   return "000000000000000000000010010010010010000000000000010010";
        if (stage == "2x2x2")  return "000000000000110110000011011011011000000000000000000000";
        if (stage == "2x2x3")  return "000000000000110110000111111111111000000011011000000000";
    } else if (dim == 2) {
        if (stage == "fl")  return "000000110011111100110011";
        if (stage == "ll")  return "111111001100000011001100";
        if (stage == "oll") return "111100000000111100000000";
    }
    return "";
}

} // namespace vc
