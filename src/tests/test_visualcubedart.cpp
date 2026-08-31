#include <iostream>
#include <cstring>
#include <string>
#include <cstdlib>

// Forward declarations of FFI functions exported by ffi.cpp
extern "C" {
const char* generate_cube_svg(
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
);

void free_svg_string(const char* str);
}

static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[FAIL] " << msg << " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl; \
            g_tests_failed++; \
        } else { \
            std::cout << "[PASS] " << msg << std::endl; \
            g_tests_passed++; \
        } \
    } while (0)

void test_default_cube_svg() {
    std::cout << "\n--- Running test_default_cube_svg ---" << std::endl;
    const char* svg = generate_cube_svg(
        3, 200, nullptr, nullptr, nullptr, 0, 100, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0, 0, 0
    );

    TEST_ASSERT(svg != nullptr, "SVG output is not null");
    if (svg) {
        std::string s(svg);
        TEST_ASSERT(s.find("<svg") != std::string::npos, "SVG contains <svg tag");
        TEST_ASSERT(s.find("</svg>") != std::string::npos, "SVG contains </svg> tag");
        TEST_ASSERT(s.find("viewBox") != std::string::npos, "SVG contains viewBox attribute");
        free_svg_string(svg);
    }
}

void test_views() {
    std::cout << "\n--- Running test_views ---" << std::endl;
    // Plan view
    const char* svg_plan = generate_cube_svg(
        3, 150, "plan", nullptr, nullptr, 0, 100, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0, 0, 0
    );
    TEST_ASSERT(svg_plan != nullptr, "Plan view SVG is not null");
    if (svg_plan) {
        std::string s(svg_plan);
        TEST_ASSERT(s.find("<svg") != std::string::npos, "Plan view contains <svg");
        free_svg_string(svg_plan);
    }

    // Trans view
    const char* svg_trans = generate_cube_svg(
        3, 150, "trans", nullptr, nullptr, 0, 100, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0, 0, 0
    );
    TEST_ASSERT(svg_trans != nullptr, "Trans view SVG is not null");
    if (svg_trans) {
        std::string s(svg_trans);
        TEST_ASSERT(s.find("<svg") != std::string::npos, "Trans view contains <svg");
        free_svg_string(svg_trans);
    }
}

void test_cfop_stages() {
    std::cout << "\n--- Running test_cfop_stages ---" << std::endl;
    const char* stages[] = {"f2l", "oll", "pll", "cll", "ell", "cross", "fl"};
    for (const char* stage : stages) {
        const char* svg = generate_cube_svg(
            3, 150, nullptr, nullptr, nullptr, 0, 100, nullptr, nullptr,
            nullptr, nullptr, stage, nullptr, nullptr, nullptr, 0, 0, 0
        );
        std::string msg = std::string("Stage '") + stage + "' generates valid SVG";
        TEST_ASSERT(svg != nullptr, msg.c_str());
        if (svg) {
            free_svg_string(svg);
        }
    }
}

void test_algorithms_and_arrows() {
    std::cout << "\n--- Running test_algorithms_and_arrows ---" << std::endl;
    const char* svg = generate_cube_svg(
        3, 200, nullptr, nullptr, nullptr, 0, 100, nullptr, nullptr,
        "R U R' U'", "F R U R' U' F'", "oll", nullptr, "U0U2,U2U8", "yellow", 10, 20, 0
    );
    TEST_ASSERT(svg != nullptr, "SVG with algorithm and arrows is generated");
    if (svg) {
        std::string s(svg);
        TEST_ASSERT(s.find("<svg") != std::string::npos, "Algorithm SVG contains <svg tag");
        free_svg_string(svg);
    }
}

void test_dimensions() {
    std::cout << "\n--- Running test_dimensions ---" << std::endl;
    int dims[] = {2, 3, 4, 5};
    for (int dim : dims) {
        const char* svg = generate_cube_svg(
            dim, 120, nullptr, nullptr, nullptr, 0, 100, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0, 0, 0
        );
        std::string msg = std::string("Dimension ") + std::to_string(dim) + "x" + std::to_string(dim) + " generates SVG";
        TEST_ASSERT(svg != nullptr, msg.c_str());
        if (svg) {
            free_svg_string(svg);
        }
    }
}

void test_null_and_free_safety() {
    std::cout << "\n--- Running test_null_and_free_safety ---" << std::endl;
    // free_svg_string with nullptr should not crash
    free_svg_string(nullptr);
    TEST_ASSERT(true, "free_svg_string(nullptr) handled safely");
}

int main() {
    std::cout << "==========================================" << std::endl;
    std::cout << "  Running VisualCubeDart Native C++ Tests " << std::endl;
    std::cout << "==========================================" << std::endl;

    test_default_cube_svg();
    test_views();
    test_cfop_stages();
    test_algorithms_and_arrows();
    test_dimensions();
    test_null_and_free_safety();

    std::cout << "\n==========================================" << std::endl;
    std::cout << "  Summary: " << g_tests_passed << " passed, " << g_tests_failed << " failed" << std::endl;
    std::cout << "==========================================" << std::endl;

    return g_tests_failed == 0 ? 0 : 1;
}
