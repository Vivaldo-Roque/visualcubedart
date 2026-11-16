// Build hook (Native Assets): compiles the VisualCube C++ library for the
// current target OS/architecture. Replaces the per-platform CMake / Gradle NDK
// / CocoaPods setup previously used by the `ffiPlugin` configuration.
import 'package:code_assets/code_assets.dart';
import 'package:hooks/hooks.dart';
import 'package:native_toolchain_c/native_toolchain_c.dart';

void main(List<String> args) async {
  await build(args, (input, output) async {
    if (!input.config.buildCodeAssets) return;

    final targetOS = input.config.code.targetOS;

    final builder = CBuilder.library(
      name: 'visualcubedart',
      // Resolves to `package:visualcubedart/visualcubedart.dart`, the library
      // holding the `@Native` declarations.
      assetName: 'visualcubedart.dart',
      sources: const [
        'src/ffi.cpp',
        'src/visualcubecpp_src/facelet.cpp',
        'src/visualcubecpp_src/renderer.cpp',
        'src/visualcubecpp_src/image_export.cpp',
        'src/visualcubecpp_src/visualcube.cpp',
      ],
      includes: const [
        'src',
        'src/visualcubecpp_include',
        'src/visualcubecpp_third_party',
      ],
      language: Language.cpp,
      std: 'c++17',
      defines: const {'DART_SHARED_LIB': null},
      // MSVC needs explicit C++ exception handling semantics.
      flags: targetOS == OS.windows ? const ['/EHsc'] : const [],
      // Android 16 KB page alignment is applied by native_toolchain_c itself.
    );

    await builder.run(input: input, output: output);
  });
}
