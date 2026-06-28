# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.10)

project(visualcubedart_library VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(visualcubedart SHARED
  "ffi.cpp"
  "visualcubecpp_src/facelet.cpp"
  "visualcubecpp_src/renderer.cpp"
  "visualcubecpp_src/image_export.cpp"
  "visualcubecpp_src/visualcube.cpp"
)

target_include_directories(visualcubedart
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/visualcubecpp_include
    ${CMAKE_CURRENT_SOURCE_DIR}/visualcubecpp_third_party
)

set_target_properties(visualcubedart PROPERTIES
  OUTPUT_NAME "visualcubedart"
)

target_compile_definitions(visualcubedart PUBLIC DART_SHARED_LIB)

if (ANDROID)
  # Support Android 15 16k page size
  target_link_options(visualcubedart PRIVATE "-Wl,-z,max-page-size=16384")
endif()
