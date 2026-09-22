import 'cube_simulator.dart';

class ArrowResult {
  final String arrows;
  final String auf;

  const ArrowResult(this.arrows, this.auf);
}

class ArrowUtils {
  /// Defines the sticker indices for the 8 pieces on the U face.
  static const Map<int, List<int>> _uPieceStickers = {
    // Corners
    0: [0, 9, 38], // UBL
    2: [2, 36, 29], // UBR
    8: [8, 27, 20], // UFR
    6: [6, 18, 11], // UFL
    // Edges
    1: [1, 37], // UB
    3: [3, 10], // UL
    5: [5, 28], // UR
    7: [7, 19], // UF
  };

  /// Map of each corner index to its adjacent edge indices on the U face.
  static const Map<int, List<int>> _cornerAdjEdges = {
    0: [1, 3], // UBL -> UB, UL
    2: [1, 5], // UBR -> UB, UR
    8: [5, 7], // UFR -> UR, UF
    6: [7, 3], // UFL -> UF, UL
  };

  /// Filters an arrow string to only retain arrows connecting corner pieces (0, 2, 6, 8).
  static String filterCornerArrows(String arrows) {
    if (arrows.isEmpty) return arrows;
    const cornerIndices = {'0', '2', '6', '8'};
    return arrows
        .split(',')
        .map((a) => a.trim())
        .where((a) {
          if (a.length < 4 || !a.startsWith('U')) return false;
          final fromChar = a[1];
          final toIndex = a.indexOf('U', 2);
          if (toIndex == -1 || toIndex + 1 >= a.length) return false;
          final toChar = a[toIndex + 1];
          return cornerIndices.contains(fromChar) && cornerIndices.contains(toChar);
        })
        .join(',');
  }

  /// Generates an arrow string for visualcubedart / VisualCube.
  /// Uses a reference cube to handle cube rotations correctly.
  /// Tests all 4 possible AUF alignments to find the core permutation (minimal arrows).
  /// If [cornersOnly] is true, returns only the arrows between corners (e.g., for 2-Look PLL Corner Permutation).
  static ArrowResult? generateArrows(
    String algorithm, {
    bool cornersOnly = false,
  }) {
    try {
      final parts = algorithm
          .replaceAll('(', '')
          .replaceAll(')', '')
          .split(RegExp(r'\s+'))
          .where((s) => s.isNotEmpty)
          .toList();

      final baseCube = CubeSimulator();
      final targetCube = CubeSimulator();

      for (var token in parts) {
        baseCube.applyAlgorithm(token);

        // Only apply whole-cube rotations to the target reference
        String base = token.replaceAll("2", "").replaceAll("'", "");
        if (base == 'x' || base == 'y' || base == 'z') {
          targetCube.applyAlgorithm(token);
        }
      }

      List<String> aufs = ["", "U", "U'", "U2"];
      List<String> bestArrows = [];
      String bestAuf = "";
      int minDisplaced = 999;
      int bestBalance = 999;
      int maxConnectedBlocks = -1;

      for (String auf in aufs) {
        final currentCube = CubeSimulator();
        currentCube.stickers = List.from(baseCube.stickers);

        // Apply AUF adjustment to align the U layer
        if (auf.isNotEmpty) {
          currentCube.applyAlgorithm(auf);
        }

        List<String> arrowParts = [];
        int dispCorners = 0;
        int dispEdges = 0;
        List<int> solvedCorners = [];
        List<int> solvedEdges = [];

        for (final pEntry in _uPieceStickers.entries) {
          int posP = pEntry.key;
          List<int> stickersP = pEntry.value;

          Set<int> currentColors =
              stickersP.map((idx) => currentCube.stickerAt(idx)).toSet();
          int originalPos = -1;

          for (final oEntry in _uPieceStickers.entries) {
            int posO = oEntry.key;
            List<int> stickersO = oEntry.value;
            Set<int> targetColors =
                stickersO.map((idx) => targetCube.stickerAt(idx)).toSet();

            if (currentColors.length == targetColors.length &&
                currentColors.containsAll(targetColors)) {
              originalPos = posO;
              break;
            }
          }

          final isCorner = (posP == 0 || posP == 2 || posP == 6 || posP == 8);
          if (originalPos != -1 && originalPos != posP) {
            arrowParts.add("U${posP}U$originalPos");
            if (isCorner) {
              dispCorners++;
            } else {
              dispEdges++;
            }
          } else {
            if (isCorner) {
              solvedCorners.add(posP);
            } else {
              solvedEdges.add(posP);
            }
          }
        }

        int displacedCount = dispCorners + dispEdges;
        int balance = (dispCorners - dispEdges).abs();

        // Count how many solved corners form an adjacent 2x1 block with a solved edge
        int connectedBlocks = 0;
        for (final sc in solvedCorners) {
          final adjEdges = _cornerAdjEdges[sc] ?? const [];
          for (final se in solvedEdges) {
            if (adjEdges.contains(se)) connectedBlocks++;
          }
        }

        bool isBetter = false;
        if (displacedCount < minDisplaced) {
          isBetter = true;
        } else if (displacedCount == minDisplaced) {
          if (balance < bestBalance) {
            isBetter = true;
          } else if (balance == bestBalance &&
              connectedBlocks > maxConnectedBlocks) {
            isBetter = true;
          }
        }

        if (isBetter) {
          minDisplaced = displacedCount;
          bestBalance = balance;
          maxConnectedBlocks = connectedBlocks;
          bestArrows = arrowParts;
          bestAuf = auf;
        }
      }

      if (bestArrows.isEmpty) return null;

      String result = bestArrows.join(',');
      if (cornersOnly) {
        result = filterCornerArrows(result);
        if (result.isEmpty) return null;
      }
      return ArrowResult(result, bestAuf);
    } catch (_) {
      return null;
    }
  }
}
