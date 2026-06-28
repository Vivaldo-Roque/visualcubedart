#pragma once
/*
    facelet.hpp — VisualCube C++ Port
    NxNxN facelet permutation system.
    Port of fcs_doperm, fcs_parse_alg, fcs_format_alg, etc. from cube_lib.php.

    Copyright (C) 2010 Conrad Rider (original PHP)
    C++ port: same LGPL v3 licence applies.
*/

#include <string>
#include <vector>

namespace vc {

// ---- Move ID mapping (mirrors fcs_move_id) ------------------------------
// Returns move index ≥ 0, or -1 if char is not a move.
int fcs_move_id(char c);

// Returns the power of a move suffix character:
//   '2' → 2, '\'' / '3' / '`' → 3, anything else → 1
int move_pow(char c);

// ---- Algorithm string processing ----------------------------------------

// Formats/cleans an algorithm string for NxN cubes.
// Removes illegal chars, normalises wide notation (Rw→r), etc.
std::string fcs_format_alg(const std::string& alg);

// Parses a formatted algorithm string into a sequence of move IDs.
// Each move is already expanded by power (e.g. R2 → [R, R]).
// dim is needed to compute wide-move indices.
std::vector<int> fcs_parse_alg(const std::string& alg, int dim);

// Inverts an NxN algorithm string (reverses order, flips powers).
std::string invert_alg(const std::string& alg);

// ---- Core facelet permutation -------------------------------------------

// Permutes a facelet state by the given algorithm.
// fcs: 6*dim*dim elements (one char or int per facelet).
//      Can be either:
//        • std::vector<char>  holding color abbreviation chars ('y','r',…)
//        • std::vector<int>   holding face indices (0=U,1=R,…)
// alg: already formatted algorithm string (use fcs_format_alg first)
// dim: cube dimension
//
// Two overloads — char version (fc/fd strings) and int version (stage masks).
std::vector<char> fcs_doperm(std::vector<char> fcs, const std::string& alg, int dim);
std::vector<int>  fcs_doperm(std::vector<int>  fcs, const std::string& alg, int dim);

// ---- Stage mask helpers --------------------------------------------------

// Returns the stage mask string for a given stage name and dimension.
// Returns "" if stage is unknown for that dimension.
std::string stage_mask(const std::string& stage, int dim);

} // namespace vc
