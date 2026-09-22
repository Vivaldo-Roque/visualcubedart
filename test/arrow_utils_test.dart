import 'package:flutter_test/flutter_test.dart';
import 'package:visualcubedart/visualcubedart.dart';

void main() {
  group('ArrowUtils - PLL Arrow Generation & AUF Alignment', () {
    // ─── G-Permutations (Ga, Gb, Gc, Gd) ───────────────────────────────────
    group('G-Permutations', () {
      // Scrambles from catalog.db for Ga, Gb, Gc, Gd
      const gaScramble = "F' D' L2 D' L2 D2 F2 R2 U2 R2 U2 B' U F' U' B U'";
      const gbScramble = "B' U2 B2 U B2 L2 U' L2 U B2 L2 U' B' U' F U' F' U";
      const gcScramble = "L' U B2 D' B2 U L2 U' B2 R2 D' R2 D2 B2 U2 L' F2 U2 F2";
      const gdScramble = "U R2 B2 L2 R2 D L2 B2 U L2 U' R2 U F2 L' R' U2 L' R'";

      test('Ga generates exactly 6 arrows (3 corners + 3 edges) with AUF U\'', () {
        final result = ArrowUtils.generateArrows(gaScramble);
        expect(result, isNotNull);
        expect(result!.auf, equals("U'"));

        final arrows = result.arrows.split(',');
        expect(arrows.length, equals(6));

        // Corners: U0, U8, U6 (U2 is solved)
        // Edges: U1, U3, U7 (U5 is solved)
        // Solved 2x1 block is U2 (UBR) and U5 (UR) -> neither U2 nor U5 should appear in arrows
        for (final a in arrows) {
          expect(a.contains('U2'), isFalse, reason: 'U2 is part of stationary 2x1 block');
          expect(a.contains('U5'), isFalse, reason: 'U5 is part of stationary 2x1 block');
        }
      });

      test('Gb generates exactly 6 arrows (3 corners + 3 edges) with AUF U', () {
        final result = ArrowUtils.generateArrows(gbScramble);
        expect(result, isNotNull);
        expect(result!.auf, equals("U"));

        final arrows = result.arrows.split(',');
        expect(arrows.length, equals(6));

        // Solved 2x1 block is U8 (UFR) and U7 (UF) -> neither U8 nor U7 should appear in arrows
        for (final a in arrows) {
          expect(a.contains('U8'), isFalse, reason: 'U8 is part of stationary 2x1 block');
          expect(a.contains('U7'), isFalse, reason: 'U7 is part of stationary 2x1 block');
        }
      });

      test('Gc generates exactly 6 arrows (3 corners + 3 edges) with AUF U', () {
        final result = ArrowUtils.generateArrows(gcScramble);
        expect(result, isNotNull);
        expect(result!.auf, equals("U"));

        final arrows = result.arrows.split(',');
        expect(arrows.length, equals(6));

        // Solved 2x1 block is U8 (UFR) and U5 (UR) -> neither U8 nor U5 should appear in arrows
        for (final a in arrows) {
          expect(a.contains('U8'), isFalse, reason: 'U8 is part of stationary 2x1 block');
          expect(a.contains('U5'), isFalse, reason: 'U5 is part of stationary 2x1 block');
        }
      });

      test('Gd generates exactly 6 arrows (3 corners + 3 edges) with AUF U\'', () {
        final result = ArrowUtils.generateArrows(gdScramble);
        expect(result, isNotNull);
        expect(result!.auf, equals("U'"));

        final arrows = result.arrows.split(',');
        expect(arrows.length, equals(6));

        // Solved 2x1 block is U2 (UBR) and U1 (UB) -> neither U2 nor U1 should appear in arrows
        for (final a in arrows) {
          expect(a.contains('U2'), isFalse, reason: 'U2 is part of stationary 2x1 block');
          expect(a.contains('U1'), isFalse, reason: 'U1 is part of stationary 2x1 block');
        }
      });
    });

    // ─── Outras Permutações PLL Clássicas ──────────────────────────────────
    group('Outros Casos de PLL', () {
      test('H-Perm gera 4 setas de troca de meios e 0 cantos', () {
        const hScramble = "M2 U M2 U2 M2 U M2";
        final result = ArrowUtils.generateArrows(hScramble);
        expect(result, isNotNull);
        final arrows = result!.arrows.split(',');
        expect(arrows.length, equals(4));
        for (final a in arrows) {
          final from = int.parse(a[1]);
          final to = int.parse(a[3]);
          expect([1, 3, 5, 7].contains(from), isTrue);
          expect([1, 3, 5, 7].contains(to), isTrue);
        }
      });

      test('Ua-Perm gera 3 setas de ciclo de meios e 0 cantos', () {
        // Ua solution: R U' R U R U R U' R' U' R2
        const uaAlg = "R U' R U R U R U' R' U' R2";
        final result = ArrowUtils.generateArrows(uaAlg);
        expect(result, isNotNull);
        final arrows = result!.arrows.split(',');
        expect(arrows.length, equals(3));
      });

      test('Ub-Perm gera 3 setas de ciclo de meios e 0 cantos', () {
        // Ub solution: R2 U R U R' U' R' U' R' U R'
        const ubAlg = "R2 U R U R' U' R' U' R' U R'";
        final result = ArrowUtils.generateArrows(ubAlg);
        expect(result, isNotNull);
        final arrows = result!.arrows.split(',');
        expect(arrows.length, equals(3));
      });

      test('Aa-Perm gera 3 setas de cantos e 0 meios', () {
        // Aa solution: x R' U R' D2 R U' R' D2 R2 x'
        const aaAlg = "x R' U R' D2 R U' R' D2 R2 x'";
        final result = ArrowUtils.generateArrows(aaAlg);
        expect(result, isNotNull);
        final arrows = result!.arrows.split(',');
        expect(arrows.length, equals(3));
      });

      test('T-Perm gera 4 setas (2 cantos trocados + 2 meios trocados)', () {
        // T-Perm solution: R U R' U' R' F R2 U' R' U' R U R' F'
        const tAlg = "R U R' U' R' F R2 U' R' U' R U R' F'";
        final result = ArrowUtils.generateArrows(tAlg);
        expect(result, isNotNull);
        final arrows = result!.arrows.split(',');
        expect(arrows.length, equals(4));
      });

      test('T-Perm com cornersOnly: true gera apenas 2 setas de troca de cantos', () {
        const tScramble = "R2 U B2 R2 U B2 R2 D L2 B2 F2 D' B2 U L' R' U2 L' R'";
        final result = ArrowUtils.generateArrows(tScramble, cornersOnly: true);
        expect(result, isNotNull);
        final arrows = result!.arrows.split(',');
        expect(arrows.length, equals(2));
        expect(arrows, contains('U2U8'));
        expect(arrows, contains('U8U2'));
      });

      test('Y-Perm com cornersOnly: true gera apenas 2 setas de troca diagonal de cantos', () {
        const yScramble = "L2 U2 L2 F2 D' F2 U2 F2 L2 D F2 L2 U2 R2 U' L' R' U2 L' R'";
        final result = ArrowUtils.generateArrows(yScramble, cornersOnly: true);
        expect(result, isNotNull);
        final arrows = result!.arrows.split(',');
        expect(arrows.length, equals(2));
        expect(arrows, contains('U0U8'));
        expect(arrows, contains('U8U0'));
      });

      test('filterCornerArrows filtra setas de meios corretamente', () {
        const mixedArrows = 'U2U8,U8U2,U3U5,U5U3';
        final filtered = ArrowUtils.filterCornerArrows(mixedArrows);
        expect(filtered, equals('U2U8,U8U2'));
      });
    });

    // ─── Casos Limite / Estados Resolvidos ─────────────────────────────────
    group('Casos Limite', () {
      test('Algoritmo vazio retorna null', () {
        final result = ArrowUtils.generateArrows('');
        expect(result, isNull);
      });

      test('Apenas rotações de cubo sem permutação retornam null', () {
        final result = ArrowUtils.generateArrows("y x2 z'");
        expect(result, isNull);
      });
    });
  });
}
