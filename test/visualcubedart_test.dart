import 'package:flutter_test/flutter_test.dart';
import 'package:visualcubedart/visualcubedart.dart';

void main() {
  group('generateCubeSvg - Basic Rendering', () {
    test('renders default 3x3 cube SVG with valid svg tags', () {
      final svg = generateCubeSvg(
        dim: 3,
        size: 200,
        co: 0,
        fo: 100,
        rotX: 0,
        rotY: 0,
        rotZ: 0,
      );

      expect(svg, isNotNull);
      expect(svg!.isNotEmpty, isTrue);
      expect(svg, contains('<svg'));
      expect(svg, contains('</svg>'));
      expect(svg, contains('viewBox'));
    });

    test('renders cubes of different dimensions (2x2, 3x3, 4x4, 5x5)', () {
      for (final dim in [2, 3, 4, 5]) {
        final svg = generateCubeSvg(
          dim: dim,
          size: 150,
          co: 0,
          fo: 100,
          rotX: 0,
          rotY: 0,
          rotZ: 0,
        );

        expect(svg, isNotNull, reason: 'Dimension $dim should generate SVG');
        expect(svg!, contains('<svg'));
      }
    });
  });

  group('generateCubeSvg - View Modes', () {
    test('renders plan view', () {
      final svg = generateCubeSvg(
        dim: 3,
        size: 200,
        view: 'plan',
        co: 0,
        fo: 100,
        rotX: 0,
        rotY: 0,
        rotZ: 0,
      );

      expect(svg, isNotNull);
      expect(svg!, contains('<svg'));
    });

    test('renders trans (translucent) view', () {
      final svg = generateCubeSvg(
        dim: 3,
        size: 200,
        view: 'trans',
        co: 0,
        fo: 100,
        rotX: 0,
        rotY: 0,
        rotZ: 0,
      );

      expect(svg, isNotNull);
      expect(svg!, contains('<svg'));
    });
  });

  group('generateCubeSvg - CFOP Stages', () {
    const stages = ['f2l', 'oll', 'pll', 'cll', 'ell', 'cross', 'fl'];

    for (final stage in stages) {
      test('renders stage "$stage" successfully', () {
        final svg = generateCubeSvg(
          dim: 3,
          size: 200,
          stage: stage,
          co: 0,
          fo: 100,
          rotX: 0,
          rotY: 0,
          rotZ: 0,
        );

        expect(svg, isNotNull, reason: 'Stage $stage should produce valid SVG');
        expect(svg!, contains('<svg'));
      });
    }
  });

  group('generateCubeSvg - Algorithms & Visual Markers', () {
    test('renders with algorithm and case algorithm', () {
      final svg = generateCubeSvg(
        dim: 3,
        size: 200,
        alg: "R U R' U'",
        caseAlg: "F R U R' U' F'",
        stage: 'oll',
        co: 0,
        fo: 100,
        rotX: 10,
        rotY: 20,
        rotZ: 0,
      );

      expect(svg, isNotNull);
      expect(svg!, contains('<svg'));
    });

    test('renders with custom arrows and arrow colors', () {
      final svg = generateCubeSvg(
        dim: 3,
        size: 200,
        stage: 'pll',
        arrows: 'U0U2,U2U8',
        ac: 'yellow',
        co: 0,
        fo: 100,
        rotX: 0,
        rotY: 0,
        rotZ: 0,
      );

      expect(svg, isNotNull);
      expect(svg!, contains('<svg'));
    });

    test('renders with custom colors and background', () {
      final svg = generateCubeSvg(
        dim: 3,
        size: 200,
        bg: 'black',
        cc: 'white',
        scheme: 'wrgyob',
        co: 10,
        fo: 80,
        rotX: 15,
        rotY: 30,
        rotZ: 45,
      );

      expect(svg, isNotNull);
      expect(svg!, contains('<svg'));
    });
  });

  group('generateCubeSvg - Memory Safety & Stress', () {
    test('generates multiple SVGs consecutively without crashes or memory corruption', () {
      for (var i = 0; i < 25; i++) {
        final svg = generateCubeSvg(
          dim: 3,
          size: 100 + (i * 2),
          stage: i % 2 == 0 ? 'oll' : 'pll',
          alg: "R U R' U'",
          co: 0,
          fo: 100,
          rotX: i,
          rotY: i * 2,
          rotZ: 0,
        );

        expect(svg, isNotNull);
        expect(svg!, contains('<svg'));
      }
    });
  });
}
