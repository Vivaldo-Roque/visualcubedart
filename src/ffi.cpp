#include "visualcubecpp_include/visualcube/visualcube.hpp"
#include <cstring>
#include <string>
#include <vector>
#include <cstdlib>

#if defined(_WIN32)
#define FFI_EXPORT __declspec(dllexport)
#else
#define FFI_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {

FFI_EXPORT const char* generate_cube_svg(
    int dim,
    int size,
    const char* view,
    const char* bg,
    const char* cc,
    int co,
    int fo,
    const char* fc,
    const char* fd,
    const char* alg,
    const char* case_alg,
    const char* stage,
    const char* scheme,
    const char* arrows,
    const char* ac,
    int rot_x,
    int rot_y,
    int rot_z
) {
    try {
        vc::Config cfg;
        cfg.dim = dim;
        cfg.size = size;
        cfg.fmt = vc::Fmt::SVG;

        if (view) {
            std::string v(view);
            if (v == "plan") cfg.view = vc::View::Plan;
            else if (v == "trans") cfg.view = vc::View::Trans;
            else cfg.view = vc::View::Normal;
        }

        if (bg) cfg.bg = bg;
        if (cc) cfg.cc = cc;
        cfg.co = co;
        cfg.fo = fo;

        if (fc) cfg.fc = fc;
        if (fd) cfg.fd = fd;
        if (alg) cfg.alg = alg;
        if (case_alg) cfg.case_alg = case_alg;
        if (stage) cfg.stage = stage;
        if (scheme) cfg.scheme = scheme;
        if (arrows) cfg.arrows = arrows;
        if (ac) cfg.ac = ac;

        // VisualCube C++ expects rotation as a vector of Rotations.
        // Let's clear default rotation and push our rot_x, rot_y, rot_z.
        cfg.rotation.clear();
        cfg.rotation.push_back({ 0, rot_x });
        cfg.rotation.push_back({ 1, rot_y });
        cfg.rotation.push_back({ 2, rot_z });

        std::string svg = vc::generate_svg(cfg);
        
        // Allocate C-string that Dart must free
        size_t len = svg.length();
        char* res = (char*)malloc(len + 1);
        if (!res) return nullptr;
        std::memcpy(res, svg.c_str(), len);
        res[len] = '\0';
        return res;
    } catch (...) {
        return nullptr;
    }
}

FFI_EXPORT void free_svg_string(const char* str) {
    if (str) {
        free((void*)str);
    }
}

}
