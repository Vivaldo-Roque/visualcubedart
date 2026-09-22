import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart';

export 'src/arrow_utils.dart';
export 'src/cube_simulator.dart';

const String _libName = 'visualcubedart';

final DynamicLibrary _dylib = () {
  if (Platform.isMacOS || Platform.isIOS) {
    return DynamicLibrary.open('$_libName.framework/$_libName');
  }
  if (Platform.isAndroid || Platform.isLinux) {
    return DynamicLibrary.open('lib$_libName.so');
  }
  if (Platform.isWindows) {
    const dllName = '$_libName.dll';
    final candidates = [
      dllName,
      '../../$dllName',
      '../$dllName',
      'build/windows_x64/shared/Debug/$dllName',
      'build/windows_x64/shared/Release/$dllName',
      '../build/windows_x64/shared/Debug/$dllName',
    ];
    for (final path in candidates) {
      if (File(path).existsSync()) {
        try {
          return DynamicLibrary.open(File(path).absolute.path);
        } catch (_) {}
      }
    }
    return DynamicLibrary.open(dllName);
  }
  throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
}();

typedef _GenerateCubeSvgC = Pointer<Utf8> Function(
  Int32 dim,
  Int32 size,
  Pointer<Utf8> view,
  Pointer<Utf8> bg,
  Pointer<Utf8> cc,
  Int32 co,
  Int32 fo,
  Pointer<Utf8> fc,
  Pointer<Utf8> fd,
  Pointer<Utf8> alg,
  Pointer<Utf8> caseAlg,
  Pointer<Utf8> stage,
  Pointer<Utf8> scheme,
  Pointer<Utf8> arrows,
  Pointer<Utf8> ac,
  Int32 rotX,
  Int32 rotY,
  Int32 rotZ,
);

typedef _GenerateCubeSvgDart = Pointer<Utf8> Function(
  int dim,
  int size,
  Pointer<Utf8> view,
  Pointer<Utf8> bg,
  Pointer<Utf8> cc,
  int co,
  int fo,
  Pointer<Utf8> fc,
  Pointer<Utf8> fd,
  Pointer<Utf8> alg,
  Pointer<Utf8> caseAlg,
  Pointer<Utf8> stage,
  Pointer<Utf8> scheme,
  Pointer<Utf8> arrows,
  Pointer<Utf8> ac,
  int rotX,
  int rotY,
  int rotZ,
);

typedef _FreeSvgStringC = Void Function(Pointer<Utf8> str);
typedef _FreeSvgStringDart = void Function(Pointer<Utf8> str);

final _GenerateCubeSvgDart _generateCubeSvg = _dylib
    .lookup<NativeFunction<_GenerateCubeSvgC>>('generate_cube_svg')
    .asFunction();

final _FreeSvgStringDart _freeSvgString = _dylib
    .lookup<NativeFunction<_FreeSvgStringC>>('free_svg_string')
    .asFunction();

String? generateCubeSvg({
  required int dim,
  required int size,
  String? view,
  String? bg,
  String? cc,
  required int co,
  required int fo,
  String? fc,
  String? fd,
  String? alg,
  String? caseAlg,
  String? stage,
  String? scheme,
  String? arrows,
  String? ac,
  required int rotX,
  required int rotY,
  required int rotZ,
}) {
  final viewPtr = view?.toNativeUtf8();
  final bgPtr = bg?.toNativeUtf8();
  final ccPtr = cc?.toNativeUtf8();
  final fcPtr = fc?.toNativeUtf8();
  final fdPtr = fd?.toNativeUtf8();
  final algPtr = alg?.toNativeUtf8();
  final caseAlgPtr = caseAlg?.toNativeUtf8();
  final stagePtr = stage?.toNativeUtf8();
  final schemePtr = scheme?.toNativeUtf8();
  final arrowsPtr = arrows?.toNativeUtf8();
  final acPtr = ac?.toNativeUtf8();

  try {
    final resultPtr = _generateCubeSvg(
      dim,
      size,
      viewPtr ?? nullptr,
      bgPtr ?? nullptr,
      ccPtr ?? nullptr,
      co,
      fo,
      fcPtr ?? nullptr,
      fdPtr ?? nullptr,
      algPtr ?? nullptr,
      caseAlgPtr ?? nullptr,
      stagePtr ?? nullptr,
      schemePtr ?? nullptr,
      arrowsPtr ?? nullptr,
      acPtr ?? nullptr,
      rotX,
      rotY,
      rotZ,
    );

    if (resultPtr == nullptr) return null;

    final result = resultPtr.toDartString();
    _freeSvgString(resultPtr);
    return result;
  } finally {
    if (viewPtr != null) calloc.free(viewPtr);
    if (bgPtr != null) calloc.free(bgPtr);
    if (ccPtr != null) calloc.free(ccPtr);
    if (fcPtr != null) calloc.free(fcPtr);
    if (fdPtr != null) calloc.free(fdPtr);
    if (algPtr != null) calloc.free(algPtr);
    if (caseAlgPtr != null) calloc.free(caseAlgPtr);
    if (stagePtr != null) calloc.free(stagePtr);
    if (schemePtr != null) calloc.free(schemePtr);
    if (arrowsPtr != null) calloc.free(arrowsPtr);
    if (acPtr != null) calloc.free(acPtr);
  }
}
